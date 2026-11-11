/*
 * tinybasic.c -- a small BASIC interpreter, used as a guest program by the PJIT tests.
 *
 * Copyright (c) 2026 Renee Cousins, the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 *
 * It is written from scratch for the tests.
 * Like Tiny BASIC it keeps the program as text and parses every statement again each time it runs, which makes it a good workload for a
 * translator: byte scanning, keyword compares, a jump per statement, calls and returns, a few 16 x 16 multiplications.
 *
 *   Immediate:  RUN  LIST  NEW  BYE        (and any statement without a line number)
 *   Statements: REM  PRINT  [LET] v=e  [LET] @(e)=e  IF a <rel> b [THEN] stmt|line  GOTO e  GOSUB e  RETURN
 *               FOR v=e TO e [STEP e]  NEXT v  END  STOP
 *   Values:     32-bit integers, variables A-Z, the array @(0..2047), ABS(e), SIZE;  e: + - * / %  unary -  ( )
 *   Relations:  =  <  >  <=  >=  <>      Lines: one statement each, up to 64 lines of 125 characters.
 *
 * Console: three routines at fixed guest addresses, filled in by whoever runs the image (the PJIT harness: TRAP #1/#2/#14 and RTS; the oracle:
 * loads and stores of a memory-mapped port).  Typed characters are not echoed: the console does that.  There is no 32-bit multiply or divide
 * instruction on a 68000, and no libgcc is linked, so mul() and divmod() are written out below.
 *
 *   cc -DTB_HOST -O1 -o tinybasic test_progs/tinybasic.c      runs it on the host with stdin and stdout as the console (for debugging)
 *   tools/data/tinybasic.py                                   builds the 68000 image, tests/data/tinybasic_image.h
 */
#include <stdint.h>

#ifdef TB_HOST
#include <stdio.h>
#include <stdlib.h>
static void out_char(int c) { putchar(c); }
static int in_char(void) { int c = getchar(); return c == EOF ? 0 : c; }
static void quit(void) { fflush(stdout); exit(0); }
#else
#define BIOS_PUTC 0x100     /* D0.B = character to print               */
#define BIOS_GETC 0x108     /* returns D0.B = the next typed character, or 0 when there is none */
#define BIOS_EXIT 0x110     /* does not return                         */
static void out_char(int c)
{
    register int d0 __asm__("d0") = c;
    __asm__ volatile("jsr 0x100" : "+d"(d0) : : "cc", "memory");
}
static int in_char(void)
{
    register int d0 __asm__("d0");
    __asm__ volatile("jsr 0x108" : "=d"(d0) : : "cc", "memory");
    return d0 & 0xFF;
}
static void quit(void)
{
    __asm__ volatile("jsr 0x110" : : : "cc", "memory");
    for (;;) {}
}
/* the entry point, first in the image */
__asm__(".pushsection .text.start,\"ax\"\n.globl _start\n_start:\n\tlea 0xF0000,%sp\n\tjsr main\n\tjsr 0x110\n1:\tbra.s 1b\n.popsection\n");
#endif

#define MAXLINES 64
#define LINELEN 126
#define ATSIZE 2048
#define FORDEPTH 8
#define IDLE_LIMIT 200000   /* empty polls of the console in a row before giving up (a harness that has run out of keystrokes) */

typedef int32_t num;

static struct { uint16_t no; char s[LINELEN]; } prog[MAXLINES];    /* sorted by line number */
static int nlines;
static num var[26], at[ATSIZE];
static const char *ip;                                              /* the statement being scanned */
static int cur, next, running, err;                                 /* index of the line being run, index of the one after it */
static struct { int v; num limit, step; int line; } fors[FORDEPTH];
static int nfor;
static int gosubs[FORDEPTH];
static int ngosub;

static void puts_(const char *s) { while (*s) out_char(*s++); }
static void newline(void) { out_char('\r'); out_char('\n'); }

static uint32_t divmod(uint32_t n, uint32_t d, uint32_t *rem)
{
    uint32_t q = 0, r = 0;
    for (int i = 31; i >= 0; i--) {
        r = (r << 1) | ((n >> i) & 1);
        if (r >= d) { r -= d; q |= 1u << i; }
    }
    *rem = r;
    return q;
}

/* the low 16 bits of a times the low 16 bits of b, as 32 bits: one MULU.W (written out because GCC may call __mulsi3 instead) */
static inline uint32_t mulu16(uint32_t a, uint32_t b)
{
#ifdef TB_HOST
    return (uint32_t)(uint16_t)a * (uint16_t)b;
#else
    __asm__("mulu.w %1,%0" : "+d"(a) : "d"(b) : "cc");
    return a;
#endif
}

/* v * 10 as two shifts and an add; the barrier stops GCC from folding it back into a multiplication, which would call __mulsi3 */
static inline num times10(num v)
{
    num t = v << 1;
#ifndef TB_HOST
    __asm__("" : "+d"(t));
#endif
    return t + (t << 2);
}

static num mul(num a, num b)
{
    uint32_t x = a < 0 ? -(uint32_t)a : (uint32_t)a, y = b < 0 ? -(uint32_t)b : (uint32_t)b;
    uint32_t r = mulu16(x, y) + ((mulu16(x >> 16, y) + mulu16(x, y >> 16)) << 16);
    return (a < 0) != (b < 0) ? -(num)r : (num)r;
}

static num div_(num a, num b, int want_rem)
{
    uint32_t x = a < 0 ? -(uint32_t)a : (uint32_t)a, y = b < 0 ? -(uint32_t)b : (uint32_t)b, r;
    uint32_t q = divmod(x, y, &r);
    if (want_rem) return a < 0 ? -(num)r : (num)r;
    return (a < 0) != (b < 0) ? -(num)q : (num)q;
}

static void put_num(num v)
{
    char buf[12];
    int n = 0;
    uint32_t u = v < 0 ? -(uint32_t)v : (uint32_t)v, r;
    if (v < 0) out_char('-');
    do { u = divmod(u, 10, &r); buf[n++] = (char)('0' + r); } while (u);
    while (n) out_char(buf[--n]);
}

static int upper(int c) { return c >= 'a' && c <= 'z' ? c - 32 : c; }
static int digit(int c) { return c >= '0' && c <= '9'; }
static void skip(void) { while (*ip == ' ') ip++; }

/* is the keyword next (blanks skipped, case ignored)?  If so, step over it */
static int match(const char *kw)
{
    const char *p = ip;
    while (*p == ' ') p++;
    while (*kw) {
        if (upper(*p) != *kw) return 0;
        p++; kw++;
    }
    ip = p;
    return 1;
}

static int expect(int c)
{
    skip();
    if (*ip != c) { err = 1; return 0; }
    ip++;
    return 1;
}

static num expr(void);

static int index_(num i)
{
    if (i < 0 || i >= ATSIZE) { err = 3; return 0; }
    return (int)i;
}

static num primary(void)
{
    skip();
    int c = upper(*ip);
    if (digit(c)) {
        num v = 0;
        while (digit(*ip)) v = times10(v) + (*ip++ - '0');
        return v;
    }
    if (c == '(') { ip++; num v = expr(); expect(')'); return v; }
    if (c == '@') { ip++; expect('('); num i = expr(); expect(')'); return at[index_(i)]; }
    if (match("ABS(")) { num v = expr(); expect(')'); return v < 0 ? -v : v; }
    if (match("SIZE")) return ATSIZE;
    if (c >= 'A' && c <= 'Z') { ip++; return var[c - 'A']; }
    err = 1;
    return 0;
}

/* a value with any number of unary minus signs in front (a flag, not a recursive negation: GCC turns that into a multiplication by -1) */
static num factor(void)
{
    int neg = 0;
    for (skip(); *ip == '-'; skip()) { neg ^= 1; ip++; }
    num v = primary();
    return neg ? -v : v;
}

static num term(void)
{
    num v = factor();
    for (;;) {
        skip();
        if (*ip == '*') { ip++; v = mul(v, factor()); }
        else if (*ip == '/' || *ip == '%') {
            int rem = *ip++ == '%';
            num d = factor();
            if (d == 0) { err = 2; return 0; }
            v = div_(v, d, rem);
        } else return v;
    }
}

static num expr(void)
{
    num v = term();
    for (;;) {
        skip();
        if (*ip == '+') { ip++; v += term(); }
        else if (*ip == '-') { ip++; v -= term(); }
        else return v;
    }
}

static int cond(void)
{
    num a = expr(), b;
    int op = 0;
    skip();
    while (*ip == '=' || *ip == '<' || *ip == '>') { op |= *ip == '=' ? 1 : *ip == '<' ? 2 : 4; ip++; }
    b = expr();
    return ((op & 1) && a == b) || ((op & 2) && a < b) || ((op & 4) && a > b);
}

static int find(num no)
{
    for (int i = 0; i < nlines; i++)
        if ((num)prog[i].no == no) return i;
    err = 4;
    return nlines;
}

static void list(void)
{
    for (int i = 0; i < nlines; i++) {
        put_num(prog[i].no);
        out_char(' ');
        puts_(prog[i].s);
        newline();
    }
}

static void statement(void);

static void run(void)
{
    nfor = ngosub = 0;
    running = 1;
    cur = 0;
    while (cur < nlines && running && !err) {
        next = cur + 1;
        ip = prog[cur].s;
        statement();
        cur = next;
    }
    if (err) {
        puts_("ERR "); put_num(err);
        if (cur < nlines) { puts_(" AT "); put_num(prog[cur].no); }
        newline();
    }
    running = err = 0;
}

static void assign(void)
{
    skip();
    if (*ip == '@') {
        ip++; expect('('); int i = index_(expr()); expect(')'); expect('=');
        at[i] = expr();
    } else {
        int c = upper(*ip);
        if (c < 'A' || c > 'Z') { err = 1; return; }
        ip++;
        if (expect('=')) var[c - 'A'] = expr();
    }
}

static void statement(void)
{
    skip();
    if (!*ip) return;
    if (match("REM")) { while (*ip) ip++; return; }
    if (match("PRINT")) {
        for (;;) {
            skip();
            if (!*ip) { newline(); return; }
            if (*ip == '"') { ip++; while (*ip && *ip != '"') out_char(*ip++); if (*ip) ip++; }
            else { num v = expr(); if (err) return; put_num(v); }
            skip();
            if (*ip == ';' || *ip == ',') { ip++; skip(); if (!*ip) return; }
            else { newline(); return; }
        }
    }
    if (match("IF")) {
        if (!cond()) { while (*ip) ip++; return; }
        match("THEN");
        skip();
        if (digit(*ip)) next = find(expr());
        else statement();
        return;
    }
    if (match("GOTO")) { next = find(expr()); return; }
    if (match("GOSUB")) {
        if (ngosub == FORDEPTH) { err = 5; return; }
        gosubs[ngosub++] = next;
        next = find(expr());
        return;
    }
    if (match("RETURN")) {
        if (ngosub == 0) { err = 5; return; }
        next = gosubs[--ngosub];
        return;
    }
    if (match("FOR")) {
        skip();
        int c = upper(*ip);
        if (c < 'A' || c > 'Z' || nfor == FORDEPTH) { err = 1; return; }
        assign();
        if (!match("TO")) { err = 1; return; }
        fors[nfor].v = c - 'A';
        fors[nfor].limit = expr();
        fors[nfor].step = match("STEP") ? expr() : 1;
        fors[nfor].line = next;
        nfor++;
        return;
    }
    if (match("NEXT")) {
        skip();
        int v = upper(*ip) - 'A';
        while (nfor && fors[nfor - 1].v != v) nfor--;               /* leave loops that were jumped out of */
        if (nfor == 0) { err = 6; return; }
        num s = fors[nfor - 1].step;
        var[v] += s;
        if (s >= 0 ? var[v] <= fors[nfor - 1].limit : var[v] >= fors[nfor - 1].limit) next = fors[nfor - 1].line;
        else nfor--;
        return;
    }
    if (match("END") || match("STOP")) { running = 0; return; }
    if (match("RUN")) { if (!running) run(); return; }
    if (match("LIST")) { list(); return; }
    if (match("NEW")) { nlines = 0; return; }
    if (match("BYE")) { puts_("BYE"); newline(); quit(); return; }
    match("LET");
    assign();
}

/* keeps the program sorted by line number; an empty text deletes the line */
static void store(num no, const char *text)
{
    int i = 0, k;
    while (i < nlines && (num)prog[i].no < no) i++;
    if (i < nlines && (num)prog[i].no == no) {
        if (*text == 0) {
            for (; i + 1 < nlines; i++) {
                prog[i].no = prog[i + 1].no;
                for (k = 0; k < LINELEN; k++) prog[i].s[k] = prog[i + 1].s[k];
            }
            nlines--;
            return;
        }
    } else {
        if (*text == 0) return;
        if (nlines == MAXLINES) { puts_("FULL"); newline(); return; }
        for (int j = nlines; j > i; j--) {
            prog[j].no = prog[j - 1].no;
            for (k = 0; k < LINELEN; k++) prog[j].s[k] = prog[j - 1].s[k];
        }
        nlines++;
        prog[i].no = (uint16_t)no;
    }
    for (k = 0; text[k] && k < LINELEN - 1; k++) prog[i].s[k] = text[k];
    prog[i].s[k] = 0;
}

static void read_line(char *buf)
{
    int n = 0, idle = 0;
    for (;;) {
        int c = in_char();
        if (c == 0) { if (++idle > IDLE_LIMIT) quit(); continue; }
        idle = 0;
        if (c == '\r' || c == '\n') break;
        if ((c == 8 || c == 127) && n) n--;
        else if (c >= ' ' && c < 127 && n < LINELEN - 1) buf[n++] = (char)c;
    }
    buf[n] = 0;
}

int main(void)
{
    char buf[LINELEN];
    puts_("TINY BASIC"); newline();
    for (;;) {
        puts_("> ");
        read_line(buf);
        newline();
        ip = buf;
        skip();
        if (digit(*ip)) {
            num no = 0;
            while (digit(*ip)) no = times10(no) + (*ip++ - '0');
            skip();
            if (no > 0 && no < 65536) store(no, ip);
        } else {
            err = 0;
            statement();
            if (err) { puts_("ERR "); put_num(err); newline(); err = 0; }
        }
    }
}
