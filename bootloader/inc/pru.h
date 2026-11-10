/* pru.h -- the PRU subsystem. */

#ifndef PRU_H_
#define PRU_H_

typedef enum {
    PRU0_DRAM,
    PRU1_DRAM,
    PRU0_IRAM,
    PRU1_IRAM,
    PRU_SHARED_RAM
} PRU_RAM_t;

typedef enum {
    NO_CORE,
    PRU0,
    PRU1
} PRU_CORE_t;

extern void PRUReset(PRU_CORE_t PRUCore);
extern void PRUEnable(PRU_CORE_t PRUCore);
extern void PRUHalt(PRU_CORE_t PRUCore);
extern void PRUFullReset(PRU_CORE_t PRUCore);

extern void PRUMemCpy(PRU_RAM_t MemoryType, uint32_t offset, uint32_t Length, const uint32_t *Pointer);
extern void PRUMemSet(PRU_RAM_t MemoryType, uint32_t offset, uint32_t Length, uint32_t Pattern);

#endif /* PRU_H_ */
