/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 */
#include "init_power.h"

#define PMIC_CNTL_I2C_SLAVE_ADDR (0x24)

#define PROT_LEVEL_NONE (0x00)
#define PROT_LEVEL_1 (0x01)
#define PROT_LEVEL_2 (0x02)

#define DEFDCDC1 (0x0E)
#define DEFDCDC2 (0x0F)
#define DEFDCDC3 (0x10)

#define DEFSLEW (0x11)

#define DCDC_GO (0x80)

#define MASK_ALL_BITS (0xFF)
#define PASSWORD_UNLOCK (0x7D)
#define PASSWORD (0x0B)

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
void TPS65217RegWrite(uint8_t port_level, uint8_t regOffset, uint8_t dest_val,
                      uint8_t mask) {
  uint8_t unlock = regOffset ^ PASSWORD_UNLOCK;
  uint8_t value = 0;

  if (mask != MASK_ALL_BITS) {
    am335x_i2c_read(AM335X_I2C0, PMIC_CNTL_I2C_SLAVE_ADDR, regOffset, &value, 1);
    dest_val = (dest_val & mask) | (value & ~(mask));
  }
  if (port_level > PROT_LEVEL_NONE) {
    am335x_i2c_write(AM335X_I2C0, PMIC_CNTL_I2C_SLAVE_ADDR, PASSWORD, &unlock, 1);
  }
  am335x_i2c_write(AM335X_I2C0, PMIC_CNTL_I2C_SLAVE_ADDR, regOffset, &dest_val, 1);
  if (port_level == PROT_LEVEL_2) {
    am335x_i2c_write(AM335X_I2C0, PMIC_CNTL_I2C_SLAVE_ADDR, PASSWORD, &unlock, 1);
    am335x_i2c_write(AM335X_I2C0, PMIC_CNTL_I2C_SLAVE_ADDR, regOffset, &dest_val, 1);
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

  switch (rail) {
    case RAIL_DCDC1:
      opRailSelector = DEFDCDC1;
      if (0) case RAIL_DCDC2:
        opRailSelector = DEFDCDC2;
      if (0) default:
      case RAIL_DCDC3:
        opRailSelector = DEFDCDC3;

      // Maximum voltage
      if (voltage >= 3.3) opVolSelector = 0x28;
      // Minimum voltage
      else if (voltage <= 0.9)
        opVolSelector = 0x00;
      // < 1.5V bits are 0.025V steps
      else if (voltage <= 1.5)
        opVolSelector = 0x18 - (1.5 - voltage) * 40.0;
      // > 1.5V bits are 0.050V steps
      else
        opVolSelector = 0x18 + (voltage - 1.5) * 20.0;

      /* set voltage level */
      TPS65217RegWrite(PROT_LEVEL_2, opRailSelector, opVolSelector,
                       MASK_ALL_BITS);

      /* set GO bit to initiate voltage transition */
      TPS65217RegWrite(PROT_LEVEL_2, DEFSLEW, DCDC_GO, DCDC_GO);
  }
}