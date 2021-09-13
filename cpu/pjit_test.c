#include "pjit.h"

uint16_t bogomips[] = {
	// 		TST.L	D0
	0x4A80,
	// 		BEQ.S	EXIT
	0x6704,
	// LOOP:
	//		SUBQ	D0, #1
	0x6380,
	//		BNE.S	LOOP
	0x66FC,
	// EXIT:
	//		RTS
	0x4E75,
};

//int main(int argc, char** argv) {
int main(void) {
	void* cache = malloc(sizeof(pjit_cache_t) 
		+ sizeof(pjit_tag_cache_t));
		
	cache_init((uint32_t)cache);

	cpu_dump_state();
	cpu_start(bogomips);
	cpu_dump_state();
}

void _sbrk(void) { }
void _exit(void) { }
void _write(void) { }
void _read(void) { }
void _close(void) { }
void _fstat(void) { }
void _isatty(void) { }
void _lseek(void) { }