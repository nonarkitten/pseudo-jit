/*
 * init_hw.h
 *
 *  Created on: Apr. 14, 2021
 *      Author: renee.cousins
 */

#include "main.h"
#include "am335x_dmtimer1.h"
#include "am335x_clock.h"
#include "cp15.h"

#ifndef INIT_HW_H_
#define INIT_HW_H_

#define CACHE_ICACHE                 (0x01)        /* Instruction cache */
#define CACHE_DCACHE                 (0x02)        /* Data and Unified cache*/
#define CACHE_ALL                    (0x03)        /* Instruction, Data and Unified Cache at all levels*/

typedef struct {
    uint16_t first_page;
    uint16_t last_page;
    uint16_t flags;
} MMU_Config_t;

/**
 * \brief   Sets the MCU clock to the specified speed
 *
 * \param   clockMHz   speed in MHz to set the MCU to
 *
 * \return  None.
 *
 **/
void InitializeMCUClock(int clockMHz);

/**
 * \brief   Initializes the Page Table with fault entries and configures CP15
 *          registers required for MMU. The Page Table passed is the master
 *          page table containing 4096 words, which will be defined by the
 *          application.
 *
 * \param   mmu_table     Page Table Base Address
 *
 * \return  None.
 *
 * \Note    Enabling Data Cache enables Unified cache also, if present.
 *
 **/
void InitializeMMU(volatile uint32_t mmu_table[4096]);

/**
 * \brief   Configures the MMU with the provided memory layout.
 *
 * \param   mmu_table     Page Table Base Address
 *
 * \param   mmu_config    Configuration table consisting of an array of
 *                        MMU_Config_t items ending with one entry that is
 *                        all zeroes.
 *
 * \return  None.
 *
 * \Note    Only TTB0 is used for page table walking.
 *
 */
void ConfigureMMU(volatile uint32_t mmu_table[4096], const MMU_Config_t* mmu_config);

/**
 * \brief   Updates the Translation Table Base with the address of the Page
 *          Table and enables MMU.
 *
 * \param   mmu_table     Page Table Base Address
 *
 * \return  None.
 *
 * \Note    Only TTB0 is used for page table walking.
 **/
void EnableMMU(volatile uint32_t mmu_table[4096]);

/**
 * \brief   Enables Cache. The levels/type of Cache to be enabled
 *          is passed as parameter.
 *
 * \param   enFlag   Caches to be enabled.
 *            'enFlag' can take one of the below values. \n
 *                CACHE_ICACHE - To enable Instruction Cache \n
 *                CACHE_DCACHE - To enable Data/Unified Cache \n
 *                CACHE_ALL - To enable all levels of Cache
 *
 * \return  None.
 *
 * \Note    Enabling Data Cache enables Unified cache also, if present.
 *
 **/
void CacheEnable(uint32_t enFlag);

#endif /* INIT_HW_H_ */
