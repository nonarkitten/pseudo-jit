/*
 * mem_be.h — big-endian accessors for code that runs little-endian.
 *
 * These wrap ARMv6+'s SETEND instruction to do one big-endian access and
 * flip straight back to little-endian, for the LE-side (boot/C) code to
 * read/write 68K (BE) memory occasionally without living in BE mode
 * permanently. JIT'd/interpreted 68K code itself is expected to run with
 * the CPU already in BE data mode the whole time instead (see
 * docs/design/OVERVIEW.md) -- these accessors are for the boundary, not the
 * hot path.
 */
#ifndef __MEM_BE_H__
#define __MEM_BE_H__

#include <stdint.h>

extern uint8_t read_BE_byte(uint32_t addr);
extern uint16_t read_BE_word(uint32_t addr);
extern uint32_t read_BE_long(uint32_t addr);
extern void write_BE_byte(uint32_t addr, uint32_t data);
extern void write_BE_word(uint32_t addr, uint32_t data);
extern void write_BE_long(uint32_t addr, uint32_t data);

#endif
