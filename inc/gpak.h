#ifndef __GPAK_H__
#define __GPAK_H__

#include "init.h"

#define REG_CONFIG 0x00
#define NVM_CONFIG 0x02
#define EEPROM     0x03

// extern void gpak_init(uint8_t bus, uint8_t slave_address);
// extern uint8_t gpak_i2caddr(uint8_t area);
// extern void gpak_read(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length);
// extern void gpak_write(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length);
// extern void gpak_write_reg(uint8_t reg, uint8_t value);
// extern uint8_t gpak_read_reg(uint8_t reg);
// extern void gpak_erase(uint8_t area);

extern void ManageGP(void);
extern int DetectGP(void);

#endif
