/// @file bbb_tinybasic.c
/// @brief The BASIC interpreter (test_progs/tinybasic.c) running a sieve (test_progs/sieve.tbas: 1000 flags, 303 odd primes) on a BeagleBone: interpretive and cached
///        cycles for the whole run (the guest is the BASIC interpreter, so this is a program-in-an-interpreter-in-a-JIT measurement). `make bbb-bbb_tinybasic`.
#define BBB 1
#define MODE0 0
#define EXPECT "303"
#define PROGRAM "10 REM SIEVE\r20 LET S=1000\r30 FOR I=0 TO S\r40 LET @(I)=1\r50 NEXT I\r60 LET C=0\r70 FOR I=0 TO S\r80 IF @(I)=0 GOTO 150\r90 LET P=I+I+3\r100 LET K=I+P\r110 IF K>S GOTO 140\r120 LET @(K)=0\r130 LET K=K+P\r135 GOTO 110\r140 LET C=C+1\r150 NEXT I\r160 PRINT C\rRUN\rBYE\r"
#include "smoke_tinybasic.c"
