#include "main.h"
#include "am335x_dmtimer1.h"
#include "am335x_clock.h"
#include "cp15.h"
#include "init_hw.h"

/*****************************************************************************
**                      INTERNAL MACRO DEFINITIONS
******************************************************************************/
#define MMU_PAGETABLE_ENTRY_FAULT    (0x00)
#define MMU_PAGEBOUND_SHIFT          (20)
#define MMU_PG_SUPSECT_SIZE_SHIFT    (14)
#define MMU_PGADDR_MASK              (0xFFF00000)
#define MMU_PGTYPE_MASK              (0x00040002)

#define CP15_CONTROL_TEXREMAP        (0x10000000)
#define CP15_CONTROL_ACCESSFLAG      (0x20000000)
#define CP15_CONTROL_ALIGN_CHCK      (0x00000002)
#define CP15_CONTROL_MMU             (0x00000001)

#define WATCHDOG_BASE                (0x44E35000)
#define MCU_DPLL_BASE                (0x44C00000)

#define CORTEX_A8_L2EN               (0x02)        /* Enable L2 cache */

#define SIZE                         (0x204FFF)
#define CM_DIV_M2_DPLL_MPU           (0x2004A8)
#define CM_CLKSEL_DPLL_MPU           (0x20042C)
#define CM_CLKMODE_DPLL_MPU_DPLL_EN  (0x200488)
#define CM_IDLEST_DPLL_MPU           (0x200420)

#define DPLL_MN_BYP_MODE             (4)
#define DPLL_LOCK_MODE               (7)

#define ST_MN_BYPASS                 (0x100)
#define ST_DPLL_LOCK                 (0x001)

#define HWREG(X) \
    *((volatile uint32_t*)(X))

#define HWREG_LE(X) \
    (*((volatile uint32_t*)(X)))


void InitializeMCUClock(int clockMHz) {
    /* Put the PLL in bypass mode and clear multiplier */
    HWREG(MCU_DPLL_BASE + CM_CLKMODE_DPLL_MPU_DPLL_EN) =
		(HWREG(MCU_DPLL_BASE + CM_CLKMODE_DPLL_MPU_DPLL_EN)
		& (~3))
		| (DPLL_MN_BYP_MODE);

    /* Wait for DPLL to go in to bypass mode */
    while(!(HWREG(MCU_DPLL_BASE + CM_IDLEST_DPLL_MPU) & (ST_MN_BYPASS)))
    	continue;

    /* Set the multiplier and divider values for the PLL */
    HWREG(MCU_DPLL_BASE + CM_CLKSEL_DPLL_MPU) |= ((clockMHz << 8) | 23);

    /* Set the CLKOUT2 divider */
    HWREG(MCU_DPLL_BASE + CM_DIV_M2_DPLL_MPU) =
            (HWREG(MCU_DPLL_BASE + CM_DIV_M2_DPLL_MPU) & (~31)) | (1);

    /* Now LOCK the PLL by enabling it */
    HWREG(MCU_DPLL_BASE + CM_CLKMODE_DPLL_MPU_DPLL_EN) |= (DPLL_LOCK_MODE);

    /* Wait for DPLL lock */
    while(!(HWREG(MCU_DPLL_BASE + CM_IDLEST_DPLL_MPU) & (ST_DPLL_LOCK))) continue;
}

void InitializeMMU(volatile uint32_t mmu_table[4096]) {
    /* Invalidate the TLB entries */
     CP15TlbInvalidate();

     /* Set domain access rights */
     CP15DomainAccessClientSet();

     /* Disable TEX remapping, Access Flag usage and alignment check */
     CP15ControlFeatureDisable((
    		 CP15_CONTROL_TEXREMAP |
			 CP15_CONTROL_ACCESSFLAG |
			 CP15_CONTROL_ALIGN_CHCK |
			 CP15_CONTROL_MMU));

     /* Configure the TTB Control register to use only TTB0 */
     CP15TtbCtlTtb0Config();

     /* Reset all memory pages to FAULT */
     for (int i=0x000; i < 0x1000; i++) mmu_table[i] = 0x0000;
}

void ConfigureMMU(volatile uint32_t mmu_table[4096], const MMU_Config_t* mmu_config) {
    /* Walk through the MMU configuration and fix up each region */
    while(mmu_config && mmu_config->last_page) {
        /* Update each 1MB page */
        for(int i=mmu_config->first_page; i<=mmu_config->last_page; i++) {
        	uint16_t physical = 0xFFF & (i + mmu_config->offset_pages);

            mmu_table[i] = (physical << MMU_PAGEBOUND_SHIFT)
            			 | (uint32_t)mmu_config->flags;
        }

        /* Advance to the next item */
        mmu_config++;
    }
}

void EnableMMU(volatile uint32_t mmu_table[4096]) {
    /* Set TTB0 register */
    CP15Ttb0Set((uint32_t)(mmu_table));

    /* Enable MMU */
    CP15MMUEnable();
}

void CacheEnable(uint32_t enFlag)
{
    if(enFlag & CACHE_ICACHE) {
        CP15ICacheFlush();
        CP15ICacheEnable();
    }

    if(enFlag & CACHE_DCACHE) {
        /* For Cortex A8, L2EN has to be enabled for L2 Cache */
        CP15AuxControlFeatureEnable((CORTEX_A8_L2EN));
        CP15DCacheFlush();
        CP15DCacheEnable();
    }
}



