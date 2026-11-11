/* Runs one 68000 instruction at a time through golden/mcl68k.c (Ted Fried's interpreter) from a given register
 * state, in batch. stdin: one case per line, hex: opcode d0..d7 a0..a6 ccr; stdout: the final d0..d7 a0..a6 ccr
 * of the same shape. The state is loaded by a preamble of MOVEs (the interpreter's run loop never returns).
 * Extended: after the opcode, `n` extension words; after the registers, a memory area (AREA, AREA_LEN bytes, hex) that is
 * loaded first and printed last. Line: op n ext... d0..d7 a0..a7 ccr areahex ; output: op d0..d7 a0..a7 ccr areahex. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
extern uint32_t m68k_data_reg[8], m68k_address_reg[8], m68k_a7_S, mc68k_pc, memory_base;
extern uint16_t mc68k_flags;
extern void (*WriteWord)(uint32_t, uint16_t); extern uint16_t (*ReadWord)(uint32_t);
extern void (*WriteByte)(uint32_t, uint8_t); extern uint8_t (*ReadByte)(uint32_t);
extern int16_t run_mcl68k(void *);
uint16_t GetIPLx(void){return 0;} int UART0Ready(void){return 0;} uint8_t UART0Read(void){return 0;} void UART0Write(uint8_t c){(void)c;}
#define MS 0x100000
static uint8_t mem[MS]; static jmp_buf jb; static uint32_t halt;
static uint8_t rb(uint32_t a){return mem[a%MS];} static void wb(uint32_t a,uint8_t d){mem[a%MS]=d;}
static uint16_t rw(uint32_t a){ if(a==halt) longjmp(jb,1); a%=MS; return mem[a]<<8|mem[a+1];}
static void ww(uint32_t a,uint16_t d){ a%=MS; mem[a]=d>>8; mem[a+1]=d;}
static uint32_t pc;
static void w16(uint16_t v){ mem[pc]=v>>8; mem[pc+1]=v; pc+=2; }
static void w32(uint32_t v){ w16(v>>16); w16(v); }
#define AREA 0x4000
#define AREA_LEN 0x1000
int main(void){
  unsigned op, next, ext[4]; uint32_t d[8], a[8], ccr; static char hex[2*AREA_LEN+8], chex[2*0x2000+8]; int have_code;
  while (scanf("%x", &op) == 1) {
    scanf("%x", &next); for (unsigned i=0;i<next;i++) scanf("%x",&ext[i]);
    for (int i=0;i<8;i++) scanf("%x",&d[i]);
    for (int i=0;i<8;i++) scanf("%x",&a[i]);
    scanf("%x",&ccr); scanf("%s", hex);
    have_code = 0; { int ch = getchar(); if (ch == ' ') { ch = getchar(); if (ch == '@') { scanf("%s", chex); have_code = 1; } else ungetc(ch, stdin); } else ungetc(ch, stdin); }       /* optional ` @ <hex>`: 0x2000 bytes of guest code at 0x2000 */
    memset(mem,0,MS); m68k_address_reg[7] = 0;                                      /* the user stack pointer starts at 0 in every case */
    mem[0]=0;mem[1]=8;mem[2]=0;mem[3]=0; mem[4]=0;mem[5]=0;mem[6]=0x10;mem[7]=0;   /* SSP 0x80000, PC 0x1000 */
    pc = 0x1000;
    for (int i=0;i<8;i++){ w16(0x203C | i<<9); w32(d[i]); }                       /* MOVE.L #d,Dn */
    for (int i=0;i<8;i++){ w16(0x207C | i<<9); w32(a[i]); }                       /* MOVEA.L #a,An (A7: the supervisor stack) */
    w16(0x44FC); w16(ccr & 0x1F);                                                   /* MOVE #ccr,CCR */
    w16(op);
    for (unsigned i=0;i<next;i++) w16(ext[i]);
    halt = pc;
    for (int i=0;i<AREA_LEN;i++){ unsigned v; sscanf(hex+2*i,"%2x",&v); mem[AREA+i]=v; }
    if (have_code) for (int i=0;i<0x2000;i++){ unsigned v; sscanf(chex+2*i,"%2x",&v); mem[0x2000+i]=v; }
    memory_base=0; ReadByte=rb;WriteByte=wb;ReadWord=rw;WriteWord=ww;
    if (setjmp(jb)==0) run_mcl68k(0);
    printf("%08x", op);
    for (int i=0;i<8;i++) printf(" %08x", m68k_data_reg[i]);
    for (int i=0;i<7;i++) printf(" %08x", m68k_address_reg[i]);
    printf(" %08x", (mc68k_flags & 0x2000) ? m68k_a7_S : m68k_address_reg[7]);      /* the live A7 */
    printf(" %04x %08x ", mc68k_flags & 0xA71F, m68k_address_reg[7]);                 /* the SR (T S IPL CCR) and the user stack pointer */
    for (int i=0;i<AREA_LEN;i++) printf("%02x", mem[AREA+i]);
    printf("\n");
  }
  return 0;
}
