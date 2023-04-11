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
    static const uint8_t password = PASSWORD;
    uint8_t unlock = regOffset ^ PASSWORD_UNLOCK;
    uint8_t value = 0;

    if (mask != MASK_ALL_BITS) {
        I2C0ReadCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &regOffset, 1, &value, 1);
        dest_val = (dest_val & mask) | (value & ~(mask));
    }
    if(port_level > PROT_LEVEL_NONE) {
        I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &password, 1, &unlock, 1);
    }
    I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &regOffset, 1, &dest_val, 1);
    if(port_level == PROT_LEVEL_2) {
        I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &password, 1, &unlock, 1);
        I2C0SendCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &regOffset, 1, &dest_val, 1);
    }
}

static uint8_t VoltageToDCDC(double voltage) {
    // Maximum voltage
    if(voltage >= 3.3) return 0x28;
    // Minimum voltage
    else if(voltage <= 0.9) return 0x00;
    // < 1.5V bits are 0.025V steps
    else if(voltage <= 1.5) return 0x18 - (1.5 - voltage) * 40.0;
    // > 1.5V bits are 0.050V steps
    else return 0x18 + (voltage - 1.5) * 20.0;
}

static double DCDCtoVoltage(uint8_t reg) {
    if(reg >= 0x28) return 3.3;
    // > 1.5V bits are 0.050V steps
    else if(reg >= 0x18) return 1.5 + (reg - 0x18) * 0.05;
    // < 1.5V bits are 0.025V steps
    else return 1.5 - (0x18 - reg) * 0.025;
}

/*************************************************************************
 * Function Name: InitPower
 * Parameters: None
 *
 * Return: None
 *
 * Description: Set VDD1 to 1.28 V
 *************************************************************************/
int InitPower(PowerRail_t rail, double voltage) {
    static const uint8_t idAddr = 0;
    uint8_t opVolSelector;
    uint8_t opRailSelector;
    uint8_t opVerify;
    uint8_t chipID;

    I2C0ReadCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &idAddr, 1, &chipID, 1);
    if(chipID == 0x72) printf("[I2C0] PMIC ID: TPS65217A\n");
    else if(chipID == 0xF2) printf("[I2C0] PMIC ID: TPS65217B\n");
    else if(chipID == 0xE2) printf("[I2C0] PMIC ID: TPS65217C\n");
    else if(chipID == 0x62) printf("[I2C0] PMIC ID: TPS65217D\n");
    else printf("[I2C0] PMIC ID invalid: %02X\n", chipID);

    switch(rail) {
    case RAIL_DCDC1:
        opRailSelector = DEFDCDC1; if(0)
    case RAIL_DCDC2:
        opRailSelector = DEFDCDC2; if(0)
    default:
    case RAIL_DCDC3:
        opRailSelector = DEFDCDC3;
        opVolSelector = VoltageToDCDC(voltage);

        /* set voltage level */
        TPS65217RegWrite(PROT_LEVEL_2, opRailSelector, opVolSelector, MASK_ALL_BITS);
        /* set GO bit to initiate voltage transition */
        TPS65217RegWrite(PROT_LEVEL_2, DEFSLEW, DCDC_GO, DCDC_GO);

        I2C0ReadCmd(PMIC_CNTL_I2C_SLAVE_ADDR, &opRailSelector, 1, &opVerify, 1);
        float v = DCDCtoVoltage(opVerify);
        printf("[I2C0] PMIC DCDC voltage: %0.2f\n", v);
        return (int)(v * 100.0) == (int)(voltage * 100.0);
    }

}