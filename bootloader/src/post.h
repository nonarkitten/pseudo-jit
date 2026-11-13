/* post.h -- the power-on self-test of the board's own hardware. */
#ifndef POST_H
#define POST_H

enum { POST_OK = 0, POST_FAILED = 1, POST_STOPPED = 2 };   /* POST_STOPPED: ESC was pressed */

int DDRLineTest(void);          /* the DDR's data and address lines (a few hundred accesses, any time after InitDDR) */
int DDRFullTest(void);          /* every word of the 512 MB, twice (the MMU and the caches on: about 11 s at 300 MHz); destroys what is in the DDR */
int FlashIDTest(void);          /* the SPI flash answers with a plausible ID */

#endif
