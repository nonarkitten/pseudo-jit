/// @file trace_tinybasic.c
/// @brief The sieve in Tiny BASIC on QEMU, interpretive, recording the opcode stream from the moment RUN has been typed (1 M instructions): trace_tinybasic.bin in the working
///        directory, for tools/study/regreuse.py. Not part of `make test`: `make build/trace_tinybasic.elf` and run it under qemu.
#define TRACE2 1
#define MODE0 0
#define EXPECT "303"
#define PROGRAM "10 REM SIEVE\r20 LET S=1000\r30 FOR I=0 TO S\r40 LET @(I)=1\r50 NEXT I\r60 LET C=0\r70 FOR I=0 TO S\r80 IF @(I)=0 GOTO 150\r90 LET P=I+I+3\r100 LET K=I+P\r110 IF K>S GOTO 140\r120 LET @(K)=0\r130 LET K=K+P\r135 GOTO 110\r140 LET C=C+1\r150 NEXT I\r160 PRINT C\rRUN\rBYE\r"
#include "smoke_tinybasic.c"
