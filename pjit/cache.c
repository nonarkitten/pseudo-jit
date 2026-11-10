/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

/// @file cache.c
/// @brief Boot-time setup: the carve-out of the DR cache and tags from the top of RAM, and cpu_t's pointers to them
///        and to the static tables. Nothing here runs while guest code runs.
///
/// The tables (pjit_arena: cpu_t, DT, OT, ext, MOVEM, inline, links; pjit_drsr) are static data in build/tables.s, written at
/// build time by the host-side generator (tools/gen/maketables.py), so there is nothing to build at run time.
/// @see pjit.h, and pjit/runtime_dr.s for the cache itself

#include "pjit.h"


// cpu_t immediately followed by the dispatch table, so r5 (= &dt[0]) reaches
// every cpu_t field at a negative offset. See pjit.h.
extern uint32_t pjit_drsr[];                            // the static DRSR (build/tables.s)
/// There is nothing to emit at run time (the tables are static); kept so the tests that call it still link.
#if defined(PJIT_BLK) || defined(PJIT_TIER)
extern void blk_boot(void), blk_flush(void);
void emit_opcode_table(void) { blk_boot(); }
#else
void emit_opcode_table(void) {}
#endif


/// @brief Invalidates the whole DR cache: a tag of 0xFFFF matches no address,
///        so every line is cleared and refilled on its next use.
#ifdef PJIT_TIER
extern uint32_t pjit_tier_cnt[];
#endif
// Self-modifying code (a 68000 has no instruction cache, so guest software never flushes): a platform that can detect writes to translated code (Buffee: write-protected pages) provides
// pjit_platform_code (called with every range of guest code the translators read), pjit_platform_flush (everything it tracks is void) and pjit_platform_codebits; the defaults do nothing.
__attribute__((weak)) void pjit_platform_code(uint32_t start, uint32_t len) { (void)start; (void)len; }
__attribute__((weak)) void pjit_platform_line(uint32_t start, uint32_t len) { (void)start; (void)len; }
__attribute__((weak)) void pjit_platform_flush(void) { }
__attribute__((weak)) uint32_t *pjit_platform_codeexact(void) { return 0; }
__attribute__((weak)) uint32_t *pjit_platform_codebits(void) { return 0; }
__attribute__((weak)) uint16_t *pjit_platform_xlat(void) { return 0; }
#if defined(PJIT_BLK) || defined(PJIT_TIER)
#include "blk.h"
extern void pjit_bk_unlink(void), pjit_blk_fast_clear(void);
/// @brief Asks for a full flush at the next router entry (callable from the abort handler while guest code is in the middle of a slot or a block): the router flushes when it next runs, so
///        the guest is never under a flush; every transfer that does not go through the router is forced into it (the latch of pjit_irq_entry: tags_goto at the all-miss array, blk_fast
///        emptied, the backward stubs unlinked).
void pjit_post_flush(void)
{
    cpu_state.flush_req = 1;
    cpu_state.irq_flag = 1;
#ifdef PJIT_TIER
    cpu_state.tags_goto = cpu_state.tags_miss;
#endif
    pjit_bk_unlink();
    pjit_blk_fast_clear();
}
#endif
void pjit_cache_flush(void)
{
    cpu_state.flush_req = 0;
    pjit_platform_flush();
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    blk_flush();
#endif
#ifdef PJIT_TIER
    for (uint32_t i = 0; i < 2 << 10; i++) pjit_tier_cnt[i] = 0;                 // the hotness counters start over with the caches
#endif
    for (uint32_t i = 0; i < DR_CL_COUNT; i++) cpu_state.tags[i] = 0xFFFF;
    for (uint32_t i = 0; i < 256; i++) { cpu_state.shadow[2 * i] = __builtin_bswap32(1); cpu_state.shadow[2 * i + 1] = 0; }   // the shadow return stack: empty, every entry's guest address odd (BE 1)
    pjit_arena.fuse_next = 0;                   // the native string loops hang off slots that are about to be rewritten: reuse the area
    pjit_arena.links.count = 0;                 // every line is invalid, so no line holds a live link
    for (uint32_t i = 0; i < DR_CL_COUNT; i++) pjit_arena.links.flag[i] = 0;
}

/// @brief Interrupt windows on or off. Off, guest code is never interrupted and the windows cost nothing;
///        on, the interpreter's prefetch handler, the router and in-line loops open a window for IRQs.
static uint16_t pjit_miss_tags[DR_CL_COUNT];
void pjit_irq_enable(int on)
{
    extern void pjit_prefetch(void);
    extern void pjit_prefetch_irq(void);
    for (uint32_t i = 0; i < DR_CL_COUNT; i++) pjit_miss_tags[i] = 0xFFFF;       // no 13-bit tag is this: every lookup misses, and the slow path opens the window
    cpu_state.tags_miss = __builtin_bswap32((uint32_t)pjit_miss_tags);
    cpu_state.irq_unmask = on ? 0x80 : 0;
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    // the latch model (runtime.s pjit_irq_entry): IRQs stay enabled in guest code, the handler only records the level and, if it beats the SR mask, sets irq_flag (and, in a tiered
    // build, points tags_goto at the all-miss array); the router, the cold back-edge core and the interpreter's prefetch poll look at the flag. No windows.
    extern void pjit_prefetch_poll(void);
    cpu_state.irq_windows = 0;
    cpu_state.tags_goto = __builtin_bswap32((uint32_t)cpu_state.tags);
    cpu_state.prefetch_hook = (uint32_t)(on ? pjit_prefetch_poll : pjit_prefetch);
#else
    cpu_state.irq_windows = on ? 1 : 0;
    cpu_state.tags_goto = __builtin_bswap32(on ? (uint32_t)pjit_miss_tags : (uint32_t)cpu_state.tags);
    cpu_state.prefetch_hook = (uint32_t)(on ? pjit_prefetch_irq : pjit_prefetch);
#endif
    pjit_cache_flush();
}

/// @brief Carves the DR cache, DRSR and cache tags out of the top of RAM and
///        initialises cpu_state's pointers to them (and to the arena's tables).
void pjit_cache_init(uint32_t top) {
    // TOP OF RAM
    uint8_t* memory = (uint8_t*)top;

    // 2MB      The DR cache            cpu->cache_data
    memory -= DR_CACHE_BYTES;
    cpu_state.cache_data = (uint32_t*)memory;
    printf("[INIT] Allocated %d bytes for the DR cache at %p\n", (int)DR_CACHE_BYTES, cpu_state.cache_data);

    // 512KB    The dispatch table -- two ARM words per 68K opcode. Lives in
    //          pjit_arena, directly after cpu_t, not carved from RAM: r5
    //          points at it (see pjit.h).
    cpu_state.opcode_table = pjit_arena.dt;
    printf("[INIT] PJIT table at %p (arena, %d bytes)\n", cpu_state.opcode_table, (int)sizeof(pjit_arena.dt));

    cpu_state.codebits = (uint32_t)pjit_platform_codebits();
    cpu_state.codeexact = (uint32_t)pjit_platform_codeexact();
    cpu_state.xlat = (uint32_t)pjit_platform_xlat();
    cpu_state.drsr = pjit_drsr;                         // static: dt, ot, ext, movem and inl are too

    // 128KB    The offset table -- lives in pjit_arena, like the DT
    cpu_state.ot = pjit_arena.ot;
    printf("[INIT] PJIT offset table at %p (arena, %d bytes)\n", cpu_state.ot, (int)sizeof(pjit_arena.ot));

    // DR_CL_COUNT * 2   The DR cache's tags (see pjit.h)
    uint32_t tags_size = DR_CL_COUNT * 2u;
    memory -= tags_size;
    cpu_state.tags = (uint16_t*)memory;
    printf("[INIT] Allocated %d bytes for DR cache tags at %p\n", tags_size, cpu_state.tags);
    cpu_state.cache_be = __builtin_bswap32((uint32_t)cpu_state.cache_data);
    cpu_state.tags_be  = __builtin_bswap32((uint32_t)cpu_state.tags);
    cpu_state.tags_goto = cpu_state.tags_be;
    pjit_cache_flush();
    cpu_state.dr_threshold = cpu_state.xlat ? 0 : DR_THRESHOLD_DEFAULT;      // with the platform's translate map (cpu->xlat) the map decides; without, the 24-bit threshold

    // Align downwards to a 1MB boundary, this is top of RAM for
    // AmigaOS and Ataro
    memory           = (uint8_t*)(((uint32_t)memory) & 0xFFF00000);

    pjit_dt = pjit_arena.dt;
    cpu_state.trap_exit_mask = 0x8000;      // TRAP #15 ends pjit_run() by default
    pjit_irq_enable(0);
}
