/*
 * test_printf.c -- a self-check of the bootloader's printf (menu item 5): each case is formatted into a buffer with sprintf and compared with the text
 * the C standard gives (the size letters h and hh mask the argument to 16 and 8 bits, as printf.c documents), then the number of failures is printed.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <string.h>

static int total, failed;

static void expect(const char *got, const char *want) {
    total++;
    if (strcmp(got, want) != 0) {
        failed++;
        printf("FAIL: got \"%s\", want \"%s\"\n", got, want);
    }
}

#define CASE(want, ...) do { char buf[80]; sprintf(buf, __VA_ARGS__); expect(buf, want); } while (0)

void test_printf(void) {
    total = failed = 0;

    CASE("42", "%d", 42);
    CASE("-42", "%d", -42);
    CASE("   42", "%5d", 42);
    CASE("42   ", "%-5d", 42);
    CASE("00042", "%05d", 42);
    CASE("+42", "%+d", 42);
    CASE("4294967295", "%u", 0xFFFFFFFFu);
    CASE("2a", "%x", 42);
    CASE("2A", "%X", 42);
    CASE("0000002A", "%08X", 42);
    CASE("beef", "%04hx", 0x1beef);
    CASE("ef", "%02hhx", 0x1beef);
    CASE("a", "%c", 'a');
    CASE("hello", "%s", "hello");
    CASE("  hello", "%7s", "hello");
    CASE("hello  ", "%-7s", "hello");
    CASE("(null)", "%s", (char *)0);
    CASE("100%", "100%%");
    CASE("3.14", "%.2f", 3.14159);
    CASE("0.500", "%0.3f", 0.5);
    CASE("x=7 y=-8", "x=%d y=%d", 7, -8);

    printf("printf: %d checks, %d failed\n", total, failed);
}
