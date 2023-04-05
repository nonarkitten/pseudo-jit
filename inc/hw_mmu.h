#ifndef __HW_MMU_H_
#define __HW_MMU_H_

#include "main.h"

/*
** Number of entries in a page table. This is only for first level page table
*/
#define MMU_PAGETABLE_NUM_ENTRY      (4096)

/*
** Alignment for Page Table. The Page Table shall be aligned to 16KB boundary.
*/
#define MMU_PAGETABLE_ALIGN_SIZE     (16 * 1024)

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

extern void InitMMU(void);
extern int MMUEnabled;

#endif
