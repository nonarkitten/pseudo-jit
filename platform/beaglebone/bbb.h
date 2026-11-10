/// @file bbb.h
/// @brief BeagleBone (AM335x, Cortex-A8) platform glue: where things are, and the small API the tests use.
///        Register addresses are from the AM335x TRM.
#ifndef BBB_H
#define BBB_H
#include <stdint.h>

// ---- memory map -------------------------------------------------------------------------------------------------------------
#define BBB_DDR_BASE      0x80000000u     // 512 MB of DDR3
#define BBB_IMAGE_BASE    0x82000000u     // U-Boot's default load address ($loadaddr); `go 0x82000000`
#define BBB_LOG_BASE      0x87F00000u     // the RAM log (a 1 MB section the page table leaves uncached); inside the 96 MB below U-Boot's FDT

// The log: a header, then text. It lives in uncached DDR, so it survives a hang and the watchdog reset that follows (DDR is not
// re-cleared by a warm reset), and the next U-Boot script can save it before it runs anything.
#define BBB_LOG_MAGIC     0x474C4A50u     // 'P' 'J' 'L' 'G' as a little-endian word
#define BBB_LOG_SIZE      0x10000u
typedef struct {
    uint32_t magic;       // BBB_LOG_MAGIC once the log is live
    uint32_t length;      // bytes of text after the header
    uint32_t state;       // BBB_RUN_*  (see below)
    uint32_t code;        // the exit code, when state == BBB_RUN_DONE
    uint32_t cycles;      // PMU cycle counter at the last update
    uint32_t spare[3];
} bbb_log_t;
#define BBB_RUN_STARTED   1u              // set at boot; still 1 when the next boot reads it = the run hung or crashed (the watchdog reset it)
#define BBB_RUN_DONE      2u              // set by bbb_exit(): a clean return to U-Boot
#define BBB_RUN_FAULT     3u              // set by the fault handler, just before it returns to U-Boot

// The user LEDs show the state without a console: USR0 = running, USR3 = finished cleanly, USR2 = a fault was caught and reported.
// (A hang ends in the watchdog reset, which turns them all off.) Bit n = USR n.
#define BBB_LED_RUNNING   0x1u
#define BBB_LED_DONE      0x8u
#define BBB_LED_FAULT     0x4u

// ---- API --------------------------------------------------------------------------------------------------------------------
void bbb_init(void);                      // MMU + caches on, log live, watchdog armed, cycle counter running (called by boot.S)
void bbb_exit(uint32_t code) __attribute__((noreturn));   // flush, stop the watchdog, put U-Boot's CPU state back, return to it
void bbb_led(uint32_t mask);               // the four user LEDs
void bbb_map_low(uint32_t phys);              // guest addresses 0..0xFFFFF -> the 1 MB-aligned RAM at phys (replaces the identity section at 0)
void bbb_wdt_arm(uint32_t seconds);       // (re)start the watchdog with this timeout
void bbb_wdt_stop(void);

// the ARM performance monitor: the cycle counter plus four event counters
void bbb_pmu_start(const uint8_t events[4]);
uint32_t bbb_pmu_cycles(void);
uint32_t bbb_pmu_event(int n);
// a few useful Cortex-A8 events
#define PMU_L1I_MISS      0x01
#define PMU_L1D_MISS      0x03
#define PMU_L1D_ACCESS    0x04
#define PMU_INSTR         0x08            // instructions architecturally executed
#define PMU_PC_CHANGE     0x0C            // software PC changes: taken branches
#define PMU_BR_MISPRED    0x10
#define PMU_BR_PRED       0x12
#endif
