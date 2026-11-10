/* Runs the BASIC interpreter image (test_progs/tinybasic.c, built into tests/data/tinybasic_image.h) on golden/mcl68k.c (Ted Fried's interpreter), as an
 * independent check of PJIT's run of the same image (tests/qemu/smoke_tinybasic.c). The image's console is three routines at fixed addresses; here they are
 * 68000 code that reads and writes a memory-mapped port (PJIT's harness uses TRAP #n instead):
 *   putc   0x100   move.b d0,$FFF00 ; rts            getc   0x108   move.b $FFF01,d0 ; rts  (0 when no key is waiting)
 *   exit   0x110   the run ends when the core fetches from here
 *   tb_oracle 'PROGRAM TEXT\r...BYE\r'        -> the console output on stdout, then the instruction count on stderr
 *   tb_oracle -f program.bas                   the keystrokes from a file (newlines become CR; BYE is typed at the end)      (make tb-oracle) */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "tinybasic_image.h"
extern uint32_t m68k_data_reg[8], m68k_address_reg[8], memory_base;
extern void (*WriteWord)(uint32_t, uint16_t); extern uint16_t (*ReadWord)(uint32_t);
extern void (*WriteByte)(uint32_t, uint8_t); extern uint8_t (*ReadByte)(uint32_t);
extern int16_t run_mcl68k(void *);
static const char *keys;
uint16_t GetIPLx(void){return 0;} int UART0Ready(void){return 0;} uint8_t UART0Read(void){return 0;} void UART0Write(uint8_t c){ (void)c; }
#define MS 0x100000
#define IO_OUT 0xFFF00u
#define IO_IN  0xFFF01u
static uint8_t mem[MS]; static jmp_buf jb; static unsigned long fetches;
static uint8_t rb(uint32_t a){ a &= MS - 1; if (a == IO_IN) return *keys ? (uint8_t)*keys++ : 0; return mem[a]; }
static void wb(uint32_t a, uint8_t d){ a &= MS - 1; if (a == IO_OUT) { if (d != '\r') putchar(d); return; } mem[a] = d; }
static uint16_t rw(uint32_t a){ a &= MS - 1; fetches++; if (a == TINYBASIC_BIOS_EXIT) longjmp(jb, 1); return mem[a] << 8 | mem[a + 1]; }
static void ww(uint32_t a, uint16_t d){ a &= MS - 1; mem[a] = d >> 8; mem[a + 1] = d; }
static void put16(uint32_t a, uint16_t v){ mem[a] = v >> 8; mem[a + 1] = (uint8_t)v; }
static void put32(uint32_t a, uint32_t v){ put16(a, v >> 16); put16(a + 2, (uint16_t)v); }
int main(int argc, char **argv){
  setvbuf(stdout, 0, _IONBF, 0);
  keys = "10 FOR I=1 TO 10\r20 LET S=S+I\r30 NEXT I\r40 PRINT S\rRUN\rBYE\r";
  if (argc > 2 && !strcmp(argv[1], "-f")) {
    static char text[65536]; FILE *f = fopen(argv[2], "rb"); size_t n;
    if (!f) { perror(argv[2]); return 1; }
    n = fread(text, 1, sizeof text - 8, f); fclose(f);
    for (size_t i = 0; i < n; i++) if (text[i] == '\n') text[i] = '\r';
    strcpy(text + n, "\rBYE\r"); keys = text;
  } else if (argc > 1) keys = argv[1];
  memcpy(mem + TINYBASIC_LOAD, tinybasic_image, tinybasic_image_size);
  put16(TINYBASIC_BIOS_PUTC, 0x13C0); put32(TINYBASIC_BIOS_PUTC + 2, IO_OUT); put16(TINYBASIC_BIOS_PUTC + 6, 0x4E75);      /* move.b d0,IO_OUT ; rts */
  put16(TINYBASIC_BIOS_GETC, 0x1039); put32(TINYBASIC_BIOS_GETC + 2, IO_IN);  put16(TINYBASIC_BIOS_GETC + 6, 0x4E75);      /* move.b IO_IN,d0 ; rts  */
  put16(TINYBASIC_BIOS_EXIT, 0x4E71);                                                                                    /* the run ends before this executes */
  put32(0, 0xE0000); put32(4, TINYBASIC_LOAD);                                                                           /* reset vectors: SSP, PC */
  memory_base = 0; ReadByte = rb; WriteByte = wb; ReadWord = rw; WriteWord = ww;
  if (setjmp(jb) == 0) run_mcl68k(0);
  fprintf(stderr, "\n[%lu word fetches]\n", fetches);
  return 0;
}
