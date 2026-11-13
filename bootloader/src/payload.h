/* payload.h -- PJIT, the program the bootloader starts: loading it (from the flash or over XMODEM), the MapROM image and the boot information it reads, starting it. */
#ifndef PAYLOAD_H
#define PAYLOAD_H
#include <stdint.h>

#define RX_BUF 0x90000000u              /* where a bundle or an image is received or read from the flash before it is unpacked or stored */

int  PJITLoad(void);                    /* the bundle from the flash, unpacked: 0 when it is ready to run (otherwise the reason is printed) */
int  PJITReceive(void);                 /* the same over XMODEM */
int  StageROM(void);                    /* the MapROM image into the DRAM and the boot information for the payload: 0 when done or not wanted */
void PJITRun(int clean_reset);          /* runs the loaded payload until it returns (ESC x3); clean_reset: pulse the Amiga's RESET and calibrate first */
void PJITBoot(void);                    /* menu: load from the flash, stage, run with a clean reset */

#endif
