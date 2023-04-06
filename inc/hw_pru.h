/*
 * init_pru.h
 *
 *  Created on: May 18, 2022
 *      Author: Renee Cousins
 */

#ifndef INIT_PRU_H_
#define INIT_PRU_H_

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

extern void PRUInit(void);

extern void PRUReset(PRU_CORE_t PRUCore);
extern void PRUEnable(PRU_CORE_t PRUCore);
extern void PRUHalt(PRU_CORE_t PRUCore);

extern void PRUMemCpy(PRU_RAM_t MemoryType, uint32_t offset, uint32_t Length, const uint32_t *Pointer);
extern void PRUMemSet(PRU_RAM_t MemoryType, uint32_t offset, uint32_t Length, uint32_t Pattern);

#endif /* INIT_PRU_H_ */
