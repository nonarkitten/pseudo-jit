#ifndef __GPAK_H__
#define __GPAK_H__

#include "init.h"

#define REG_CONFIG 0x00
#define NVM_CONFIG 0x02
#define EEPROM     0x03

extern void ManageGP(void);
extern int DetectGP(void);

#endif
