#ifndef __MEM_BE_H__
#define __MEM_BE_H__

extern uint8_t read_BE_byte(uint32_t addr);
extern uint16_t read_BE_word(uint32_t addr);
extern uint32_t read_BE_long(uint32_t addr);
extern void write_BE_byte(uint32_t addr, uint32_t data);
extern void write_BE_word(uint32_t addr, uint32_t data);
extern void write_BE_long(uint32_t addr, uint32_t data);

#endif