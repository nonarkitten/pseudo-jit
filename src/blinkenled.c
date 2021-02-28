#include "main.h"

//four leds, 0x0 - 0xf
#define  LED_PATTERN     0x1

#define UINT_PTR 				 uint32_t*
#define write32(ADDR,VAL) 		*((UINT_PTR)(ADDR))=(__builtin_bswap32(VAL))
#define clrbits_le32(ADDR,MASK) *((UINT_PTR)(ADDR))&=~(__builtin_bswap32(MASK))
#define setbits_le32(ADDR,MASK) *((UINT_PTR)(ADDR))|=(__builtin_bswap32(MASK))

// load mmc 1:2 0x98000000 /home/root/test/code.bin; go 0x98000000 
 

void init_led_output() {
    printf("[TEST] Initializing LEDs...\n");
    
    //enable the GPIO module
    //CM_PER Registers -> CM_PER_GPIO1_CLKCTRL
    write32(0x44e00000 + 0xac, (0x2 << 0) | (1 << 18));

    //GPIO1 -> GPIO_IRQSTATUS_CLR_0 (3Ch)
    setbits_le32(0x4804c000 + 0x3c, 0xf << 21);

    //enable output
    //GPIO1 -> GPIO_OE (134h)
    clrbits_le32(0x4804c000 + 0x134, 0xf << 21);
}
 
void heartbeat_forever(void) {
    printf("[TEST] Entering %s\n", __func__ );
    for (;;) {

        //on
        setbits_le32(0x4804c000 + 0x13c, LED_PATTERN << 21);
        putchar('.');
        am335x_dmtimer1_wait_ms(60);

        //off
        clrbits_le32(0x4804c000 + 0x13c, LED_PATTERN << 21);
        am335x_dmtimer1_wait_ms(160); 

        //on
        putchar(':');
        setbits_le32(0x4804c000 + 0x13c, LED_PATTERN << 21);
        am335x_dmtimer1_wait_ms(180);

        //off
        clrbits_le32(0x4804c000 + 0x13c, LED_PATTERN << 21);
        putchar(' ');
        am335x_dmtimer1_wait_ms(340);
    }
}
