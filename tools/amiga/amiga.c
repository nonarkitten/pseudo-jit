#include "amiga.h"

#define INTEN 0x4000u
static const uint8_t lvl_of_bit[14] = { 1, 1, 1, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 6 };

static uint64_t cycles_of(uint64_t icount) { return icount * AM_CPI; }
static void hash_in(am_t *m, uint32_t addr, int size, uint32_t val, int wr, uint64_t ic)
{
    uint64_t h = m->hash;
    h = (h ^ ((uint64_t)addr | ((uint64_t)size << 28) | ((uint64_t)wr << 32))) * 1099511628211ull;
    h = (h ^ val) * 1099511628211ull;
    h = (h ^ ic) * 1099511628211ull;
    m->hash = h; m->n_access++;
}

/* ---- CIAs ---- */
static void cia_reset(am_cia_t *c, uint64_t tod_period)
{
    *c = (am_cia_t){ .ta = 0xFFFF, .tb = 0xFFFF, .ta_latch = 0xFFFF, .tb_latch = 0xFFFF, .tod_period = tod_period };
}
static void cia_flag(am_t *m, am_cia_t *c, int bit, int is_b)
{
    c->icr_data |= bit;
    if (c->icr_mask & bit) { c->icr_data |= 0x80; m->intreq |= is_b ? 0x2000u : 0x0008u; }
}
/* advance the timers to E tick `now`; timer B can count timer A's underflows */
static void cia_sync(am_t *m, am_cia_t *c, uint64_t now, int is_b)
{
    while (c->t_sync < now) {
        uint64_t n = now - c->t_sync, ua = 0;
        int a_run = c->cra & 1, b_run = c->crb & 1;
        /* step to the nearest underflow of a running timer, at most n ticks */
        uint64_t step = n;
        if (a_run && (uint64_t)c->ta + 1 < step) step = (uint64_t)c->ta + 1;
        if (b_run && !(c->crb & 0x60) && (uint64_t)c->tb + 1 < step) step = (uint64_t)c->tb + 1;
        if (step == 0) step = 1;
        c->t_sync += step;
        if (a_run) {
            if (step > c->ta) {                               /* underflow */
                ua = 1; cia_flag(m, c, 1, is_b);
                if (c->cra & 8) { c->cra &= ~1; c->ta = c->ta_latch; } else c->ta = c->ta_latch;
            } else c->ta -= (uint16_t)step;
        }
        if (b_run) {
            if ((c->crb & 0x60) == 0) {
                if (step > c->tb) { cia_flag(m, c, 2, is_b); if (c->crb & 8) { c->crb &= ~1; } c->tb = c->tb_latch; } else c->tb -= (uint16_t)step;
            } else if ((c->crb & 0x60) == 0x40 && ua) {
                if (c->tb == 0) { cia_flag(m, c, 2, is_b); if (c->crb & 8) c->crb &= ~1; c->tb = c->tb_latch; } else c->tb--;
            }
        }
    }
}
static uint32_t tod_now(am_cia_t *c, uint64_t cyc) { return c->tod_running ? (c->tod + (uint32_t)((cyc - c->tod_t0) / c->tod_period)) & 0xFFFFFF : c->tod; }

static uint8_t cia_read(am_t *m, am_cia_t *c, int reg, int is_b, uint64_t ic)
{
    uint64_t cyc = cycles_of(ic);
    cia_sync(m, c, cyc / 10, is_b);
    switch (reg) {
    case 0: return is_b ? (uint8_t)((c->pra & c->ddra) | (0xFC & ~c->ddra) | (c->ddra & 3 ? 0 : 3)) : (uint8_t)((c->pra & c->ddra) | (0xFF & ~c->ddra));     /* inputs read high (no drive, no mouse button, no joystick) */
    case 1: return (uint8_t)((c->prb & c->ddrb) | (0xFF & ~c->ddrb));
    case 2: return c->ddra;
    case 3: return c->ddrb;
    case 4: return (uint8_t)c->ta;
    case 5: return (uint8_t)(c->ta >> 8);
    case 6: return (uint8_t)c->tb;
    case 7: return (uint8_t)(c->tb >> 8);
    case 8: { uint32_t v = c->tod_latched ? c->tod_latch : tod_now(c, cyc); if (c->tod_latched) {} return (uint8_t)v; }
    case 9: { uint32_t v = c->tod_latched ? c->tod_latch : tod_now(c, cyc); return (uint8_t)(v >> 8); }
    case 10: { uint32_t v = c->tod_latched ? c->tod_latch : tod_now(c, cyc); if (!c->tod_latched) { c->tod_latch = v; c->tod_latched = 1; } return (uint8_t)(v >> 16); }
    case 12: return c->sdr;
    case 13: { uint8_t v = c->icr_data; c->icr_data = 0; return v; }
    case 14: return c->cra & 0xEF;
    case 15: return c->crb;
    }
    return 0xFF;
}
static void cia_write(am_t *m, am_cia_t *c, int reg, uint8_t v, int is_b, uint64_t ic)
{
    uint64_t cyc = cycles_of(ic);
    cia_sync(m, c, cyc / 10, is_b);
    switch (reg) {
    case 0: c->pra = v; break;
    case 1: c->prb = v; break;
    case 2: c->ddra = v; break;
    case 3: c->ddrb = v; break;
    case 4: c->ta_latch = (c->ta_latch & 0xFF00) | v; break;
    case 5: c->ta_latch = (c->ta_latch & 0x00FF) | (uint16_t)(v << 8); if (!(c->cra & 1)) c->ta = c->ta_latch; break;
    case 6: c->tb_latch = (c->tb_latch & 0xFF00) | v; break;
    case 7: c->tb_latch = (c->tb_latch & 0x00FF) | (uint16_t)(v << 8); if (!(c->crb & 1)) c->tb = c->tb_latch; break;
    case 8: if (c->crb & 0x80) c->tod_alarm = (c->tod_alarm & 0xFFFF00) | v; else { c->tod = (c->tod & 0xFFFF00) | v; c->tod_running = 1; c->tod_t0 = cyc; } break;
    case 9: if (c->crb & 0x80) c->tod_alarm = (c->tod_alarm & 0xFF00FF) | ((uint32_t)v << 8); else c->tod = (c->tod & 0xFF00FF) | ((uint32_t)v << 8); break;
    case 10: if (c->crb & 0x80) c->tod_alarm = (c->tod_alarm & 0x00FFFF) | ((uint32_t)v << 16); else { c->tod = (c->tod & 0x00FFFF) | ((uint32_t)v << 16); c->tod_running = 0; } break;
    case 12: c->sdr = v; break;
    case 13: if (v & 0x80) c->icr_mask |= v & 0x1F; else c->icr_mask &= ~(v & 0x1F);
             if (c->icr_data & c->icr_mask) { c->icr_data |= 0x80; m->intreq |= is_b ? 0x2000u : 0x0008u; }
             break;
    case 14: if (v & 0x10) c->ta = c->ta_latch; c->cra = v & 0xEF; break;
    case 15: if (v & 0x10) c->tb = c->tb_latch; c->crb = v & 0xEF; break;
    }
    if (c == &m->a || c == &m->b) { (void)is_b; }
}
static void cia_tod_tick_period(am_cia_t *c, uint64_t p) { c->tod_period = p; }

/* ---- chip identification and the custom registers ---- */
static uint16_t custom_read(am_t *m, uint32_t off, uint64_t ic)
{
    uint64_t cyc = cycles_of(ic), fr = cyc % AM_FRAME_CYC;
    uint32_t line = (uint32_t)(fr / AM_LINE_CYC), cck = (uint32_t)((fr % AM_LINE_CYC) / 2);
    switch (off & 0x1FE) {
    case 0x002: return (uint16_t)(m->dmacon & 0x03FF);                    /* DMACONR: the blitter is never busy */
    case 0x004: return (uint16_t)(0xA000u | ((line >> 8) & 1));           /* VPOSR: LOF, ECS Agnus (id $20) */
    case 0x006: return (uint16_t)(((line & 0xFF) << 8) | (cck & 0xFF));
    case 0x00A: case 0x00C: return 0;                                     /* JOY0DAT / JOY1DAT */
    case 0x010: return 0;                                                 /* ADKCONR */
    case 0x012: case 0x014: return 0;                                     /* POT0DAT / POT1DAT */
    case 0x016: return 0xFF00;                                            /* POTGOR */
    case 0x018: return 0x3000;                                            /* SERDATR: TBE | TSRE */
    case 0x01A: return 0;                                                 /* DSKBYTR */
    case 0x01C: return (uint16_t)m->intena;
    case 0x01E: return (uint16_t)m->intreq;
    case 0x07C: return 0xFFFC;                                            /* DENISEID: ECS Denise */
    default: return 0xFFFF;
    }
}
static void custom_write(am_t *m, uint32_t off, uint16_t v)
{
    switch (off & 0x1FE) {
    case 0x096: if (v & 0x8000) m->dmacon |= v & 0x7FFF; else m->dmacon &= ~v; break;
    case 0x09A: if (v & 0x8000) m->intena |= v & 0x7FFF; else m->intena &= ~v; break;
    case 0x09C: if (v & 0x8000) m->intreq |= v & 0x7FFF; else m->intreq &= ~v; break;
    case 0x09E: if (v & 0x8000) m->adkcon |= v & 0x7FFF; else m->adkcon &= ~v; break;
    case 0x058: m->intreq |= 0x0040; break;                                /* BLTSIZE: the blit is done at once: BLIT interrupt */
    case 0x054: break;
    default: break;
    }
}

void am_init(am_t *m, uint8_t *rom)
{
    *m = (am_t){ .rom = rom, .hash = 14695981039346656037ull, .next_vblank = AM_FRAME_CYC };
    cia_reset(&m->a, AM_FRAME_CYC); cia_reset(&m->b, AM_LINE_CYC);
}
int am_overlay(const am_t *m) { return !(m->a.ddra & 1) || (m->a.pra & 1); }
int am_is_hw(uint32_t a)
{
    a &= 0xFFFFFF;
    return a >= 0x200000 && a < 0xF80000;
}

uint32_t am_read(am_t *m, uint32_t addr, int size, uint64_t ic)
{
    addr &= 0xFFFFFF; uint32_t v = 0xFFFFFFFFu;
    if (addr >= 0xBFD000 && addr < 0xBFF000) {                              /* the CIAs: byte lanes */
        uint32_t lane_b = (addr & 0xF000) == 0xD000, reg = (addr >> 8) & 15;
        if (size == 1) v = ((addr & 1) == (lane_b ? 0u : 1u)) ? (lane_b ? cia_read(m, &m->b, reg, 1, ic) : cia_read(m, &m->a, reg, 0, ic)) : 0xFF;
        else {
            uint8_t hi = lane_b ? cia_read(m, &m->b, reg, 1, ic) : 0xFF, lo = lane_b ? 0xFF : cia_read(m, &m->a, reg, 0, ic);
            v = (uint32_t)(hi << 8 | lo); if (size == 4) v = v << 16 | 0xFFFF;
        }
    } else if (addr >= 0xDFF000 && addr < 0xE00000) {
        uint16_t w = custom_read(m, addr, ic);
        v = size == 1 ? ((addr & 1) ? (w & 0xFF) : (w >> 8)) : size == 2 ? w : ((uint32_t)w << 16 | custom_read(m, addr + 2, ic));
    }
    m->n_access++;                                                         /* reads are not hashed: the 68000 reads before CLR / read-modify-write, PJIT does not */
    return size == 1 ? (v & 0xFF) : size == 2 ? (v & 0xFFFF) : v;
}
void am_write(am_t *m, uint32_t addr, int size, uint32_t val, uint64_t ic)
{
    addr &= 0xFFFFFF;
    hash_in(m, addr, size, val, 1, ic);
    if (addr >= 0xBFD000 && addr < 0xBFF000) {
        uint32_t lane_b = (addr & 0xF000) == 0xD000, reg = (addr >> 8) & 15;
        if (size == 1) { if ((addr & 1) == (lane_b ? 0u : 1u)) { if (lane_b) cia_write(m, &m->b, reg, (uint8_t)val, 1, ic); else cia_write(m, &m->a, reg, (uint8_t)val, 0, ic); } }
        else { uint8_t b = size == 2 ? (uint8_t)(lane_b ? val >> 8 : val) : (uint8_t)(lane_b ? val >> 24 : val >> 16); if (lane_b) cia_write(m, &m->b, reg, b, 1, ic); else cia_write(m, &m->a, reg, b, 0, ic); }
    } else if (addr >= 0xDFF000 && addr < 0xE00000) {
        if (size == 1) custom_write(m, addr, (uint16_t)(val << 8 | (val & 0xFF)));
        else if (size == 2) custom_write(m, addr, (uint16_t)val);
        else { custom_write(m, addr, (uint16_t)(val >> 16)); custom_write(m, addr + 2, (uint16_t)val); }
    }
}

uint64_t am_deadline(am_t *m)
{
    uint64_t d = m->next_vblank;
    for (int k = 0; k < 2; k++) {
        am_cia_t *c = k ? &m->b : &m->a;
        if (!(c->cra & 1) && !(c->crb & 1)) continue;
        uint64_t best = ~0ull;
        if (c->cra & 1) best = c->t_sync + c->ta + 1;
        if ((c->crb & 1) && !(c->crb & 0x60) && c->t_sync + c->tb + 1 < best) best = c->t_sync + c->tb + 1;
        if ((c->crb & 1) && (c->crb & 0x60) == 0x40 && (c->cra & 1)) { uint64_t t = c->t_sync + ((uint64_t)c->tb + 1) * ((uint64_t)c->ta_latch + 1); if (t < best) best = t; }
        if (best != ~0ull && best * 10 < d) d = best * 10;
    }
    return (d + AM_CPI - 1) / AM_CPI;                                      /* first instruction count at or after it */
}
void am_events(am_t *m, uint64_t icount)
{
    uint64_t cyc = cycles_of(icount);
    m->icount = icount;
    while (m->next_vblank <= cyc) {
        m->intreq |= 0x0020;                                               /* VERTB */
        if (m->a.tod_running) { /* CIAA TOD counts vertical blanks (handled lazily by tod_now) */ }
        m->next_vblank += AM_FRAME_CYC;
    }
    cia_sync(m, &m->a, cyc / 10, 0);
    cia_sync(m, &m->b, cyc / 10, 1);
}
int am_ipl(am_t *m)
{
    uint16_t x = m->intreq & m->intena & 0x3FFF;
    if (!(m->intena & INTEN) || !x) return 0;
    int lvl = 0;
    for (int b = 0; b < 14; b++) if ((x >> b) & 1 && lvl_of_bit[b] > lvl) lvl = lvl_of_bit[b];
    return lvl;
}
