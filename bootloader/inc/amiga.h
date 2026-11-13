/* amiga.h -- the bootloader's own accesses to the Amiga's bus, the same on a board with the byte-swapper and one without. */
#ifndef AMIGA_H
#define AMIGA_H
#include <stdint.h>

enum { LANES_SWAPPER, LANES_STRAIGHT, LANES_UNKNOWN };     /* how the GPMC's data lines meet the Amiga's: through the swapper board (crossed) or straight */

enum { LANES_FROM_NONE, LANES_FROM_RECORD, LANES_FROM_TEST, LANES_FROM_ROM };    /* where amiga_lanes came from */

extern int amiga_lanes;                                     /* LANES_SWAPPER or LANES_STRAIGHT (LANES_SWAPPER until something says otherwise) */
extern int amiga_lanes_from;                                /* LANES_FROM_* */
void     AmigaFindLanes(void);                              /* before the bus runs: the EEPROM's board kind, or the GreenPAK supply test (stored); picks the GreenPAK design */
void     AmigaCheckLanes(void);                             /* once the host let RESET go: the Kickstart's JMP word (see amiga.c for when it counts) */
int      AmigaRetestLanes(void);                            /* the supply test again (menu), stored: LANES_*, or -1 when it cannot tell */
const char* AmigaLanesName(void);
const char* AmigaLanesFrom(void);
uint16_t AmigaRead16(uint32_t a);                           /* the word at an even address, as the 68000 sees it */
void     AmigaWrite16(uint32_t a, uint16_t v);
uint8_t  AmigaRead8(uint32_t a);
void     AmigaWrite8(uint32_t a, uint8_t v);
void     AmigaCopy(void* dst, uint32_t src, uint32_t bytes);   /* bytes (even) from the Amiga's bus into RAM, in the Amiga's byte order */

#endif
