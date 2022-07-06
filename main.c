#include "main.h"
//#include "support/cp15.h"
// #include "init_hw.h"
// #include "am335x_pru.h"
// #include "am335x_gpio.h"
// #include "pinmux.h"
 
__attribute__ ((used, aligned (MMU_PAGETABLE_ALIGN_SIZE)))
static volatile uint32_t pageTable[MMU_PAGETABLE_NUM_ENTRY] = { 0 };
 
/* Section Flags
 *
 * 31~20	High 12-bits of base address
 * 19		NS				Not secure
 * 18		Must be 0
 * 17		nG				Not Global
 * 16		S				Shareable
 * 15		AP[2]
 * 14~12	TEX[2:0]
 * 11~10	AP[1:0]
 * 9		User
 * 8~5		Domain			Leave as zero for now
 * 4		XN				Execute never
 * 3		C
 * 2		B
 * 1		Must be 1
 * 0		PXN				Privileged execute never
 *
 *  TEX C B Mode
 *  000 0 0	Strongly-Ordered
 *  000 0 1	Shareable Device
 *  000 1 0	L2/L1 Write-thru, no Write-allocate
 *  000 1 1	L2/L1 Write-back, no Write-allocate
 *  001 0 0	L2/L1 Non-cacheable
 *  001 1 1	L2/L1 Write-back, Write-allocate
 *  010 0 0	Non-shareable Device
 *  100 0 0	L2 Non-Cacheable; L1 Non-Cacheable
 *  100 0 1	L2 Non-Cacheable; L1 Write-back, Write-allocate
 *  100 1 0	L2 Non-Cacheable; L1 Write-thru, no Write-allocate
 *  100 1 1	L2 Non-Cacheable; L1 Write-back, no Write-allocate
 *  101 0 0	L2 Write-back, Write-allocate; L1 Non-Cacheable
 *  101 0 1	L2 Write-back, Write-allocate; L1 Write-back, Write-allocate
 *  101 1 0	L2 Write-back, Write-allocate; L1 Write-thru, no Write-allocate
 *  101 1 1	L2 Write-back, Write-allocate; L1 Write-back, no Write-allocate
 *  110 0 0	L2 Write-thru, no Write-allocate; L1 Non-Cacheable
 *  110 0 1	L2 Write-thru, no Write-allocate; L1 Write-back, Write-allocate
 *  110 1 0	L2 Write-thru, no Write-allocate; L1 Write-thru, no Write-allocate
 *  110 1 1	L2 Write-thru, no Write-allocate; L1 Write-back, no Write-allocate
 *  111 0 0	L2 Write-back, no Write-allocate; L1 Non-Cacheable
 *  111 0 1	L2 Write-back, no Write-allocate; L1 Write-back, Write-allocate
 *  111 1 0	L2 Write-back, no Write-allocate; L1 Write-thru, no Write-allocate
 *  111 1 1	L2 Write-back, no Write-allocate; L1 Write-back, no Write-allocate
 *
 *  AP
 *  210 PL1+ 	PL0 	Description
 *  000	No		No		All access generate fault
 *  001	RW		No		Access only at PL1+
 *  010	RW		RO		Writes at PL0 generate fault
 *  011	RW		RW		Full access
 *  100	-		-
 *  101	RO		No		Read-only, only at PL1+
 *  110	RO		RO		Read-only, any PL (deprecated)
 *  111	RO		RO		Read-only, any PL
 *
 */
#define MMU_ORDERED_NO_CACHE    (0x1C02)
//(0x0C06)
#define MMU_CACHE_WRITE_THRU    (0x1C0E)

static const MMU_Config_t mmu_config[] = {
    /* M68K DATA */
    { 0x000, 0x00F, 0x020, MMU_ORDERED_NO_CACHE },
    /* M68K INST */
    { 0x010, 0x01F, 0x020, MMU_ORDERED_NO_CACHE },
    /* ARM SRAM */
    { 0x402, 0x403, 0x000, MMU_CACHE_WRITE_THRU },
    /* DEVICES */
    { 0x440, 0x7FF, 0x000, MMU_ORDERED_NO_CACHE },
    /* SDRAM */
    { 0x800, 0x9FF, 0x000, MMU_CACHE_WRITE_THRU },
    /* End of config */
    { 0, 0, 0, 0 }
};

//static uint32_t cache[PJIT_CACHE_SIZE + PJIT_TAG_SIZE];

int QueryIRQ(int level) {
	// check GPIO and return real levels
    (void)level;
	return 7;
}

//static volatile uint8_t test[] = { 0x01, 0x02, 0x03, 0x04 };
// 0 is 0x11 0x11 0x4F 0xF9


int main(void) {
	CP15MMUDisable();
	CP15DCacheDisable();
	CP15ICacheDisable();

    // printf("[BOOT] PJIT Built on %s at %s\n", __DATE__, __TIME__);

    InitializeMMU(pageTable);
    ConfigureMMU(pageTable, mmu_config);
    EnableMMU(pageTable);
    CacheEnable(CACHE_ALL);

//    printf("[BOOT] Resetting emulation\n");
//    HWReset();
//    printf("[BOOT] Starting emulation\n");
//    CPURun();

    // test_native_bogomips();
    // test_pjit_bogomips();

    // PJIT: 249.99 BogoMIPS no inline, register in SRAM
    // PJIT: 285.71 BogoMIPS no inline, register in SRAM, Os
    // PJIT: 333.33 BogoMIPS no inline, register variables
    // PJIT: 666.67 BogoMIPS with inline, register variables

	while(1); // die

//	if(!cache) {
//		fprintf( stderr, "Fatal allocation error.\n" );
//		exit(1);
//	}	
//	debug("Cache allocated %d bytes\n", size);
	// cache_init((uint32_t)&cache);
	// debug("Cache initialized.\n");
	// cpu = &cpu_state;
	// cpu_dump_state();
	// debug("Starting CPU.\n");
	// cpu_start(bogomips);
	// cpu_dump_state();
}

// void _sbrk(void) { }
// extern void _exit(int nada) { (void)nada; }
// void _write(void) { }
// void _read(void) { }
// void _close(void) { }
// void _fstat(void) { }
// void _isatty(void) { }
// void _lseek(void) { }
