#include "m68k_common.h"

int debug = 0;
int lines = 0;
char *(*ldx)(int) = ldsx;

enum { STREAM_SET, STREAM_INC, STREAM_GET };

static char * stream(const char * set, int offset, int mode) {
	static volatile char * _stream;
	switch(mode) {
	case 0: _stream = (volatile char *)set; break;
	case 1: _stream += offset; break;
	}
	return (char *)_stream;
}

void emit(char * format, ...) {
	extern int lines;
	char *_stream = stream( NULL, 0, STREAM_GET );
	int len;
	
	va_list args;
	va_start (args, format);
	len = vsprintf(_stream, format, args);
	if(debug) printf("@ %s", _stream);
	// if(_stream[len] != '\n') {
	// 	printf("@ warning, missing carriage return.\n");
	// 	if(!debug) printf("@ %s", _stream);
	// }
	_stream = stream( NULL, len, STREAM_INC );
	va_end (args);
	lines += 1;
}

void emit_reset(const char* buffer) {
	stream( buffer, 0, STREAM_SET );
}

char *ldsx(int size) {
	if(size == 1) return "ldrsb"; else
	if(size == 2) return "ldrsh"; else
				  return "ldr  ";
}

char *ldux(int size) {
	if(size == 1) return "ldrb "; else
	if(size == 2) return "ldrh "; else
				  return "ldr  ";
}

char *stx(int size) {
	if(size == 1) return "strb"; else
	if(size == 2) return "strh"; else
				  return "str ";
}

void emit_fixup(const char* old, const char* new) {
	char *_stream = stream( NULL, 0, STREAM_GET );
	int len = strlen(old);
	int lines = 3;
	while(lines) {
		_stream--;
		if(!memcmp(_stream, old, len)) {
			memcpy(_stream, new, len);
			return;
		}
		if(*_stream == '\n')
			lines--;
	}	
}

uint16_t lines_ext(uint16_t lines, uint16_t sEA, uint16_t dEA, uint16_t size) {
	uint16_t r = lines;
	
	if((sEA == EA_ADIS) || (sEA == EA_PDIS) || (sEA == EA_ABSW) || ((sEA == EA_IMMD) && (size < 4)))
		r |= EXT_WORD_SRC_16B;
	else if((sEA == EA_ABSL) || ((sEA == EA_IMMD) && (size == 4)))
		r |= EXT_WORD_SRC_32B;
	else if((sEA == EA_AIDX) || (sEA == EA_PIDX))
		r |= EXT_WORD_SRC_EXT;
	
	if((dEA == EA_ADIS) || (dEA == EA_PDIS) || (dEA == EA_ABSW) || ((dEA == EA_IMMD) && (size < 4)))
		r |= EXT_WORD_DST_16B;
	else if((dEA == EA_ABSL) || ((dEA == EA_IMMD) && (size == 4)))
		r |= EXT_WORD_DST_32B;
	else if((dEA == EA_AIDX) || (dEA == EA_PIDX))
		r |= EXT_WORD_DST_EXT;
	
	return r;
}
