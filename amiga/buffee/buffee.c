/* buffee -- the Buffee board from an AmigaOS Shell: its settings, its clock, pace and caches, and the PJIT and Kickstart images in its flash (docs/buffee/BUFFEE_CLI.md).
 * It talks to the board through PJIT's control page (platform/buffee/control.h); on anything else (a real 68000, an older PJIT) it finds no control page and changes nothing.
 * Kickstart 1.2 and later. The arguments follow one template: dos's ReadArgs reads them on Kickstart 2.0 and later (`buffee ?` shows the template), and read_args
 * below, which takes the same template's keywords into the same array, on 1.2 and 1.3.
 * The program can be resident (amiga/buffee/resident.S): one copy that every run shares, so its only writable data is the bss, which amiga/lib/start.S zeroes at each start.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdint.h>
#include "amiga_os.h"
#include "control.h"

#define VERSION   "1.0"
#define CHUNK     16384u                /* bytes per upload */
#define REPLY_MAX 8192u
#define RC_OK 0
#define RC_WARN 5
#define RC_ERROR 10
#define RC_FAIL 20

void *DOSBase;
static int32_t out;
static char *reply;                     /* REPLY_MAX bytes: the board's text */

__attribute__((used)) static const char version[] = "$VER: buffee " VERSION " (23.11.2026)";

static const char usage[] =                                   /* lines under 60 columns: a Shell is about 77 wide */
    "buffee " VERSION ": the Buffee board's settings and firmware\n"
    "With no options it shows the board and its settings;\n"
    "buffee ? shows the template, HELP this text.\n"
    "  PJIT=<file>          store a PJIT bundle *\n"
    "  ROM=<file>           store a Kickstart for MapROM *\n"
    "  COPYROM              store this Amiga's Kickstart *\n"
    "  VERIFY               compare PJIT= or ROM= with the flash\n"
    "  MAPROM=OFF|FLASH|COPY  MapROM's source *\n"
    "  MAPROMADDR=<hex>     MapROM's address *\n"
    "  MAPROMSIZE=<KB>      MapROM's size *\n"
    "  FASTRAM=<hex>/<MB>[B][I][F],...|NONE  board RAM *\n"
    "  BUSCLOCK=<MHz>|0     the bus clock, 0 measures it *\n"
    "  SELFTEST=SHORT|FULL,BUS|NOBUS  the boot's self-test *\n"
    "  REPORT=QUIET|VERBOSE the boot report *\n"
    "  VOLTAGE=<V>          the ARM's core voltage *\n"
    "  PACE=<percent>       68000 pace, 0 = off\n"
    "  CLOCK=<MHz>          the ARM's clock\n"
    "  L1=ON|OFF L2=ON|OFF  the ARM's caches\n"
    "  SAVE                 keep PACE and CLOCK\n"
    "  NOREBOOT             do not reboot after a *\n"
    "* used from the next boot: the board reboots\n";

/* ---- small library ----------------------------------------------------------------------------------------------------------------- */
void *memcpy(void *d, const void *s, uint32_t n) { uint8_t *a = d; const uint8_t *b = s; while (n--) *a++ = *b++; return d; }
void *memset(void *d, int c, uint32_t n) { uint8_t *a = d; while (n--) *a++ = (uint8_t)c; return d; }
static uint32_t slen(const char *s) { uint32_t n = 0; while (s[n]) n++; return n; }
static void puts_(const char *s) { Write(out, s, (int32_t)slen(s)); }
static void putn(int32_t v)                                           /* decimal, without a divide (no libgcc) */
{
    static const uint32_t ten[] = { 1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u, 1u };
    char b[13]; int n = 0, k; uint32_t u = v < 0 ? 0u - (uint32_t)v : (uint32_t)v;
    if (v < 0) b[n++] = '-';
    for (k = 0; k < 10; k++) { char d = '0'; while (u >= ten[k]) { u -= ten[k]; d++; } if (d != '0' || n > (v < 0) || k == 9) b[n++] = d; }
    b[n] = 0; puts_(b);
}
static int upper(int c) { return c >= 'a' && c <= 'z' ? c - 32 : c; }
static int same(const char *a, const char *b) { while (*a && upper(*a) == upper(*b)) a++, b++; return !*a && !*b; }

static uint32_t crc_table[256];
static uint32_t crc32(uint32_t crc, const uint8_t *p, uint32_t n)
{
    if (!crc_table[1]) for (uint32_t i = 0; i < 256; i++) { uint32_t c = i; for (int k = 0; k < 8; k++) c = (c & 1) ? (c >> 1) ^ 0xEDB88320u : c >> 1; crc_table[i] = c; }
    while (n--) crc = (crc >> 8) ^ crc_table[(crc ^ *p++) & 0xFF];
    return crc;
}

/* ---- the board ------------------------------------------------------------------------------------------------------------------------ */
static volatile bfc_req_t req;

static int board_present(void)
{
    volatile const uint32_t *page = (volatile const uint32_t *)BFC_PAGE;
    return page[0] == BFC_MAGIC && page[1] >= 1;
}

/* One request; the board's text is printed. Returns its result (BFC_OK or BFC_E_*). */
static int32_t ask(uint32_t cmd, uint32_t a0, uint32_t a1, uint32_t a2, const void *data, uint32_t len, int show)
{
    req.magic = BFC_REQ_MAGIC; req.cmd = cmd; req.arg[0] = a0; req.arg[1] = a1; req.arg[2] = a2; req.arg[3] = 0;
    req.data = (uint32_t)data; req.len = len; req.reply = (uint32_t)reply; req.reply_max = REPLY_MAX; req.result = BFC_E_COMMAND;
    reply[0] = 0;
    __asm volatile ("" ::: "memory");
    *(volatile uint32_t *)BFC_PAGE = (uint32_t)&req;        /* PJIT serves the request before this instruction ends */
    __asm volatile ("" ::: "memory");
    if (show) puts_(reply);
    return req.result;
}

/* ---- files --------------------------------------------------------------------------------------------------------------------------- */
/* Sends a file to the board's staging buffer in pieces (no 1 MB of contiguous memory needed on the Amiga) and returns its length and CRC; 0 on failure. */
static int send_file(const char *name, uint32_t *len, uint32_t *crc, int upload)
{
    int32_t fh = Open(name, MODE_OLDFILE), n;
    uint8_t *buf;
    uint32_t total = 0, c = 0xFFFFFFFFu;
    int ok = 1;

    if (!fh) { puts_("Cannot open "); puts_(name); puts_("\n"); return 0; }
    buf = AllocMem(CHUNK, MEMF_PUBLIC);
    if (!buf) { Close(fh); puts_("Out of memory\n"); return 0; }
    while ((n = Read(fh, buf, CHUNK)) > 0) {
        if (total + (uint32_t)n > BFC_STAGE_MAX) { puts_(name); puts_(" is larger than 1 MB\n"); ok = 0; break; }
        c = crc32(c, buf, (uint32_t)n);
        if (upload && ask(BFC_UPLOAD, total, 0, 0, buf, (uint32_t)n, 1) != BFC_OK) { ok = 0; break; }
        total += (uint32_t)n;
    }
    if (n < 0) { puts_("Cannot read "); puts_(name); puts_("\n"); ok = 0; }
    if (ok && total == 0) { puts_(name); puts_(" is empty\n"); ok = 0; }
    FreeMem(buf, CHUNK);
    Close(fh);
    *len = total; *crc = ~c;
    return ok;
}

/* Whether `buffee` is resident (Kickstart 3.0 and later: resident.S): dos has it in the Shell's resident list; or else what the tag's init reported to the board (`init`, BFC_RES_*),
   or, if it reported nothing, whether exec found the tag (rt_Init runs only for a tag in exec's list of resident modules, ExecBase->ResModules: an array of tag
   pointers, a pointer with bit 31 set continuing at another array). */
static void show_resident(uint32_t init, uint32_t uc_added)
{
    const uint8_t *seg;
    static const char why[][40] = { "", "dos.library was not up", "no memory for it", "AddSegment refused it", "added, then removed" };   /* BFC_RES_* */
    const uint8_t *sys = SysBase;
    const uint32_t *m = *(const uint32_t *const *)(sys + 300);
    int tag = 0;

    if (*(const uint16_t *)(sys + 20) < 39) { puts_("Resident  no (Kickstart 3.0 and later)\n"); return; }
    if (FindSegment("buffee", 0, 0)) { puts_("Resident  yes\n"); return; }      /* as the Shell looks it up */
    if ((seg = FindSegment("buffee", 0, 1))) {                        /* listed, but not as a command (a negative use count, seg_UC) */
        puts_("Resident  no: listed with use count "); putn(*(const int32_t *)(seg + 4)); puts_("\n");
        return;
    }
    while (m && *m) {
        if (*m & 0x80000000u) m = (const uint32_t *)(*m & 0x7FFFFFFFu);
        else if (*m++ == BFC_CMD_TAG) tag = 1;
    }
    if (init == BFC_RES_ADDED && uc_added == 0x80000000u) puts_("Resident  no: added, but the Shell does not find it\n");
    else if (init > BFC_RES_NOT_RUN && init <= BFC_RES_ADDED) { puts_("Resident  no: when the tag ran, "); puts_(why[init]); puts_("\n"); }
    else puts_(tag ? "Resident  no: Kickstart found the tag but did not run it\n" : "Resident  no: Kickstart did not find the tag\n");
}

/* PJIT=<file> or ROM=<file>: store it, or with VERIFY compare it. Returns a return code. */
static int image(int what, const char *file, int verify, int *changed)
{
    uint32_t len, crc;

    puts_(file); puts_(verify ? ": comparing\n" : ": sending\n");
    if (!send_file(file, &len, &crc, !verify)) return RC_ERROR;
    if (verify) {
        if (ask(BFC_VERIFY, (uint32_t)what, len, crc, 0, 0, 1) != BFC_OK) return RC_ERROR;
        return req.out[0] ? RC_OK : RC_WARN;
    }
    if (ask(BFC_STORE, (uint32_t)what, len, crc, 0, 0, 1) != BFC_OK) { puts_("Nothing changed\n"); return RC_ERROR; }
    *changed = 1;
    return RC_OK;
}

/* ---- the arguments ------------------------------------------------------------------------------------------------------------------- */
/* One template for both ways of reading the arguments: each keyword takes a value (/K: KEY=value or KEY value, the value maybe in quotes; /N: a number) or is a
   switch (/S). The array holds, by keyword, 0 when it was not given, else a pointer to the value (a string, or the LONG of a /N) or, for a switch, -1. */
static const char template_[] = "PJIT/K,ROM/K,COPYROM/S,VERIFY/S,MAPROM/K,MAPROMADDR/K,MAPROMSIZE/K,FASTRAM/K,BUSCLOCK/K,SELFTEST/K,REPORT/K,VOLTAGE/K,"
                                "PACE/K/N,CLOCK/K/N,L1/K,L2/K,SAVE/S,NOREBOOT/S,STATUS/S,HELP/S";
enum { A_PJIT, A_ROM, A_COPYROM, A_VERIFY, A_MAPROM, A_MAPROMADDR, A_MAPROMSIZE, A_FASTRAM, A_BUSCLOCK, A_SELFTEST, A_REPORT, A_VOLTAGE,
       A_PACE, A_CLOCK, A_L1, A_L2, A_SAVE, A_NOREBOOT, A_STATUS, A_HELP, A_COUNT };

static int dec(const char *s, uint32_t *v) { uint32_t x = 0; int d = 0; for (; *s >= '0' && *s <= '9'; s++, d++) x = x * 10 + (uint32_t)(*s - '0'); *v = x; return d > 0 && !*s; }
static int onoff(const char *s) { return s && same(s, "ON") ? 1 : s && same(s, "OFF") ? 0 : -1; }

/* dos's texts for the errors ReadArgs reports (dos.h: ERROR_BAD_NUMBER 115 ... ERROR_LINE_TOO_LONG 120), for read_args, which has no PrintFault to call. */
static const char faults[][36] = { "bad number", "required argument missing", "value after keyword missing", "wrong number of arguments", "unmatched quotes",
                                   "argument line invalid or too long" };

/* Whether the template's options after a keyword (from its first '/' to the next ',') include /c. */
static int has(const char *o, int c) { for (; *o && *o != ','; o++) if (o[0] == '/' && upper(o[1]) == c) return 1; return 0; }

/* ReadArgs for Kickstart 1.2 and 1.3: the Shell's argument line into `arr` by the template. Returns 0, -1 for a lone '?', or the dos error (115-120). */
static char line[256];
static int32_t nums[A_COUNT];
static int32_t read_args(uint32_t len, const char *args, int32_t *arr)
{
    uint32_t i = 0, n = 0;

    for (; n < len && args[n] != '\n'; n++) { if (n == sizeof line - 1) return 120; line[n] = args[n]; }
    line[n] = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    if (line[i] == '?' && !line[i + 1]) return -1;
    while (line[i]) {
        uint32_t w = i, wl, k, j;
        const char *t = template_, *opts = 0;
        int found = -1;
        char sep, *v;

        while (line[i] && line[i] != ' ' && line[i] != '\t' && line[i] != '=') i++;
        wl = i - w; sep = line[i];
        for (k = 0; *t; k++) {
            const char *s = t;
            while (*t && *t != '/' && *t != ',') t++;
            if ((uint32_t)(t - s) == wl) { for (j = 0; j < wl && upper(s[j]) == upper(line[w + j]); j++) { } if (j == wl) { found = (int)k; opts = t; } }
            while (*t && *t != ',') t++;
            if (*t) t++;
        }
        if (found < 0) return 118;                                     /* a word that is no keyword: the template takes nothing by position */
        if (sep) line[i++] = 0;
        if (has(opts, 'S')) { if (sep == '=') return 118; arr[found] = -1; }
        else {
            if (sep != '=') while (line[i] == ' ' || line[i] == '\t') i++;
            if (!line[i]) return 117;
            if (line[i] == '"') { v = line + ++i; while (line[i] && line[i] != '"') i++; if (!line[i]) return 119; line[i++] = 0; }
            else { v = line + i; while (line[i] && line[i] != ' ' && line[i] != '\t') i++; if (line[i]) line[i++] = 0; }
            if (has(opts, 'N')) { uint32_t x; if (!dec(v, &x)) return 115; nums[found] = (int32_t)x; arr[found] = (int32_t)&nums[found]; }
            else arr[found] = (int32_t)v;
        }
        while (line[i] == ' ' || line[i] == '\t') i++;
    }
    return 0;
}

/* A setting by keyword -> "item=value" of the settings page (bootloader/src/settings_set.c), appended to `set`. */
static const struct { uint8_t arg; char item[3]; } named[] = {
    { A_MAPROM, "1" }, { A_MAPROMADDR, "2" }, { A_MAPROMSIZE, "3" }, { A_FASTRAM, "4" }, { A_BUSCLOCK, "6" }, { A_VOLTAGE, "8" }, { A_REPORT, "9" }, { A_SELFTEST, "10" },
};

static void append(char *dst, uint32_t max, const char *s) { uint32_t n = slen(dst); while (*s && n < max - 1) dst[n++] = *s++; dst[n] = 0; }

static int bad(const char *what) { puts_("buffee: "); puts_(what); puts_("\n"); return RC_ERROR; }

int amiga_main(uint32_t len, const char *args)
{
    static char set[200];
    int32_t a[A_COUNT];
    void *rda = 0;
    int changed = 0, any = 0, rc = RC_OK, k, i;
    int pace = -1, clock = -1, l1 = -1, l2 = -1;

    DOSBase = OpenLibrary("dos.library", 0);
    if (!DOSBase) return RC_FAIL;
    out = Output();
    for (k = 0; k < A_COUNT; k++) a[k] = 0;
    if (*(const uint16_t *)((const uint8_t *)DOSBase + 20) >= 36) {
        if (!(rda = ReadArgs(template_, a, 0))) { PrintFault(IoErr(), "buffee"); rc = RC_ERROR; goto done; }
    } else if ((k = read_args(len, args, a)) != 0) {
        if (k < 0) { puts_(template_); puts_("\n"); goto done; }
        rc = bad(faults[k - 115]); goto done;
    }
    if (a[A_HELP]) { puts_(usage); goto done; }
    for (k = 0; k < A_COUNT; k++) if (a[k]) any = 1;

    set[0] = 0;
    for (i = 0; i < (int)(sizeof named / sizeof named[0]); i++)
        if (a[named[i].arg]) {
            const char *V = (const char *)a[named[i].arg];
            if (named[i].arg == A_FASTRAM && same(V, "NONE")) V = "0";
            append(set, sizeof set, set[0] ? ";" : ""); append(set, sizeof set, named[i].item); append(set, sizeof set, "="); append(set, sizeof set, V);
        }
    if (a[A_PACE] && (uint32_t)(pace = *(const int32_t *)a[A_PACE]) > 255) { rc = bad("PACE is 0 to 255 (percent)"); goto done; }
    if (a[A_CLOCK] && ((clock = *(const int32_t *)a[A_CLOCK]) < 24 || clock > 1100)) { rc = bad("CLOCK is 24 to 1100 (MHz)"); goto done; }
    if (a[A_L1] && (l1 = onoff((const char *)a[A_L1])) < 0) { rc = bad("L1 is ON or OFF"); goto done; }
    if (a[A_L2] && (l2 = onoff((const char *)a[A_L2])) < 0) { rc = bad("L2 is ON or OFF"); goto done; }

    if (!board_present()) { puts_("No Buffee here, or its PJIT is too old\n"); rc = RC_FAIL; goto done; }
    reply = AllocMem(REPLY_MAX, MEMF_PUBLIC | MEMF_CLEAR);
    if (!reply) { puts_("Out of memory\n"); rc = RC_FAIL; goto done; }

    if (!any || a[A_STATUS]) { ask(BFC_STATUS, 0, 0, 0, 0, 0, 1); show_resident(req.out[0], req.out[1]); }
    /* the images first: a file that cannot be read or is refused leaves the settings as they were */
    if (a[A_PJIT] && (k = image(BFC_IMG_PJIT, (const char *)a[A_PJIT], a[A_VERIFY] != 0, &changed)) > rc) rc = k;
    if (rc >= RC_ERROR) goto done;
    if (a[A_ROM] && (k = image(BFC_IMG_ROM, (const char *)a[A_ROM], a[A_VERIFY] != 0, &changed)) > rc) rc = k;
    if (rc >= RC_ERROR) goto done;
    if (a[A_COPYROM]) { if (ask(BFC_COPY_ROM, 0, 0, 0, 0, 0, 1) != BFC_OK) { rc = RC_ERROR; goto done; } changed = 1; }
    if (set[0]) {
        if (ask(BFC_SETTINGS, 0, 0, 0, set, slen(set) + 1, 1) != BFC_OK) { puts_("Settings not changed\n"); rc = RC_ERROR; goto done; }
        if (req.out[0]) changed = 1;
    }
    if (pace >= 0 && ask(BFC_PACE, (uint32_t)pace, a[A_SAVE] != 0, 0, 0, 0, 1) != BFC_OK) rc = RC_ERROR;
    if (clock >= 0 && ask(BFC_CLOCK, (uint32_t)clock, a[A_SAVE] != 0, 0, 0, 0, 1) != BFC_OK) rc = RC_ERROR;
    if (l1 >= 0 || l2 >= 0) {
        uint32_t s = 0, c = 0;
        if (l1 == 1) s |= BFC_CACHE_L1I | BFC_CACHE_L1D; else if (l1 == 0) c |= BFC_CACHE_L1I | BFC_CACHE_L1D;
        if (l2 == 1) s |= BFC_CACHE_L2; else if (l2 == 0) c |= BFC_CACHE_L2;
        if (ask(BFC_CACHE, s, c, 0, 0, 0, 1) != BFC_OK) rc = RC_ERROR;
    }

    if (changed && a[A_NOREBOOT]) puts_("Used from the next boot\n");
    if (changed && !a[A_NOREBOOT]) {
        puts_("Rebooting\n");
        Delay(50);
        ask(BFC_REBOOT, 0, 0, 0, 0, 0, 0);
    }
done:
    if (reply) FreeMem(reply, REPLY_MAX);
    if (rda) FreeArgs(rda);
    CloseLibrary(DOSBase);
    return rc;
}
