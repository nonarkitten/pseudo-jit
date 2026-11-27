/* amiga_host.c -- runs an Amiga Shell program (an AmigaDOS executable) on the golden 68000 core (golden/mcl68k.c) with just enough of exec.library and dos.library for
 * the programs in amiga/, and with PJIT's control page at $F7F000 served by platform/buffee/control.c against a simulated flash and EEPROM (mock_board.c). The test of
 * the `buffee` command (tests/amiga/test_buffee.py) runs it once per command line, or several command lines on one copy of the program.
 *   amiga_host [-f flash.bin] [-e eeprom.bin] [-r rom.bin] [-d dir] [-V version] <program> [arguments...] [-- arguments...]
 *   amiga_host [...] -R resident.bin [arguments...] [-- arguments...]
 * -f / -e: the simulated flash (16 MB) and EEPROM (32 KB), read at the start (if the file exists) and written at the end; -r: the "Amiga's own ROM" for COPYROM;
 * -d: the directory that file names in the program's arguments are relative to (a device name such as DF0: is dropped). The program's output goes to stdout; the
 * board's console (what PJIT prints on the UART) to stderr. -R instead of a program: the resident command's RomTag (amiga/buffee/resident.S) at its address, its init run
 * as Kickstart runs it once dos.library is up, then the command it added to the resident list run with the arguments. Each `--` starts another run of the same copy, as a
 * Shell runs a resident command again or a program whose memory is not reloaded; each run's return code goes to stderr as "[rc n]". -V: dos.library's version (40,
 * Kickstart 3.1; 34 is Kickstart 1.3, which has no ReadArgs); -X: exec's (40; 37 is Kickstart 2.04 and 2.05). ReadArgs takes /K, /S and /N keywords as dos 36's does; FreeMem fails the run for a block that is not
 * allocated (a block freed twice). Exit status: the last run's return code, or 100 + n for a failure of the harness.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>
#include <strings.h>
#include "control.h"

extern uint32_t m68k_data_reg[8], m68k_address_reg[8], m68k_a7_S, mc68k_pc, memory_base;
extern uint16_t mc68k_flags;
extern void (*WriteWord)(uint32_t, uint16_t); extern uint16_t (*ReadWord)(uint32_t);
extern void (*WriteByte)(uint32_t, uint8_t); extern uint8_t (*ReadByte)(uint32_t);
extern void (*mcl68k_step_hook)(void);
extern int16_t run_mcl68k(void *start);
uint16_t GetIPLx(void) { return 0; }
int UART0Ready(void) { return 0; } uint8_t UART0Read(void) { return 0; } void UART0Write(uint8_t c) { (void)c; }

void bfc_serve(uint32_t a);                                 /* platform/buffee/control.c */
void mock_board_load(const char *flash, const char *eeprom, const char *rom);
void mock_board_save(void);
extern jmp_buf mock_reboot;                                 /* mock_board.c: a BFC_REBOOT ends the run here */

#define MEM        0x1000000u
#define EXEC_BASE  0x001000u                               /* the libraries' bases; their jump tables below them hold JMPs into TRAP_AREA */
#define DOS_BASE   0x002000u
#define TRAP_AREA  0xE00000u                               /* + library * 0x1000 + offset: the hook serves the call */
#define EXIT_ADDR  0xE0FFF0u
#define INIT_DONE  0xE0FFE0u                               /* where the resident tag's init returns to */
#define HUNK_AT    0x010000u
#define STACK_TOP  0x00F000u
#define HEAP       0x100000u                               /* AllocMem from here up */
uint8_t mem[MEM];
static uint8_t page[0x1000];                               /* the control page as PJIT shows it */
static uint32_t heap = HEAP, ctl_hi;
static jmp_buf done;
static int ret_code, started;
static uint64_t icount;
static const char *root = ".";
#define RES_MODULES 0x003000u                              /* ExecBase->ResModules: the tag pointers exec found */
#define SEG_NODE    0x003100u                              /* the Segment AddSegment made: seg_UC at +4 */
static uint32_t resident_at, segment;
static int no_dos_at_init, in_init;                         /* -N: dos.library does not open while the tag's init runs */
static int uc_forced; static uint32_t uc_value;             /* -U n: AddSegment gives the entry use count n, whatever it was asked for */                     /* -R: the tag's address; the segment list AddSegment was given */
static char seg_name[32];
static const char *runs[16]; static int nruns, run_no;        /* the command lines, separated by -- */
static int32_t io_err;                                      /* IoErr */
static int dos_version = 40, exec_version = 40;
static struct { uint32_t at, size; } blocks[256];           /* the blocks AllocMem gave out and FreeMem has not taken back */
static FILE *fhs[16];
static uint32_t ring[64], nring, last_pc;                 /* the last 32 jumps, for a failure report */
static void dump_ring(void) { for (uint32_t k = 64; k > 0; k -= 2) fprintf(stderr, " %06X->%06X", ring[(nring - k) & 63], ring[(nring - k + 1) & 63]); fprintf(stderr, "\n"); }

static uint32_t rd32(uint32_t a) { a &= MEM - 1; return (uint32_t)mem[a] << 24 | mem[a + 1] << 16 | mem[a + 2] << 8 | mem[a + 3]; }
static void wr32(uint32_t a, uint32_t v) { a &= MEM - 1; mem[a] = v >> 24; mem[a + 1] = v >> 16; mem[a + 2] = v >> 8; mem[a + 3] = (uint8_t)v; }
static void wr16(uint32_t a, uint16_t v) { a &= MEM - 1; mem[a] = v >> 8; mem[a + 1] = (uint8_t)v; }

static int in_page(uint32_t a) { return (a & 0xFFF000u) == BFC_PAGE; }
static uint8_t rb(uint32_t a) { a &= 0xFFFFFF; return in_page(a) ? page[a & 0xFFF] : mem[a]; }
static uint16_t rw(uint32_t a) { return (uint16_t)(rb(a) << 8 | rb(a + 1)); }
static void wb(uint32_t a, uint8_t v) { a &= 0xFFFFFF; if (!in_page(a)) mem[a] = v; }
static void ww(uint32_t a, uint16_t v)
{
    a &= 0xFFFFFF;
    if (a == BFC_PAGE) { ctl_hi = v; return; }                          /* a long arrives as two words, the high one first */
    if (a == BFC_PAGE + 2) { bfc_serve(ctl_hi << 16 | v); return; }
    if (!in_page(a)) wr16(a, v);
}

static uint32_t sp(void) { return (mc68k_flags & 0x2000) ? m68k_a7_S : m68k_address_reg[7]; }
static void set_sp(uint32_t v) { if (mc68k_flags & 0x2000) m68k_a7_S = v; else m68k_address_reg[7] = v; }
static void cstr(uint32_t a, char *buf, size_t max) { size_t i = 0; while (i < max - 1 && mem[(a + i) & (MEM - 1)]) { buf[i] = (char)mem[(a + i) & (MEM - 1)]; i++; } buf[i] = 0; }

static FILE *host_open(const char *name)
{
    char path[1024];
    const char *colon = strchr(name, ':');
    snprintf(path, sizeof path, "%s/%s", root, colon ? colon + 1 : name);
    return fopen(path, "rb");
}

/* exec: library 0, dos: library 1. D0 is the result. */
/* ReadArgs(template, array, 0) for this run's argument line: each keyword of the template takes a value (/K, KEY=value or KEY value, maybe in quotes; /N, a number,
   stored as a LONG the array points at) or is a switch (/S, -1). The values are copied to memory from the heap. A lone '?' prints the template and reads an empty line,
   as if Return were pressed. Returns the RDArgs (any non-zero address) or 0 with the error in IoErr. */
static uint32_t read_args(uint32_t tmpl, uint32_t arr)
{
    char t[512], l[512], kw[64][32], opt[64][8];
    int nk = 0, i = 0;
    cstr(tmpl, t, sizeof t);
    for (char *p = t; *p && nk < 64; nk++) {
        int n = 0, o = 0;
        while (*p && *p != '/' && *p != ',') kw[nk][n++] = *p++;
        kw[nk][n] = 0;
        while (*p && *p != ',') { if (*p != '/') opt[nk][o++] = (char)toupper(*p); p++; }
        opt[nk][o] = 0;
        if (*p) p++;
    }
    snprintf(l, sizeof l, "%s", runs[run_no]);
    l[strcspn(l, "\n")] = 0;
    if (!strcmp(l, "?")) { printf("%s: ", t); fflush(stdout); l[0] = 0; }
    io_err = 0;
    for (;;) {
        while (l[i] == ' ' || l[i] == '\t') i++;
        if (!l[i]) break;
        int w = i, k;
        while (l[i] && l[i] != ' ' && l[i] != '\t' && l[i] != '=') i++;
        char sep = l[i];
        for (k = 0; k < nk; k++) if ((int)strlen(kw[k]) == i - w && !strncasecmp(kw[k], l + w, (size_t)(i - w))) break;
        if (k == nk) { io_err = 118; return 0; }
        if (sep) i++;
        if (strchr(opt[k], 'S')) { if (sep == '=') { io_err = 118; return 0; } wr32(arr + 4 * k, 0xFFFFFFFFu); continue; }
        if (sep != '=') while (l[i] == ' ' || l[i] == '\t') i++;
        if (!l[i]) { io_err = 117; return 0; }
        char v[256]; int n = 0;
        if (l[i] == '"') { i++; while (l[i] && l[i] != '"') v[n++] = l[i++]; if (!l[i]) { io_err = 119; return 0; } i++; }
        else while (l[i] && l[i] != ' ' && l[i] != '\t') v[n++] = l[i++];
        v[n] = 0;
        uint32_t at = heap; heap += 260;
        if (strchr(opt[k], 'N')) {
            char *end; long x = strtol(v, &end, 10);
            if (!n || *end) { io_err = 115; return 0; }
            wr32(at, (uint32_t)x);
        } else memcpy(mem + at, v, (size_t)n + 1);
        wr32(arr + 4 * k, at);
    }
    uint32_t rda = heap; heap += 8;
    return rda;
}

static void serve(unsigned lib, unsigned off)
{
    uint32_t *d = m68k_data_reg, *a = m68k_address_reg;
    char name[256];
    if (lib == 0) switch (off) {
        case 552: cstr(a[1], name, sizeof name); d[0] = strcmp(name, "dos.library") == 0 && !(in_init && no_dos_at_init) ? DOS_BASE : 0; return;   /* OpenLibrary */
        case 414: return;                                                                                         /* CloseLibrary */
        case 198: {                                                                                               /* AllocMem */
            uint32_t n = (d[0] + 7) & ~7u;
            if (heap + n > 0xD00000u) { d[0] = 0; return; }
            if (d[1] & 0x10000u) memset(mem + heap, 0, n); else memset(mem + heap, 0xA5, n);
            for (int k = 0; k < 256; k++) if (!blocks[k].at) { blocks[k].at = heap; blocks[k].size = d[0]; break; }
            d[0] = heap; heap += n; return;
        }
        case 210: {                                                                                               /* FreeMem */
            for (int k = 0; k < 256; k++) if (blocks[k].at == a[1] && blocks[k].size == d[0]) { blocks[k].at = 0; return; }
            fprintf(stderr, "amiga_host: FreeMem(%06X, %u) of a block that is not allocated (freed twice?)\n", a[1], d[0]);
            exit(106);
        }
        case 132: case 138: return;                                                                               /* Forbid, Permit */
    } else switch (off) {
        case 30: {                                                                                                 /* Open */
            cstr(d[1], name, sizeof name);
            for (int k = 3; k < 16; k++) if (!fhs[k]) { fhs[k] = host_open(name); d[0] = fhs[k] ? (uint32_t)k : 0; return; }
            d[0] = 0; return;
        }
        case 36: if (d[1] < 16 && fhs[d[1]]) { fclose(fhs[d[1]]); fhs[d[1]] = 0; } d[0] = 1; return;              /* Close */
        case 42: {                                                                                                 /* Read */
            if (d[1] >= 16 || !fhs[d[1]]) { d[0] = (uint32_t)-1; return; }
            size_t n = fread(mem + (d[2] & (MEM - 1)), 1, d[3], fhs[d[1]]); d[0] = (uint32_t)n; return;
        }
        case 48: if (d[1] == 1) { fwrite(mem + (d[2] & (MEM - 1)), 1, d[3], stdout); fflush(stdout); } d[0] = d[3]; return;   /* Write */
        case 54: d[0] = 2; return;                                                                                 /* Input */
        case 60: d[0] = 1; return;                                                                                 /* Output */
        case 66: {                                                                                                 /* Seek */
            FILE *f = d[1] < 16 ? fhs[d[1]] : 0; if (!f) { d[0] = (uint32_t)-1; return; }
            long before = ftell(f); fseek(f, (int32_t)d[2], (int32_t)d[3] == -1 ? SEEK_SET : (int32_t)d[3] == 0 ? SEEK_CUR : SEEK_END); d[0] = (uint32_t)before; return;
        }
        case 132: d[0] = (uint32_t)io_err; return;                                                                 /* IoErr */
        case 798: d[0] = read_args(d[1], d[2]); return;                                                            /* ReadArgs */
        case 858: return;                                                                                          /* FreeArgs */
        case 474: {                                                                                                /* PrintFault */
            static const char *const t[] = { "bad number", "required argument missing", "value after keyword missing", "wrong number of arguments",
                                             "unmatched quotes", "argument line invalid or too long" };
            cstr(d[2], name, sizeof name);
            if (d[1] >= 115 && d[1] <= 120) printf("%s: %s\n", name, t[d[1] - 115]); else printf("%s: error %d\n", name, (int)d[1]);
            fflush(stdout); return;
        }
        case 198: return;                                                                                          /* Delay */
        case 774: cstr(d[1], seg_name, sizeof seg_name); segment = d[2]; wr32(SEG_NODE + 4, uc_forced ? uc_value : d[3]); d[0] = 1; return;   /* AddSegment */
        case 780: {                                                    /* FindSegment as Kickstart 3.1's: FALSE returns a use count above 0, TRUE a negative one */
            int32_t uc = (int32_t)rd32(SEG_NODE + 4);
            cstr(d[1], name, sizeof name);
            d[0] = segment && !strcmp(name, seg_name) && (d[3] ? uc < 0 : uc > 0) ? SEG_NODE : 0; return;
        }
    }
    fprintf(stderr, "amiga_host: library %u call -%u is not simulated; the last jumps:", lib, off);
    dump_ring();
    exit(101);
}

static uint32_t load_hunks(const char *file)
{
    FILE *f = fopen(file, "rb");
    static uint8_t b[1 << 20];
    size_t n = f ? fread(b, 1, sizeof b, f) : 0;
    if (f) fclose(f);
    #define L(o) ((uint32_t)b[o] << 24 | b[(o) + 1] << 16 | b[(o) + 2] << 8 | b[(o) + 3])
    if (n < 32 || L(0) != 0x3F3 || L(4) != 0 || L(8) != 1) { fprintf(stderr, "amiga_host: %s is not a one-hunk executable\n", file); exit(102); }
    uint32_t o = 24, at = HUNK_AT;
    for (;;) {
        uint32_t t = L(o) & 0x3FFFFFFF; o += 4;
        if (t == 0x3E9 || t == 0x3EA) { uint32_t nl = L(o); o += 4; memcpy(mem + at, b + o, nl * 4); o += nl * 4; }
        else if (t == 0x3EC) { for (uint32_t cnt; (cnt = L(o)) != 0;) { o += 8; for (uint32_t k = 0; k < cnt; k++, o += 4) wr32(at + L(o), rd32(at + L(o)) + at); } o += 4; }
        else if (t == 0x3F2) return at;
        else { fprintf(stderr, "amiga_host: hunk type %X\n", t); exit(103); }
    }
}

/* The Shell's call of the program for run `run_no`: the argument line in A0 / D0, a return to EXIT_ADDR. */
static void start_run(uint32_t entry)
{
    uint32_t args = 0x00F000u + 0x100, len = (uint32_t)strlen(runs[run_no]);
    memcpy(mem + args, runs[run_no], len);
    m68k_address_reg[7] = STACK_TOP - 4; wr32(STACK_TOP - 4, EXIT_ADDR);
    m68k_address_reg[0] = args; m68k_data_reg[0] = len;
    mc68k_pc = entry;
}

static void hook(void)
{
    if (!started) {                                                   /* before the first instruction: the Shell's call of the program */
        started = 1;
        mc68k_flags = 0x0700;                                         /* user mode */
        wr32(4, EXEC_BASE);
        start_run(HUNK_AT);
        if (resident_at) {                                           /* the tag's init first: d0 = 0, a0 = 0, a6 = SysBase */
            wr32(STACK_TOP - 4, INIT_DONE);
            m68k_address_reg[6] = EXEC_BASE; m68k_data_reg[0] = 0; m68k_address_reg[0] = 0;
            mc68k_pc = rd32(resident_at + 22);
            in_init = 1;
        }
        return;
    }
    if (mc68k_pc == INIT_DONE) {                                      /* then the command it added, as the Shell runs a resident one */
        in_init = 0;
        if (!segment && no_dos_at_init) { fprintf(stderr, "amiga_host: the init added nothing, as expected without dos\n"); longjmp(done, 1); }
        if (!segment || strcmp(seg_name, "buffee")) { fprintf(stderr, "amiga_host: the resident tag added no 'buffee' segment\n"); exit(105); }
        start_run((segment << 2) + 4);
        return;
    }
    if (mc68k_pc - last_pc > 10) { ring[nring++ & 63] = last_pc; ring[nring++ & 63] = mc68k_pc; }    /* (a jump: where from, where to) */
    last_pc = mc68k_pc;
    if (++icount > 2000000000ull) { fprintf(stderr, "amiga_host: no end after 2e9 instructions\n"); exit(104); }
    if ((mc68k_pc & 0xFF0000u) != TRAP_AREA) return;
    if (mc68k_pc == EXIT_ADDR) {
        ret_code = (int)m68k_data_reg[0];
        fprintf(stderr, "[rc %d]\n", ret_code);
        if (++run_no == nruns) longjmp(done, 1);
        start_run(resident_at ? (segment << 2) + 4 : HUNK_AT);
        return;
    }
    serve((mc68k_pc >> 12) & 0xF ? 1 : 0, mc68k_pc & 0xFFF);
    mc68k_pc = rd32(sp()); set_sp(sp() + 4);                          /* RTS */
}

int main(int argc, char **argv)
{
    const char *flash = 0, *eeprom = 0, *rom = 0;
    static char lines[16][1024];
    int i = 1;

    const char *resident = 0;
    int tag_listed = -1;                                               /* -T 1: exec's list holds the tag (default with -R), -T 0: it does not */
    for (; i < argc - 1 && argv[i][0] == '-'; i += 2) {
        if (!strcmp(argv[i], "-f")) flash = argv[i + 1];
        else if (!strcmp(argv[i], "-e")) eeprom = argv[i + 1];
        else if (!strcmp(argv[i], "-r")) rom = argv[i + 1];
        else if (!strcmp(argv[i], "-d")) root = argv[i + 1];
        else if (!strcmp(argv[i], "-R")) resident = argv[i + 1];
        else if (!strcmp(argv[i], "-T")) tag_listed = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-N")) no_dos_at_init = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-U")) { uc_forced = 1; uc_value = (uint32_t)atoi(argv[i + 1]); }
        else if (!strcmp(argv[i], "-V")) dos_version = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-X")) exec_version = atoi(argv[i + 1]);
        else break;
    }
    if (!resident && i >= argc) { fprintf(stderr, "usage: amiga_host [-f flash] [-e eeprom] [-r rom] [-d dir] [-T 0|1] [-N 1] [-U n] [-V n] [-X n] <program> | -R <resident.bin> [arguments...] [-- arguments...]\n"); return 100; }
    const char *program = resident ? 0 : argv[i++];
    for (int k = i; k < argc && nruns < 16; k++) {                    /* the command lines, -- between them */
        if (!strcmp(argv[k], "--")) { strcat(lines[nruns], "\n"); runs[nruns] = lines[nruns]; nruns++; continue; }
        if (lines[nruns][0]) strcat(lines[nruns], " ");
        strcat(lines[nruns], argv[k]);
    }
    strcat(lines[nruns], "\n"); runs[nruns] = lines[nruns]; nruns++;
    mock_board_load(flash, eeprom, rom);
    wr32(0, STACK_TOP); wr32(4, HUNK_AT);                             /* the reset vectors (the hook takes over before the first instruction) */
    for (unsigned lib = 0; lib < 2; lib++)                            /* the jump tables: JMP TRAP_AREA + lib * 0x1000 + offset, every 6 bytes below the base */
        for (unsigned off = 6; off <= 900; off += 6) { uint32_t at = (lib ? DOS_BASE : EXEC_BASE) - off; wr16(at, 0x4EF9); wr32(at + 2, TRAP_AREA + lib * 0x1000 + off); }
    wr16(EXEC_BASE + 20, (uint16_t)exec_version); wr16(DOS_BASE + 20, (uint16_t)dos_version);   /* lib_Version: Kickstart 3.1, or -X / -V */
    if (tag_listed < 0) tag_listed = resident != 0;
    wr32(EXEC_BASE + 300, RES_MODULES); wr32(RES_MODULES, 0xF80100u);  /* ResModules: a tag of the ROM's, the board's if listed, the end */
    wr32(RES_MODULES + 4, tag_listed ? 0xF01000u : 0); wr32(RES_MODULES + 8, 0);
    if (resident) {
        FILE *h = fopen(resident, "rb");
        if (!h) { fprintf(stderr, "amiga_host: %s?\n", resident); return 100; }
        resident_at = 0xF01000u;
        size_t n = fread(mem + resident_at, 1, 0x10000, h); (void)n; fclose(h);
    } else load_hunks(program);
    memcpy(page, "BUFC\0\0\0\1", 8);
    memory_base = 0; ReadByte = rb; WriteByte = wb; ReadWord = rw; WriteWord = ww;
    mcl68k_step_hook = hook;
    if (setjmp(mock_reboot)) { fprintf(stderr, "[board reset]\n"); mock_board_save(); return 0; }
    if (setjmp(done) == 0) run_mcl68k(0);
    mock_board_save();
    return ret_code;
}
