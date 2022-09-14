#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#ifndef __M68K_COMMON_H__
#define __M68K_COMMON_H__

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

#define __PJIT_BIG_ENDIAN

#ifndef assert
#define assert()
#endif

extern int debug;
extern int lines;

extern char *ldsx(int size);
extern char *ldux(int size);
extern char *stx(int size);
extern char *(*ldx)(int);

#ifndef EXT_WORD_DEFINED
#define EXT_WORD_DEFINED
typedef enum {
	EXT_WORD_SRC_EXT = 0x0100,
	EXT_WORD_SRC_16B = 0x0200,
	EXT_WORD_SRC_32B = 0x0300,
	EXT_WORD_DST_EXT = 0x1000,
	EXT_WORD_DST_16B = 0x2000,
	EXT_WORD_DST_32B = 0x3000,
} ext_word_t;
#endif

typedef enum { 
	// Basic effective addressing modes
	EA_DREG, EA_AREG, EA_ADDR, EA_AINC, EA_ADEC, 
	// Extension word based addressing modes
	EA_ADIS, EA_AIDX, 
	// End of any register-used modes
	EA_ABSW, EA_ABSL, EA_PDIS, EA_PIDX, EA_IMMD
} EA_MODE_t;

void emit(char * format, ...);

void emit_reset(const char * buffer);

void emit_fixup(const char* old, const char* new);

uint16_t lines_ext(uint16_t lines, uint16_t sEA, uint16_t dEA, uint16_t size);

#endif
