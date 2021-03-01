#include "pll.h"

#define OFFSET                      0x44C00000

#define CM_DIV_M2_DPLL_MPU          0x002004A8
#define CM_CLKSEL_DPLL_MPU          0x0020042C
#define CM_CLKMODE_DPLL_MPU         0x00200488
#define CM_IDLEST_DPLL_MPU          0x00200420

#define DPLL_MN_BYP_MODE            4
#define DPLL_LOCK_MODE              7
#define ST_MN_BYPASS                1
#define ST_DPLL_CLK                 1
#define CLKINP                      24

typedef struct Dpll_mpu_struct {
	uint8_t N_divider;
	uint16_t M_multiplier;
	uint8_t M2_divider;
} Dpll_mpu;

static void set_dpll_mpu(Dpll_mpu* dpll_mpu) {	
/* Buggy at the moment
   volatile uint8_t* map = (volatile uint8_t*)OFFSET;
   
   bool pll_needs_lock = (dpll_mpu-> N_divider != (map[CM_CLKSEL_DPLL_MPU] & 0x7f))
       || (dpll_mpu->M_multiplier != (map[CM_CLKSEL_DPLL_MPU + 1] | ((map[CM_CLKSEL_DPLL_MPU + 2] & 0x3f) << 8)));
       
   if(pll_needs_lock) {
       // Switch PLL to bypass mode by setting CM_CLKMODE_DPLL_MPU.DPLL_EN to 0x4
       // This also clears CM_CLKSEL_DPLL_MPU register to 0
       map[CM_CLKMODE_DPLL_MPU] = (map[CM_CLKMODE_DPLL_MPU] & ~7) | DPLL_MN_BYP_MODE;
       putchar('1');

       // Wait for CM_IDLEST_DPLL_MPU.ST_MN_BYPASS = 1 to ensure PLL is in bypass
       while ( !(map[CM_IDLEST_DPLL_MPU+1] & ST_MN_BYPASS) );
       putchar('2');
   
       // Configure Multiply by setting CM_CLKSEL_DPLL_MPU.DPLL_MULT to the desired value.
       map[CM_CLKSEL_DPLL_MPU + 1] = dpll_mpu->M_multiplier & 0xff;
       putchar('3');
       map[CM_CLKSEL_DPLL_MPU + 2] = (dpll_mpu->M_multiplier >> 8) & 0x07;
       putchar('4');

       // Configure Divide by setting CM_CLKSEL_DPLL_MPU.DPLL_DIV to the desired value.
       map[CM_CLKSEL_DPLL_MPU] = dpll_mpu->N_divider & 0x7f;
       putchar('5');
   }
       
   // M2 divider can also be changed on-the-fly; there is no need to put the PLL in bypass and back to lock mode
   // Configure M2 divider by setting CM_DIV_M2_DPLL_MPU.DPLL_CLKOUT_DIV to the desired value.
	if (dpll_mpu-> M2_divider !=  (map[CM_DIV_M2_DPLL_MPU] & 0x1f )) {
       map[CM_DIV_M2_DPLL_MPU] = dpll_mpu->M2_divider & 0x1f;
       putchar('6');
   }

   if(pll_needs_lock) {
       // Switch over to lock mode by setting CM_CLKMODE_DPLL_MPU.DPLL_EN to 0x7.
       map[CM_CLKMODE_DPLL_MPU] = (map[CM_CLKMODE_DPLL_MPU] & ~7) | DPLL_LOCK_MODE;
       putchar('7');
       
       // Wait for CM_IDLEST_DPLL_MPU.ST_DPLL_CLK = 1 to ensure PLL is locked
       while( !(map[CM_IDLEST_DPLL_MPU] & ST_DPLL_CLK) );
       putchar('8'); 
   }
*/
}

static void get_dpll_mpu(Dpll_mpu* dpll_mpu) {
    uint8_t* map = (uint8_t*)OFFSET;
    
	dpll_mpu->N_divider = map[CM_CLKSEL_DPLL_MPU] & 0x7f;
	dpll_mpu->M_multiplier = map[CM_CLKSEL_DPLL_MPU + 1] | ((map[CM_CLKSEL_DPLL_MPU + 2] & 0x07) << 8);
	dpll_mpu->M2_divider = map[CM_DIV_M2_DPLL_MPU] & 0x1f;
}

double get_cpu_mhz(void) {
    Dpll_mpu mpu_pll;
    get_dpll_mpu(&mpu_pll);
    
    double MHz = (1.0 / ((double)mpu_pll.N_divider+1.0)) * \
        (double) mpu_pll.M_multiplier * CLKINP * (1.0 / (double)mpu_pll.M2_divider);

    return MHz;
}
