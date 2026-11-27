/// @file smoke_printf.c
/// @brief The payload's own snprintf (platform/pjit_libc.c) on the formats the payloads use, among them the
///        `l` and `z` length modifiers of the code shared with the bootloader (platform/buffee/control.c,
///        bootloader/src/settings.c): a modifier it did not know would print verbatim and shift every later
///        argument of the line.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);

static int pass, fail;

static void check(const char *got, const char *want)
{
    int i = 0;
    while (got[i] && got[i] == want[i]) i++;
    if (got[i] == want[i]) { pass++; return; }
    fail++;
    printf("[FAIL] got \"%s\", want \"%s\"\n", got, want);
}

int main(void)
{
    char b[128];

    printf("=== PJIT smoke test: snprintf formats ===\n");
    snprintf(b, sizeof b, "%u %d %x %X", 42u, -7, 0xBEEFu, 0xBEEFu);                          check(b, "42 -7 beef BEEF");
    snprintf(b, sizeof b, "%08X|%-6s|%6s|%c", 0x1234u, "ab", "cd", 'z');                      check(b, "00001234|ab    |    cd|z");
    snprintf(b, sizeof b, "%s, $%06lX, %lu KB", "copy", 0xF80000ul, 512ul);                   check(b, "copy, $F80000, 512 KB");
    snprintf(b, sizeof b, "%lX/%lu%s", 0x200000ul, 8ul, "BIF");                               check(b, "200000/8BIF");
    snprintf(b, sizeof b, "%lu bytes, CRC %08lX, #%lu", 150004ul, 0x0BADF00Dul, 3ul);         check(b, "150004 bytes, CRC 0BADF00D, #3");
    snprintf(b, sizeof b, "%ld %li %lx %zu", -5l, 6l, 0xABCul, (size_t)99);                   check(b, "-5 6 abc 99");
    snprintf(b, sizeof b, "%d%%", 100);                                                       check(b, "100%");

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
