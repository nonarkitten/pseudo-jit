/* payload.h -- PJIT, the program the bootloader starts: loading it (from the flash or over XMODEM), the MapROM image and the boot information it reads, starting it. */
#ifndef PAYLOAD_H
#define PAYLOAD_H
#include <stdint.h>

#define RX_BUF 0x90000000u              /* where a bundle or an image is received or read from the flash before it is unpacked or stored */

enum { SLOT_EMPTY, SLOT_GOOD, SLOT_BAD, SLOT_NONE = 0xFF };
typedef struct { uint8_t state[2], booted; uint32_t seq[2]; } pjit_slots_t;     /* [0] the backup slot, [1] the update slot (store.h) */
extern pjit_slots_t pjit_slots;

int  PJITLoad(void);                    /* the newer good bundle from the flash, unpacked: 0 when it is ready to run (otherwise the reason is printed) */
void PJITScanSlots(void);               /* pjit_slots from the slots' headers */
uint32_t PJITNextSeq(void);             /* the sequence number for the next bundle stored */
int  PJITStore(int slot, const void* src, uint32_t len);    /* 0 = backup, 1 = update: STORE_OK or an error */
void PJITEraseSlot(int slot);
const char* PJITSlotName(int slot);
int  PJITFromMemory(const void* src, uint32_t len);        /* a bundle already in the DRAM, unpacked (not stored): 0 when it is ready to run */
int  PJITReceive(void);                 /* the same over XMODEM */
int  StageROM(void);                    /* the MapROM image into the DRAM and the boot information for the payload: 0 when done or not wanted */
void PJITRun(int clean_reset);          /* runs the loaded payload until it returns (ESC x3); clean_reset: pulse the Amiga's RESET and calibrate first */
void PJITBoot(void);                    /* menu: load from the flash, stage, run with a clean reset */

#endif
