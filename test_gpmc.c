/*
 * test_gpmc.c
 *
 *  Created on: Apr. 18, 2021
 *      Author: renee.cousins
 */

#define CFG_MOD_BASE       (0x44E10000)      //REVISIT

#define CONF_GPMC_AD0       0x800   // Data bit 0       MODE(0)
#define CONF_GPMC_AD1       0x804   // Data bit 1       MODE(0)
#define CONF_GPMC_AD2       0x808   // Data bit 2       MODE(0)
#define CONF_GPMC_AD3       0x80C   // Data bit 3       MODE(0)
#define CONF_GPMC_AD4       0x810   // Data bit 4       MODE(0)
#define CONF_GPMC_AD5       0x814   // Data bit 5       MODE(0)
#define CONF_GPMC_AD6       0x818   // Data bit 6       MODE(0)
#define CONF_GPMC_AD7       0x81C   // Data bit 7       MODE(0)
#define CONF_GPMC_AD8       0x820   // Data bit 8       MODE(0)
#define CONF_GPMC_AD9       0x824   // Data bit 9       MODE(0)
#define CONF_GPMC_AD10      0x828   // Data bit 10      MODE(0)
#define CONF_GPMC_AD11      0x82C   // Data bit 11      MODE(0)
#define CONF_GPMC_AD12      0x830   // Data bit 12      MODE(0)
#define CONF_GPMC_AD13      0x834   // Data bit 13      MODE(0)
#define CONF_GPMC_AD14      0x838   // Data bit 14      MODE(0)
#define CONF_GPMC_AD15      0x83C   // Data bit 15      MODE(0)
#define CONF_GPMC_A1        0x844   // Address bit 1    MODE(0)
#define CONF_GPMC_A2        0x848   // Address bit 2    MODE(0)
#define CONF_GPMC_A3        0x84C   // Address bit 3    MODE(0)
#define CONF_GPMC_A4        0x850   // Address bit 4    MODE(0)
#define CONF_GPMC_A5        0x854   // Address bit 5    MODE(0)
#define CONF_GPMC_A6        0x858   // Address bit 6    MODE(0)
#define CONF_GPMC_A7        0x85C   // Address bit 7    MODE(0)
#define CONF_GPMC_A8        0x860   // Address bit 8    MODE(0)
#define CONF_GPMC_A9        0x864   // Address bit 9    MODE(0)
#define CONF_GPMC_A10       0x868   // Address bit 10   MODE(0)
#define CONF_GPMC_A11       0x86C   // Address bit 11   MODE(0)

#define CONF_LCD_DATA8      0x8C0   // Address bit 12   MODE(1)
#define CONF_LCD_DATA8      0x8C4   // Address bit 13   MODE(1)
#define CONF_LCD_DATA8      0x8C8   // Address bit 14   MODE(1)
#define CONF_LCD_DATA8      0x8CC   // Address bit 15   MODE(1)
#define CONF_LCD_DATA8      0x8D0   // Address bit 16   MODE(1)
#define CONF_LCD_DATA8      0x8D4   // Address bit 17   MODE(1)
#define CONF_LCD_DATA8      0x8D8   // Address bit 18   MODE(1)
#define CONF_LCD_DATA8      0x8DC   // Address bit 19   MODE(1)
#define CONF_MMC0_DAT3      0x8F0   // Address bit 20   MODE(1)
#define CONF_MMC0_DAT2      0x8F4   // Address bit 21   MODE(1)
#define CONF_MMC0_DAT1      0x8F8   // Address bit 22   MODE(1)
#define CONF_MMC0_DAT0      0x8FC   // Address bit 23   MODE(1)
#define CONF_MMC0_CLK       0x900   // Address bit 24   MODE(1)

#define CONF_GPMC_WAIT0     0x870   // MODE(0)
#define CONF_GPMC_BEN1      0x878   // MODE(0)
#define CONF_GPMC_ADVN_ALE  0x890   // MODE(0)
#define CONF_GPMC_WEN		0x898   // MODE(0)
#define CONF_GPMC_BEN0_CLE	0x89C   // MODE(0)


#define PULL_UD_DISABLE  (1 << 3) /* PULL UP/DOWN disabled */
#define PULL_TYPESEL     (1 << 4) /* PULL UP Select */
#define RXACTIVE         (1 << 5)
#define SLEW_CTRL        (1 << 7)
#define PULL_UP_EN       (PULL_TYPESEL) /* PUL UP Select, PULL_UD_EN = 0 */

/* Pin mux for m68k bus module */
static struct pin_muxing {
	uint32_t offset;
	uint32_t val;
} m68k_pin_mux[] = {
		{ CONF_GPMC_AD0,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD1,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD2,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD3,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD4,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD5,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD6,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD7,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD8,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD9,        MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD10,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD11,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD12,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD13,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD14,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_AD15,       MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_A1,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A2,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A3,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A4,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A5,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A6,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A7,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A8,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A9,         MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A10,        MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_A11,        MODE(0) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_LCD_DATA8,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT3,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT2,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT1,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_DAT0,       MODE(1) | PULL_UD_DISABLE },
		{ CONF_MMC0_CLK,        MODE(1) | PULL_UD_DISABLE },
		{ CONF_GPMC_WAIT0,      MODE(0) | PULL_UP_EN | RXACTIVE },
		{ CONF_GPMC_BEN1,       MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_ADVN_ALE,   MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_WEN		,   MODE(0) | PULL_UD_DISABLE },
		{ CONF_GPMC_BEN0_CLE,   MODE(0) | PULL_UD_DISABLE },
		{ 0xFFFFFFFF },
};

static void config_mux(struct pin_muxing* pin_mux) {
	while(pin_mux->offset != 0xffffffff) {
		*(volatile uint32_t*)(CFG_MOD_BASE + pin_mux->offset) = pin_mux->val;
		pin_mux++;
	}
}

#define NUMARGS(...)	(sizeof((int[]){__VA_ARGS__})/sizeof(int))
#define SET_BIT(...)	(adv_set_bit(NUMARGS(__VA_ARGS__), __VA_ARGS__))
static u32 adv_set_bit(int num, ...)
{
	u32 res = 0;
	va_list ap;

	va_start(ap, num);
	while(num--)
		res |= BIT(va_arg(ap, int));
	va_end(ap);

	return res;
}

static void _set_gpmc_conf(u32 *conf)
{
	/*
	 * Config 1, register sets signal control parameters per chip select.
	 *	BIT0~1	: GPMC_CLK = 11 = GPMC_FCLK(internal max 100MHz) / 4
	 *	BIT4	: Signals timing latencies scalar factor **unknow**
	 *	BIT8~9	: muxadd data  **unkonw**
	 *	BIT10~11: Device Type = 00 =  NOR Flash like, asynchronous and synchronous devices
	 *	BIT12~13: Device size = 01 = 16bit **unknow**
	 *	BIT16~17: Wait pin0 or wait pin 1	**unkonw**
	 *	BIT21	: Write wait pin monitor
	 *	BIT22	: read wait pin monitor
	 *	BIT23~24: device page burst length
	*/
	*conf = SET_BIT(12);
	/*
	 * Config 2, chip-select signal timing parameter configuration
	 *	BIT0~3	: assertion time form start cycle time = 001 = 1 GPMC_FCLK cycle
	 *	BIT7	: chip-select timing control signal delay
	 *	BIT8~12 : de-assertion time from start cycle time for read accesses
	 *	BIT16~20: de-assertion time from start cycle time for write accesses
	*/
	*(conf+1) = SET_BIT(0, 8, 16);
	/*
	 * Config 3, ADV#(ADVn pin) signal timing parameter configuration
	 *	BIT0~3	: assertion time form start cycle time in ADV#
	 *	BIT4~6	: assertion for first address phase when using the AAD-Multiplexed in ADV#
	 *	BIT7	: add extra half GPMC_FCLK cycle in ADV#
	 *	BIT8~12 : de-assertion time from start cycle time for READ accesses in ADV#
	 *	BIT16~20: de-assertion time from start cycle time for WRITE accesses in ADV#
	 *	BIT24~26: assertion time for first address phase when using the ADD-Multiplexed in ADV#
	 *	BIT28~30: de-assertion time for first address phase when using the ADD-Multiplexed in ADV#
	*/
	*(conf+2) = SET_BIT(0, 10, 18);
	/*
	 * Config 4, WE# and OE# signal timing parameter configuration
	 *	BIT0~3	: assertion time for start cycle time in OE#
	 *	BIT4~6	: assertion for first address phase when using the AAD-Multiplexed access in OE#
	 *	BIT7	: add extra half GPMC_FCLK cycle in OE#
	 *	BIT8~12	: de-assertion time form start cycle time in OE#
	 *	BIT13~15: de-assertion timr for the first address phase in an AAD-Multiplexed access in OE#
	 *	BIT16~19: assertion time for start cycle time in WE#
	 *	BIT23	: add extra half GPMC_FCLK cycle in WE#
	 *	BIT24~28: de-assertion time form start cycle time in WE#
	*/
	*(conf+3) = SET_BIT(0, 2, 12, 16, 18, 28);
	/*
	 * Config 5, RdAccessTime and CycleTime timing parameters configuration.
	 *	BIT0~4	: total READ cycle time
	 *	BIT8~12	: total write cycle time
	 *	BIT16~20: Delay between start cycle time and first data vaild
	 *	BIT24~27: Delay between successive words in multiple access
	*/
	*(conf+4) = SET_BIT(0, 1, 2, 3, 4, 8, 9, 10, 11, 12, 17, 19, 20, 25);
	/*
	 * Config 6, WrAccessTime, WrDataOnADmuxBus, Cycle2Cycle, and BusTurnAround parameters configuration
	 *	BIT0~3	: Bus turn around latency between two successive accesses to the
	 *				same chip-select (read to write) or to a different chip-select (read to
	 *				read and read to write)
	 *	BIT4	: Add Cycle2CycleDelay between two successive accesses to a
	 *				different chip-select (any access type)
	 *	BIT7	: Add Cycle2CycleDelay between two successive accesses to the
	 *				same chip-select (any access type)
	 *	BIT8~11 : Chip-select high pulse delay between two successive accesses
	 *	BIT16~19: Specifies on which GPMC.FCLK rising edge the first data of the
	 *				synchronous burst write is driven in the add/data multiplexed bus.
	 *				Reset value is 0x7.
	 *	BIT24~28: Delay from StartAccessTime to the GPMC.FCLK rising edge
	 *				corresponding the the GPMC.CLK rising edge used by the attached
  	 *				memory for the first data capture.
	 *				Reset value is 0xF.
	*/
	*(conf+5) = SET_BIT(16, 18, 24, 25, 26, 27, 31);
	/*
	 * << The GPMC requare memory block >>
	 * Config 7, chip-select address mapping configuration,
	 *	BIT0~5  : Baseaddr = A24 = 000001 = 32 Mbyte
	 *	BIT6    : chip select enable/disable **must be enable**
	 *	BIT8~11 : Mask Address = 1111 = 16 Mbyte
	*/
	*(conf+6) = SET_BIT(2, 3, 5, 8, 9, 10, 11);
}



