/// @file mcl68k_testsuite.c
/// @brief Runs Ted Fried's own hand-written, self-checking MC68000 opcode
///        test suite (MicroCoreLabs/Projects, MCL68/MC68000_Test_Code)
///        through the real PJIT core. Unlike golden/oracle.c's random
///        cases, this needs no separate oracle at all: the suite checks
///        its own results and either falls through to `ALL_DONE:` (every
///        one of its ~60 opcode-family subroutines passed) or hangs in a
///        subroutine-specific `bra *` self-loop (a failure).
///
/// mcl68k_test_image.h (generated -- see golden/README.md for how) is a
/// byte-exact reconstruction of the assembled test binary, taken from
/// EASy68K's own listing output and cross-checked byte-for-byte against
/// the project's own Test_Suite.bin (the only real differences were in
/// unreachable padding between ORG'd regions -- this project fills gaps
/// with NOP, the original binary with 0xFF, neither ever executed).
///
/// The suite hardcodes absolute 68K addresses throughout -- not just the
/// vector table and JSR/JMP between subroutines, but plain scratch data
/// pointers too (e.g. `move.l #$100,a0` then `move.l d0,12(a0,d7)`: 0x100
/// is not a label, just "any valid low RAM address will do"). PJIT's own
/// memory model is "68K address == raw ARM pointer" throughout (there is
/// no base-offset translation anywhere), so relocating the suite would
/// mean patching every labeled JSR/JMP/vector reference and correctly
/// classifying *every* absolute-looking immediate load as "real address"
/// or "arbitrary test data" (many of those really are pure sign-extension/
/// overflow test patterns, e.g. `movea.l #$80000000,a0`, never meant to
/// be dereferenced), which isn't reliably decidable from the bytes alone.
///
/// So the image is not relocated. QEMU's "virt" machine has no usable RAM at
/// low addresses (real RAM starts at 0x40000000; see platform/qemu_virt/
/// virt.ld), but this program turns the ARM MMU on (the TTBR0, domain, TLB
/// and enable helpers of platform/cp15.S) and builds a
/// full 4GB identity map with a single 1MB section redirected: virtual
/// 0x00000000-0x000FFFFF -> test_mem's own physical 1MB-aligned buffer.
/// Every other address, including this program's own code/stack/cache
/// pool, keeps mapping to itself. The suite's image then loads completely
/// unmodified at its native addresses -- no relocation, no patch list, no
/// risk of misclassifying an instruction.
///
/// Pass/fail detection is a single-word TRAP #n patched directly over each of
/// ALL_DONE and the suite's 9 `*_FAIL` self-loops (each a 1-word
/// `bra $` in the suite, so the patch never overruns into whatever follows). The harness sets
/// cpu->trap_exit_mask so every TRAP #n ends pjit_run(), which returns the
/// vector: the vector number, not a register value, says which site fired.
///
/// Two modes (make golden (mk/qemu.mk)): -DGOLDEN_INTERP runs the whole suite through
/// the interpretive runtime only; -DGOLDEN_DR makes every address cacheable
/// (dr_threshold = 0) so it runs out of the direct-mapped DR cache.
#include <stdint.h>
#include "pjit.h"
#include "cp15.h"

#include "mcl68k_test_image.h"

// -DGOLDEN_DR: the same pjit_run() entry as GOLDEN_INTERP, but with every address
// cacheable (dr_threshold = 0), so the suite runs out of the direct-mapped DR cache.
#if !defined(GOLDEN_DR) && !defined(GOLDEN_INTERP)
#define GOLDEN_INTERP 1
#endif

extern void qemu_exit(void);
extern void emit_opcode_table(void);

// pjit_cache_init()'s own carve-out (cache_data + maprom + two PJIT
// tables) is several MB by itself -- needs a real pool, same as
// pjit_side.c's ram_pool, entirely separate from test_mem below (which
// only holds the 68K test image itself).
static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
// 1MB-aligned: its own physical address doubles as the redirect target
// for the MMU's virtual section 0, so a 68K address X inside the image
// lands at ARM address test_mem+X with zero arithmetic needed.
static uint8_t  test_mem[MCL68K_TEST_IMAGE_SIZE] __attribute__((aligned(1024 * 1024)));
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));

// ARMv7 short-descriptor section format, 1MB granularity: base | type=
// section(0b10) | B | C | AP[1:0]=full(0b11) -- cacheable, bufferable,
// full read/write access, domain 0 (CP15DomainAccessClientSet's fixed
// 0x55555555 already marks every domain "client").
#define SECTION_DESC(pa) (((pa) & 0xFFF00000u) | 0xC0Eu)

static void mmu_identity_map_with_redirect(void)
{
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = SECTION_DESC(i << 20);
    mmu_pagetable[0] = SECTION_DESC((uint32_t)test_mem);

    CP15TtbCtlTtb0Config();      // TTBCR.N=0: TTBR0 alone covers all 4GB
    CP15Ttb0Set((uint32_t)mmu_pagetable);
    CP15DomainAccessClientSet();
    CP15TlbInvalidate();
    CP15MMUEnable();
}

// Names for the 9 *_FAIL self-check labels, indexed the same way
// fail_addrs[] below is, and the same way each site's patched TRAP
// vector number (TRAP #0 = index 0, ...) identifies it to
// pjit_test_exit(). TRAP #9 is reserved for ALL_DONE, kept out of this
// array (its own "which" value is FAIL_COUNT, handled separately).
static const char *const fail_names[] = {
    "ORI_TO_CCR_FAIL", "ORI_TO_SR_FAIL", "EORI_TO_CCR_FAIL", "EORI_TO_SR_FAIL",
    "ANDI_TO_CCR_FAIL", "ANDI_TO_SR_FAIL", "BTST_FAIL", "BCHG_FAIL", "BSET_FAIL",
};
#define FAIL_COUNT (sizeof(fail_names) / sizeof(fail_names[0]))

static const uint32_t fail_addrs[FAIL_COUNT] = {
    0x05AE, 0x05E2, 0x061A, 0x0652, 0x068A, 0x06C2, 0x0B74, 0x1006, 0x1854,
};
#define ALL_DONE_ADDR 0xF000u

// Expected-value patches for checks where the suite's own reference and
// PJIT deliberately disagree -- see golden/README.md's "BCD with invalid
// digits". The suite's cmpi.l constants for its exhaustive ABCD/SBCD/NBCD
// sweeps over *every* byte (0x00-0x99, so including invalid BCD digits
// A-F) were generated from Ted Fried's own digit-wise model (each nibble
// > 9 wraps by 10), which is not what the real 68000's ABCD/SBCD/NBCD do
// on invalid digits. PJIT follows the hardware-derived bc/dc/corf
// algorithm (pjit/handle_bcd.s), so those constants get replaced with
// ones from an independent model of that same algorithm (a few lines of
// Python, reproduced in golden/README.md), cross-checked against PJIT's
// own D3/D5 results.
static const struct { uint32_t addr; uint32_t was; uint32_t now; } expect_patches[] = {
    // ABCD Dx,Dy / -(Ax),-(Ay) sweep, X cleared each pass: D4, D5, D3
    { 0x2D7A, 0x00005AFC, 0x00005C0A }, { 0x2D82, 0x001C9A34, 0x001C45D4 }, { 0x2D8A, 0x00000034, 0x000000D4 },
    // same sweep, X set on entry each pass
    { 0x2DF4, 0x00005B60, 0x00005CA4 }, { 0x2DFC, 0x001CCFC8, 0x001C59A8 }, { 0x2E04, 0x00000034, 0x000000D4 },
    // SBCD sweeps (X cleared / X set): only the borrow count (D4) differs; D5 and D3 agree
    { 0x2E90, 0x00005C0A, 0x00005D18 }, { 0x2F0A, 0x00005CA4, 0x00005DE8 },
};

// The suite's MOVE to SR / MOVE from SR round trips use SR values with the reserved bits 7-5 set ($275A, $257A, $2766, $2733, $2777):
// Ted's C model keeps whatever bits are written, so it hands them back. A 68000 has no such bits (they read 0; Musashi and MAME
// mask the SR with $A71F), and PJIT now masks too (tools/gen: SR bits 14, 12, 11 and 7-5 always read 0). So the same round trips are
// run with the reserved bits clear -- both the value written and the value expected; the real bits (T, S, the mask, X N Z V C) still
// have to survive. 16-bit words: address of the immediate, the image's value, the patched one.
static const struct { uint32_t addr; uint16_t was; uint16_t now; } expect_patches16[] = {
    { 0x2396, 0x275A, 0x271A }, { 0x239C, 0x275A, 0x271A },     // move.w #,sr / move.w sr,d0 / cmpi.w #,d0
    { 0x23A8, 0x275A, 0x271A }, { 0x23AE, 0x275A, 0x271A },     // ... (a0)
    { 0x23BA, 0x257A, 0x251A }, { 0x23C6, 0x257A, 0x251A },     // ... (a0)+
    { 0x23D2, 0x2766, 0x2706 }, { 0x23DE, 0x2766, 0x2706 },     // ... -(a0)
    { 0x23EA, 0x2733, 0x2713 }, { 0x23F2, 0x2733, 0x2713 },     // ... d16(a0)
    { 0x2402, 0x275A, 0x271A }, { 0x240A, 0x275A, 0x271A },     // ... d8(a0,d0.l)
    { 0x2412, 0x2777, 0x2717 }, { 0x241A, 0x2777, 0x2717 },     // ... abs.w
    { 0x2422, 0x2777, 0x2717 }, { 0x242C, 0x2777, 0x2717 },     // ... abs.l
};

// Reports which patched site fired: `which` is the TRAP vector's own index (0-8 =
// one of the 9 *_FAIL sites above, 9 = ALL_DONE).
static void report_exit(uint32_t which)
{
    if (which == FAIL_COUNT) {
        printf("[ ok ] reached ALL_DONE -- every opcode-family subroutine in the suite passed\n");
#ifdef GOLDEN_PATCH
        for (int i = 0; i < 8; i++) printf("  D%d=%08X", i, (unsigned)pjit_get_dn(i));
        printf("  SR=%04X\n", (unsigned)(pjit_get_sr() & 0xFFFF));
#endif
        printf("PASS\n");
    } else if (which < FAIL_COUNT) {
        printf("[FAIL] %s reached\n", fail_names[which]);
        for (int i = 0; i < 8; i++) printf("  D%d=%08X", i, (unsigned)pjit_get_dn(i));
        printf("  SR=%04X A0=%08X\n", (unsigned)(pjit_get_sr() & 0xFFFF), (unsigned)pjit_get_an(0));
    } else {
        printf("[FAIL] unexpected TRAP #%u reached\n", which);
    }
    qemu_exit();
}

int main(void)
{
    printf("=== MC68000 opcode test suite (MicroCoreLabs/Projects) on PJIT ===\n");
#if defined(GOLDEN_DR)
    printf("[MODE] DR cache: the whole suite runs through pjit_run() out of the direct-mapped cache (pjit/runtime_dr.s)\n");
#else
    printf("[MODE] interpretive: the whole suite runs through pjit_run() (pjit/runtime.s), no cache\n");
#endif

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifdef GOLDEN_DR
    cpu->dr_threshold = 0;                      // everything is cacheable
#else
    cpu->dr_threshold = 0xFFFFFFFFu;            // nothing is: the interpreter only
#endif
    emit_opcode_table();

    memcpy(test_mem, mcl68k_test_image, MCL68K_TEST_IMAGE_SIZE);

    // Patch every detection site's `bra $` (always exactly 1 word) into
    // a single-word TRAP #n *before* anything runs -- these are ordinary
    // 68K source bytes, picked up whenever the interpreter or the cache
    // first reaches them.
    for (unsigned i = 0; i < FAIL_COUNT; i++) {
        uint32_t a = fail_addrs[i];
        test_mem[a] = 0x4E; test_mem[a + 1] = (uint8_t)(0x40 | i);   // TRAP #i
    }
    for (unsigned i = 0; i < sizeof(expect_patches) / sizeof(expect_patches[0]); i++) {
        uint32_t a = expect_patches[i].addr;
        uint32_t cur = ((uint32_t)test_mem[a] << 24) | ((uint32_t)test_mem[a + 1] << 16) |
                       ((uint32_t)test_mem[a + 2] << 8) | test_mem[a + 3];
        if (cur != expect_patches[i].was) {
            printf("[FAIL] expect_patches[%u] @0x%04X: image has %08X, table thinks %08X\n",
                   i, (unsigned)a, (unsigned)cur, (unsigned)expect_patches[i].was);
            qemu_exit();
            return 1;
        }
        uint32_t v = expect_patches[i].now;
        test_mem[a] = v >> 24; test_mem[a + 1] = v >> 16; test_mem[a + 2] = v >> 8; test_mem[a + 3] = v;
    }
    for (unsigned i = 0; i < sizeof(expect_patches16) / sizeof(expect_patches16[0]); i++) {
        uint32_t a = expect_patches16[i].addr;
        uint16_t cur = (uint16_t)(((uint32_t)test_mem[a] << 8) | test_mem[a + 1]);
        if (cur != expect_patches16[i].was) {
            printf("[FAIL] expect_patches16[%u] @0x%04X: image has %04X, table thinks %04X\n",
                   i, (unsigned)a, (unsigned)cur, (unsigned)expect_patches16[i].was);
            qemu_exit();
            return 1;
        }
        test_mem[a] = expect_patches16[i].now >> 8; test_mem[a + 1] = (uint8_t)expect_patches16[i].now;
    }
#ifdef GOLDEN_WORD_ADDR
    // Probe aid: overwrite one arbitrary word (e.g. with `move sr,d7`).
    test_mem[GOLDEN_WORD_ADDR] = GOLDEN_WORD_VAL >> 8; test_mem[GOLDEN_WORD_ADDR + 1] = GOLDEN_WORD_VAL & 0xFF;
#endif
#ifdef GOLDEN_PATCH
    test_mem[GOLDEN_PATCH] = 0x4E; test_mem[GOLDEN_PATCH + 1] = (uint8_t)(0x40 | FAIL_COUNT);
#endif
    test_mem[ALL_DONE_ADDR]     = 0x4E;
    test_mem[ALL_DONE_ADDR + 1] = (uint8_t)(0x40 | FAIL_COUNT);      // TRAP #9

    printf("[MMU] identity-mapping 4GB, redirecting virtual 0x00000000 section to test_mem=%p\n",
           (void*)test_mem);
    mmu_identity_map_with_redirect();

    // The 68000 has no VBR of its own -- vectors are always at address 0
    // -- and now address 0 is real (redirected to test_mem), so this
    // needs no relocation either.
    cpu->vbr = 0;
    pjit_set_sr(0x2700);
    {
        uint32_t ssp = ((uint32_t)test_mem[0] << 24) | ((uint32_t)test_mem[1] << 16)
                     | ((uint32_t)test_mem[2] << 8)  |  (uint32_t)test_mem[3];
        cpu->ssp = cpu->usp = __builtin_bswap32(ssp);   // the stack shadows are big-endian
        for (int r = 0; r < 8; r++) { pjit_set_an(r, 0); pjit_set_dn(r, 0); }
        pjit_set_an(7, ssp);
    }

    // Every TRAP #n ends pjit_run(); the vector number says which patched site
    // fired. The suite's own first instruction sets A7, so there is nothing
    // else to seed -- unless a debug entry point is asked for.
    cpu->trap_exit_mask = 0xFFFFu;
    uint32_t entry = 0x400u;
#ifdef GOLDEN_ENTRY
    // Debug aid: start mid-suite at GOLDEN_ENTRY, with a fake return
    // address of ALL_DONE on a private A7.
    test_mem[0x3F0] = 0; test_mem[0x3F1] = 0; test_mem[0x3F2] = 0xF0; test_mem[0x3F3] = 0;
    pjit_set_an(7, 0x3F0);
    entry = GOLDEN_ENTRY;
#endif
#ifdef GOLDEN_DR
    printf("[RUN] entry=0x%X (DR cache; ~17KB of real 68K code)\n", (unsigned)entry);
#else
    printf("[RUN] entry=0x%X (interpretive; ~17KB of real 68K code)\n", (unsigned)entry);
#endif
    {
        uint32_t vec = pjit_run((uint16_t *)entry);
        report_exit(vec - TRAP0);        // prints PASS/FAIL and exits
    }
    return 1;
}
