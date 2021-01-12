#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef __MAKEOPS_H__
#define __MAKEOPS_H__

typedef enum {
    RESET_Initial_SSP,
    RESET_Initial_PC,
    Bus_Error,
    Address_Error,
    Illegal_Instruction,
    Division_by_Zero,
    CHK_Instruction,
    TRAPV_Instruction,
    Privilege_Violation,
    Trace,
    Unimplemented_ALine,
    Unimplemented_FLine,

    Level_1_Autovector = 25,
    Level_2_Autovector,
    Level_3_Autovector,
    Level_4_Autovector,
    Level_5_Autovector,
    Level_6_Autovector,
    Level_7_Autovector,

    Trap_0_Instrucion,
    Trap_1_Instrucion,
    Trap_2_Instrucion,
    Trap_3_Instrucion,
    Trap_4_Instrucion,
    Trap_5_Instrucion,
    Trap_6_Instrucion,
    Trap_7_Instrucion,
    Trap_8_Instrucion,
    Trap_9_Instrucion,
    Trap_10_Instrucion,
    Trap_11_Instrucion,
    Trap_12_Instrucion,
    Trap_13_Instrucion,
    Trap_14_Instrucion,
    Trap_15_Instrucion
} M68_EXCEPTIONS_T;

#endif
