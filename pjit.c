// Type your code here, or load an example.
#include "pjit.h"

// ARM: N Z C V .... (32-bit)
// 68K: .... X N Z V C (16-bit)

uint8_t sr_ccr[16] = {
    0x00 | 0x00 | 0x00 | 0x00, 
    0x00 | 0x00 | 0x00 | 0x02, 
    0x00 | 0x00 | 0x11 | 0x00, 
    0x00 | 0x00 | 0x11 | 0x02, 
    0x00 | 0x40 | 0x00 | 0x00, 
    0x00 | 0x40 | 0x00 | 0x02, 
    0x00 | 0x40 | 0x11 | 0x00, 
    0x00 | 0x40 | 0x11 | 0x02, 
    0x08 | 0x00 | 0x00 | 0x00, 
    0x08 | 0x00 | 0x00 | 0x02, 
    0x08 | 0x00 | 0x11 | 0x00, 
    0x08 | 0x00 | 0x11 | 0x02, 
    0x08 | 0x40 | 0x00 | 0x00, 
    0x08 | 0x40 | 0x00 | 0x02, 
    0x08 | 0x40 | 0x11 | 0x00, 
    0x08 | 0x40 | 0x11 | 0x02
};

extern uint32_t fast_opcodes[65536];
extern uint32_t flag_opcodes[65536];
#ifdef FAST_BRANCHING
extern uint32_t fast_branch[4096];
#endif

static uint32_t cache[(1<<INDEX_BITS)][(1<<OFFSET_BITS) + CACHE_PADDING];

#if INDEX_BITS > 16
#error INDEX_BITS is too big
#endif
#if OFFSET_BITS <= 8
static uint8_t tags[1<<INDEX_BITS];
#elif OFFSET_BITS <= 16
static uint16_t tags[1<<INDEX_BITS];
#else
#error OFFSET_BITS is too big
#endif

static int m68_is_cond(uint16_t op) {
    // BCC
    if(op >= 0x6200 && op < 0x7000) return 1;
    // DBCC/SCC
    if(op >= 0x50C0 && op < 0x6000) return 1;
    // Not Conditional
    return 0;
}

static void m68_decode(void) {
    pc.offset = ip.offset;
    uint16_t op = __m68k_read_memory_16(pc.op);
    uint32_t fn;

    if(m68_is_cond(op)) {

    }

#ifdef FAST_BRANCHING
    if((op & 0x6000) != 0x6000) fn = fast_opcodes[op];
    else {
        int32_t diff = (int32_t)(uint16_t)ip.offset + (int8_t)(op & 0xFF);
        if(diff < 0 || diff > ((1<<OFFSET_BITS)-1)) fn = fast_opcodes[op];
        else fn = fast_branch[op & 0xFFF];
    }
#else
    fn = fast_opcodes[op];
#endif
    cache[ip.index][ip.offset] = fn;
    goto *(void*)fn;
}

__attribute__((optimize(3),leaf)) int32_t m68_addrext(void) {
    uint16_t ext = *pc_wp++;
    int32_t x = (int8_t)ext;
    switch(ext >> 11) {
        case 0: x += d0_w[0]; break; 
        case 1: x += d0_l[0]; break; 
        case 2: x += d1_w[0]; break; 
        case 3: x += d1_l[0]; break; 
        case 4: x += d2_w[0]; break; 
        case 5: x += d2_l[0]; break; 
        case 6: x += d3_w[0]; break; 
        case 7: x += d3_l[0]; break; 
        case 8: x += d4_w[0]; break; 
        case 9: x += d4_l[0]; break; 
        case 10: x += d5_w[0]; break; 
        case 11: x += d5_l[0]; break; 
        case 12: x += d6_w[0]; break; 
        case 13: x += d6_l[0]; break; 
        case 14: x += d7_w[0]; break; 
        case 15: x += d7_l[0]; break; 
        case 16: x += a0_w; break; 
        case 17: x += a0_l; break; 
        case 18: x += a1_w; break; 
        case 19: x += a1_l; break; 
        case 20: x += a2_w; break; 
        case 21: x += a2_l; break; 
        case 22: x += a3_w; break; 
        case 23: x += a3_l; break; 
        case 24: x += a4_w; break; 
        case 25: x += a4_l; break; 
        case 26: x += a5_w; break; 
        case 27: x += a5_l; break; 
        case 28: x += a6_w; break; 
        case 29: x += a6_l; break; 
        case 30: x += a7_w; break; 
        case 31: x += a7_l; break; 
    }
    return x;
}

void m68_enter(uint32_t addr) {
    pc.u = addr;
    if (tags[pc.index] != pc.tag) {
        int i;
        tags[pc.index] = pc.tag;
        for(i=0; i<(1<<OFFSET_BITS); i++) {
            cache[pc.index][i] = (uint32_t)m68_decode;
        }
    }
    ip.ux = &cache[pc.index][pc.offset];
}

void m68_start(uint32_t m68_address) {
    for(int i=0; i<(1<<INDEX_BITS);i++) {
        for(int j=0; j<CACHE_PADDING; j++) {
            cache[i][j + (1<<OFFSET_BITS)] = (uint32_t)m68_enter;
        }
    }
	pc.u = *(uint32_t*)m68_address;
	m68_enter();
    m68_decode();
}