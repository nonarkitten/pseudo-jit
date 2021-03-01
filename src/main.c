#include "main.h"

#include "arm.h"
#include "config.h"
#include "support.h"
#include "tlsf.h"
#include "devicetree.h"
#include "m68k.h"
#include "hunkloader.h"
#include "duffcopy.h"
#include "emulogo.h"
#include "emufeatures.h"

extern uint8_t m68kcode[];

static const char bootstrapName_BE[] = ;

__attribute__ ((used, aligned (0x4000)))
static uint32_t mmu_table[4096] = { 0 }; 

/* Linker variables */
extern char _stack_end, _stack_top, _bss_start, _bss_end;

/*  AM335x Physical Memory Map (1MB resolution)

    INTERNAL ADDRESSES                   M68K ADDRESSES
    =================================    ===================================================
    000 - 1FF GPMC                       000 - 0FF Amiga 16-bit memory, default do not cache
                                         002 - 009 Amiga Zorro-II space, cache optional
                                         00F - 00F Amiga Kickstart ROM, always cache
                                      
    800 - 97F 384MB User space           080 - 1FF Amiga 32-bit memory
    980 - AFF 384MB User space (1GB)     200 - 37F Amiga 32-bit memory
                                      
    400 - 5FF Internal Hardware          400 - 5FF Zorro-III Autoconfig Space
                                      
    980 - 9FF Application space       
    A00 - BFF Application space (1GB)
    
    
    1C0E    Set: 12, 11, 10, 4, 3, and 2
    
    
    31-20 BADR      Section Base Address. Must be 1MB aligned
    19    NS        Non-secure bit. This is used by the security extensions
    17    nG        Not global. Determines how this is marked in the TLB.
    18    0         Should be zero
    16    S         Shareable. 0 - non-shareable, 1 - shareable
    15    APX       Access Permissions Table
    14-12 TEX       
    11-10 AP[0:1]   
    9     IMP       Implementation defined
    8-5   Domain    Used as a memory protection mechanism. 16 possible domains
    4     XN        Execute never. Stops execution of page.
    3     C         Cacheable. 0 - inner non-cacheable, 1 - inner cacheable
    2     B         
    1     1         Should be one
    0     PXN       Privileged Execute Never - SBZ if PXN is not supported
    
    98000000 Entry point
    
*/

static double seconds(void) {
    uint32_t up = am335x_dmtimer1_get_counter();
    return (double)up / 24000000.0;
}

static inline char* ptime(double s) {
    static char buffer[32];
    uint32_t intPart = (uint32_t)s;
    double f = s - intPart;
    uint32_t fracPart = (uint32_t)(f * 1000000.0);
    sprintf(buffer, "%d.%06d seconds", intPart, fracPart);
    return buffer;
}

static inline char* psize(uint32_t size) {
    static char buffer[32];
    char *sfx;
    if(size < 1024) sfx = "B";
    else if(size < 1024 * 1024) { sfx = "KB"; size += 512; size /= 1024; }
    else if(size < 1024 * 1024 * 1024) { sfx = "MB"; size += 512 * 1024; size /= 1024 * 1024; }
    else { sfx = "GB"; size += 512 * 1024 * 1024; size /= 1024 * 1024 * 1024; }
    sprintf(buffer, "%d%s", size, sfx);
    return buffer;
}

/*     __  __       _         _____       _              
**    |  \/  | __ _(_)_ __   | ____|_ __ | |_ _ __ _   _ 
**    | |\/| |/ _` | | '_ \  |  _| | '_ \| __| '__| | | |
**    | |  | | (_| | | | | | | |___| | | | |_| |  | |_| |
**    |_|  |_|\__,_|_|_| |_| |_____|_| |_|\__|_|   \__, |
**                                                 |___/ 
*/ 
__attribute__((naked, section(".startup @")))
void main(void) { 
    board_init();

    // Brian Fraser's fix for UBoot and enable big endian mode
    asm volatile (
    "           sub     r0, r0, r0              \n" /* Brian Fraser's fix for UBoot            */
    "           mcr     p15, 0, r0, c1, c0, 0   \n" /* Disable MMU, instruction and data cache */
    "           setend  be                      \n" /* Switch to big endian (FTW!)             */
    );

    // Initialize system stacks
    asm volatile (
    "           msr     cpsr_c, #0xd1           \n" /* FIQ                                     */
    "           ldr     sp,=_stack_end + 0x10   \n"                               
    "           msr     cpsr_c, #0xd2           \n" /* IRQ                                     */
    "           ldr     sp,=_stack_end + 0x18   \n"                               
    "           msr     cpsr_c, #0xd7           \n" /* Abort                                   */
    "           ldr     sp,=_stack_end + 0x20   \n"                               
    "           msr     cpsr_c, #0xdb           \n" /* Undefined                               */
    "           ldr     sp,=_stack_end + 0x28   \n"                               
    "           msr     cpsr_c, #0xdf           \n" /* System                                  */
    "           ldr     sp,=_stack_end + 0x1000 \n"                               
    "           msr     cpsr_c, #0xd3           \n" /* Supervisor                              */
    "           ldr     sp,=_stack_top          \n"
    );

    // Invalidate cache and Enable Branch Prediction 
    // Allow unaligned access, effective only when MMU is enabled
    // Enable both instruction and data caches
    asm volatile (                                                                      
    "           mov r0, #0                      \n"                                     
    "           mcr p15, #0, r0, c7, c5, #6     \n"                                     
    "           mrc p15, 0, r0, c1, c0, 0       \n"
    "           orr r0, r0, #0x000004           \n" // Enable data cache
    "           orr r0, r0, #0x001800           \n" // Branch prediction and instruction cache
    "           orr r0, r0, #0x400000           \n" // Enable unaligned data
    "           bic r0, r0, #0x000002           \n" // Disable strict alignment check
    "           mcr p15, 0, r0, c1, c0, 0       \n"
    "           isb                             \n"                                     
    "           isb                             \n"                                     
    "           isb                             \n"                                     
    "           isb                             \n"                                     
    );                                                                                  
                                                                                         
    // Enable Neon/VFP Co-Processor
    asm volatile (                                                                      
    "           mrc     p15, #0, r1, c1, c0, #2 \n"    /* r1 = Access Control Register         */
    "           orr     r1, r1, #(0xf << 20)    \n"    /* enable full access for p10,11        */
    "           mcr     p15, #0, r1, c1, c0, #2 \n"    /* Access Control Register = r1         */
    "           mov     r1, #0                  \n"                                     
    "           mcr     p15, #0, r1, c7, c5, #4 \n"    /* flush prefetch buffer                */
    "           mov     r0,#0x40000000          \n"                                     
    "           fmxr    fpexc, r0               \n"    /* Set Neon/VFP Enable bit              */
    );                                                                                  
#if 0                                                                                        
    // disable am335x watchdog
    asm volatile (                                                                      
    "           ldr r0, =0x44e35000             \n"    /* load SOC_WDT_1_REGS                  */
    "           ldr r1, =0xaaaa0000             \n"                                     
    "           str r1, [r0, #0x48]             \n"    /* store 0xaaaa to WDT_WSPR             */
    "1:         ldr r1, [r0, #0x34]             \n"    /* loop until WDT_WWPS is 0             */
    "           cmp r1, #0x0                    \n"                                     
    "           bne 1b                          \n"                                     
    "           ldr r1, =0x55550000             \n"                                     
    "           str r1, [r0, #0x48]             \n"    /* store 0x5555 to WDT_WSPR             */
    "2:         ldr r1, [r0, #0x34]             \n"    /* loop until WDT_WWPS is 0             */
    "           cmp r1, #0x0                    \n"
    "           bne 2b                          \n"
    );
#endif
    // Clear the .bss section (zero init)
    asm volatile (                                                                      
    "           mov r0, #0                      \n"
    "           ldr r1, =_bss_start             \n"
    "           ldr r2, =_bss_end               \n"
    "1:         cmp r1, r2                      \n"
    "           strlo   r0, [r1], #4            \n"
    "           blo     1b                      \n"
    );

 	// Initialize bsp
    am335x_clock_enable_l3_l4wkup();
    am335x_dmtimer1_init();

    printf("PJIT starting up...");     
    printf("[PJIT] Built on %s at %s\n", __DATE__, __TIME__);
    printf("[PJIT] CPU Clock Speed = %d MHz\n", (int)get_cpu_mhz());	
    printf("[PJIT] Stack %p-%p (size=%s)\n", &_stack_end, &_stack_top, psize(&_stack_top - &_stack_end));
    printf("[PJIT] BSS %p-%p (size=%s)\n", &_bss_end, 0x9FFFFFFF, psize((char*)0xA0000000 - &_bss_end));
    
    // 0x000 to 0x7FF, GPMC/PIO strongly ordered, no caching
    for (int i=0x000; i < 0x800; i++) mmu_table[i] = (i << 20) | 0x0c06;
    // 0x800 to 0x9FF, SDRAM, Caches write-through, write allocate, access for all
    for (int i=0x800; i < 0xA00; i++) mmu_table[i] = (i << 20) | 0x1c0e;
    // 0x400 to 0x403, internal ROM and SRAM, Caches write-through, write allocate, access for all
    for (int i=0x400; i < 0x403; i++) mmu_table[i] = (i << 20) | 0x1c0e;
    
    arm_flush_cache((uint32_t)mmu_table, sizeof(mmu_table));

    // Load new pointer to the mmu table    
    asm volatile(
    "       dsb                         \n"
    "       mcr     p15,0,r0,c7,c5,0    \n" /* invalidate icache */
    "       isb                         \n"
    "       ldr     r4,=mmu_table       \n" /* Load MMU table pointer */
    "       mcr     p15,0,r4,c2,c0,0    \n" /* Write page_dir address to ttbr0 */
    "       mov     r8, #0              \n"
    "       mcr     p15,0,r8,c2,c0,2    \n" /* Write ttbr control N = 0 (use only ttbr0) */
    "       mov     r4, #1              \n"
    "       mcr     p15,0,r4,c3,c0,0    \n" /* Set domains - Dom0 is usable, rest is disabled */
    "       mrc     p15,0,r4,c1,c0,0    \n" /* Load control register */
    "       orr     r4,r4,#0x800000     \n" /* v6 page tables, subpages disabled */
    "       orr     r4,r4,#1            \n" /* MMU enabled */
    "       orr     r4,r4,#1<<25        \n" /* MMU tables in big endian */
    "       dsb                         \n"
    "       mcr     p15,0,r4,c1,c0,0    \n" /* Set control register and thus really enable mmu */
    "       isb                         \n"
    );
	printf("[PJIT] Boot complete in %s seconds.\n", ptime(seconds()));
    
    init_led_output();
    heartbeat_forever();
    while(1);
}
