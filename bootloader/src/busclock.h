/* busclock.h -- the Amiga's bus clock (CLK7) as the PRU measures it, the host it identifies, and the E clock the PRU makes from it. */
#ifndef BUSCLOCK_H
#define BUSCLOCK_H

double      BusClockMHz(int full, int (*stop)(void));  /* the bus clock for the GPMC timing, per the settings (full: always the 40 ms measurement), after waiting for a late
                                                          one (up to 10 s, or until stop(): ESC); 0 when there is none or it is outside 5-16 MHz */
const char* HostName(void);             /* the machine the stored clock identifies */
int         HostIsAmiga(void);          /* that machine is an Amiga (PAL, NTSC, or an AdSPEED's doubled clock) */
void        StartEClock(void);          /* ends the measurement, starts the PRU's main loop (the E clock, VMA / VPA) */
void        RestartPRU(void);           /* resets the PRU and starts it again */

#endif
