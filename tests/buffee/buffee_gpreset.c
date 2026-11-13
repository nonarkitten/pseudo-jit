/// @file buffee_gpreset.c
/// @brief Can the board power-cycle its GreenPAK? On the v0.8 the GreenPAK's VDD and VDD2 come from ball M1 (GPMC_ADVN_ALE, as a GPIO GPIO2_2); on the v0.6 nothing is connected to
///        M1. The registers (our design) are compared with the NVM (what a power-on loads); then, for each pulse length, our design is put back, M1 is driven low for the pulse (the
///        GreenPAK is probed on I2C just after M1 falls and just before it rises), then high, and the registers are read again (a pulse of 0 ms leaves M1 alone: the control). Registers back at the NVM's contents = the GreenPAK
///        was reset (a v0.8); unchanged = it was not (a v0.6, or a chip held up through its input pins' protection diodes).
///        One line per pulse: `GPRESET pulse <ms> ms: I2C after the fall <r>, before the rise <r>, first answer <us> us after the rise; <b> of <d> differing registers back at the NVM,
///        <k> kept` (r: ack, nack, busy = SDA or SCL held low, timeout). The NVM is only read, never written. M1 is left a GPIO driven high.
///        Ends with our design written back and BUFFEE_REROLL_EXIT(2): the bootloader loads the design again and recalibrates; the restarted payload prints `GPRESET done`.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"

#define R32(a)          (*(volatile uint32_t *)(a))
#define I2C(off)        R32(0x44E0B000u + (off))
#define I2C_AL_NACK     3u
#define I2C_ARDY        (1u << 2)
#define I2C_RRDY        (1u << 3)
#define I2C_XRDY        (1u << 4)
#define I2C_BF          (1u << 8)
#define I2C_BB          (1u << 12)
#define GPIO2           0x481AC000u
#define G_OE            0x134
#define G_DATAIN        0x138
#define G_CLEARDATAOUT  0x190
#define G_SETDATAOUT    0x194
#define CONF_ADVN_ALE   0x44E10890u     /* ball M1's pad control */
#define PAD_GPIO_OUT    0x2Fu           /* mode 7 (GPIO2_2), pull disabled, input buffer on (DATAIN reads the pad) */
#define M1_BIT          (1u << 2)
#define TIMEOUT         2000000u        /* cycles (2 ms): an I2C transfer that takes longer is abandoned */
#define REGS            0xC8            /* the design: 0x00-0x73 and 0x80-0xC7 (0x74-0x7F are the live matrix inputs, read only) */
#define REG_OE          0x44            /* bits 7:2: the pin 20 (ASn) output enable, written last */

static const char *const rc_name[] = { "ack", "nack", "busy", "timeout" };
static uint8_t r0[REGS], nvm[REGS], r1[REGS], ours[REGS];

static uint32_t cyc(void) { return bbb_pmu_cycles(); }
static void wait_us(uint32_t us) { uint32_t t = cyc(); while (cyc() - t < us * 1000u) ; }
static int design_reg(int a) { return a < 0x74 || a >= 0x80; }

/// I2C0 back to idle after an abandoned transfer: the module off and on again keeps its clock setup.
static void i2c_recover(void)
{
    I2C(0xA4) = 0;
    wait_us(10);
    I2C(0xA4) = 0x8000;
    I2C(0x94) |= (1u << 14) | (1u << 6);
    I2C(0x28) = 0x7FFF;
    I2C(0x98) = 0;
}

/// One transfer to I2C address `sa`: the register number, then `n` bytes written or read. 0 ack, 1 nack, 2 busy (SDA or SCL held low), 3 timeout.
static int xfer(uint8_t sa, uint8_t reg, uint8_t *data, int n, int rd)
{
    uint32_t t = cyc(), s;
    int out = rd ? 1 : 1 + n, sent = 0, k = 0, rc = 0;

    I2C(0xAC) = sa; I2C(0x98) = out; I2C(0x28) = 0x7FFF; I2C(0xA4) = 0x8600;                 /* enabled, master, transmit */
    while (I2C(0x24) & I2C_BB) if (cyc() - t > TIMEOUT) { i2c_recover(); return 2; }
    I2C(0xA4) |= 1;                                                                         /* START */
    while (sent < out) {
        s = I2C(0x24);
        if (s & I2C_AL_NACK) { rc = 1; goto stop; }
        if (cyc() - t > TIMEOUT) { rc = 3; goto stop; }
        if (s & I2C_XRDY) { I2C(0x9C) = sent ? data[sent - 1] : reg; sent++; I2C(0x28) = I2C_XRDY; }
    }
    while (!((s = I2C(0x24)) & (I2C_ARDY | I2C_AL_NACK))) if (cyc() - t > TIMEOUT) { rc = 3; goto stop; }
    if (s & I2C_AL_NACK) { rc = 1; goto stop; }
    I2C(0x28) = I2C_ARDY;
    if (rd) {
        I2C(0x98) = n; I2C(0xA4) = (I2C(0xA4) | 3) & ~(1u << 9);                           /* repeated START, STOP after n, receive */
        while (k < n) {
            s = I2C(0x24);
            if (s & I2C_AL_NACK) { rc = 1; goto stop; }
            if (cyc() - t > TIMEOUT) { rc = 3; goto stop; }
            if (s & I2C_RRDY) { data[k++] = (uint8_t)I2C(0x9C); I2C(0x28) = I2C_RRDY; }
        }
    } else I2C(0xA4) |= 2;                                                                  /* STOP */
    while (!(I2C(0x24) & I2C_BF)) if (cyc() - t > TIMEOUT) { i2c_recover(); return 3; }
    I2C(0x94) |= (1u << 14) | (1u << 6); I2C(0x28) = 0x7FFF; I2C(0x98) = 0;
    return 0;
stop:
    I2C(0xA4) |= 2;
    t = cyc();
    while (!(I2C(0x24) & I2C_BF) && cyc() - t < TIMEOUT) ;
    i2c_recover();
    return rc;
}

static int read_block(uint8_t sa, uint8_t *b)
{
    int off, rc;

    for (off = 0; off < REGS; off += 8) if ((rc = xfer(sa, (uint8_t)off, b + off, 8, 1)) != 0) return rc;
    return 0;
}

/// Our design back into the registers (as the bootloader writes it: 16-byte pages, the ASn output enable held off until the end). Returns the number of refused pages.
static int write_design(uint8_t sa, const uint8_t *img)
{
    uint8_t buf[16];
    int off, n, k, refused = 0;

    for (off = 0; off < REGS; off += 16) {
        n = (off == 0x70) ? 4 : (off == 0xC0) ? 8 : 16;
        for (k = 0; k < n; k++) buf[k] = img[off + k];
        if (off == 0x40) buf[REG_OE - 0x40] &= 0x03;
        if (xfer(sa, (uint8_t)off, buf, n, 0)) refused++;
    }
    buf[0] = img[REG_OE];
    if (xfer(sa, REG_OE, buf, 1, 0)) refused++;
    return refused;
}

int main(void)
{
    static const uint32_t pulse_ms[] = { 0, 1, 10, 100, 1000 };                            /* 0: M1 left high, the control */
    uint8_t probe, sa_reg, sa_nvm;
    int cc, a, d, back, kept, other, rc, rc_fall, rc_rise, bad;
    uint32_t p, t, first_us;

    if (*BUFFEE_MAILBOX == 1) { *BUFFEE_MAILBOX = 0; printf("GPRESET done (the bootloader loaded the design again)\n"); return 0; }
    printf("=== GreenPAK power-cycle test: M1 pad %08X (the reset default), GPIO2 OE %08X ===\n", (unsigned)R32(CONF_ADVN_ALE), (unsigned)R32(GPIO2 + G_OE));
    for (cc = 0; cc < 8; cc++) if (xfer((uint8_t)((cc << 3) | 2), 0, &probe, 1, 1) == 0) break;
    if (cc == 8) { printf("GPRESET: no GreenPAK answers on I2C0\n"); return 0; }
    sa_reg = (uint8_t)(cc << 3); sa_nvm = (uint8_t)((cc << 3) | 2);
    if ((rc = read_block(sa_reg, r0)) != 0 || (rc = read_block(sa_nvm, nvm)) != 0) { printf("GPRESET: reading the GreenPAK at $%02X failed (%s)\n", sa_reg, rc_name[rc]); return 0; }
    for (a = 0, d = 0; a < REGS; a++) { ours[a] = r0[a]; if (design_reg(a) && r0[a] != nvm[a]) d++; }
    printf("GreenPAK at $%02X; %d of the design's registers differ from the NVM:", sa_reg, d);
    for (a = 0, bad = 0; a < REGS && bad < 12; a++) if (design_reg(a) && r0[a] != nvm[a]) { printf(" %02X:%02X/%02X", a, r0[a], nvm[a]); bad++; }
    printf(" (register: ours / NVM)\n");
    if (d == 0) {                                                                           /* the NVM holds this very design: the ASn output enable off is the marker */
        r0[REG_OE] &= 0x03;
        printf("GPRESET: the NVM holds this design; the marker is the ASn output enable off (register %02X = %02X, the NVM's %02X)\n", REG_OE, r0[REG_OE], nvm[REG_OE]);
        if (r0[REG_OE] == nvm[REG_OE]) { printf("GPRESET: no marker possible\n"); return 0; }
        d = 1;
    }
    R32(GPIO2 + G_SETDATAOUT) = M1_BIT;                                                    /* M1 (an input with the pad's pull-up) becomes a GPIO output driving high, then the pad is set: no dip */
    R32(GPIO2 + G_OE) &= ~M1_BIT;
    R32(CONF_ADVN_ALE) = PAD_GPIO_OUT;
    wait_us(1000);
    for (p = 0; p < sizeof pulse_ms / sizeof pulse_ms[0]; p++) {
        bad = write_design(sa_reg, r0);
        if (read_block(sa_reg, r1) != 0) bad++;
        else for (a = 0; a < REGS; a++) if (design_reg(a) && r1[a] != r0[a]) bad++;
        if (bad) { printf("GPRESET pulse %u ms: our design did not go back in (%d pages / registers wrong)\n", (unsigned)pulse_ms[p], bad); continue; }
        if (pulse_ms[p]) {
            R32(GPIO2 + G_CLEARDATAOUT) = M1_BIT;
            t = cyc();
            wait_us(50);
            rc_fall = xfer(sa_nvm, 0, &probe, 1, 1);
            while (cyc() - t < pulse_ms[p] * 1000000u - 3000000u * (pulse_ms[p] > 3)) ;
            rc_rise = pulse_ms[p] > 3 ? xfer(sa_nvm, 0, &probe, 1, 1) : rc_fall;
            while (cyc() - t < pulse_ms[p] * 1000000u) ;
            R32(GPIO2 + G_SETDATAOUT) = M1_BIT;
        } else rc_fall = rc_rise = xfer(sa_nvm, 0, &probe, 1, 1);
        t = cyc(); first_us = 0xFFFFFFFFu;
        while (cyc() - t < 50000000u) if (xfer(sa_nvm, 0, &probe, 1, 1) == 0) { first_us = (cyc() - t) / 1000u; break; }
        wait_us(5000);
        if ((rc = read_block(sa_reg, r1)) != 0) { printf("GPRESET pulse %u ms: the GreenPAK does not answer after the rise (%s)\n", (unsigned)pulse_ms[p], rc_name[rc]); continue; }
        for (a = 0, back = 0, kept = 0, other = 0; a < REGS; a++) {
            if (!design_reg(a) || r0[a] == nvm[a]) continue;
            if (r1[a] == nvm[a]) back++; else if (r1[a] == r0[a]) kept++; else other++;
        }
        printf("GPRESET pulse %u ms: I2C after the fall %s, before the rise %s, first answer %d us after the rise; %d of %d differing registers back at the NVM, %d kept, %d neither (pad reads %u)\n",
               (unsigned)pulse_ms[p], rc_name[rc_fall], rc_name[rc_rise], first_us == 0xFFFFFFFFu ? -1 : (int)first_us, back, d, kept, other, (unsigned)((R32(GPIO2 + G_DATAIN) & M1_BIT) != 0));
    }
    bad = write_design(sa_reg, ours);                                                       /* our design back, so the exit report's bus reads work; the bootloader loads it again too */
    printf("GPRESET: our design back in the registers (%d pages refused); M1 is left a GPIO driving high\n", bad);
    return BUFFEE_REROLL_EXIT(2);
}
