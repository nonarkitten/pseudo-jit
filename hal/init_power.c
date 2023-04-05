#include "init_power.h"

#define PMIC_CNTL_I2C_SLAVE_ADDR       (0x24)

#define PROT_LEVEL_NONE                (0x00)
#define PROT_LEVEL_1                   (0x01)
#define PROT_LEVEL_2                   (0x02)

#define DEFDCDC1                       (0x0E)
#define DEFDCDC2                       (0x0F)
#define DEFDCDC3                       (0x10)

#define DEFSLEW                        (0x11)

#define DCDC_GO                        (0x80)

#define MASK_ALL_BITS                  (0xFF)
#define PASSWORD_UNLOCK                (0x7D)
#define PASSWORD                       (0x0B)

/**
 *  \brief            - Generic function that can write a TPS65217 PMIC
 *                      register or bit field regardless of protection
 *                      level.
 *
 * \param prot_level  - Register password protection.
 *                      use PROT_LEVEL_NONE, PROT_LEVEL_1, or PROT_LEVEL_2
 * \param regOffset:  - Register address to write.
 *
 * \param dest_val    - Value to write.
 *
 * \param mask        - Bit mask (8 bits) to be applied.  Function will only
 *                      change bits that are set in the bit mask.
 *
 * \return:            None.
 */
void TPS65217RegWrite(uint8_t port_level, uint8_t regOffset, uint8_t dest_val, uint8_t mask) {
    uint8_t unlock = regOffset ^ PASSWORD_UNLOCK;
    uint8_t value = 0;

    if (mask != MASK_ALL_BITS) {
        I2C0ReadCmd(PMIC_CNTL_I2C_SLAVE_ADDR, regOffset, &value, 1);
        dest_val = (dest_val & mask) | (value & ~(mask));
    }
    if(port_level > PROT_LEVEL_NONE) {
        I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, PASSWORD, &unlock, 1);
    }
    I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, regOffset, &dest_val, 1);
    if(port_level == PROT_LEVEL_2) {
        I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, PASSWORD, &unlock, 1);
        I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, regOffset, &dest_val, 1);
    }
}

/*************************************************************************
 * Function Name: InitPower
 * Parameters: None
 *
 * Return: None
 *
 * Description: Set VDD1 to 1.28 V
 *************************************************************************/
void InitPower(PowerRail_t rail, double voltage) {
    uint8_t opVolSelector;
    uint8_t opRailSelector;
    uint8_t opVerify;

    switch(rail) {
    case RAIL_DCDC1:
        opRailSelector = DEFDCDC1; if(0)
    case RAIL_DCDC2:
        opRailSelector = DEFDCDC2; if(0)
    default:
    case RAIL_DCDC3:
        opRailSelector = DEFDCDC3;

        // Maximum voltage
        if(voltage >= 3.3) opVolSelector = 0x28;
        // Minimum voltage
        else if(voltage <= 0.9) opVolSelector = 0x00;
        // < 1.5V bits are 0.025V steps
        else if(voltage <= 1.5) opVolSelector = 0x18 - (1.5 - voltage) * 40.0;
        // > 1.5V bits are 0.050V steps
        else opVolSelector = 0x18 + (voltage - 1.5) * 20.0;

        /* set voltage level */
        TPS65217RegWrite(PROT_LEVEL_2, opRailSelector, opVolSelector, MASK_ALL_BITS);
        I2C0ReadCmd(PMIC_CNTL_I2C_SLAVE_ADDR, opRailSelector, &opVerify, 1);
        if(opVerify != opVolSelector) {
            printf("[I2C0] Error setting PMIC voltage\n");
        }

        /* set GO bit to initiate voltage transition */
        TPS65217RegWrite(PROT_LEVEL_2, DEFSLEW, DCDC_GO, DCDC_GO);
    }
}