#include "main.h"
#include "init_hw.h"

__attribute__ ((used, aligned (MMU_PAGETABLE_ALIGN_SIZE)))
static volatile uint32_t pageTable[MMU_PAGETABLE_NUM_ENTRY] = { 0 };

#define MMU_ORDERED_NO_CACHE    (0x0C06)
#define MMU_CACHE_WRITE_THRU    (0x1C0E)

static const MMU_Config_t mmu_config[] = {
    /* M68K DATA */
    { 0x000, 0x00F, MMU_ORDERED_NO_CACHE },
    /* M68K INST */
    { 0x010, 0x01F, MMU_ORDERED_NO_CACHE },
    /* ARM SRAM */
    { 0x402, 0x403, MMU_CACHE_WRITE_THRU },
    /* DEVICES */
    { 0x440, 0x7FF, MMU_ORDERED_NO_CACHE },
    /* SDRAM */
    { 0x800, 0x9FF, MMU_CACHE_WRITE_THRU },
    /* End of config */
    { 0, 0, 0 }
};

int main(void) {

    printf("\n");

    printf("[BOOT] PJIT Built on %s at %s\n", __DATE__, __TIME__);

    printf("[BOOT] Initializing clock to %dMHz ... ", STARTUP_CLOCK);
    InitializeMCUClock(STARTUP_CLOCK);
    printf("Set\n");

    printf("[BOOT] Initializing MMU ... ");
    InitializeMMU(pageTable);
    printf("Config ... ");
    ConfigureMMU(pageTable, mmu_config);
    printf("Enabling ... ");
    EnableMMU(pageTable);
    printf("Done\n");

    printf("[BOOT] Initializing caches ... ");
    CacheEnable(CACHE_ALL);
    printf("Cool\n");

    test_native_bogomips();
    test_pjit_bogomips();

    // PJIT: 249.99 BogoMIPS no inline, register in SRAM
    // PJIT: 285.71 BogoMIPS no inline, register in SRAM, Os
    // PJIT: 333.33 BogoMIPS no inline, register variables
    // PJIT: 666.67 BogoMIPS with inline, register variables

	while(1); // die
}
