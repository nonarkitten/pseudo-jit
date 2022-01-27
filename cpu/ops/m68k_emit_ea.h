#include "m68k_common.h"

#ifndef __EMIT_EA_H__
#define __EMIT_EA_H__

// find and optionally load reg, return 1 on success
int emit_get_reg(uint8_t* reg_arm, uint8_t reg_68k, uint8_t size);

// perform source effective address load, return 1 on success
int get_source_data( uint8_t* sRR, uint16_t sEA, uint8_t sR, uint16_t size );

// perform destination effective address calculation and load
// data into temporary register; pass NULL to tRR if no load is needed
int get_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint8_t dR, uint16_t size );

// write data to destination effective address
int set_destination_data( uint8_t* dRR, uint8_t* tRR, uint16_t dEA, uint16_t size );

uint16_t lines_ext(uint16_t lines, uint16_t sEA, uint16_t dEA, uint16_t size) ;

#endif // __EMIT_EA_H__
