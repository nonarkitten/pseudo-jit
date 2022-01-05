#include "pjit.h"

extern void debug(const char* format,...);

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

// register cpu_t* cpu asm("r12");
cpu_t cpu_state;

//int main(int argc, char** argv) {
int main(void) {
	//uint32_t size = ;
	static uint32_t cache[PJIT_CACHE_SIZE + PJIT_TAG_SIZE];
//	if(!cache) {
//		fprintf( stderr, "Fatal allocation error.\n" );
//		exit(1);
//	}	
//	debug("Cache allocated %d bytes\n", size);
	cache_init((uint32_t)&cache);
	debug("Cache initialized.\n");
	cpu = &cpu_state;
	cpu_dump_state();
	debug("Starting CPU.\n");
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