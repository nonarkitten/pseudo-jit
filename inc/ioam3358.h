/***************************************************************************
 **
 **    This file defines the Special Function Registers for
 **    Texas Instruments AM3358
 **
 **    Used with ARM IAR C/C++ Compiler and Assembler.
 **
 **    (c) Copyright IAR Systems 2011
 **    (c) Copyright 2022 the Buffee Project
 **
 **    $Revision: 10831 $
 **
 **    Note: Only little endian addressing of 8 bit registers.
 ***************************************************************************/

#ifndef __IOAM3358_H
#define __IOAM3358_H

#include "io_macros.h"

/***************************************************************************
 ***************************************************************************
 **
 **    AM3358 SPECIAL FUNCTION REGISTERS
 **
 ***************************************************************************
 ***************************************************************************/

/* PRUSS_INTC REVID Register */
typedef struct {
  __REG32 REV_MINOR               : 6;
  __REG32 REV_CUSTOM              : 2;
  __REG32 REV_MAJOR               : 3;
  __REG32 REV_RTL                 : 5;
  __REG32 REV_MODULE              :14;
  __REG32 REV_SCHEME              : 2;
} __pruss_intc_revid_bits;

/* PRUSS_INTC CR Register */
typedef struct {
  __REG32                         : 2;
  __REG32 NEST_MODE               : 2;
  __REG32                         :28;
} __pruss_intc_cr_bits;

/* PRUSS_INTC GER Register */
typedef struct {
  __REG32 ENABLE_HINT_ANY         : 1;
  __REG32                         :31;
} __pruss_intc_ger_bits;

/* PRUSS_INTC GNLR Register */
typedef struct {
  __REG32 GLB_NEST_LEVEL          : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_gnlr_bits;

#define PRUSS_INT_INDEX          (0x000003FFUL)

/* PRUSS_INTC GPIR Register */
typedef struct {
  __REG32 GLB_PRI_INTR            :10;
  __REG32                         :21;
  __REG32 GLB_NONE                : 1;
} __pruss_intc_gpir_bits;

/* PRUSS_INTC SRSR0 Register */
typedef struct {
  __REG32 RAW_STATUS_0            : 1;
  __REG32 RAW_STATUS_1            : 1;
  __REG32 RAW_STATUS_2            : 1;
  __REG32 RAW_STATUS_3            : 1;
  __REG32 RAW_STATUS_4            : 1;
  __REG32 RAW_STATUS_5            : 1;
  __REG32 RAW_STATUS_6            : 1;
  __REG32 RAW_STATUS_7            : 1;
  __REG32 RAW_STATUS_8            : 1;
  __REG32 RAW_STATUS_9            : 1;
  __REG32 RAW_STATUS_10           : 1;
  __REG32 RAW_STATUS_11           : 1;
  __REG32 RAW_STATUS_12           : 1;
  __REG32 RAW_STATUS_13           : 1;
  __REG32 RAW_STATUS_14           : 1;
  __REG32 RAW_STATUS_15           : 1;
  __REG32 RAW_STATUS_16           : 1;
  __REG32 RAW_STATUS_17           : 1;
  __REG32 RAW_STATUS_18           : 1;
  __REG32 RAW_STATUS_19           : 1;
  __REG32 RAW_STATUS_20           : 1;
  __REG32 RAW_STATUS_21           : 1;
  __REG32 RAW_STATUS_22           : 1;
  __REG32 RAW_STATUS_23           : 1;
  __REG32 RAW_STATUS_24           : 1;
  __REG32 RAW_STATUS_25           : 1;
  __REG32 RAW_STATUS_26           : 1;
  __REG32 RAW_STATUS_27           : 1;
  __REG32 RAW_STATUS_28           : 1;
  __REG32 RAW_STATUS_29           : 1;
  __REG32 RAW_STATUS_30           : 1;
  __REG32 RAW_STATUS_31           : 1;
} __pruss_intc_srsr0_bits;

/* PRUSS_INTC SRSR1 Register */
typedef struct {
  __REG32 RAW_STATUS_32           : 1;
  __REG32 RAW_STATUS_33           : 1;
  __REG32 RAW_STATUS_34           : 1;
  __REG32 RAW_STATUS_35           : 1;
  __REG32 RAW_STATUS_36           : 1;
  __REG32 RAW_STATUS_37           : 1;
  __REG32 RAW_STATUS_38           : 1;
  __REG32 RAW_STATUS_39           : 1;
  __REG32 RAW_STATUS_40           : 1;
  __REG32 RAW_STATUS_41           : 1;
  __REG32 RAW_STATUS_42           : 1;
  __REG32 RAW_STATUS_43           : 1;
  __REG32 RAW_STATUS_44           : 1;
  __REG32 RAW_STATUS_45           : 1;
  __REG32 RAW_STATUS_46           : 1;
  __REG32 RAW_STATUS_47           : 1;
  __REG32 RAW_STATUS_48           : 1;
  __REG32 RAW_STATUS_49           : 1;
  __REG32 RAW_STATUS_50           : 1;
  __REG32 RAW_STATUS_51           : 1;
  __REG32 RAW_STATUS_52           : 1;
  __REG32 RAW_STATUS_53           : 1;
  __REG32 RAW_STATUS_54           : 1;
  __REG32 RAW_STATUS_55           : 1;
  __REG32 RAW_STATUS_56           : 1;
  __REG32 RAW_STATUS_57           : 1;
  __REG32 RAW_STATUS_58           : 1;
  __REG32 RAW_STATUS_59           : 1;
  __REG32 RAW_STATUS_60           : 1;
  __REG32 RAW_STATUS_61           : 1;
  __REG32 RAW_STATUS_62           : 1;
  __REG32 RAW_STATUS_63           : 1;
} __pruss_intc_srsr1_bits;

/* PRUSS_INTC SECR0 Register */
typedef struct {
  __REG32 ENA_STATUS_0            : 1;
  __REG32 ENA_STATUS_1            : 1;
  __REG32 ENA_STATUS_2            : 1;
  __REG32 ENA_STATUS_3            : 1;
  __REG32 ENA_STATUS_4            : 1;
  __REG32 ENA_STATUS_5            : 1;
  __REG32 ENA_STATUS_6            : 1;
  __REG32 ENA_STATUS_7            : 1;
  __REG32 ENA_STATUS_8            : 1;
  __REG32 ENA_STATUS_9            : 1;
  __REG32 ENA_STATUS_10           : 1;
  __REG32 ENA_STATUS_11           : 1;
  __REG32 ENA_STATUS_12           : 1;
  __REG32 ENA_STATUS_13           : 1;
  __REG32 ENA_STATUS_14           : 1;
  __REG32 ENA_STATUS_15           : 1;
  __REG32 ENA_STATUS_16           : 1;
  __REG32 ENA_STATUS_17           : 1;
  __REG32 ENA_STATUS_18           : 1;
  __REG32 ENA_STATUS_19           : 1;
  __REG32 ENA_STATUS_20           : 1;
  __REG32 ENA_STATUS_21           : 1;
  __REG32 ENA_STATUS_22           : 1;
  __REG32 ENA_STATUS_23           : 1;
  __REG32 ENA_STATUS_24           : 1;
  __REG32 ENA_STATUS_25           : 1;
  __REG32 ENA_STATUS_26           : 1;
  __REG32 ENA_STATUS_27           : 1;
  __REG32 ENA_STATUS_28           : 1;
  __REG32 ENA_STATUS_29           : 1;
  __REG32 ENA_STATUS_30           : 1;
  __REG32 ENA_STATUS_31           : 1;
} __pruss_intc_secr0_bits;

/* PRUSS_INTC SECR1 Register */
typedef struct {
  __REG32 ENA_STATUS_32           : 1;
  __REG32 ENA_STATUS_33           : 1;
  __REG32 ENA_STATUS_34           : 1;
  __REG32 ENA_STATUS_35           : 1;
  __REG32 ENA_STATUS_36           : 1;
  __REG32 ENA_STATUS_37           : 1;
  __REG32 ENA_STATUS_38           : 1;
  __REG32 ENA_STATUS_39           : 1;
  __REG32 ENA_STATUS_40           : 1;
  __REG32 ENA_STATUS_41           : 1;
  __REG32 ENA_STATUS_42           : 1;
  __REG32 ENA_STATUS_43           : 1;
  __REG32 ENA_STATUS_44           : 1;
  __REG32 ENA_STATUS_45           : 1;
  __REG32 ENA_STATUS_46           : 1;
  __REG32 ENA_STATUS_47           : 1;
  __REG32 ENA_STATUS_48           : 1;
  __REG32 ENA_STATUS_49           : 1;
  __REG32 ENA_STATUS_50           : 1;
  __REG32 ENA_STATUS_51           : 1;
  __REG32 ENA_STATUS_52           : 1;
  __REG32 ENA_STATUS_53           : 1;
  __REG32 ENA_STATUS_54           : 1;
  __REG32 ENA_STATUS_55           : 1;
  __REG32 ENA_STATUS_56           : 1;
  __REG32 ENA_STATUS_57           : 1;
  __REG32 ENA_STATUS_58           : 1;
  __REG32 ENA_STATUS_59           : 1;
  __REG32 ENA_STATUS_60           : 1;
  __REG32 ENA_STATUS_61           : 1;
  __REG32 ENA_STATUS_62           : 1;
  __REG32 ENA_STATUS_63           : 1;
} __pruss_intc_secr1_bits;

/* PRUSS_INTC ESR0 Register */
typedef struct {
  __REG32 ENABLE_SET_0            : 1;
  __REG32 ENABLE_SET_1            : 1;
  __REG32 ENABLE_SET_2            : 1;
  __REG32 ENABLE_SET_3            : 1;
  __REG32 ENABLE_SET_4            : 1;
  __REG32 ENABLE_SET_5            : 1;
  __REG32 ENABLE_SET_6            : 1;
  __REG32 ENABLE_SET_7            : 1;
  __REG32 ENABLE_SET_8            : 1;
  __REG32 ENABLE_SET_9            : 1;
  __REG32 ENABLE_SET_10           : 1;
  __REG32 ENABLE_SET_11           : 1;
  __REG32 ENABLE_SET_12           : 1;
  __REG32 ENABLE_SET_13           : 1;
  __REG32 ENABLE_SET_14           : 1;
  __REG32 ENABLE_SET_15           : 1;
  __REG32 ENABLE_SET_16           : 1;
  __REG32 ENABLE_SET_17           : 1;
  __REG32 ENABLE_SET_18           : 1;
  __REG32 ENABLE_SET_19           : 1;
  __REG32 ENABLE_SET_20           : 1;
  __REG32 ENABLE_SET_21           : 1;
  __REG32 ENABLE_SET_22           : 1;
  __REG32 ENABLE_SET_23           : 1;
  __REG32 ENABLE_SET_24           : 1;
  __REG32 ENABLE_SET_25           : 1;
  __REG32 ENABLE_SET_26           : 1;
  __REG32 ENABLE_SET_27           : 1;
  __REG32 ENABLE_SET_28           : 1;
  __REG32 ENABLE_SET_29           : 1;
  __REG32 ENABLE_SET_30           : 1;
  __REG32 ENABLE_SET_31           : 1;
} __pruss_intc_esr0_bits;

/* PRUSS_INTC ERS1 Register */
typedef struct {
  __REG32 ENABLE_SET_32           : 1;
  __REG32 ENABLE_SET_33           : 1;
  __REG32 ENABLE_SET_34           : 1;
  __REG32 ENABLE_SET_35           : 1;
  __REG32 ENABLE_SET_36           : 1;
  __REG32 ENABLE_SET_37           : 1;
  __REG32 ENABLE_SET_38           : 1;
  __REG32 ENABLE_SET_39           : 1;
  __REG32 ENABLE_SET_40           : 1;
  __REG32 ENABLE_SET_41           : 1;
  __REG32 ENABLE_SET_42           : 1;
  __REG32 ENABLE_SET_43           : 1;
  __REG32 ENABLE_SET_44           : 1;
  __REG32 ENABLE_SET_45           : 1;
  __REG32 ENABLE_SET_46           : 1;
  __REG32 ENABLE_SET_47           : 1;
  __REG32 ENABLE_SET_48           : 1;
  __REG32 ENABLE_SET_49           : 1;
  __REG32 ENABLE_SET_50           : 1;
  __REG32 ENABLE_SET_51           : 1;
  __REG32 ENABLE_SET_52           : 1;
  __REG32 ENABLE_SET_53           : 1;
  __REG32 ENABLE_SET_54           : 1;
  __REG32 ENABLE_SET_55           : 1;
  __REG32 ENABLE_SET_56           : 1;
  __REG32 ENABLE_SET_57           : 1;
  __REG32 ENABLE_SET_58           : 1;
  __REG32 ENABLE_SET_59           : 1;
  __REG32 ENABLE_SET_60           : 1;
  __REG32 ENABLE_SET_61           : 1;
  __REG32 ENABLE_SET_62           : 1;
  __REG32 ENABLE_SET_63           : 1;
} __pruss_intc_esr1_bits;

/* PRUSS_INTC ECR0 Register */
typedef struct {
  __REG32 ENABLE_CLR_0            : 1;
  __REG32 ENABLE_CLR_1            : 1;
  __REG32 ENABLE_CLR_2            : 1;
  __REG32 ENABLE_CLR_3            : 1;
  __REG32 ENABLE_CLR_4            : 1;
  __REG32 ENABLE_CLR_5            : 1;
  __REG32 ENABLE_CLR_6            : 1;
  __REG32 ENABLE_CLR_7            : 1;
  __REG32 ENABLE_CLR_8            : 1;
  __REG32 ENABLE_CLR_9            : 1;
  __REG32 ENABLE_CLR_10           : 1;
  __REG32 ENABLE_CLR_11           : 1;
  __REG32 ENABLE_CLR_12           : 1;
  __REG32 ENABLE_CLR_13           : 1;
  __REG32 ENABLE_CLR_14           : 1;
  __REG32 ENABLE_CLR_15           : 1;
  __REG32 ENABLE_CLR_16           : 1;
  __REG32 ENABLE_CLR_17           : 1;
  __REG32 ENABLE_CLR_18           : 1;
  __REG32 ENABLE_CLR_19           : 1;
  __REG32 ENABLE_CLR_20           : 1;
  __REG32 ENABLE_CLR_21           : 1;
  __REG32 ENABLE_CLR_22           : 1;
  __REG32 ENABLE_CLR_23           : 1;
  __REG32 ENABLE_CLR_24           : 1;
  __REG32 ENABLE_CLR_25           : 1;
  __REG32 ENABLE_CLR_26           : 1;
  __REG32 ENABLE_CLR_27           : 1;
  __REG32 ENABLE_CLR_28           : 1;
  __REG32 ENABLE_CLR_29           : 1;
  __REG32 ENABLE_CLR_30           : 1;
  __REG32 ENABLE_CLR_31           : 1;
} __pruss_intc_ecr0_bits;

/* PRUSS_INTC ECR1 Register */
typedef struct {
  __REG32 ENABLE_CLR_32           : 1;
  __REG32 ENABLE_CLR_33           : 1;
  __REG32 ENABLE_CLR_34           : 1;
  __REG32 ENABLE_CLR_35           : 1;
  __REG32 ENABLE_CLR_36           : 1;
  __REG32 ENABLE_CLR_37           : 1;
  __REG32 ENABLE_CLR_38           : 1;
  __REG32 ENABLE_CLR_39           : 1;
  __REG32 ENABLE_CLR_40           : 1;
  __REG32 ENABLE_CLR_41           : 1;
  __REG32 ENABLE_CLR_42           : 1;
  __REG32 ENABLE_CLR_43           : 1;
  __REG32 ENABLE_CLR_44           : 1;
  __REG32 ENABLE_CLR_45           : 1;
  __REG32 ENABLE_CLR_46           : 1;
  __REG32 ENABLE_CLR_47           : 1;
  __REG32 ENABLE_CLR_48           : 1;
  __REG32 ENABLE_CLR_49           : 1;
  __REG32 ENABLE_CLR_50           : 1;
  __REG32 ENABLE_CLR_51           : 1;
  __REG32 ENABLE_CLR_52           : 1;
  __REG32 ENABLE_CLR_53           : 1;
  __REG32 ENABLE_CLR_54           : 1;
  __REG32 ENABLE_CLR_55           : 1;
  __REG32 ENABLE_CLR_56           : 1;
  __REG32 ENABLE_CLR_57           : 1;
  __REG32 ENABLE_CLR_58           : 1;
  __REG32 ENABLE_CLR_59           : 1;
  __REG32 ENABLE_CLR_60           : 1;
  __REG32 ENABLE_CLR_61           : 1;
  __REG32 ENABLE_CLR_62           : 1;
  __REG32 ENABLE_CLR_63           : 1;
} __pruss_intc_ecr1_bits;

#define   PRUSS_INT_0            (0x1UL<<0)
#define   PRUSS_INT_1            (0x1UL<<1)
#define   PRUSS_INT_2            (0x1UL<<2)
#define   PRUSS_INT_3            (0x1UL<<3)
#define   PRUSS_INT_4            (0x1UL<<4)
#define   PRUSS_INT_5            (0x1UL<<5)
#define   PRUSS_INT_6            (0x1UL<<6)
#define   PRUSS_INT_7            (0x1UL<<7)
#define   PRUSS_INT_8            (0x1UL<<8)
#define   PRUSS_INT_9            (0x1UL<<9)
#define   PRUSS_INT_10           (0x1UL<<10)
#define   PRUSS_INT_11           (0x1UL<<11)
#define   PRUSS_INT_12           (0x1UL<<12)
#define   PRUSS_INT_13           (0x1UL<<13)
#define   PRUSS_INT_14           (0x1UL<<14)
#define   PRUSS_INT_15           (0x1UL<<15)
#define   PRUSS_INT_16           (0x1UL<<16)
#define   PRUSS_INT_17           (0x1UL<<17)
#define   PRUSS_INT_18           (0x1UL<<18)
#define   PRUSS_INT_19           (0x1UL<<19)
#define   PRUSS_INT_20           (0x1UL<<20)
#define   PRUSS_INT_21           (0x1UL<<21)
#define   PRUSS_INT_22           (0x1UL<<22)
#define   PRUSS_INT_23           (0x1UL<<23)
#define   PRUSS_INT_24           (0x1UL<<24)
#define   PRUSS_INT_25           (0x1UL<<25)
#define   PRUSS_INT_26           (0x1UL<<26)
#define   PRUSS_INT_27           (0x1UL<<27)
#define   PRUSS_INT_28           (0x1UL<<28)
#define   PRUSS_INT_29           (0x1UL<<29)
#define   PRUSS_INT_30           (0x1UL<<30)
#define   PRUSS_INT_31           (0x1UL<<31)
#define   PRUSS_INT_32           (0x1UL<<0)
#define   PRUSS_INT_33           (0x1UL<<1)
#define   PRUSS_INT_34           (0x1UL<<2)
#define   PRUSS_INT_35           (0x1UL<<3)
#define   PRUSS_INT_36           (0x1UL<<4)
#define   PRUSS_INT_37           (0x1UL<<5)
#define   PRUSS_INT_38           (0x1UL<<6)
#define   PRUSS_INT_39           (0x1UL<<7)
#define   PRUSS_INT_40           (0x1UL<<8)
#define   PRUSS_INT_41           (0x1UL<<9)
#define   PRUSS_INT_42           (0x1UL<<10)
#define   PRUSS_INT_43           (0x1UL<<11)
#define   PRUSS_INT_44           (0x1UL<<12)
#define   PRUSS_INT_45           (0x1UL<<13)
#define   PRUSS_INT_46           (0x1UL<<14)
#define   PRUSS_INT_47           (0x1UL<<15)
#define   PRUSS_INT_48           (0x1UL<<16)
#define   PRUSS_INT_49           (0x1UL<<17)
#define   PRUSS_INT_50           (0x1UL<<18)
#define   PRUSS_INT_51           (0x1UL<<19)
#define   PRUSS_INT_52           (0x1UL<<20)
#define   PRUSS_INT_53           (0x1UL<<21)
#define   PRUSS_INT_54           (0x1UL<<22)
#define   PRUSS_INT_55           (0x1UL<<23)
#define   PRUSS_INT_56           (0x1UL<<24)
#define   PRUSS_INT_57           (0x1UL<<25)
#define   PRUSS_INT_58           (0x1UL<<26)
#define   PRUSS_INT_59           (0x1UL<<27)
#define   PRUSS_INT_60           (0x1UL<<28)
#define   PRUSS_INT_61           (0x1UL<<29)
#define   PRUSS_INT_62           (0x1UL<<30)
#define   PRUSS_INT_63           (0x1UL<<31)

/* PRUSS_INTC CMR0 Register */
typedef struct {
  __REG32 CH_MAP_0                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_1                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_2                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_3                : 4;
  __REG32                         : 4;
} __pruss_intc_cmr0_bits;

/* PRUSS_INTC CMR1 Register */
typedef struct {
  __REG32 CH_MAP_4                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_5                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_6                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_7                : 4;
  __REG32                         : 4;
} __pruss_intc_cmr1_bits;

/* PRUSS_INTC CMR2 Register */
typedef struct {
  __REG32 CH_MAP_8                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_9                : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_10               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_11               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr2_bits;

/* PRUSS_INTC CMR3 Register */
typedef struct {
  __REG32 CH_MAP_12               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_13               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_14               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_15               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr3_bits;

/* PRUSS_INTC CMR4 Register */
typedef struct {
  __REG32 CH_MAP_16               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_17               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_18               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_19               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr4_bits;

/* PRUSS_INTC CMR5 Register */
typedef struct {
  __REG32 CH_MAP_20               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_21               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_22               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_23               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr5_bits;

/* PRUSS_INTC CMR6 Register */
typedef struct {
  __REG32 CH_MAP_24               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_25               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_26               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_27               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr6_bits;

/* PRUSS_INTC CMR7 Register */
typedef struct {
  __REG32 CH_MAP_28               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_29               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_30               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_31               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr7_bits;

/* PRUSS_INTC CMR8 Register */
typedef struct {
  __REG32 CH_MAP_32               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_33               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_34               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_35               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr8_bits;

/* PRUSS_INTC CMR9 Register */
typedef struct {
  __REG32 CH_MAP_36               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_37               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_38               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_39               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr9_bits;

/* PRUSS_INTC CMR10 Register */
typedef struct {
  __REG32 CH_MAP_40               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_41               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_42               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_43               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr10_bits;

/* PRUSS_INTC CMR11 Register */
typedef struct {
  __REG32 CH_MAP_44               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_45               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_46               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_47               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr11_bits;

/* PRUSS_INTC CMR12 Register */
typedef struct {
  __REG32 CH_MAP_48               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_49               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_50               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_51               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr12_bits;

/* PRUSS_INTC CMR13 Register */
typedef struct {
  __REG32 CH_MAP_52               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_53               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_54               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_55               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr13_bits;

/* PRUSS_INTC CMR14 Register */
typedef struct {
  __REG32 CH_MAP_56               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_57               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_58               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_59               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr14_bits;

/* PRUSS_INTC CMR15 Register */
typedef struct {
  __REG32 CH_MAP_60               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_61               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_62               : 4;
  __REG32                         : 4;
  __REG32 CH_MAP_63               : 4;
  __REG32                         : 4;
} __pruss_intc_cmr15_bits;

/* PRUSS_INTC HMR0 Register */
typedef struct {
  __REG32 HINT_MAP_0              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_1              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_2              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_3              : 4;
  __REG32                         : 4;
} __pruss_intc_hmr0_bits;

/* PRUSS_INTC HMR1 Register */
typedef struct {
  __REG32 HINT_MAP_4              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_5              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_6              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_7              : 4;
  __REG32                         : 4;
} __pruss_intc_hmr1_bits;

/* PRUSS_INTC HMR2 Register */
typedef struct {
  __REG32 HINT_MAP_8              : 4;
  __REG32                         : 4;
  __REG32 HINT_MAP_9              : 4;
  __REG32                         :20;
} __pruss_intc_hmr2_bits;

/* PRUSS_INTC HIPIR0 Register */
typedef struct {
  __REG32 PRI_HINT_0              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_0             : 1;
} __pruss_intc_hipir0_bits;

/* PRUSS_INTC HIPIR1 Register */
typedef struct {
  __REG32 PRI_HINT_1              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_1             : 1;
} __pruss_intc_hipir1_bits;

/* PRUSS_INTC HIPIR2 Register */
typedef struct {
  __REG32 PRI_HINT_2              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_2             : 1;
} __pruss_intc_hipir2_bits;

/* PRUSS_INTC HIPIR3 Register */
typedef struct {
  __REG32 PRI_HINT_3              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_3             : 1;
} __pruss_intc_hipir3_bits;

/* PRUSS_INTC HIPIR4 Register */
typedef struct {
  __REG32 PRI_HINT_4              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_4             : 1;
} __pruss_intc_hipir4_bits;

/* PRUSS_INTC HIPIR5 Register */
typedef struct {
  __REG32 PRI_HINT_5              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_5             : 1;
} __pruss_intc_hipir5_bits;

/* PRUSS_INTC HIPIR6 Register */
typedef struct {
  __REG32 PRI_HINT_6              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_6             : 1;
} __pruss_intc_hipir6_bits;

/* PRUSS_INTC HIPIR7 Register */
typedef struct {
  __REG32 PRI_HINT_7              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_7             : 1;
} __pruss_intc_hipir7_bits;

/* PRUSS_INTC HIPIR8 Register */
typedef struct {
  __REG32 PRI_HINT_8              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_8             : 1;
} __pruss_intc_hipir8_bits;

/* PRUSS_INTC HIPIR9 Register */
typedef struct {
  __REG32 PRI_HINT_9              :10;
  __REG32                         :21;
  __REG32 NONE_HINT_9             : 1;
} __pruss_intc_hipir9_bits;

/* PRUSS_INTC SIPR0 Register */
typedef struct {
  __REG32 POLARITY_0              : 1;
  __REG32 POLARITY_1              : 1;
  __REG32 POLARITY_2              : 1;
  __REG32 POLARITY_3              : 1;
  __REG32 POLARITY_4              : 1;
  __REG32 POLARITY_5              : 1;
  __REG32 POLARITY_6              : 1;
  __REG32 POLARITY_7              : 1;
  __REG32 POLARITY_8              : 1;
  __REG32 POLARITY_9              : 1;
  __REG32 POLARITY_10             : 1;
  __REG32 POLARITY_11             : 1;
  __REG32 POLARITY_12             : 1;
  __REG32 POLARITY_13             : 1;
  __REG32 POLARITY_14             : 1;
  __REG32 POLARITY_15             : 1;
  __REG32 POLARITY_16             : 1;
  __REG32 POLARITY_17             : 1;
  __REG32 POLARITY_18             : 1;
  __REG32 POLARITY_19             : 1;
  __REG32 POLARITY_20             : 1;
  __REG32 POLARITY_21             : 1;
  __REG32 POLARITY_22             : 1;
  __REG32 POLARITY_23             : 1;
  __REG32 POLARITY_24             : 1;
  __REG32 POLARITY_25             : 1;
  __REG32 POLARITY_26             : 1;
  __REG32 POLARITY_27             : 1;
  __REG32 POLARITY_28             : 1;
  __REG32 POLARITY_29             : 1;
  __REG32 POLARITY_30             : 1;
  __REG32 POLARITY_31             : 1;
} __pruss_intc_sipr0_bits;

/* PRUSS_INTC SIPR1 Register */
typedef struct {
  __REG32 POLARITY_32             : 1;
  __REG32 POLARITY_33             : 1;
  __REG32 POLARITY_34             : 1;
  __REG32 POLARITY_35             : 1;
  __REG32 POLARITY_36             : 1;
  __REG32 POLARITY_37             : 1;
  __REG32 POLARITY_38             : 1;
  __REG32 POLARITY_39             : 1;
  __REG32 POLARITY_40             : 1;
  __REG32 POLARITY_41             : 1;
  __REG32 POLARITY_42             : 1;
  __REG32 POLARITY_43             : 1;
  __REG32 POLARITY_44             : 1;
  __REG32 POLARITY_45             : 1;
  __REG32 POLARITY_46             : 1;
  __REG32 POLARITY_47             : 1;
  __REG32 POLARITY_48             : 1;
  __REG32 POLARITY_49             : 1;
  __REG32 POLARITY_50             : 1;
  __REG32 POLARITY_51             : 1;
  __REG32 POLARITY_52             : 1;
  __REG32 POLARITY_53             : 1;
  __REG32 POLARITY_54             : 1;
  __REG32 POLARITY_55             : 1;
  __REG32 POLARITY_56             : 1;
  __REG32 POLARITY_57             : 1;
  __REG32 POLARITY_58             : 1;
  __REG32 POLARITY_59             : 1;
  __REG32 POLARITY_60             : 1;
  __REG32 POLARITY_61             : 1;
  __REG32 POLARITY_62             : 1;
  __REG32 POLARITY_63             : 1;
} __pruss_intc_sipr1_bits;

/* PRUSS_INTC SITR0 Register */
typedef struct {
  __REG32 TYPE_0                  : 1;
  __REG32 TYPE_1                  : 1;
  __REG32 TYPE_2                  : 1;
  __REG32 TYPE_3                  : 1;
  __REG32 TYPE_4                  : 1;
  __REG32 TYPE_5                  : 1;
  __REG32 TYPE_6                  : 1;
  __REG32 TYPE_7                  : 1;
  __REG32 TYPE_8                  : 1;
  __REG32 TYPE_9                  : 1;
  __REG32 TYPE_10                 : 1;
  __REG32 TYPE_11                 : 1;
  __REG32 TYPE_12                 : 1;
  __REG32 TYPE_13                 : 1;
  __REG32 TYPE_14                 : 1;
  __REG32 TYPE_15                 : 1;
  __REG32 TYPE_16                 : 1;
  __REG32 TYPE_17                 : 1;
  __REG32 TYPE_18                 : 1;
  __REG32 TYPE_19                 : 1;
  __REG32 TYPE_20                 : 1;
  __REG32 TYPE_21                 : 1;
  __REG32 TYPE_22                 : 1;
  __REG32 TYPE_23                 : 1;
  __REG32 TYPE_24                 : 1;
  __REG32 TYPE_25                 : 1;
  __REG32 TYPE_26                 : 1;
  __REG32 TYPE_27                 : 1;
  __REG32 TYPE_28                 : 1;
  __REG32 TYPE_29                 : 1;
  __REG32 TYPE_30                 : 1;
  __REG32 TYPE_31                 : 1;
} __pruss_intc_sitr0_bits;

/* PRUSS_INTC SITR1 Register */
typedef struct {
  __REG32 TYPE_32                 : 1;
  __REG32 TYPE_33                 : 1;
  __REG32 TYPE_34                 : 1;
  __REG32 TYPE_35                 : 1;
  __REG32 TYPE_36                 : 1;
  __REG32 TYPE_37                 : 1;
  __REG32 TYPE_38                 : 1;
  __REG32 TYPE_39                 : 1;
  __REG32 TYPE_40                 : 1;
  __REG32 TYPE_41                 : 1;
  __REG32 TYPE_42                 : 1;
  __REG32 TYPE_43                 : 1;
  __REG32 TYPE_44                 : 1;
  __REG32 TYPE_45                 : 1;
  __REG32 TYPE_46                 : 1;
  __REG32 TYPE_47                 : 1;
  __REG32 TYPE_48                 : 1;
  __REG32 TYPE_49                 : 1;
  __REG32 TYPE_50                 : 1;
  __REG32 TYPE_51                 : 1;
  __REG32 TYPE_52                 : 1;
  __REG32 TYPE_53                 : 1;
  __REG32 TYPE_54                 : 1;
  __REG32 TYPE_55                 : 1;
  __REG32 TYPE_56                 : 1;
  __REG32 TYPE_57                 : 1;
  __REG32 TYPE_58                 : 1;
  __REG32 TYPE_59                 : 1;
  __REG32 TYPE_60                 : 1;
  __REG32 TYPE_61                 : 1;
  __REG32 TYPE_62                 : 1;
  __REG32 TYPE_63                 : 1;
} __pruss_intc_sitr1_bits;

/* PRUSS_INTC HINLR0 Register */
typedef struct {
  __REG32 NEST_HINT_0             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr0_bits;

/* PRUSS_INTC HINLR1 Register */
typedef struct {
  __REG32 NEST_HINT_1             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr1_bits;

/* PRUSS_INTC HINLR2 Register */
typedef struct {
  __REG32 NEST_HINT_2             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr2_bits;

/* PRUSS_INTC HINLR3 Register */
typedef struct {
  __REG32 NEST_HINT_3             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr3_bits;

/* PRUSS_INTC HINLR4 Register */
typedef struct {
  __REG32 NEST_HINT_4             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr4_bits;

/* PRUSS_INTC HINLR5 Register */
typedef struct {
  __REG32 NEST_HINT_5             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr5_bits;

/* PRUSS_INTC HINLR6 Register */
typedef struct {
  __REG32 NEST_HINT_6             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr6_bits;

/* PRUSS_INTC HINLR7 Register */
typedef struct {
  __REG32 NEST_HINT_7             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr7_bits;

/* PRUSS_INTC HINLR8 Register */
typedef struct {
  __REG32 NEST_HINT_8             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr8_bits;

/* PRUSS_INTC HINLR9 Register */
typedef struct {
  __REG32 NEST_HINT_9             : 9;
  __REG32                         :22;
  __REG32 AUTO_OVERRIDE           : 1;
} __pruss_intc_hinlr9_bits;


/* PRUSS_INTC HIER Register */
typedef struct {
  __REG32 ENABLE_HINT0            : 1;
  __REG32 ENABLE_HINT1            : 1;
  __REG32 ENABLE_HINT2            : 1;
  __REG32 ENABLE_HINT3            : 1;
  __REG32 ENABLE_HINT4            : 1;
  __REG32 ENABLE_HINT5            : 1;
  __REG32 ENABLE_HINT6            : 1;
  __REG32 ENABLE_HINT7            : 1;
  __REG32 ENABLE_HINT8            : 1;
  __REG32 ENABLE_HINT9            : 1;
  __REG32                         :22;
} __pruss_intc_hier_bits;

/* PRU0/PRU1 CONTROL Register */
typedef struct {
  __REG32 SOFT_RST_N              : 1;
  __REG32 ENABLE                  : 1;
  __REG32 SLEEPING                : 1;
  __REG32 COUNTER_ENABLE          : 1;
  __REG32                         : 4;
  __REG32 SINGLE_STEP             : 1;
  __REG32                         : 6;
  __REG32 RUNSTATE                : 1;
  __REG32 PCOUNTER_RST_VAL        :16;
} __pru_ctrl_bits;

/* PRU0/PRU1 STATUS Register */
typedef struct {
  __REG32 PCOUNTER                :16;
  __REG32                         :16;
} __pru_status_bits;

/* PRU0/PRU1 WAKEUP_EN Register */
typedef struct {
  __REG32 BITWISE_ENABLE0         : 1;
  __REG32 BITWISE_ENABLE1         : 1;
  __REG32 BITWISE_ENABLE2         : 1;
  __REG32 BITWISE_ENABLE3         : 1;
  __REG32 BITWISE_ENABLE4         : 1;
  __REG32 BITWISE_ENABLE5         : 1;
  __REG32 BITWISE_ENABLE6         : 1;
  __REG32 BITWISE_ENABLE7         : 1;
  __REG32 BITWISE_ENABLE8         : 1;
  __REG32 BITWISE_ENABLE9         : 1;
  __REG32 BITWISE_ENABLE10        : 1;
  __REG32 BITWISE_ENABLE11        : 1;
  __REG32 BITWISE_ENABLE12        : 1;
  __REG32 BITWISE_ENABLE13        : 1;
  __REG32 BITWISE_ENABLE14        : 1;
  __REG32 BITWISE_ENABLE15        : 1;
  __REG32 BITWISE_ENABLE16        : 1;
  __REG32 BITWISE_ENABLE17        : 1;
  __REG32 BITWISE_ENABLE18        : 1;
  __REG32 BITWISE_ENABLE19        : 1;
  __REG32 BITWISE_ENABLE20        : 1;
  __REG32 BITWISE_ENABLE21        : 1;
  __REG32 BITWISE_ENABLE22        : 1;
  __REG32 BITWISE_ENABLE23        : 1;
  __REG32 BITWISE_ENABLE24        : 1;
  __REG32 BITWISE_ENABLE25        : 1;
  __REG32 BITWISE_ENABLE26        : 1;
  __REG32 BITWISE_ENABLE27        : 1;
  __REG32 BITWISE_ENABLE28        : 1;
  __REG32 BITWISE_ENABLE29        : 1;
  __REG32 BITWISE_ENABLE30        : 1;
  __REG32 BITWISE_ENABLE31        : 1;
} __pru_wakeup_en_bits;

/* PRU0/PRU1 CTBIR0 Register */
typedef struct {
  __REG32 C24_BLK_INDEX           : 8;
  __REG32                         : 8;
  __REG32 C25_BLK_INDEX           : 8;
  __REG32                         : 8;
} __pru_ctbir0_bits;

/* PRU0/PRU1 CTBIR1 Register */
typedef struct {
  __REG32 C26_BLK_INDEX           : 8;
  __REG32                         : 8;
  __REG32 C27_BLK_INDEX           : 8;
  __REG32                         : 8;
} __pru_ctbir1_bits;

/* PRU0/PRU1 CTPPR0 Register */
typedef struct {
  __REG32 C28_POINTER             :16;
  __REG32 C29_POINTER             :16;
} __pru_ctppr0_bits;

/* PRU0/PRU1 CTPPR1 Register */
typedef struct {
  __REG32 C30_POINTER             :16;
  __REG32 C31_POINTER             :16;
} __pru_ctppr1_bits;

/* PRUSS UART Receiver Buffer Register (RBR) */
/* PRUSS UART Transmitter Holding Register (THR) */
typedef struct {
  __REG32 DATA                    : 8;
  __REG32                         :24;
} __pruss_uart_rbr_bits;

/* PRUSS UART Interrupt Enable Register (IER) */
typedef struct {
  __REG32 ERBI                    : 1;
  __REG32 ETBEI                   : 1;
  __REG32 ELSI                    : 1;
  __REG32 EDSSI                   : 1;
  __REG32                         :28;
} __pruss_uart_ier_bits;

/* PRUSS UART Interrupt Identification Register (IIR) */
/* PRUSS UART FIFO Control Register (FCR) */
typedef union {
  /*PRUSS_UART0_IIR*/
  struct {
    __REG32 IPEND                   : 1;
    __REG32 INTID                   : 3;
    __REG32                         : 2;
    __REG32 FIFOEN                  : 2;
    __REG32                         :24;
  };
  /*PRUSS_UART0_FCR*/
  struct {
    __REG32 FIFOEN                  : 1;
    __REG32 RXCLR                   : 1;
    __REG32 TXCLR                   : 1;
    __REG32 DMAMODE1                : 1;
    __REG32                         : 2;
    __REG32 RXFIFTL                 : 2;
    __REG32                         :24;
  }__fcr_bits;
} __pruss_uart_iir_bits;

/* PRUSS UART Line Control Register (LCR) */
typedef struct {
  __REG32 WLS                     : 2;
  __REG32 STB                     : 1;
  __REG32 PEN                     : 1;
  __REG32 EPS                     : 1;
  __REG32 SP                      : 1;
  __REG32 BC                      : 1;
  __REG32 DLAB                    : 1;
  __REG32                         :24;
} __pruss_uart_lcr_bits;

/* PRUSS UART Modem Control Register (MCR) */
typedef struct {
  __REG32                         : 1;
  __REG32 RTS                     : 1;
  __REG32 OUT1                    : 1;
  __REG32 OUT2                    : 1;
  __REG32 LOOP                    : 1;
  __REG32 AFE                     : 1;
  __REG32                         :26;
} __pruss_uart_mcr_bits;

/* PRUSS UART Line Status Register (LSR) */
typedef struct {
  __REG32 DR                      : 1;
  __REG32 OE                      : 1;
  __REG32 PE                      : 1;
  __REG32 FE                      : 1;
  __REG32 BI                      : 1;
  __REG32 THRE                    : 1;
  __REG32 TEMT                    : 1;
  __REG32 RXFIFOE                 : 1;
  __REG32                         :24;
} __pruss_uart_lsr_bits;

/* PRUSS UART Modem Status Register (MSR) */
typedef struct {
  __REG32 DCTS                    : 1;
  __REG32 DDSR                    : 1;
  __REG32 TERI                    : 1;
  __REG32 DCD                     : 1;
  __REG32 CTS                     : 1;
  __REG32 DSR                     : 1;
  __REG32 RI                      : 1;
  __REG32 CD                      : 1;
  __REG32                         :24;
} __pruss_uart_msr_bits;

/* PRUSS UART Scratch Pad Register (SCR) */
typedef struct {
  __REG32 SCR                     : 8;
  __REG32                         :24;
} __pruss_uart_scr_bits;

/* PRUSS UART Divisor Latches (DLL) */
typedef struct {
  __REG32 DLL                     : 8;
  __REG32                         :24;
} __pruss_uart_dll_bits;

/* PRUSS UART Divisor Latches (DLH) */
typedef struct {
  __REG32 DLH                     : 8;
  __REG32                         :24;
} __pruss_uart_dlh_bits;

/* PRUSS UART Revision Identification Register 2 (REVID2) */
typedef struct {
  __REG32 REVID2                  : 8;
  __REG32                         :24;
} __pruss_uart_revid2_bits;

/* PRUSS UART Power and Emulation Management Register (PWREMU_MGMT) */
typedef struct {
  __REG32 FREE                    : 1;
  __REG32                         :12;
  __REG32 URRST                   : 1;
  __REG32 UTRST                   : 1;
  __REG32                         :17;
} __pruss_uart_pwremu_mgmt_bits;

/* PRUSS UART Mode Definition Register (MDR) */
typedef struct {
  __REG32 OSM_SEL                 : 1;
  __REG32                         :31;
} __pruss_uart_mdr_bits;

/* ECAP Control Register 1 (ECCTL1) */
typedef struct {
  __REG16 CAP1POL                 : 1;
  __REG16 CTRRST1                 : 1;
  __REG16 CAP2POL                 : 1;
  __REG16 CTRRST2                 : 1;
  __REG16 CAP3POL                 : 1;
  __REG16 CTRRST3                 : 1;
  __REG16 CAP4POL                 : 1;
  __REG16 CTRRST4                 : 1;
  __REG16 CAPLDEN                 : 1;
  __REG16 PRESCALE                : 5;
  __REG16 FREE_SOFT               : 2;
} __ecap_ecctl1_bits;

/* ECAP Control Register 2 (ECCTL2) */
typedef struct {
  __REG16 CONT_ONESHT             : 1;
  __REG16 STOP_WRAP               : 2;
  __REG16 REARM                   : 1;
  __REG16 TSCTRSTOP               : 1;
  __REG16 SYNCI_EN                : 1;
  __REG16 SYNCO_SEL               : 2;
  __REG16 SWSYNC                  : 1;
  __REG16 CAP_APWM                : 1;
  __REG16 APWMPOL                 : 1;
  __REG16                         : 5;
} __ecap_ecctl2_bits;

/* ECAP Interrupt Enable Register (ECEINT) */
/* ECAP Interrupt Forcing Register (ECFRC) */
typedef struct {
  __REG16                         : 1;
  __REG16 CEVT1                   : 1;
  __REG16 CEVT2                   : 1;
  __REG16 CEVT3                   : 1;
  __REG16 CEVT4                   : 1;
  __REG16 CTROVF                  : 1;
  __REG16 CTR_PRD                 : 1;
  __REG16 CTR_CMP                 : 1;
  __REG16                         : 8;
} __ecap_eceint_bits;

/* ECAP Interrupt Flag Register (ECFLG) */
/* ECAP Interrupt Clear Register (ECCLR) */
typedef struct {
  __REG16 INT                     : 1;
  __REG16 CEVT1                   : 1;
  __REG16 CEVT2                   : 1;
  __REG16 CEVT3                   : 1;
  __REG16 CEVT4                   : 1;
  __REG16 CTROVF                  : 1;
  __REG16 CTR_PRD                 : 1;
  __REG16 CTR_CMP                 : 1;
  __REG16                         : 8;
} __ecap_ecflg_bits;

/* PRUSS CFG SYSCFG Register */
typedef struct {
  __REG32 IDLE_MODE               : 2;
  __REG32 STANDBY_MODE            : 2;
  __REG32 STANDBY_INIT            : 1;
  __REG32 SUB_MWAIT               : 1;
  __REG32                         :26;
} __pruss_cfg_syscfg_bits;

/* PRUSS CFG GPCFG0 Register */
typedef struct {
  __REG32 PRU0_GPI_MODE           : 2;
  __REG32 PRU0_GPI_CLK_MODE       : 1;
  __REG32 PRU0_GPI_DIV0           : 5;
  __REG32 PRU0_GPI_DIV1           : 5;
  __REG32 PRU0_GPI_SB             : 1;
  __REG32 PRU0_GPO_MODE           : 1;
  __REG32 PRU0_GPO_DIV0           : 5;
  __REG32 PRU0_GPO_DIV1           : 5;
  __REG32 PRU0_GPO_SH_SEL         : 1;
  __REG32                         : 6;
} __pruss_cfg_gpcfg0_bits;

/* PRUSS CFG GPCFG1 Register */
typedef struct {
  __REG32 PRU1_GPI_MODE           : 2;
  __REG32 PRU1_GPI_CLK_MODE       : 1;
  __REG32 PRU1_GPI_DIV0           : 5;
  __REG32 PRU1_GPI_DIV1           : 5;
  __REG32 PRU1_GPI_SB             : 1;
  __REG32 PRU1_GPO_MODE           : 1;
  __REG32 PRU1_GPO_DIV0           : 5;
  __REG32 PRU1_GPO_DIV1           : 5;
  __REG32 PRU1_GPO_SH_SEL         : 1;
  __REG32                         : 6;
} __pruss_cfg_gpcfg1_bits;

/* PRUSS CFG CGR Register */
typedef struct {
  __REG32 PRU0_CLK_STOP_REQ       : 1;
  __REG32 PRU0_CLK_STOP_ACK       : 1;
  __REG32 PRU0_CLK_EN             : 1;
  __REG32 PRU1_CLK_STOP_REQ       : 1;
  __REG32 PRU1_CLK_STOP_ACK       : 1;
  __REG32 PRU1_CLK_EN             : 1;
  __REG32 INTC_CLK_STOP_REQ       : 1;
  __REG32 INTC_CLK_STOP_ACK       : 1;
  __REG32 INTC_CLK_EN             : 1;
  __REG32 UART_CLK_STOP_REQ       : 1;
  __REG32 UART_CLK_STOP_ACK       : 1;
  __REG32 UART_CLK_EN             : 1;
  __REG32 ECAP_CLK_STOP_REQ       : 1;
  __REG32 ECAP_CLK_STOP_ACK       : 1;
  __REG32 ECAP_CLK_EN             : 1;
  __REG32 IEP_CLK_STOP_REQ        : 1;
  __REG32 IEP_CLK_STOP_ACK        : 1;
  __REG32 IEP_CLK_EN              : 1;
  __REG32                         :14;
} __pruss_cfg_cgr_bits;

/* PRUSS CFG ISRP Register */
typedef struct {
  __REG32 PRU0_IMEM_PE_RAW0       : 1;
  __REG32 PRU0_IMEM_PE_RAW1       : 1;
  __REG32 PRU0_IMEM_PE_RAW2       : 1;
  __REG32 PRU0_IMEM_PE_RAW3       : 1;
  __REG32 PRU0_DMEM_PE_RAW0       : 1;
  __REG32 PRU0_DMEM_PE_RAW1       : 1;
  __REG32 PRU0_DMEM_PE_RAW2       : 1;
  __REG32 PRU0_DMEM_PE_RAW3       : 1;
  __REG32 PRU1_IMEM_PE_RAW0       : 1;
  __REG32 PRU1_IMEM_PE_RAW1       : 1;
  __REG32 PRU1_IMEM_PE_RAW2       : 1;
  __REG32 PRU1_IMEM_PE_RAW3       : 1;
  __REG32 PRU1_DMEM_PE_RAW0       : 1;
  __REG32 PRU1_DMEM_PE_RAW1       : 1;
  __REG32 PRU1_DMEM_PE_RAW2       : 1;
  __REG32 PRU1_DMEM_PE_RAW3       : 1;
  __REG32 RAM_PE_RAW0             : 1;
  __REG32 RAM_PE_RAW1             : 1;
  __REG32 RAM_PE_RAW2             : 1;
  __REG32 RAM_PE_RAW3             : 1;
  __REG32                         :12;
} __pruss_cfg_isrp_bits;

/* PRUSS CFG ISP Register */
typedef struct {
  __REG32 PRU0_IMEM_PE0           : 1;
  __REG32 PRU0_IMEM_PE1           : 1;
  __REG32 PRU0_IMEM_PE2           : 1;
  __REG32 PRU0_IMEM_PE3           : 1;
  __REG32 PRU0_DMEM_PE0           : 1;
  __REG32 PRU0_DMEM_PE1           : 1;
  __REG32 PRU0_DMEM_PE2           : 1;
  __REG32 PRU0_DMEM_PE3           : 1;
  __REG32 PRU1_IMEM_PE0           : 1;
  __REG32 PRU1_IMEM_PE1           : 1;
  __REG32 PRU1_IMEM_PE2           : 1;
  __REG32 PRU1_IMEM_PE3           : 1;
  __REG32 PRU1_DMEM_PE0           : 1;
  __REG32 PRU1_DMEM_PE1           : 1;
  __REG32 PRU1_DMEM_PE2           : 1;
  __REG32 PRU1_DMEM_PE3           : 1;
  __REG32 RAM_PE0                 : 1;
  __REG32 RAM_PE1                 : 1;
  __REG32 RAM_PE2                 : 1;
  __REG32 RAM_PE3                 : 1;
  __REG32                         :12;
} __pruss_cfg_isp_bits;

/* PRUSS CFG IESP Register */
typedef struct {
  __REG32 PRU0_IMEM_PE_SET0       : 1;
  __REG32 PRU0_IMEM_PE_SET1       : 1;
  __REG32 PRU0_IMEM_PE_SET2       : 1;
  __REG32 PRU0_IMEM_PE_SET3       : 1;
  __REG32 PRU0_DMEM_PE_SET0       : 1;
  __REG32 PRU0_DMEM_PE_SET1       : 1;
  __REG32 PRU0_DMEM_PE_SET2       : 1;
  __REG32 PRU0_DMEM_PE_SET3       : 1;
  __REG32 PRU1_IMEM_PE_SET0       : 1;
  __REG32 PRU1_IMEM_PE_SET1       : 1;
  __REG32 PRU1_IMEM_PE_SET2       : 1;
  __REG32 PRU1_IMEM_PE_SET3       : 1;
  __REG32 PRU1_DMEM_PE_SET0       : 1;
  __REG32 PRU1_DMEM_PE_SET1       : 1;
  __REG32 PRU1_DMEM_PE_SET2       : 1;
  __REG32 PRU1_DMEM_PE_SET3       : 1;
  __REG32 RAM_PE_SET0             : 1;
  __REG32 RAM_PE_SET1             : 1;
  __REG32 RAM_PE_SET2             : 1;
  __REG32 RAM_PE_SET3             : 1;
  __REG32                         :12;
} __pruss_cfg_iesp_bits;

/* PRUSS CFG IESP Register */
typedef struct {
  __REG32 PRU0_IMEM_PE_CLR0       : 1;
  __REG32 PRU0_IMEM_PE_CLR1       : 1;
  __REG32 PRU0_IMEM_PE_CLR2       : 1;
  __REG32 PRU0_IMEM_PE_CLR3       : 1;
  __REG32 PRU0_DMEM_PE_CLR0       : 1;
  __REG32 PRU0_DMEM_PE_CLR1       : 1;
  __REG32 PRU0_DMEM_PE_CLR2       : 1;
  __REG32 PRU0_DMEM_PE_CLR3       : 1;
  __REG32 PRU1_IMEM_PE_CLR0       : 1;
  __REG32 PRU1_IMEM_PE_CLR1       : 1;
  __REG32 PRU1_IMEM_PE_CLR2       : 1;
  __REG32 PRU1_IMEM_PE_CLR3       : 1;
  __REG32 PRU1_DMEM_PE_CLR0       : 1;
  __REG32 PRU1_DMEM_PE_CLR1       : 1;
  __REG32 PRU1_DMEM_PE_CLR2       : 1;
  __REG32 PRU1_DMEM_PE_CLR3       : 1;
  __REG32 RAM_PE_CLR0             : 1;
  __REG32 RAM_PE_CLR1             : 1;
  __REG32 RAM_PE_CLR2             : 1;
  __REG32 RAM_PE_CLR3             : 1;
  __REG32                         :12;
} __pruss_cfg_iecp_bits;

/*PRUSS CFG Parity error bits*/
#define PRUSS_CFG_PRU0_IMEM_PE0   (0x1UL<<0)
#define PRUSS_CFG_PRU0_IMEM_PE1   (0x1UL<<1)
#define PRUSS_CFG_PRU0_IMEM_PE2   (0x1UL<<2)
#define PRUSS_CFG_PRU0_IMEM_PE3   (0x1UL<<3)
#define PRUSS_CFG_PRU0_DMEM_PE0   (0x1UL<<4)
#define PRUSS_CFG_PRU0_DMEM_PE1   (0x1UL<<5)
#define PRUSS_CFG_PRU0_DMEM_PE2   (0x1UL<<6)
#define PRUSS_CFG_PRU0_DMEM_PE3   (0x1UL<<7)
#define PRUSS_CFG_PRU1_IMEM_PE0   (0x1UL<<8)
#define PRUSS_CFG_PRU1_IMEM_PE1   (0x1UL<<9)
#define PRUSS_CFG_PRU1_IMEM_PE2   (0x1UL<<10)
#define PRUSS_CFG_PRU1_IMEM_PE3   (0x1UL<<11)
#define PRUSS_CFG_PRU1_DMEM_PE0   (0x1UL<<12)
#define PRUSS_CFG_PRU1_DMEM_PE1   (0x1UL<<13)
#define PRUSS_CFG_PRU1_DMEM_PE2   (0x1UL<<14)
#define PRUSS_CFG_PRU1_DMEM_PE3   (0x1UL<<15)
#define PRUSS_CFG_RAM_PE0         (0x1UL<<16)
#define PRUSS_CFG_RAM_PE1         (0x1UL<<17)
#define PRUSS_CFG_RAM_PE2         (0x1UL<<18)
#define PRUSS_CFG_RAM_PE3         (0x1UL<<19)

/* PRUSS CFG SCRP Register */
typedef struct {
  __REG32 SCRP_1                  : 2;
  __REG32 SCRP_2                  : 2;
  __REG32 SCRP_3                  : 2;
  __REG32 SCRP_4                  : 2;
  __REG32 SCRP_5                  : 1;
  __REG32 SCRP_6                  : 1;
  __REG32 SCRP_7                  : 1;
  __REG32 SCRP_8                  : 1;
  __REG32 SCRP_9                  : 1;
  __REG32 SCRP_10                 : 1;
  __REG32 SCRP_11                 : 1;
  __REG32 SCRP_12                 : 1;
  __REG32 SCRP_13                 : 1;
  __REG32 SCRP_14                 : 1;
  __REG32 SCRP_15                 : 1;
  __REG32 SCRP_16                 : 1;
  __REG32 SCRP_17                 : 1;
  __REG32 SCRP_18                 : 1;
  __REG32                         :10;
} __pruss_cfg_scrp_bits;

/* PRUSS CFG PMAO Register */
typedef struct {
  __REG32 PMAO_PRU0               : 1;
  __REG32 PMAO_PRU1               : 1;
  __REG32                         :30;
} __pruss_cfg_pmao_bits;

/* PRUSS CFG MII_RT Register */
typedef struct {
  __REG32 MII_RT_EVENT_EN         : 1;
  __REG32                         :31;
} __pruss_cfg_mii_rt_bits;

/* PRUSS CFG IEPCLK Register */
typedef struct {
  __REG32 OCP_EN                  : 1;
  __REG32                         :31;
} __pruss_cfg_iepclk_bits;

/* PRUSS CFG SPP Register */
typedef struct {
  __REG32 PRU1_PAD_HP_EN          : 1;
  __REG32 XFR_SHIFT_EN            : 1;
  __REG32                         :30;
} __pruss_cfg_spp_bits;

/* PRUSS CFG PIN_MX Register */
typedef struct {
  __REG32 PIN_MUX_SEL             : 8;
  __REG32                         :24;
} __pruss_cfg_pin_mx_bits;

/* INTC_REVISION */
typedef struct {
   __REG32 REV              : 8;
   __REG32                  :24;
} __intc_revision_bits;

/* INTC_SYSCONFIG */
typedef struct {
   __REG32 AUTOIDLE         : 1;
   __REG32 SOFTRESET        : 1;
   __REG32                  :30;
} __intc_sysconfig_bits;

/* INTC_SYSSTATUS */
typedef struct {
   __REG32 RESETDONE        : 1;
   __REG32                  :31;
} __intc_sysstatus_bits;

/* INTC_SIR_IRQ */
typedef struct {
   __REG32 ACTIVEIRQ        : 7;
   __REG32 SPURIOUSIRQ      :25;
} __intc_sir_irq_bits;

/* INTC_SIR_FIQ */
typedef struct {
   __REG32 ACTIVEFIQ        : 7;
   __REG32 SPURIOUSFIQ      :25;
} __intc_sir_fiq_bits;

/* INTC_CONTROL bits*/
#define INTC_CONTROL_NEWIRQAGR    (0x1UL<<0)
#define INTC_CONTROL_NEWFIQAGR    (0x1UL<<1)

/* INTC_PROTECTION */
typedef struct {
   __REG32 PROTECTION       : 1;
   __REG32                  :31;
} __intc_protection_bits;

/* INTC_IDLE */
typedef struct {
   __REG32 FUNCIDLE         : 1;
   __REG32 TURBO            : 1;
   __REG32                  :30;
} __intc_idle_bits;

/* INTC_IRQ_PRIORITY */
typedef struct {
   __REG32 IRQPRIORITY      : 7;
   __REG32 SPURIOUSIRQFLAG  :25;
} __intc_irq_priority_bits;

/* INTC_FIQ_PRIORITY */
typedef struct {
   __REG32 FIQPRIORITY      : 7;
   __REG32 SPURIOUSFIQFLAG  :25;
} __intc_fiq_priority_bits;

/* INTC_THRESHOLD */
typedef struct {
   __REG32 PRIORITYTHRESHOLD : 8;
   __REG32                   :24;
} __intc_threshold_bits;

/* INTC_ITR0 */
typedef struct {
   __REG32 ITR0    : 1;
   __REG32 ITR1    : 1;
   __REG32 ITR2    : 1;
   __REG32 ITR3    : 1;
   __REG32 ITR4    : 1;
   __REG32 ITR5    : 1;
   __REG32 ITR6    : 1;
   __REG32 ITR7    : 1;
   __REG32 ITR8    : 1;
   __REG32 ITR9    : 1;
   __REG32 ITR10    : 1;
   __REG32 ITR11    : 1;
   __REG32 ITR12    : 1;
   __REG32 ITR13    : 1;
   __REG32 ITR14    : 1;
   __REG32 ITR15    : 1;
   __REG32 ITR16    : 1;
   __REG32 ITR17    : 1;
   __REG32 ITR18    : 1;
   __REG32 ITR19    : 1;
   __REG32 ITR20    : 1;
   __REG32 ITR21    : 1;
   __REG32 ITR22    : 1;
   __REG32 ITR23    : 1;
   __REG32 ITR24    : 1;
   __REG32 ITR25    : 1;
   __REG32 ITR26    : 1;
   __REG32 ITR27    : 1;
   __REG32 ITR28    : 1;
   __REG32 ITR29    : 1;
   __REG32 ITR30    : 1;
   __REG32 ITR31    : 1;
} __intc_itr0_bits;

/* INTC_ITR1 */
typedef struct {
   __REG32 ITR32    : 1;
   __REG32 ITR33    : 1;
   __REG32 ITR34    : 1;
   __REG32 ITR35    : 1;
   __REG32 ITR36    : 1;
   __REG32 ITR37    : 1;
   __REG32 ITR38    : 1;
   __REG32 ITR39    : 1;
   __REG32 ITR40    : 1;
   __REG32 ITR41    : 1;
   __REG32 ITR42    : 1;
   __REG32 ITR43    : 1;
   __REG32 ITR44    : 1;
   __REG32 ITR45    : 1;
   __REG32 ITR46    : 1;
   __REG32 ITR47    : 1;
   __REG32 ITR48    : 1;
   __REG32 ITR49    : 1;
   __REG32 ITR50    : 1;
   __REG32 ITR51    : 1;
   __REG32 ITR52    : 1;
   __REG32 ITR53    : 1;
   __REG32 ITR54    : 1;
   __REG32 ITR55    : 1;
   __REG32 ITR56    : 1;
   __REG32 ITR57    : 1;
   __REG32 ITR58    : 1;
   __REG32 ITR59    : 1;
   __REG32 ITR60    : 1;
   __REG32 ITR61    : 1;
   __REG32 ITR62    : 1;
   __REG32 ITR63    : 1;
} __intc_itr1_bits;

/* INTC_ITR2 */
typedef struct {
   __REG32 ITR64    : 1;
   __REG32 ITR65    : 1;
   __REG32 ITR66    : 1;
   __REG32 ITR67    : 1;
   __REG32 ITR68    : 1;
   __REG32 ITR69    : 1;
   __REG32 ITR70    : 1;
   __REG32 ITR71    : 1;
   __REG32 ITR72    : 1;
   __REG32 ITR73    : 1;
   __REG32 ITR74    : 1;
   __REG32 ITR75    : 1;
   __REG32 ITR76    : 1;
   __REG32 ITR77    : 1;
   __REG32 ITR78    : 1;
   __REG32 ITR79    : 1;
   __REG32 ITR80    : 1;
   __REG32 ITR81    : 1;
   __REG32 ITR82    : 1;
   __REG32 ITR83    : 1;
   __REG32 ITR84    : 1;
   __REG32 ITR85    : 1;
   __REG32 ITR86    : 1;
   __REG32 ITR87    : 1;
   __REG32 ITR88    : 1;
   __REG32 ITR89    : 1;
   __REG32 ITR90    : 1;
   __REG32 ITR91    : 1;
   __REG32 ITR92    : 1;
   __REG32 ITR93    : 1;
   __REG32 ITR94    : 1;
   __REG32 ITR95    : 1;
} __intc_itr2_bits;

/* INTC_ITR3 */
typedef struct {
   __REG32 ITR96    : 1;
   __REG32 ITR97    : 1;
   __REG32 ITR98    : 1;
   __REG32 ITR99    : 1;
   __REG32 ITR100   : 1;
   __REG32 ITR101   : 1;
   __REG32 ITR102   : 1;
   __REG32 ITR103   : 1;
   __REG32 ITR104   : 1;
   __REG32 ITR105   : 1;
   __REG32 ITR106   : 1;
   __REG32 ITR107   : 1;
   __REG32 ITR108   : 1;
   __REG32 ITR109   : 1;
   __REG32 ITR110   : 1;
   __REG32 ITR111   : 1;
   __REG32 ITR112   : 1;
   __REG32 ITR113   : 1;
   __REG32 ITR114   : 1;
   __REG32 ITR115   : 1;
   __REG32 ITR116   : 1;
   __REG32 ITR117   : 1;
   __REG32 ITR118   : 1;
   __REG32 ITR119   : 1;
   __REG32 ITR120   : 1;
   __REG32 ITR121   : 1;
   __REG32 ITR122   : 1;
   __REG32 ITR123   : 1;
   __REG32 ITR124   : 1;
   __REG32 ITR125   : 1;
   __REG32 ITR126   : 1;
   __REG32 ITR127   : 1;
} __intc_itr3_bits;

/* INTC_MIR0 */
typedef struct {
   __REG32 MIR0     : 1;
   __REG32 MIR1     : 1;
   __REG32 MIR2     : 1;
   __REG32 MIR3     : 1;
   __REG32 MIR4     : 1;
   __REG32 MIR5     : 1;
   __REG32 MIR6     : 1;
   __REG32 MIR7     : 1;
   __REG32 MIR8     : 1;
   __REG32 MIR9     : 1;
   __REG32 MIR10    : 1;
   __REG32 MIR11    : 1;
   __REG32 MIR12    : 1;
   __REG32 MIR13    : 1;
   __REG32 MIR14    : 1;
   __REG32 MIR15    : 1;
   __REG32 MIR16    : 1;
   __REG32 MIR17    : 1;
   __REG32 MIR18    : 1;
   __REG32 MIR19    : 1;
   __REG32 MIR20    : 1;
   __REG32 MIR21    : 1;
   __REG32 MIR22    : 1;
   __REG32 MIR23    : 1;
   __REG32 MIR24    : 1;
   __REG32 MIR25    : 1;
   __REG32 MIR26    : 1;
   __REG32 MIR27    : 1;
   __REG32 MIR28    : 1;
   __REG32 MIR29    : 1;
   __REG32 MIR30    : 1;
   __REG32 MIR31    : 1;
} __intc_mir0_bits;

/* INTC_MIR1 */
typedef struct {
   __REG32 MIR32    : 1;
   __REG32 MIR33    : 1;
   __REG32 MIR34    : 1;
   __REG32 MIR35    : 1;
   __REG32 MIR36    : 1;
   __REG32 MIR37    : 1;
   __REG32 MIR38    : 1;
   __REG32 MIR39    : 1;
   __REG32 MIR40    : 1;
   __REG32 MIR41    : 1;
   __REG32 MIR42    : 1;
   __REG32 MIR43    : 1;
   __REG32 MIR44    : 1;
   __REG32 MIR45    : 1;
   __REG32 MIR46    : 1;
   __REG32 MIR47    : 1;
   __REG32 MIR48    : 1;
   __REG32 MIR49    : 1;
   __REG32 MIR50    : 1;
   __REG32 MIR51    : 1;
   __REG32 MIR52    : 1;
   __REG32 MIR53    : 1;
   __REG32 MIR54    : 1;
   __REG32 MIR55    : 1;
   __REG32 MIR56    : 1;
   __REG32 MIR57    : 1;
   __REG32 MIR58    : 1;
   __REG32 MIR59    : 1;
   __REG32 MIR60    : 1;
   __REG32 MIR61    : 1;
   __REG32 MIR62    : 1;
   __REG32 MIR63    : 1;
} __intc_mir1_bits;

/* INTC_MIR2 */
typedef struct {
   __REG32 MIR64    : 1;
   __REG32 MIR65    : 1;
   __REG32 MIR66    : 1;
   __REG32 MIR67    : 1;
   __REG32 MIR68    : 1;
   __REG32 MIR69    : 1;
   __REG32 MIR70    : 1;
   __REG32 MIR71    : 1;
   __REG32 MIR72    : 1;
   __REG32 MIR73    : 1;
   __REG32 MIR74    : 1;
   __REG32 MIR75    : 1;
   __REG32 MIR76    : 1;
   __REG32 MIR77    : 1;
   __REG32 MIR78    : 1;
   __REG32 MIR79    : 1;
   __REG32 MIR80    : 1;
   __REG32 MIR81    : 1;
   __REG32 MIR82    : 1;
   __REG32 MIR83    : 1;
   __REG32 MIR84    : 1;
   __REG32 MIR85    : 1;
   __REG32 MIR86    : 1;
   __REG32 MIR87    : 1;
   __REG32 MIR88    : 1;
   __REG32 MIR89    : 1;
   __REG32 MIR90    : 1;
   __REG32 MIR91    : 1;
   __REG32 MIR92    : 1;
   __REG32 MIR93    : 1;
   __REG32 MIR94    : 1;
   __REG32 MIR95    : 1;
} __intc_mir2_bits;

/* INTC_MIR3 */
typedef struct {
   __REG32 MIR96    : 1;
   __REG32 MIR97    : 1;
   __REG32 MIR98    : 1;
   __REG32 MIR99    : 1;
   __REG32 MIR100   : 1;
   __REG32 MIR101   : 1;
   __REG32 MIR102   : 1;
   __REG32 MIR103   : 1;
   __REG32 MIR104   : 1;
   __REG32 MIR105   : 1;
   __REG32 MIR106   : 1;
   __REG32 MIR107   : 1;
   __REG32 MIR108   : 1;
   __REG32 MIR109   : 1;
   __REG32 MIR110   : 1;
   __REG32 MIR111   : 1;
   __REG32 MIR112   : 1;
   __REG32 MIR113   : 1;
   __REG32 MIR114   : 1;
   __REG32 MIR115   : 1;
   __REG32 MIR116   : 1;
   __REG32 MIR117   : 1;
   __REG32 MIR118   : 1;
   __REG32 MIR119   : 1;
   __REG32 MIR120   : 1;
   __REG32 MIR121   : 1;
   __REG32 MIR122   : 1;
   __REG32 MIR123   : 1;
   __REG32 MIR124   : 1;
   __REG32 MIR125   : 1;
   __REG32 MIR126   : 1;
   __REG32 MIR127   : 1;
} __intc_mir3_bits;

/* INTC_ISR_SET0 */
typedef struct {
   __REG32 ISRSET0    : 1;
   __REG32 ISRSET1    : 1;
   __REG32 ISRSET2    : 1;
   __REG32 ISRSET3    : 1;
   __REG32 ISRSET4    : 1;
   __REG32 ISRSET5    : 1;
   __REG32 ISRSET6    : 1;
   __REG32 ISRSET7    : 1;
   __REG32 ISRSET8    : 1;
   __REG32 ISRSET9    : 1;
   __REG32 ISRSET10    : 1;
   __REG32 ISRSET11    : 1;
   __REG32 ISRSET12    : 1;
   __REG32 ISRSET13    : 1;
   __REG32 ISRSET14    : 1;
   __REG32 ISRSET15    : 1;
   __REG32 ISRSET16    : 1;
   __REG32 ISRSET17    : 1;
   __REG32 ISRSET18    : 1;
   __REG32 ISRSET19    : 1;
   __REG32 ISRSET20    : 1;
   __REG32 ISRSET21    : 1;
   __REG32 ISRSET22    : 1;
   __REG32 ISRSET23    : 1;
   __REG32 ISRSET24    : 1;
   __REG32 ISRSET25    : 1;
   __REG32 ISRSET26    : 1;
   __REG32 ISRSET27    : 1;
   __REG32 ISRSET28    : 1;
   __REG32 ISRSET29    : 1;
   __REG32 ISRSET30    : 1;
   __REG32 ISRSET31    : 1;
} __intc_isr_set0_bits;

/* INTC_ISR_SET1 */
typedef struct {
   __REG32 ISRSET32    : 1;
   __REG32 ISRSET33    : 1;
   __REG32 ISRSET34    : 1;
   __REG32 ISRSET35    : 1;
   __REG32 ISRSET36    : 1;
   __REG32 ISRSET37    : 1;
   __REG32 ISRSET38    : 1;
   __REG32 ISRSET39    : 1;
   __REG32 ISRSET40    : 1;
   __REG32 ISRSET41    : 1;
   __REG32 ISRSET42    : 1;
   __REG32 ISRSET43    : 1;
   __REG32 ISRSET44    : 1;
   __REG32 ISRSET45    : 1;
   __REG32 ISRSET46    : 1;
   __REG32 ISRSET47    : 1;
   __REG32 ISRSET48    : 1;
   __REG32 ISRSET49    : 1;
   __REG32 ISRSET50    : 1;
   __REG32 ISRSET51    : 1;
   __REG32 ISRSET52    : 1;
   __REG32 ISRSET53    : 1;
   __REG32 ISRSET54    : 1;
   __REG32 ISRSET55    : 1;
   __REG32 ISRSET56    : 1;
   __REG32 ISRSET57    : 1;
   __REG32 ISRSET58    : 1;
   __REG32 ISRSET59    : 1;
   __REG32 ISRSET60    : 1;
   __REG32 ISRSET61    : 1;
   __REG32 ISRSET62    : 1;
   __REG32 ISRSET63    : 1;
} __intc_isr_set1_bits;

/* INTC_ISR_SET2 */
typedef struct {
   __REG32 ISRSET64    : 1;
   __REG32 ISRSET65    : 1;
   __REG32 ISRSET66    : 1;
   __REG32 ISRSET67    : 1;
   __REG32 ISRSET68    : 1;
   __REG32 ISRSET69    : 1;
   __REG32 ISRSET70    : 1;
   __REG32 ISRSET71    : 1;
   __REG32 ISRSET72    : 1;
   __REG32 ISRSET73    : 1;
   __REG32 ISRSET74    : 1;
   __REG32 ISRSET75    : 1;
   __REG32 ISRSET76    : 1;
   __REG32 ISRSET77    : 1;
   __REG32 ISRSET78    : 1;
   __REG32 ISRSET79    : 1;
   __REG32 ISRSET80    : 1;
   __REG32 ISRSET81    : 1;
   __REG32 ISRSET82    : 1;
   __REG32 ISRSET83    : 1;
   __REG32 ISRSET84    : 1;
   __REG32 ISRSET85    : 1;
   __REG32 ISRSET86    : 1;
   __REG32 ISRSET87    : 1;
   __REG32 ISRSET88    : 1;
   __REG32 ISRSET89    : 1;
   __REG32 ISRSET90    : 1;
   __REG32 ISRSET91    : 1;
   __REG32 ISRSET92    : 1;
   __REG32 ISRSET93    : 1;
   __REG32 ISRSET94    : 1;
   __REG32 ISRSET95    : 1;
} __intc_isr_set2_bits;

/* INTC_ISR_SET3 */
typedef struct {
   __REG32 ISRSET96    : 1;
   __REG32 ISRSET97    : 1;
   __REG32 ISRSET98    : 1;
   __REG32 ISRSET99    : 1;
   __REG32 ISRSET100   : 1;
   __REG32 ISRSET101   : 1;
   __REG32 ISRSET102   : 1;
   __REG32 ISRSET103   : 1;
   __REG32 ISRSET104   : 1;
   __REG32 ISRSET105   : 1;
   __REG32 ISRSET106   : 1;
   __REG32 ISRSET107   : 1;
   __REG32 ISRSET108   : 1;
   __REG32 ISRSET109   : 1;
   __REG32 ISRSET110   : 1;
   __REG32 ISRSET111   : 1;
   __REG32 ISRSET112   : 1;
   __REG32 ISRSET113   : 1;
   __REG32 ISRSET114   : 1;
   __REG32 ISRSET115   : 1;
   __REG32 ISRSET116   : 1;
   __REG32 ISRSET117   : 1;
   __REG32 ISRSET118   : 1;
   __REG32 ISRSET119   : 1;
   __REG32 ISRSET120   : 1;
   __REG32 ISRSET121   : 1;
   __REG32 ISRSET122   : 1;
   __REG32 ISRSET123   : 1;
   __REG32 ISRSET124   : 1;
   __REG32 ISRSET125   : 1;
   __REG32 ISRSET126   : 1;
   __REG32 ISRSET127   : 1;
} __intc_isr_set3_bits;

/* INTC_PENDING_IRQ0 */
typedef struct {
   __REG32 PENDINGIRQ0    : 1;
   __REG32 PENDINGIRQ1    : 1;
   __REG32 PENDINGIRQ2    : 1;
   __REG32 PENDINGIRQ3    : 1;
   __REG32 PENDINGIRQ4    : 1;
   __REG32 PENDINGIRQ5    : 1;
   __REG32 PENDINGIRQ6    : 1;
   __REG32 PENDINGIRQ7    : 1;
   __REG32 PENDINGIRQ8    : 1;
   __REG32 PENDINGIRQ9    : 1;
   __REG32 PENDINGIRQ10    : 1;
   __REG32 PENDINGIRQ11    : 1;
   __REG32 PENDINGIRQ12    : 1;
   __REG32 PENDINGIRQ13    : 1;
   __REG32 PENDINGIRQ14    : 1;
   __REG32 PENDINGIRQ15    : 1;
   __REG32 PENDINGIRQ16    : 1;
   __REG32 PENDINGIRQ17    : 1;
   __REG32 PENDINGIRQ18    : 1;
   __REG32 PENDINGIRQ19    : 1;
   __REG32 PENDINGIRQ20    : 1;
   __REG32 PENDINGIRQ21    : 1;
   __REG32 PENDINGIRQ22    : 1;
   __REG32 PENDINGIRQ23    : 1;
   __REG32 PENDINGIRQ24    : 1;
   __REG32 PENDINGIRQ25    : 1;
   __REG32 PENDINGIRQ26    : 1;
   __REG32 PENDINGIRQ27    : 1;
   __REG32 PENDINGIRQ28    : 1;
   __REG32 PENDINGIRQ29    : 1;
   __REG32 PENDINGIRQ30    : 1;
   __REG32 PENDINGIRQ31    : 1;
} __intc_pending_irq0_bits;

/* INTC_PENDING_IRQ1 */
typedef struct {
   __REG32 PENDINGIRQ32    : 1;
   __REG32 PENDINGIRQ33    : 1;
   __REG32 PENDINGIRQ34    : 1;
   __REG32 PENDINGIRQ35    : 1;
   __REG32 PENDINGIRQ36    : 1;
   __REG32 PENDINGIRQ37    : 1;
   __REG32 PENDINGIRQ38    : 1;
   __REG32 PENDINGIRQ39    : 1;
   __REG32 PENDINGIRQ40    : 1;
   __REG32 PENDINGIRQ41    : 1;
   __REG32 PENDINGIRQ42    : 1;
   __REG32 PENDINGIRQ43    : 1;
   __REG32 PENDINGIRQ44    : 1;
   __REG32 PENDINGIRQ45    : 1;
   __REG32 PENDINGIRQ46    : 1;
   __REG32 PENDINGIRQ47    : 1;
   __REG32 PENDINGIRQ48    : 1;
   __REG32 PENDINGIRQ49    : 1;
   __REG32 PENDINGIRQ50    : 1;
   __REG32 PENDINGIRQ51    : 1;
   __REG32 PENDINGIRQ52    : 1;
   __REG32 PENDINGIRQ53    : 1;
   __REG32 PENDINGIRQ54    : 1;
   __REG32 PENDINGIRQ55    : 1;
   __REG32 PENDINGIRQ56    : 1;
   __REG32 PENDINGIRQ57    : 1;
   __REG32 PENDINGIRQ58    : 1;
   __REG32 PENDINGIRQ59    : 1;
   __REG32 PENDINGIRQ60    : 1;
   __REG32 PENDINGIRQ61    : 1;
   __REG32 PENDINGIRQ62    : 1;
   __REG32 PENDINGIRQ63    : 1;
} __intc_pending_irq1_bits;

/* INTC_PENDING_IRQ2 */
typedef struct {
   __REG32 PENDINGIRQ64    : 1;
   __REG32 PENDINGIRQ65    : 1;
   __REG32 PENDINGIRQ66    : 1;
   __REG32 PENDINGIRQ67    : 1;
   __REG32 PENDINGIRQ68    : 1;
   __REG32 PENDINGIRQ69    : 1;
   __REG32 PENDINGIRQ70    : 1;
   __REG32 PENDINGIRQ71    : 1;
   __REG32 PENDINGIRQ72    : 1;
   __REG32 PENDINGIRQ73    : 1;
   __REG32 PENDINGIRQ74    : 1;
   __REG32 PENDINGIRQ75    : 1;
   __REG32 PENDINGIRQ76    : 1;
   __REG32 PENDINGIRQ77    : 1;
   __REG32 PENDINGIRQ78    : 1;
   __REG32 PENDINGIRQ79    : 1;
   __REG32 PENDINGIRQ80    : 1;
   __REG32 PENDINGIRQ81    : 1;
   __REG32 PENDINGIRQ82    : 1;
   __REG32 PENDINGIRQ83    : 1;
   __REG32 PENDINGIRQ84    : 1;
   __REG32 PENDINGIRQ85    : 1;
   __REG32 PENDINGIRQ86    : 1;
   __REG32 PENDINGIRQ87    : 1;
   __REG32 PENDINGIRQ88    : 1;
   __REG32 PENDINGIRQ89    : 1;
   __REG32 PENDINGIRQ90    : 1;
   __REG32 PENDINGIRQ91    : 1;
   __REG32 PENDINGIRQ92    : 1;
   __REG32 PENDINGIRQ93    : 1;
   __REG32 PENDINGIRQ94    : 1;
   __REG32 PENDINGIRQ95    : 1;
} __intc_pending_irq2_bits;

/* INTC_PENDING_IRQ3 */
typedef struct {
   __REG32 PENDINGIRQ96    : 1;
   __REG32 PENDINGIRQ97    : 1;
   __REG32 PENDINGIRQ98    : 1;
   __REG32 PENDINGIRQ99    : 1;
   __REG32 PENDINGIRQ100   : 1;
   __REG32 PENDINGIRQ101   : 1;
   __REG32 PENDINGIRQ102   : 1;
   __REG32 PENDINGIRQ103   : 1;
   __REG32 PENDINGIRQ104   : 1;
   __REG32 PENDINGIRQ105   : 1;
   __REG32 PENDINGIRQ106   : 1;
   __REG32 PENDINGIRQ107   : 1;
   __REG32 PENDINGIRQ108   : 1;
   __REG32 PENDINGIRQ109   : 1;
   __REG32 PENDINGIRQ110   : 1;
   __REG32 PENDINGIRQ111   : 1;
   __REG32 PENDINGIRQ112   : 1;
   __REG32 PENDINGIRQ113   : 1;
   __REG32 PENDINGIRQ114   : 1;
   __REG32 PENDINGIRQ115   : 1;
   __REG32 PENDINGIRQ116   : 1;
   __REG32 PENDINGIRQ117   : 1;
   __REG32 PENDINGIRQ118   : 1;
   __REG32 PENDINGIRQ119   : 1;
   __REG32 PENDINGIRQ120   : 1;
   __REG32 PENDINGIRQ121   : 1;
   __REG32 PENDINGIRQ122   : 1;
   __REG32 PENDINGIRQ123   : 1;
   __REG32 PENDINGIRQ124   : 1;
   __REG32 PENDINGIRQ125   : 1;
   __REG32 PENDINGIRQ126   : 1;
   __REG32 PENDINGIRQ127   : 1;
} __intc_pending_irq3_bits;

/* INTC_PENDING_FIQ0 */
typedef struct {
   __REG32 PENDINGFIQ0     : 1;
   __REG32 PENDINGFIQ1     : 1;
   __REG32 PENDINGFIQ2     : 1;
   __REG32 PENDINGFIQ3     : 1;
   __REG32 PENDINGFIQ4     : 1;
   __REG32 PENDINGFIQ5     : 1;
   __REG32 PENDINGFIQ6     : 1;
   __REG32 PENDINGFIQ7     : 1;
   __REG32 PENDINGFIQ8     : 1;
   __REG32 PENDINGFIQ9     : 1;
   __REG32 PENDINGFIQ10    : 1;
   __REG32 PENDINGFIQ11    : 1;
   __REG32 PENDINGFIQ12    : 1;
   __REG32 PENDINGFIQ13    : 1;
   __REG32 PENDINGFIQ14    : 1;
   __REG32 PENDINGFIQ15    : 1;
   __REG32 PENDINGFIQ16    : 1;
   __REG32 PENDINGFIQ17    : 1;
   __REG32 PENDINGFIQ18    : 1;
   __REG32 PENDINGFIQ19    : 1;
   __REG32 PENDINGFIQ20    : 1;
   __REG32 PENDINGFIQ21    : 1;
   __REG32 PENDINGFIQ22    : 1;
   __REG32 PENDINGFIQ23    : 1;
   __REG32 PENDINGFIQ24    : 1;
   __REG32 PENDINGFIQ25    : 1;
   __REG32 PENDINGFIQ26    : 1;
   __REG32 PENDINGFIQ27    : 1;
   __REG32 PENDINGFIQ28    : 1;
   __REG32 PENDINGFIQ29    : 1;
   __REG32 PENDINGFIQ30    : 1;
   __REG32 PENDINGFIQ31    : 1;
} __intc_pending_fiq0_bits;

/* INTC_PENDING_FIQ1 */
typedef struct {
   __REG32 PENDINGFIQ32    : 1;
   __REG32 PENDINGFIQ33    : 1;
   __REG32 PENDINGFIQ34    : 1;
   __REG32 PENDINGFIQ35    : 1;
   __REG32 PENDINGFIQ36    : 1;
   __REG32 PENDINGFIQ37    : 1;
   __REG32 PENDINGFIQ38    : 1;
   __REG32 PENDINGFIQ39    : 1;
   __REG32 PENDINGFIQ40    : 1;
   __REG32 PENDINGFIQ41    : 1;
   __REG32 PENDINGFIQ42    : 1;
   __REG32 PENDINGFIQ43    : 1;
   __REG32 PENDINGFIQ44    : 1;
   __REG32 PENDINGFIQ45    : 1;
   __REG32 PENDINGFIQ46    : 1;
   __REG32 PENDINGFIQ47    : 1;
   __REG32 PENDINGFIQ48    : 1;
   __REG32 PENDINGFIQ49    : 1;
   __REG32 PENDINGFIQ50    : 1;
   __REG32 PENDINGFIQ51    : 1;
   __REG32 PENDINGFIQ52    : 1;
   __REG32 PENDINGFIQ53    : 1;
   __REG32 PENDINGFIQ54    : 1;
   __REG32 PENDINGFIQ55    : 1;
   __REG32 PENDINGFIQ56    : 1;
   __REG32 PENDINGFIQ57    : 1;
   __REG32 PENDINGFIQ58    : 1;
   __REG32 PENDINGFIQ59    : 1;
   __REG32 PENDINGFIQ60    : 1;
   __REG32 PENDINGFIQ61    : 1;
   __REG32 PENDINGFIQ62    : 1;
   __REG32 PENDINGFIQ63    : 1;
} __intc_pending_fiq1_bits;

/* INTC_PENDING_FIQ2 */
typedef struct {
   __REG32 PENDINGFIQ64    : 1;
   __REG32 PENDINGFIQ65    : 1;
   __REG32 PENDINGFIQ66    : 1;
   __REG32 PENDINGFIQ67    : 1;
   __REG32 PENDINGFIQ68    : 1;
   __REG32 PENDINGFIQ69    : 1;
   __REG32 PENDINGFIQ70    : 1;
   __REG32 PENDINGFIQ71    : 1;
   __REG32 PENDINGFIQ72    : 1;
   __REG32 PENDINGFIQ73    : 1;
   __REG32 PENDINGFIQ74    : 1;
   __REG32 PENDINGFIQ75    : 1;
   __REG32 PENDINGFIQ76    : 1;
   __REG32 PENDINGFIQ77    : 1;
   __REG32 PENDINGFIQ78    : 1;
   __REG32 PENDINGFIQ79    : 1;
   __REG32 PENDINGFIQ80    : 1;
   __REG32 PENDINGFIQ81    : 1;
   __REG32 PENDINGFIQ82    : 1;
   __REG32 PENDINGFIQ83    : 1;
   __REG32 PENDINGFIQ84    : 1;
   __REG32 PENDINGFIQ85    : 1;
   __REG32 PENDINGFIQ86    : 1;
   __REG32 PENDINGFIQ87    : 1;
   __REG32 PENDINGFIQ88    : 1;
   __REG32 PENDINGFIQ89    : 1;
   __REG32 PENDINGFIQ90    : 1;
   __REG32 PENDINGFIQ91    : 1;
   __REG32 PENDINGFIQ92    : 1;
   __REG32 PENDINGFIQ93    : 1;
   __REG32 PENDINGFIQ94    : 1;
   __REG32 PENDINGFIQ95    : 1;
} __intc_pending_fiq2_bits;

/* INTC_PENDING_FIQ3 */
typedef struct {
   __REG32 PENDINGFIQ96    : 1;
   __REG32 PENDINGFIQ97    : 1;
   __REG32 PENDINGFIQ98    : 1;
   __REG32 PENDINGFIQ99    : 1;
   __REG32 PENDINGFIQ100   : 1;
   __REG32 PENDINGFIQ101   : 1;
   __REG32 PENDINGFIQ102   : 1;
   __REG32 PENDINGFIQ103   : 1;
   __REG32 PENDINGFIQ104   : 1;
   __REG32 PENDINGFIQ105   : 1;
   __REG32 PENDINGFIQ106   : 1;
   __REG32 PENDINGFIQ107   : 1;
   __REG32 PENDINGFIQ108   : 1;
   __REG32 PENDINGFIQ109   : 1;
   __REG32 PENDINGFIQ110   : 1;
   __REG32 PENDINGFIQ111   : 1;
   __REG32 PENDINGFIQ112   : 1;
   __REG32 PENDINGFIQ113   : 1;
   __REG32 PENDINGFIQ114   : 1;
   __REG32 PENDINGFIQ115   : 1;
   __REG32 PENDINGFIQ116   : 1;
   __REG32 PENDINGFIQ117   : 1;
   __REG32 PENDINGFIQ118   : 1;
   __REG32 PENDINGFIQ119   : 1;
   __REG32 PENDINGFIQ120   : 1;
   __REG32 PENDINGFIQ121   : 1;
   __REG32 PENDINGFIQ122   : 1;
   __REG32 PENDINGFIQ123   : 1;
   __REG32 PENDINGFIQ124   : 1;
   __REG32 PENDINGFIQ125   : 1;
   __REG32 PENDINGFIQ126   : 1;
   __REG32 PENDINGFIQ127   : 1;
} __intc_pending_fiq3_bits;

/* INTC_ILRm */
typedef struct {
   __REG32 FIQNIRQ         : 1;
   __REG32                 : 1;
   __REG32 PRIORITY        : 6;
   __REG32                 :24;
} __intc_ilr_bits;

/*INTC Interrupt Bits*/
#define INTC_INT0_BIT      (0x1UL<<0)
#define INTC_INT1_BIT      (0x1UL<<1)
#define INTC_INT2_BIT      (0x1UL<<2)
#define INTC_INT3_BIT      (0x1UL<<3)
#define INTC_INT4_BIT      (0x1UL<<4)
#define INTC_INT5_BIT      (0x1UL<<5)
#define INTC_INT6_BIT      (0x1UL<<6)
#define INTC_INT7_BIT      (0x1UL<<7)
#define INTC_INT8_BIT      (0x1UL<<8)
#define INTC_INT9_BIT      (0x1UL<<9)
#define INTC_INT10_BIT     (0x1UL<<10)
#define INTC_INT11_BIT     (0x1UL<<11)
#define INTC_INT12_BIT     (0x1UL<<12)
#define INTC_INT13_BIT     (0x1UL<<13)
#define INTC_INT14_BIT     (0x1UL<<14)
#define INTC_INT15_BIT     (0x1UL<<15)
#define INTC_INT16_BIT     (0x1UL<<16)
#define INTC_INT17_BIT     (0x1UL<<17)
#define INTC_INT18_BIT     (0x1UL<<18)
#define INTC_INT19_BIT     (0x1UL<<19)
#define INTC_INT20_BIT     (0x1UL<<20)
#define INTC_INT21_BIT     (0x1UL<<21)
#define INTC_INT22_BIT     (0x1UL<<22)
#define INTC_INT23_BIT     (0x1UL<<23)
#define INTC_INT24_BIT     (0x1UL<<24)
#define INTC_INT25_BIT     (0x1UL<<25)
#define INTC_INT26_BIT     (0x1UL<<26)
#define INTC_INT27_BIT     (0x1UL<<27)
#define INTC_INT28_BIT     (0x1UL<<28)
#define INTC_INT29_BIT     (0x1UL<<29)
#define INTC_INT30_BIT     (0x1UL<<30)
#define INTC_INT31_BIT     (0x1UL<<31)
#define INTC_INT32_BIT     (0x1UL<<0)
#define INTC_INT33_BIT     (0x1UL<<1)
#define INTC_INT34_BIT     (0x1UL<<2)
#define INTC_INT35_BIT     (0x1UL<<3)
#define INTC_INT36_BIT     (0x1UL<<4)
#define INTC_INT37_BIT     (0x1UL<<5)
#define INTC_INT38_BIT     (0x1UL<<6)
#define INTC_INT39_BIT     (0x1UL<<7)
#define INTC_INT40_BIT     (0x1UL<<8)
#define INTC_INT41_BIT     (0x1UL<<9)
#define INTC_INT42_BIT     (0x1UL<<10)
#define INTC_INT43_BIT     (0x1UL<<11)
#define INTC_INT44_BIT     (0x1UL<<12)
#define INTC_INT45_BIT     (0x1UL<<13)
#define INTC_INT46_BIT     (0x1UL<<14)
#define INTC_INT47_BIT     (0x1UL<<15)
#define INTC_INT48_BIT     (0x1UL<<16)
#define INTC_INT49_BIT     (0x1UL<<17)
#define INTC_INT50_BIT     (0x1UL<<18)
#define INTC_INT51_BIT     (0x1UL<<19)
#define INTC_INT52_BIT     (0x1UL<<20)
#define INTC_INT53_BIT     (0x1UL<<21)
#define INTC_INT54_BIT     (0x1UL<<22)
#define INTC_INT55_BIT     (0x1UL<<23)
#define INTC_INT56_BIT     (0x1UL<<24)
#define INTC_INT57_BIT     (0x1UL<<25)
#define INTC_INT58_BIT     (0x1UL<<26)
#define INTC_INT59_BIT     (0x1UL<<27)
#define INTC_INT60_BIT     (0x1UL<<28)
#define INTC_INT61_BIT     (0x1UL<<29)
#define INTC_INT62_BIT     (0x1UL<<30)
#define INTC_INT63_BIT     (0x1UL<<31)
#define INTC_INT64_BIT     (0x1UL<<0)
#define INTC_INT65_BIT     (0x1UL<<1)
#define INTC_INT66_BIT     (0x1UL<<2)
#define INTC_INT67_BIT     (0x1UL<<3)
#define INTC_INT68_BIT     (0x1UL<<4)
#define INTC_INT69_BIT     (0x1UL<<5)
#define INTC_INT70_BIT     (0x1UL<<6)
#define INTC_INT71_BIT     (0x1UL<<7)
#define INTC_INT72_BIT     (0x1UL<<8)
#define INTC_INT73_BIT     (0x1UL<<9)
#define INTC_INT74_BIT     (0x1UL<<10)
#define INTC_INT75_BIT     (0x1UL<<11)
#define INTC_INT76_BIT     (0x1UL<<12)
#define INTC_INT77_BIT     (0x1UL<<13)
#define INTC_INT78_BIT     (0x1UL<<14)
#define INTC_INT79_BIT     (0x1UL<<15)
#define INTC_INT80_BIT     (0x1UL<<16)
#define INTC_INT81_BIT     (0x1UL<<17)
#define INTC_INT82_BIT     (0x1UL<<18)
#define INTC_INT83_BIT     (0x1UL<<19)
#define INTC_INT84_BIT     (0x1UL<<20)
#define INTC_INT85_BIT     (0x1UL<<21)
#define INTC_INT86_BIT     (0x1UL<<22)
#define INTC_INT87_BIT     (0x1UL<<23)
#define INTC_INT88_BIT     (0x1UL<<24)
#define INTC_INT89_BIT     (0x1UL<<25)
#define INTC_INT90_BIT     (0x1UL<<26)
#define INTC_INT91_BIT     (0x1UL<<27)
#define INTC_INT92_BIT     (0x1UL<<28)
#define INTC_INT93_BIT     (0x1UL<<29)
#define INTC_INT94_BIT     (0x1UL<<30)
#define INTC_INT95_BIT     (0x1UL<<31)
#define INTC_INT96_BIT     (0x1UL<<0)
#define INTC_INT97_BIT     (0x1UL<<1)
#define INTC_INT98_BIT     (0x1UL<<2)
#define INTC_INT99_BIT     (0x1UL<<3)
#define INTC_INT100_BIT    (0x1UL<<4)
#define INTC_INT101_BIT    (0x1UL<<5)
#define INTC_INT102_BIT    (0x1UL<<6)
#define INTC_INT103_BIT    (0x1UL<<7)
#define INTC_INT104_BIT    (0x1UL<<8)
#define INTC_INT105_BIT    (0x1UL<<9)
#define INTC_INT106_BIT    (0x1UL<<10)
#define INTC_INT107_BIT    (0x1UL<<11)
#define INTC_INT108_BIT    (0x1UL<<12)
#define INTC_INT109_BIT    (0x1UL<<13)
#define INTC_INT110_BIT    (0x1UL<<14)
#define INTC_INT111_BIT    (0x1UL<<15)
#define INTC_INT112_BIT    (0x1UL<<16)
#define INTC_INT113_BIT    (0x1UL<<17)
#define INTC_INT114_BIT    (0x1UL<<18)
#define INTC_INT115_BIT    (0x1UL<<19)
#define INTC_INT116_BIT    (0x1UL<<20)
#define INTC_INT117_BIT    (0x1UL<<21)
#define INTC_INT118_BIT    (0x1UL<<22)
#define INTC_INT119_BIT    (0x1UL<<23)
#define INTC_INT120_BIT    (0x1UL<<24)
#define INTC_INT121_BIT    (0x1UL<<25)
#define INTC_INT122_BIT    (0x1UL<<26)
#define INTC_INT123_BIT    (0x1UL<<27)
#define INTC_INT124_BIT    (0x1UL<<28)
#define INTC_INT125_BIT    (0x1UL<<29)
#define INTC_INT126_BIT    (0x1UL<<30)
#define INTC_INT127_BIT    (0x1UL<<31)

/* GPMC_REVISION */
typedef struct {
   __REG32 REV             : 8;
   __REG32                 :24;
} __gpmc_revision_bits;

/* GPMC_SYSCONFIG */
typedef struct {
   __REG32 AUTOIDLE        : 1;
   __REG32 SOFTRESET       : 1;
   __REG32                 : 1;
   __REG32 SIDLEMODE       : 2;
   __REG32                 :27;
} __gpmc_sysconfig_bits;

/* GPMC_SYSSTATUS */
typedef struct {
   __REG32 RESETDONE       : 1;
   __REG32                 :31;
} __gpmc_sysstatus_bits;

/* GPMC_IRQSTATUS */
typedef struct {
   __REG32 FIFOEVENTSTATUS           : 1;
   __REG32 TERMINALCOUNTSTATUS       : 1;
   __REG32                           : 6;
   __REG32 WAIT0EDGEDETECTIONSTATUS  : 1;
   __REG32 WAIT1EDGEDETECTIONSTATUS  : 1;
   __REG32                           :22;
} __gpmc_irqstatus_bits;

/* GPMC_IRQENABLE */
typedef struct {
   __REG32 FIFOEVENTENABLE           : 1;
   __REG32 TERMINALCOUNTEVENTENABLE  : 1;
   __REG32                           : 6;
   __REG32 WAIT0EDGEDETECTIONENABLE  : 1;
   __REG32 WAIT1EDGEDETECTIONENABLE  : 1;
   __REG32                           :22;
} __gpmc_irqenable_bits;

/* GPMC_TIMEOUT_CONTROL */
typedef struct {
   __REG32 TIMEOUTENABLE             : 1;
   __REG32                           : 3;
   __REG32 TIMEOUTSTARTVALUE         : 9;
   __REG32                           :19;
} __gpmc_timeout_control_bits;

/* GPMC_ERR_ADDRESS */
typedef struct {
   __REG32 ILLEGALADD                :31;
   __REG32                           : 1;
} __gpmc_err_address_bits;

/* GPMC_ERR_TYPE */
typedef struct {
   __REG32 ERRORVALID                : 1;
   __REG32                           : 1;
   __REG32 ERRORTIMEOUT              : 1;
   __REG32 ERRORNOTSUPPMCMD          : 1;
   __REG32 ERRORNOTSUPPADD           : 1;
   __REG32                           : 3;
   __REG32 ILLEGALMCMD               : 3;
   __REG32                           :21;
} __gpmc_err_type_bits;

/* GPMC_CONFIG */
typedef struct {
   __REG32 NANDFORCEPOSTEDWRITE      : 1;
   __REG32 LIMITEDADDRESS            : 1;
   __REG32                           : 2;
   __REG32 WRITEPROTECT              : 1;
   __REG32                           : 3;
   __REG32 WAIT0PINPOLARITY          : 1;
   __REG32 WAIT1PINPOLARITY          : 1;
   __REG32                           :22;
} __gpmc_config_bits;

/* GPMC_STATUS */
typedef struct {
   __REG32 EMPTYWRITEBUFFERSTATUS    : 1;
   __REG32                           : 7;
   __REG32 WAIT0STATUS               : 1;
   __REG32 WAIT1STATUS               : 1;
   __REG32                           :22;
} __gpmc_status_bits;

/* GPMC_CONFIG1_i */
typedef struct {
   __REG32 GPMCFCLKDIVIDER           : 2;
   __REG32                           : 2;
   __REG32 TIMEPARAGRANULARITY       : 1;
   __REG32                           : 3;
   __REG32 MUXADDDATA                : 2;
   __REG32 DEVICETYPE                : 2;
   __REG32 DEVICESIZE                : 2;
   __REG32                           : 2;
   __REG32 WAITPINSELECT             : 2;
   __REG32 WAITMONITORINGTIME        : 2;
   __REG32                           : 1;
   __REG32 WAITWRITEMONITORING       : 1;
   __REG32 WAITREADMONITORING        : 1;
   __REG32 ATTACHEDDEVICEPAGELENGTH  : 2;
   __REG32 CLKACTIVATIONTIME         : 2;
   __REG32 WRITETYPE                 : 1;
   __REG32 WRITEMULTIPLE             : 1;
   __REG32 READTYPE                  : 1;
   __REG32 READMULTIPLE              : 1;
   __REG32 WRAPBURST                 : 1;
} __gpmc_config1_bits;

/* GPMC_CONFIG2_i */
typedef struct {
   __REG32 CSONTIME                  : 4;
   __REG32                           : 3;
   __REG32 CSEXTRADELAY              : 1;
   __REG32 CSRDOFFTIME               : 5;
   __REG32                           : 3;
   __REG32 CSWROFFTIME               : 5;
   __REG32                           :11;
} __gpmc_config2_bits;

/* GPMC_CONFIG3_i */
typedef struct {
   __REG32 ADVONTIME                 : 4;
   __REG32 ADVAADMUXONTIME           : 3;
   __REG32 ADVEXTRADELAY             : 1;
   __REG32 ADVRDOFFTIME              : 5;
   __REG32                           : 3;
   __REG32 ADVWROFFTIME              : 5;
   __REG32                           : 3;
   __REG32 ADVAADMUXRDOFFTIME        : 3;
   __REG32                           : 1;
   __REG32 ADVAADMUXWROFFTIME        : 3;
   __REG32                           : 1;
} __gpmc_config3_bits;

/* GPMC_CONFIG4_i */
typedef struct {
   __REG32 OEONTIME                  : 4;
   __REG32 OEAADMUXONTIME            : 3;
   __REG32 OEEXTRADELAY              : 1;
   __REG32 OEOFFTIME                 : 5;
   __REG32 OEAADMUXOFFTIME           : 3;
   __REG32 WEONTIME                  : 4;
   __REG32                           : 3;
   __REG32 WEEXTRADELAY              : 1;
   __REG32 WEOFFTIME                 : 5;
   __REG32                           : 3;
} __gpmc_config4_bits;

/* GPMC_CONFIG5_i */
typedef struct {
   __REG32 RDCYCLETIME               : 5;
   __REG32                           : 3;
   __REG32 WRCYCLETIME               : 5;
   __REG32                           : 3;
   __REG32 RDACCESSTIME              : 5;
   __REG32                           : 3;
   __REG32 PAGEBURSTACCESSTIME       : 4;
   __REG32                           : 4;
} __gpmc_config5_bits;

/* GPMC_CONFIG6_i */
typedef struct {
   __REG32 BUSTURNAROUND             : 4;
   __REG32                           : 2;
   __REG32 CYCLE2CYCLEDIFFCSEN       : 1;
   __REG32 CYCLE2CYCLESAMECSEN       : 1;
   __REG32 CYCLE2CYCLEDELAY          : 4;
   __REG32                           : 4;
   __REG32 WRDATAONADMUXBUS          : 4;
   __REG32                           : 4;
   __REG32 WRACCESSTIME              : 5;
   __REG32                           : 3;
} __gpmc_config6_bits;

/* GPMC_CONFIG7_i */
typedef struct {
   __REG32 BASEADDRESS               : 6;
   __REG32 CSVALID                   : 1;
   __REG32                           : 1;
   __REG32 MASKADDRESS               : 4;
   __REG32                           :20;
} __gpmc_config7_bits;

/* GPMC_PREFETCH_CONFIG1 */
typedef struct {
   __REG32 ACCESSMODE                : 1;
   __REG32                           : 1;
   __REG32 DMAMODE                   : 1;
   __REG32 SYNCHROMODE               : 1;
   __REG32 WAITPINSELECTOR           : 2;
   __REG32                           : 1;
   __REG32 ENABLEENGINE              : 1;
   __REG32 FIFOTHRESHOLD             : 7;
   __REG32                           : 1;
   __REG32 PFPWWEIGHTEDPRIO          : 4;
   __REG32                           : 3;
   __REG32 PFPWENROUNDROBIN          : 1;
   __REG32 ENGINECSSELECTOR          : 3;
   __REG32 ENABLEOPTIMIZEDACCESS     : 1;
   __REG32 CYCLEOPTIMIZATION         : 3;
   __REG32                           : 1;
} __gpmc_prefetch_config1_bits;

/* GPMC_PREFETCH_CONFIG2 */
typedef struct {
   __REG32 TRANSFERCOUNT             :14;
   __REG32                           :18;
} __gpmc_prefetch_config2_bits;

/* GPMC_PREFETCH_CONTROL */
typedef struct {
   __REG32 STARTENGINE               : 1;
   __REG32                           :31;
} __gpmc_prefetch_control_bits;

/* GPMC_PREFETCH_STATUS */
typedef struct {
   __REG32 COUNTVALUE                :14;
   __REG32                           : 2;
   __REG32 FIFOTHRESHOLDSTATUS       : 1;
   __REG32                           : 7;
   __REG32 FIFOPOINTER               : 7;
   __REG32                           : 1;
} __gpmc_prefetch_status_bits;

/* GPMC_ECC_CONFIG */
typedef struct {
   __REG32 ECCENABLE                 : 1;
   __REG32 ECCCS                     : 3;
   __REG32 ECCTOPSECTOR              : 3;
   __REG32 ECC16B                    : 1;
   __REG32 ECCWRAPMODE               : 4;
   __REG32 ECCBCHTSEL                : 2;
   __REG32                           : 2;
   __REG32 ECCALGORITHM              : 1;
   __REG32                           :15;
} __gpmc_ecc_config_bits;

/* GPMC_ECC_CONTROL */
typedef struct {
   __REG32 ECCPOINTER                : 4;
   __REG32                           : 4;
   __REG32 ECCCLEAR                  : 1;
   __REG32                           :23;
} __gpmc_ecc_control_bits;

/* GPMC_ECC_SIZE_CONFIG */
typedef struct {
   __REG32 ECC1RESULTSIZE            : 1;
   __REG32 ECC2RESULTSIZE            : 1;
   __REG32 ECC3RESULTSIZE            : 1;
   __REG32 ECC4RESULTSIZE            : 1;
   __REG32 ECC5RESULTSIZE            : 1;
   __REG32 ECC6RESULTSIZE            : 1;
   __REG32 ECC7RESULTSIZE            : 1;
   __REG32 ECC8RESULTSIZE            : 1;
   __REG32 ECC9RESULTSIZE            : 1;
   __REG32                           : 3;
   __REG32 ECCSIZE0                  : 8;
   __REG32                           : 2;
   __REG32 ECCSIZE1                  : 8;
   __REG32                           : 2;
} __gpmc_ecc_size_config_bits;

/* GPMC_ECCj_RESULT */
typedef struct {
   __REG32 P1e                       : 1;
   __REG32 P2e                       : 1;
   __REG32 P4e                       : 1;
   __REG32 P8e                       : 1;
   __REG32 P16e                      : 1;
   __REG32 P32e                      : 1;
   __REG32 P64e                      : 1;
   __REG32 P128e                     : 1;
   __REG32 P256e                     : 1;
   __REG32 P512e                     : 1;
   __REG32 P1024e                    : 1;
   __REG32 P2048e                    : 1;
   __REG32                           : 4;
   __REG32 P1o                       : 1;
   __REG32 P2o                       : 1;
   __REG32 P4o                       : 1;
   __REG32 P8o                       : 1;
   __REG32 P16o                      : 1;
   __REG32 P32o                      : 1;
   __REG32 P64o                      : 1;
   __REG32 P128o                     : 1;
   __REG32 P256o                     : 1;
   __REG32 P512o                     : 1;
   __REG32 P1024o                    : 1;
   __REG32 P2048o                    : 1;
   __REG32                           : 4;
} __gpmc_ecc_result_bits;

/* GPMC_BCH_SWDATA */
typedef struct {
   __REG32 BCH_DATA                  :16;
   __REG32                           :16;
} __gpmc_bch_swdata_bits;

/* EMIF_MOD_ID_REV */
typedef struct {
  __REG32 REG_MINOR_REVISION        : 6;
  __REG32                           : 2;
  __REG32 REG_MAJOR_REVISION        : 3;
  __REG32 REG_RTL_VERSION           : 5;
  __REG32 REG_MODULE_ID             :12;
  __REG32                           : 2;
  __REG32 REG_SCHEME                : 2;
} __emif_mod_id_rev_bits;

/* EMIF_STATUS */
typedef struct {
  __REG32                           : 2;
  __REG32 REG_PHY_DLL_READY         : 1;
  __REG32                           :26;
  __REG32 REG_FAST_INIT             : 1;
  __REG32 REG_DUAL_CLK_MODE         : 1;
  __REG32 REG_BE                    : 1;
} __emif_status_bits;

/* EMIF_SDRAM_CONFIG */
typedef struct {
  __REG32 REG_PAGESIZE              : 3;
  __REG32 REG_EBANK                 : 1;
  __REG32 REG_IBANK                 : 3;
  __REG32 REG_ROWSIZE               : 3;
  __REG32 REG_CL                    : 4;
  __REG32 REG_NARROW_MODE           : 2;
  __REG32 REG_CWL                   : 2;
  __REG32 REG_SDRAM_DRIVE           : 2;
  __REG32 REG_DDR_DISABLE_DLL       : 1;
  __REG32 REG_DYN_ODT               : 2;
  __REG32 REG_DDR2_DDQS             : 1;
  __REG32 REG_DDR_TERM              : 3;
  __REG32 REG_IBANK_POS             : 2;
  __REG32 REG_SDRAM_TYPE            : 3;
} __emif_sdram_config_bits;

/* EMIF_SDRAM_CONFIG_2 */
typedef struct {
  __REG32                           :27;
  __REG32 REG_EBANK_POS             : 1;
  __REG32                           : 4;
} __emif_sdram_config_2_bits;

/* EMIF_SDRAM_REF_CTRL */
typedef struct {
  __REG32 REG_REFRESH_RATE          :16;
  __REG32                           : 8;
  __REG32 REG_PASR                  : 3;
  __REG32                           : 1;
  __REG32 REG_ASR                   : 1;
  __REG32 REG_SRT                   : 1;
  __REG32                           : 1;
  __REG32 REG_INITREF_DIS           : 1;
} __emif_sdram_ref_ctrl_bits;

/* EMIF_SDRAM_REF_CTRL_SHDW */
typedef struct {
  __REG32 REG_REFRESH_RATE_SHDW     :16;
  __REG32                           :16;
} __emif_sdram_ref_ctrl_shdw_bits;

/* EMIF_SDRAM_TIM_1 */
typedef struct {
  __REG32 REG_T_WTR                 : 3;
  __REG32 REG_T_RRD                 : 3;
  __REG32 REG_T_RC                  : 6;
  __REG32 REG_T_RAS                 : 5;
  __REG32 REG_T_WR                  : 4;
  __REG32 REG_T_RCD                 : 4;
  __REG32 REG_T_RP                  : 4;
  __REG32                           : 3;
} __emif_sdram_tim_1_bits;

/* EMIF_SDRAM_TIM_1_SHDW */
typedef struct {
  __REG32 REG_T_WTR_SHDW            : 3;
  __REG32 REG_T_RRD_SHDW            : 3;
  __REG32 REG_T_RC_SHDW             : 6;
  __REG32 REG_T_RAS_SHDW            : 5;
  __REG32 REG_T_WR_SHDW             : 4;
  __REG32 REG_T_RCD_SHDW            : 4;
  __REG32 REG_T_RP_SHDW             : 4;
  __REG32                           : 3;
} __emif_sdram_tim_1_shdw_bits;

/* EMIF_SDRAM_TIM_2 */
typedef struct {
  __REG32 REG_T_CKE                 : 3;
  __REG32 REG_T_RTP                 : 3;
  __REG32 REG_T_XSRD                :10;
  __REG32 REG_T_XSNR                : 9;
  __REG32 REG_T_ODT                 : 3;
  __REG32 REG_T_XP                  : 3;
  __REG32                           : 1;
} __emif_sdram_tim_2_bits;

/* EMIF_SDRAM_TIM_2_SHDW */
typedef struct {
  __REG32 REG_T_CKE_SHDW            : 3;
  __REG32 REG_T_RTP_SHDW            : 3;
  __REG32 REG_T_XSRD_SHDW           :10;
  __REG32 REG_T_XSNR_SHDW           : 9;
  __REG32 REG_T_ODT_SHDW            : 3;
  __REG32 REG_T_XP_SHDW             : 3;
  __REG32                           : 1;
} __emif_sdram_tim_2_shdw_bits;

/* EMIF_SDRAM_TIM_3 */
typedef struct {
  __REG32 REG_T_RAS_MAX             : 4;
  __REG32 REG_T_RFC                 : 9;
  __REG32                                       : 2;
  __REG32 REG_ZQ_ZQCS               : 6;
  __REG32                           : 7;
  __REG32 REG_T_PDLL_UL             : 4;
} __emif_sdram_tim_3_bits;

/* EMIF_SDRAM_TIM_3_SHDW */
typedef struct {
  __REG32 REG_T_RAS_MAX_SHDW        : 4;
  __REG32 REG_T_RFC_SHDW            : 9;
  __REG32                                             : 2;
  __REG32 REG_ZQ_ZQCS_SHDW          : 6;
  __REG32                           : 7;
  __REG32 REG_T_PDLL_UL_SHDW        : 4;
} __emif_sdram_tim_3_shdw_bits;

/* EMIF_PWR_MGMT_CTRL */
typedef struct {
  __REG32 REG_CS_TIM                : 4;
  __REG32 REG_SR_TIM                : 4;
  __REG32 REG_LP_MODE               : 3;
  __REG32 REG_DPD_EN                : 1;
  __REG32 REG_PD_TIM                : 4;
  __REG32                           :16;
} __emif_pwr_mgmt_ctrl_bits;

/* EMIF_PWR_MGMT_CTRL_SHDW */
typedef struct {
  __REG32 REG_CS_TIM_SHDW           : 4;
  __REG32 REG_SR_TIM_SHDW           : 4;
  __REG32 REG_PD_TIM_SHDW           : 4;
  __REG32                           :20;
} __emif_pwr_mgmt_ctrl_shdw_bits;

/* Interface Configuration Register */
typedef struct {
   __REG32 REG_PR_OLD_COUNT          : 8;
   __REG32 REG_COS_COUNT_2           : 8;
   __REG32 REG_COS_COUNT_1           : 8;
   __REG32                           : 8;
} __emif_ic_bits;

/* Interface Configuration Value 1 Register */
typedef struct {
  __REG32 REG_CMD_FIFO_DEPTH        : 8;
  __REG32 REG_WR_FIFO_DEPTH         : 8;
  __REG32                           :14;
  __REG32 REG_SYS_BUS_WIDTH         : 2;
} __emif_icv1_bits;

/* Interface Configuration Value 2 Register */
typedef struct {
  __REG32 REG_RCMD_FIFO_DEPTH       : 8;
  __REG32 REG_RSD_FIFO_DEPTH        : 8;
  __REG32 REG_RREG_FIFO_DEPTH       : 8;
  __REG32                           : 8;
} __emif_icv2_bits;

/* EMIF_PERF_CNT_CFG */
typedef struct {
  __REG32 REG_CNTR1_CFG             : 4;
  __REG32                           :10;
  __REG32 REG_CNTR1_REGION_EN       : 1;
  __REG32 REG_CNTR1_MCONNID_EN      : 1;
  __REG32 REG_CNTR2_CFG             : 4;
  __REG32                           :10;
  __REG32 REG_CNTR2_REGION_EN       : 1;
  __REG32 REG_CNTR2_MCONNID_EN      : 1;
} __emif_perf_cnt_cfg_bits;

/* EMIF_PERF_CNT_SEL */
typedef struct {
  __REG32 REG_REGION_SEL1           : 2;
  __REG32                           : 6;
  __REG32 REG_MCONNID1              : 8;
  __REG32 REG_REGION_SEL2           : 2;
  __REG32                           : 6;
  __REG32 REG_MCONNID2              : 8;
} __emif_perf_cnt_sel_bits;

/* EMIF_READ_IDLE_CTRL */
typedef struct {
  __REG32 REG_READ_IDLE_INTERVAL    : 9;
  __REG32                           : 7;
  __REG32 REG_READ_IDLE_LEN         : 4;
  __REG32                           :12;
} __emif_read_idle_ctrl_bits;

/* EMIF_READ_IDLE_CTRL_SHDW */
typedef struct {
  __REG32 REG_READ_IDLE_INTERVAL_SHDW : 9;
  __REG32                             : 7;
  __REG32 REG_READ_IDLE_LEN_SHDW      : 4;
  __REG32                             :12;
} __emif_read_idle_ctrl_shdw_bits;

/* EMIF_IRQSTATUS_RAW_SYS */
typedef struct {
  __REG32 REG_ERR_SYS               : 1;
  __REG32 REG_TA_SYS                : 1;
  __REG32                           :30;
} __emif_irqstatus_raw_sys_bits;

/* EMIF_IRQSTATUS_SYS */
typedef struct {
  __REG32 REG_ERR_SYS               : 1;
  __REG32 REG_TA_SYS                : 1;
  __REG32                           :30;
} __emif_irqstatus_sys_bits;

/* EMIF_IRQENABLE_SET_SYS */
typedef struct {
  __REG32 REG_EN_ERR_SYS            : 1;
  __REG32 REG_EN_TA_SYS             : 1;
  __REG32                           :30;
} __emif_irqenable_set_sys_bits;

/* EMIF_IRQENABLE_CLR_SYS */
typedef struct {
  __REG32 REG_EN_ERR_SYS            : 1;
  __REG32 REG_EN_TA_SYS             : 1;
  __REG32                           :30;
} __emif_irqenable_clr_sys_bits;

/* EMIF_ZQ_CONFIG */
typedef struct {
  __REG32 REG_ZQ_REFINTERVAL        :16;
  __REG32 REG_ZQ_ZQCL_MULT          : 2;
  __REG32 REG_ZQ_ZQINIT_MULT        : 2;
  __REG32                           : 8;
  __REG32 REG_ZQ_SFEXITEN           : 1;
  __REG32 REG_ZQ_DUALCALEN          : 1;
  __REG32 REG_ZQ_CS0EN              : 1;
  __REG32 REG_ZQ_CS1EN              : 1;
} __emif_zq_config_bits;

/* Read-Write Leveling Ramp Window Register */
typedef struct {
  __REG32 REG_RDWRLVLINC_RMP_WIN    :13;
  __REG32                           :19;
} __emif_rwlrw_bits;

/* Read-Write Leveling Ramp Control Register */
typedef struct {
  __REG32 REG_WRLVLINC_RMP_INT      : 8;
  __REG32 REG_RDLVLGATEINC_RMP_INT  : 8;
  __REG32 REG_RDLVLINC_RMP_INT        : 8;
  __REG32 REG_RDWRLVLINC_RMP_PRE      : 7;
  __REG32 REG_RDWRLVL_EN                      : 1;
} __emif_rwlrc_bits;

/* Read-Write Leveling Control Register */
typedef struct {
  __REG32 REG_WRLVLINC_INT              : 8;
  __REG32 REG_RDLVLGATEINC_INT      : 8;
  __REG32 REG_RDLVLINC_INT              : 8;
  __REG32 REG_RDWRLVLINC_PRE            : 7;
  __REG32 REG_RDWRLVLFULL_START       : 1;
} __emif_rwlc_bits;

/* EMIF_DDR_PHY_CTRL_1 */
/* EMIF_DDR_PHY_CTRL_1_SHDW */
typedef struct {
  __REG32 REG_READ_LATENCY                  : 5;
  __REG32                                                   : 3;
  __REG32 REG_PHY_RD_LOCAL_ODT              : 2;
  __REG32 REG_PHY_WR_LOCAL_ODT              : 2;
  __REG32 REG_PHY_IDLE_LOCAL_ODT            : 2;
  __REG32                                                   : 1;
  __REG32 REG_PHY_RST_N                             : 1;
  __REG32                                                   : 4;
  __REG32 REG_PHY_ENABLE_DYNAMIC_PWRDN  : 1;
  __REG32                                                   :11;
} __emif_ddr_phy_ctrl_1_bits;

/* Priority to Class of Service Mapping Register */
typedef struct {
  __REG32 REG_PRI_0_COS                     : 2;
  __REG32 REG_PRI_1_COS                     : 2;
  __REG32 REG_PRI_2_COS                     : 2;
  __REG32 REG_PRI_3_COS                     : 2;
  __REG32 REG_PRI_4_COS                     : 2;
  __REG32 REG_PRI_5_COS                     : 2;
  __REG32 REG_PRI_6_COS                     : 2;
  __REG32 REG_PRI_7_COS                     : 2;
  __REG32                                                   :15;
  __REG32 REG_PRI_COS_MAP_EN                : 1;
} __emif_ptcosm_bits;

/* Connection ID to Class of Service 1 Mapping Register */
typedef struct {
  __REG32 REG_MSK_3_COS_1                   : 2;
  __REG32 REG_CONNID_3_COS_1                : 8;
  __REG32 REG_MSK_2_COS_1                   : 2;
  __REG32 REG_CONNID_2_COS_1                : 8;
  __REG32 REG_MSK_1_COS_1                   : 3;
  __REG32 REG_CONNID_1_COS_1                : 8;
  __REG32 REG_CONNID_COS_1_MAP_EN           : 1;
} __emif_cidtcos1m_bits;

/* Connection ID to Class of Service 1 Mapping Register */
typedef struct {
  __REG32 REG_MSK_3_COS_2                   : 2;
  __REG32 REG_CONNID_3_COS_2                : 8;
  __REG32 REG_MSK_2_COS_2                   : 2;
  __REG32 REG_CONNID_2_COS_2                : 8;
  __REG32 REG_MSK_1_COS_2                   : 3;
  __REG32 REG_CONNID_1_COS_2                : 8;
  __REG32 REG_CONNID_COS_2_MAP_EN           : 1;
} __emif_cidtcos2m_bits;

/* Read Write Execution Threshold Register */
typedef struct {
  __REG32 REG_RD_THRSH                      : 5;
  __REG32                                                   : 3;
  __REG32 REG_WR_THRSH                      : 5;
  __REG32                                                   :19;
} __emif_rwet_bits;

/* DDR PHY Command 0/1/2 Address/Command Slave Ratio Register
(CMD0/1/2_REG_PHY_CTRL_SLAVE_RATIO_0) */
typedef struct {
  __REG32 CMD_SLAVE_RATIO           :10;
  __REG32                           :22;
} __cmd_reg_phy_ctrl_slave_ratio_0_bits;

/* DDR PHY Command 0/1/2 Address/Command DLL Lock Difference Register(
CMD0/1/2_REG_PHY_DLL_LOCK_DIFF_0) */
typedef struct {
  __REG32 DLL_LOCK_DIFF             : 4;
  __REG32                           :28;
} __cmd_reg_phy_dll_lock_diff_0_bits;

/* DDR PHY Command 0/1/2 Invert Clockout Selection Register(
CMD0/1/2_REG_PHY_INVERT_CLKOUT_0) */
typedef struct {
  __REG32 INVERT_CLK_SEL            : 1;
  __REG32                           :31;
} __cmd_reg_phy_invert_clkout_0_bits;

/* DDR PHY Data Macro 0/1 Read DQS Slave Ratio Register
(DATA0/1_REG_PHY_RD_DQS_SLAVE_RATIO_0) */
typedef struct {
  __REG32 RD_DQS_SLAVE_RATIO_CS0    :10;
  __REG32                           :22;
} __data_reg_phy_rd_dqs_slave_ratio_0_bits;

/* DDR PHY Data Macro 0/1 Write DQS Slave Ratio Register
(DATA0/1_REG_PHY_WR_DQS_SLAVE_RATIO_0) */
typedef struct {
  __REG32 WR_DQS_SLAVE_RATIO_CS0    :10;
  __REG32                           :22;
} __data_reg_phy_wr_dqs_slave_ratio_0_bits;

/* DDR PHY Data Macro 0/1 Write Leveling Init Ratio Register
(DATA0/1_REG_PHY_WRLVL_INIT_RATIO_0) */
typedef struct {
  __REG32 WRLVL_INIT_RATIO_CS0      :10;
  __REG32                           :22;
} __data_reg_phy_wrlvl_init_ratio_0_bits;

/* DDR PHY Data Macro 0 Write Leveling Init Mode Ratio Selection Register
(DATA0/1_REG_PHY_WRLVL_INIT_MODE_0) */
typedef struct {
  __REG32 WRLVL_INIT_MODE_SEL       : 1;
  __REG32                           :31;
} __data_reg_phy_wrlvl_init_mode_0_bits;

/* DDR PHY Data Macro 0 DQS Gate Training Init Ratio Register
(DATA0_REG_PHY_GATELVL_INIT_RATIO_0) */
typedef struct {
  __REG32 GATELVL_INIT_RATIO_CS0    :10;
  __REG32                           :22;
} __data_reg_phy_gatelvl_init_ratio_0_bits;

/* DDR PHY Data Macro 0/1 DQS Gate Training Init Mode Ratio Selection Register
(DATA0/1_REG_PHY_GATELVL_INIT_MODE_0) */
typedef struct {
  __REG32 GATELVL_INIT_MODE_SEL     : 1;
  __REG32                           :31;
} __data_reg_phy_gatelvl_init_mode_0_bits;

/* DDR PHY Data Macro 0/1 DQS Gate Slave Ratio Register
(DATA0/1_REG_PHY_FIFO_WE_SLAVE_RATIO_0) */
typedef struct {
  __REG32 FIFO_WE_SLAVE_RATIO_CS0   :10;
  __REG32                           :22;
} __data_reg_phy_fifo_we_slave_ratio_0_bits;

/* DDR PHY Data Macro 0/1 Write Data Slave Ratio Register
(DATA0/1_REG_PHY_WR_DATA_SLAVE_RATIO_0) */
typedef struct {
  __REG32 WR_DATA_SLAVE_RATIO_CS0   :10;
  __REG32                           :22;
} __data_reg_phy_wr_data_slave_ratio_0_bits;

/* DDR PHY Data Macro 0/1 Delay Selection Register
(DATA0/1_REG_PHY_USE_RANK0_DELAYS) */
typedef struct {
  __REG32 RANK0_DELAY               : 1;
  __REG32                           :31;
} __data_reg_phy_use_rank0_delays_bits;

/* DDR PHY Data 0/1 DLL Lock Difference Register
 (DATA0/1_REG_PHY_DLL_LOCK_DIFF_0) */
typedef struct {
  __REG32 DLL_LOCK_DIFF             : 4;
  __REG32                           :28;
} __data_reg_phy_dll_lock_diff_0_bits;

/* ELM System Configuration Register (ELM_SYSCONFIG) */
typedef struct {
  __REG32 AUTOGATING          : 1;
  __REG32 SOFTRESET           : 1;
  __REG32                     : 1;
  __REG32 SIDLEMODE           : 2;
  __REG32                     : 3;
  __REG32 CLOCKACTIVITYOCP    : 1;
  __REG32                     :23;
} __elm_sysconfig_bits;

/* ELM System Status Register (ELM_SYSSTATUS) */
typedef struct {
  __REG32 RESETDONE           : 1;
  __REG32                     :31;
} __elm_sysstatus_bits;

/* ELM Interrupt Status Register (ELM_IRQSTATUS) */
typedef struct {
  __REG32 LOC_VALID_0         : 1;
  __REG32 LOC_VALID_1         : 1;
  __REG32 LOC_VALID_2         : 1;
  __REG32 LOC_VALID_3         : 1;
  __REG32 LOC_VALID_4         : 1;
  __REG32 LOC_VALID_5         : 1;
  __REG32 LOC_VALID_6         : 1;
  __REG32 LOC_VALID_7         : 1;
  __REG32 PAGE_VALID          : 1;
  __REG32                     :23;
} __elm_irqstatus_bits;

/* ELM Interrupt Enable Register (ELM_IRQENABLE) */
typedef struct {
  __REG32 LOCATION_MASK_0     : 1;
  __REG32 LOCATION_MASK_1     : 1;
  __REG32 LOCATION_MASK_2     : 1;
  __REG32 LOCATION_MASK_3     : 1;
  __REG32 LOCATION_MASK_4     : 1;
  __REG32 LOCATION_MASK_5     : 1;
  __REG32 LOCATION_MASK_6     : 1;
  __REG32 LOCATION_MASK_7     : 1;
  __REG32 PAGE_MASK           : 1;
  __REG32                     :23;
} __elm_irqenable_bits;

/* ELM Location Configuration Register (ELM_LOCATION_CONFIG) */
typedef struct {
  __REG32 ECC_BCH_LEVEL       : 2;
  __REG32                     :14;
  __REG32 ECC_SIZE            :11;
  __REG32                     : 5;
} __elm_location_config_bits;

/* ELM Page Definition Register (ELM_PAGE_CTRL) */
typedef struct {
  __REG32 SECTOR_0            : 1;
  __REG32 SECTOR_1            : 1;
  __REG32 SECTOR_2            : 1;
  __REG32 SECTOR_3            : 1;
  __REG32 SECTOR_4            : 1;
  __REG32 SECTOR_5            : 1;
  __REG32 SECTOR_6            : 1;
  __REG32 SECTOR_7            : 1;
  __REG32                     :24;
} __elm_page_ctrl_bits;

/* ELM_SYNDROME_FRAGMENT_0_i */
typedef struct {
  __REG32 SYNDROME_0          :32;
} __elm_syndrome_fragment_0_bits;

/* ELM_SYNDROME_FRAGMENT_1_i */
typedef struct {
  __REG32 SYNDROME_1          :32;
} __elm_syndrome_fragment_1_bits;

/* ELM_SYNDROME_FRAGMENT_2_i */
typedef struct {
  __REG32 SYNDROME_2          :32;
} __elm_syndrome_fragment_2_bits;

/* ELM_SYNDROME_FRAGMENT_3_i */
typedef struct {
  __REG32 SYNDROME_3          :32;
} __elm_syndrome_fragment_3_bits;

/* ELM_SYNDROME_FRAGMENT_4_i */
typedef struct {
  __REG32 SYNDROME_4          :32;
} __elm_syndrome_fragment_4_bits;

/* ELM_SYNDROME_FRAGMENT_5_i */
typedef struct {
  __REG32 SYNDROME_5          :32;
} __elm_syndrome_fragment_5_bits;

/* ELM_SYNDROME_FRAGMENT_6_i */
typedef struct {
  __REG32 SYNDROME_6          :16;
  __REG32 SYNDROME_VALID      : 1;
  __REG32                     :15;
} __elm_syndrome_fragment_6_bits;

/* ELM_LOCATION_STATUS_i */
typedef struct {
  __REG32 ECC_NB_ERRORS       : 5;
  __REG32                     : 3;
  __REG32 ECC_CORRECTABL      : 1;
  __REG32                     :23;
} __elm_location_status_bits;

/* ELM_ERROR_LOCATION_0-15_i */
typedef struct {
  __REG32 ECC_ERROR_LOCATION  :13;
  __REG32                     :19;
} __elm_error_location_bits;

/* CM_PER_L4LS_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                 : 2;
  __REG32                           : 6;
  __REG32 CLKACTIVITY_L4LS_GCLK     : 1;
  __REG32                           : 1;
  __REG32 CLKACTIVITY_UART_GFCLK    : 1;
  __REG32 CLKACTIVITY_CAN_CLK       : 1;
  __REG32                           : 1;
  __REG32 CLKACTIVITY_TIMER7_GCLK   : 1;
  __REG32 CLKACTIVITY_TIMER2_GCLK   : 1;
  __REG32 CLKACTIVITY_TIMER3_GCLK   : 1;
  __REG32 CLKACTIVITY_TIMER4_GCLK   : 1;
  __REG32 CLKACTIVITY_LCDC_GCLK     : 1;
  __REG32                           : 1;
  __REG32 CLKACTIVITY_GPIO_1_GDBCLK : 1;
  __REG32 CLKACTIVITY_GPIO_2_GDBCLK : 1;
  __REG32 CLKACTIVITY_GPIO_3_GDBCLK : 1;
  __REG32                           : 2;
  __REG32 CLKACTIVITY_I2C_FCLK      : 1;
  __REG32 CLKACTIVITY_SPI_GCLK      : 1;
  __REG32                           : 1;
  __REG32 CLKACTIVITY_TIMER5_GCLK   : 1;
  __REG32 CLKACTIVITY_TIMER6_GCLK   : 1;
  __REG32                           : 3;
} __cm_per_l4ls_clkstctrl_bits;

/* CM_PER_L3S_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                 : 2;
  __REG32                           : 1;
  __REG32 CLKACTIVITY_L3S_GCLK      : 1;
  __REG32                           :28;
} __cm_per_l3s_clkstctrl_bits;

/* CM_PER_L3_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                 : 2;
  __REG32 CLKACTIVITY_EMIF_GCLK     : 1;
  __REG32 CLKACTIVITY_MMC_FCLK      : 1;
  __REG32 CLKACTIVITY_L3_GCLK       : 1;
  __REG32                           : 1;
  __REG32 CLKACTIVITY_CPTS_RFT_GCLK : 1;
  __REG32 CLKACTIVITY_MCASP_GCLK    : 1;
  __REG32                           :24;
} __cm_per_l3_clkstctrl_bits;

/* CM_PER_CPGMAC0_CLKCTRL Register */
/* CM_PER_LCDC_CLKCTRL Register */
/* CM_PER_USB0_CLKCTRL Register */
/* CM_PER_TPTC0_CLKCTRL Register */
/* CM_PER_IEEE5000_CLKCTRL Register */
/* CM_PER_PRUSS_CLKCTRL Register */
/* CM_PER_TPTC1_CLKCTRL Register */
/* CM_PER_TPTC2_CLKCTRL Register */
/* CM_PER_OCPWP_CLKCTRL Register */
/* CM_MPU_MPU_CLKCTRL Register */
/* CM_GFX_GFX_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32 STBYST                    : 1;
  __REG32                           :13;
} __cm_clkctrl0_bits;

/* CM_PER_EMIF_CLKCTRL Register */
/* CM_PER_OCMCRAM_CLKCTRL Register */
/* CM_PER_GPMC_CLKCTRL Register */
/* CM_PER_MCASP0_CLKCTRL Register */
/* CM_PER_UART5_CLKCTRL Register */
/* CM_PER_MMC0_CLKCTRL Register */
/* CM_PER_ELM_CLKCTRL Register */
/* CM_PER_I2C2_CLKCTRL Register */
/* CM_PER_I2C1_CLKCTRL Register */
/* CM_PER_SPI0_CLKCTRL Register */
/* CM_PER_SPI1_CLKCTRL Register */
/* CM_PER_L4LS_CLKCTRL Register */
/* CM_PER_MCASP1_CLKCTRL Register */
/* CM_PER_UART1_CLKCTRL Register */
/* CM_PER_UART2_CLKCTRL Register */
/* CM_PER_UART3_CLKCTRL Register */
/* CM_PER_UART4_CLKCTRL Register */
/* CM_PER_TIMER7_CLKCTRL Register */
/* CM_PER_TIMER2_CLKCTRL Register */
/* CM_PER_TIMER3_CLKCTRL Register */
/* CM_PER_TIMER4_CLKCTRL Register */
/* CM_PER_DCAN0_CLKCTRL Register */
/* CM_PER_DCAN1_CLKCTRL Register */
/* CM_PER_EPWMSS1_CLKCTRL Register */
/* CM_PER_EPWMSS0_CLKCTRL Register */
/* CM_PER_EPWMSS2_CLKCTRL Register */
/* CM_PER_L3_INSTR_CLKCTRL Register */
/* CM_PER_L3_CLKCTRL Register */
/* CM_PER_TIMER5_CLKCTRL Register */
/* CM_PER_TIMER6_CLKCTRL Register */
/* CM_PER_MMC1_CLKCTRL Register */
/* CM_PER_MMC2_CLKCTRL Register */
/* CM_PER_SPINLOCK_CLKCTRL Register */
/* CM_PER_MAILBOX0_CLKCTRL Register */
/* CM_PER_L4HS_CLKCTRL Register */
/* CM_PER_CLKDIV32K_CLKCTRL Register */
/* CM_WKUP_CONTROL_CLKCTRL Register */
/* CM_WKUP_L4WKUP_CLKCTRL Register */
/* CM_WKUP_TIMER0_CLKCTRL Register */
/* CM_WKUP_UART0_CLKCTRL Register */
/* CM_WKUP_I2C0_CLKCTRL Register */
/* CM_WKUP_ADC_TSC_CLKCTRL Register */
/* CM_WKUP_SMARTREFLEX0_CLKCTRL Register */
/* CM_WKUP_TIMER1_CLKCTRL Register */
/* CM_WKUP_SMARTREFLEX1_CLKCTRL Register */
/* CM_WKUP_SMARTREFLEX1_CLKCTRL Register */
/* CM_WKUP_WDT1_CLKCTRL Register */
/* CM_RTC_RTC_CLKCTRL Register */
/* CM_GFX_MMUCFG_CLKCTRL Register */
/* CM_GFX_MMUDATA_CLKCTRL Register */
/* CM_CEFUSE_CEFUSE_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32                           :14;
} __cm_clkctrl1_bits;

/* CM_PER_GPIO1_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32 OPTFCLKEN_GPIO_1_GDBCLK   : 1;
  __REG32                           :13;
} __cm_per_gpio1_clkctrl_bits;

/* CM_PER_GPIO2_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32 OPTFCLKEN_GPIO_2_GDBCLK   : 1;
  __REG32                           :13;
} __cm_per_gpio2_clkctrl_bits;

/* CM_PER_GPIO3_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32 OPTFCLKEN_GPIO_3_GDBCLK   : 1;
  __REG32                           :13;
} __cm_per_gpio3_clkctrl_bits;

/* CM_PER_L4HS_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32                               : 1;
  __REG32 CLKACTIVITY_L4HS_GCLK         : 1;
  __REG32 CLKACTIVITY_CPSW_250MHZ_GCLK  : 1;
  __REG32 CLKACTIVITY_CPSW_50MHZ_GCLK   : 1;
  __REG32 CLKACTIVITY_CPSW_5MHZ_GCLK    : 1;
  __REG32                               :25;
} __cm_per_l4hs_clkstctrl_bits;

/* CM_PER_OCPWP_L3_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32                               : 2;
  __REG32 CLKACTIVITY_OCPWP_L3_GCLK     : 1;
  __REG32 CLKACTIVITY_OCPWP_L4_GCLK     : 1;
  __REG32                               :26;
} __cm_per_ocpwp_l3_clkstctrl_bits;

/* CM_PER_PRUSS_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32                               : 2;
  __REG32 CLKACTIVITY_PRUSS_OCP_GCLK    : 1;
  __REG32 CLKACTIVITY_PRUSS_IEP_GCLK    : 1;
  __REG32 CLKACTIVITY_PRUSS_UART_GCLK   : 1;
  __REG32                               :25;
} __cm_per_pru_icss_clkstctrl_bits;

/* CM_PER_CPSW_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32                               : 2;
  __REG32 CLKACTIVITY_CPSW_125MHz_GCLK  : 1;
  __REG32                               :27;
} __cm_per_cpsw_clkstctrl_bits;

/* CM_PER_LCDC_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32                               : 2;
  __REG32 CLKACTIVITY_LCDC_L3_OCP_GCLK  : 1;
  __REG32 CLKACTIVITY_LCDC_L4_OCP_GCLK  : 1;
  __REG32                               :26;
} __cm_per_lcdc_clkstctrl_bits;

/* CM_PER_CLK_24MHZ_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32                               : 2;
  __REG32 CLKACTIVITY_CLK_24MHZ_GCLK    : 1;
  __REG32                               :27;
} __cm_per_clk_24mhz_clkstctrl_bits;

/* CM_WKUP_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                     : 2;
  __REG32 CLKACTIVITY_L4_WKUP_GCLK      : 1;
  __REG32 CLKACTIVITY_SR_SYSCLK         : 1;
  __REG32 CLKACTIVITY_WDT1_GCLK         : 1;
  __REG32                               : 3;
  __REG32 CLKACTIVITY_GPIO0_GDBCLK      : 1;
  __REG32                               : 1;
  __REG32 CLKACTIVITY_TIMER0_GCLK       : 1;
  __REG32 CLKACTIVITY_I2C0_GFCLK        : 1;
  __REG32 CLKACTIVITY_UART0_GFCLK       : 1;
  __REG32 CLKACTIVITY_TIMER1_GCLK       : 1;
  __REG32 CLKACTIVITY_ADC_FCLK          : 1;
  __REG32                               :17;
} __cm_wkup_clkstctrl_bits;

/* CM_WKUP_GPIO0_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32 OPTFCLKEN_GPIO0_GDBCLK    : 1;
  __REG32                           :13;
} __cm_wkup_gpio0_clkctrl_bits;

/* CM_WKUP_DEBUGSS_CLKCTRL Register */
typedef struct {
  __REG32 MODULEMODE                : 2;
  __REG32                           :14;
  __REG32 IDLEST                    : 2;
  __REG32 STBYST                    : 1;
  __REG32 OPTFCLKEN_DBGSYSCLK       : 1;
  __REG32 TRC_PMD_CLKSEL            : 2;
  __REG32 STM_PMD_CLKSEL            : 2;
  __REG32 TRC_PMD_CLKDIVSEL         : 3;
  __REG32 STM_PMD_CLKDIVSEL         : 3;
  __REG32 OPTCLK_DEBUG_CLKA         : 1;
  __REG32                           : 1;
} __cm_wkup_debugss_clkctrl_bits;

/* CM_L3_AON_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                 : 2;
  __REG32 CLKACTIVITY_DBGSYSCLK     : 1;
  __REG32 CLKACTIVITY_L3_AON_GCLK   : 1;
  __REG32 CLKACTIVITY_DEBUG_CLKA    : 1;
  __REG32                           :27;
} __cm_l3_aon_clkstctrl_bits;

/* CM_AUTOIDLE_DPLL_MPU Register */
/* CM_AUTOIDLE_DPLL_DDR Register */
/* CM_AUTOIDLE_DPLL_DISP Register */
/* CM_AUTOIDLE_DPLL_CORE Register */
/* CM_AUTOIDLE_DPLL_PER Register */
typedef struct {
  __REG32 AUTO_DPLL_MODE            : 3;
  __REG32                           :29;
} __cm_autoidle_dpll_bits;

/* CM_IDLEST_DPLL_MPU Register */
/* CM_IDLEST_DPLL_DDR Register */
/* CM_IDLEST_DPLL_DISP Register */
/* CM_IDLEST_DPLL_CORE Register */
/* CM_IDLEST_DPLL_PER Register */
typedef struct {
  __REG32 ST_DPLL_CLK         : 1;
  __REG32                     : 7;
  __REG32 ST_MN_BYPASS        : 1;
  __REG32                     :23;
} __cm_idlest_dpll_bits;

/* CM_SSC_DELTAMSTEP_DPLL_MPU Register */
/* CM_SSC_DELTAMSTEP_DPLL_DDR Register */
/* CM_SSC_DELTAMSTEP_DPLL_DISP Register */
/* CM_SSC_DELTAMSTEP_DPLL_CORE Register */
/* CM_SSC_DELTAMSTEP_DPLL_PER Register */
typedef struct {
  __REG32 DELTAMSTEP          :20;
  __REG32                     :12;
} __cm_ssc_deltamstep_dpll_bits;

/* CM_SSC_MODFREQDIV_DPLL_MPU Register */
/* CM_SSC_MODFREQDIV_DPLL_DDR Register */
/* CM_SSC_MODFREQDIV_DPLL_DISP Register */
/* CM_SSC_MODFREQDIV_DPLL_CORE Register */
/* CM_SSC_MODFREQDIV_DPLL_PER Register */
typedef struct {
  __REG32 MODFREQDIV_MANTISSA : 7;
  __REG32                     : 1;
  __REG32 MODFREQDIV_EXPONENT : 3;
  __REG32                     :21;
} __cm_ssc_modfreqdiv_dpll_bits;

/* CM_CLKSEL_DPLL_MPU Register */
/* CM_CLKSEL_DPLL_DDR Register */
/* CM_CLKSEL_DPLL_DISP Register */
/* CM_CLKSEL_DPLL_CORE Register */
typedef struct {
  __REG32 DPLL_DIV            : 7;
  __REG32                     : 1;
  __REG32 DPLL_MULT           :11;
  __REG32                     : 4;
  __REG32 DPLL_BYP_CLKSEL     : 1;
  __REG32                     : 8;
} __cm_clksel_dpll_bits;

/* CM_CLKDCOLDO_DPLL_PER Register */
typedef struct {
  __REG32                           : 8;
  __REG32 DPLL_CLKDCOLDO_GATE_CTRL  : 1;
  __REG32 ST_DPLL_CLKDCOLDO         : 1;
  __REG32                           : 2;
  __REG32 DPLL_CLKDCOLDO_PWDN       : 1;
  __REG32                           :19;
} __cm_clkdcoldo_dpll_per_bits;

/* CM_DIV_M4_DPLL_CORE Register */
typedef struct {
  __REG32 HSDIVIDER_CLKOUT1_DIV       : 5;
  __REG32 HSDIVIDER_CLKOUT1_DIVCHACK  : 1;
  __REG32                             : 2;
  __REG32 HSDIVIDER_CLKOUT1_GATE_CTRL : 1;
  __REG32 ST_HSDIVIDER_CLKOUT1        : 1;
  __REG32                             : 2;
  __REG32 HSDIVIDER_CLKOUT1_PWDN      : 1;
  __REG32                             :19;
} __cm_div_m4_dpll_core_bits;

/* CM_DIV_M5_DPLL_CORE Register */
typedef struct {
  __REG32 HSDIVIDER_CLKOUT2_DIV       : 5;
  __REG32 HSDIVIDER_CLKOUT2_DIVCHACK  : 1;
  __REG32                             : 2;
  __REG32 HSDIVIDER_CLKOUT2_GATE_CTRL : 1;
  __REG32 ST_HSDIVIDER_CLKOUT2        : 1;
  __REG32                             : 2;
  __REG32 HSDIVIDER_CLKOUT2_PWDN      : 1;
  __REG32                             :19;
} __cm_div_m5_dpll_core_bits;

/* CM_CLKMODE_DPLL_MPU Register */
typedef struct {
  __REG32 DPLL_EN                     : 3;
  __REG32 DPLL_RAMP_LEVEL             : 2;
  __REG32 DPLL_RAMP_RATE              : 3;
  __REG32 DPLL_DRIFTGUARD_EN          : 1;
  __REG32 DPLL_RELOCK_RAMP_EN         : 1;
  __REG32 DPLL_LPMODE_EN              : 1;
  __REG32 DPLL_REGM4XEN               : 1;
  __REG32 DPLL_SSC_EN                 : 1;
  __REG32 DPLL_SSC_ACK                : 1;
  __REG32 DPLL_SSC_DOWNSPREAD         : 1;
  __REG32 DPLL_SSC_TYPE               : 1;
  __REG32                             :16;
} __cm_clkmode_dpll_bits;

/* CM_CLKMODE_DPLL_PER Register */
typedef struct {
  __REG32 DPLL_EN                     : 3;
  __REG32                             : 9;
  __REG32 DPLL_SSC_EN                 : 1;
  __REG32 DPLL_SSC_ACK                : 1;
  __REG32 DPLL_SSC_DOWNSPREAD         : 1;
  __REG32 DPLL_SSC_TYPE               : 1;
  __REG32                             :16;
} __cm_clkmode_dpll_per_bits;

/* CM_CLKSEL_DPLL_PERIPH Register */
typedef struct {
  __REG32 DPLL_DIV            : 8;
  __REG32 DPLL_MULT           :12;
  __REG32                     : 4;
  __REG32 DPLL_SD_DIV         : 8;
} __cm_clksel_dpll_periph_bits;

/* CM_DIV_M2_DPLL_DDR Register */
/* CM_DIV_M2_DPLL_DISP Register */
/* CM_DIV_M2_DPLL_MPU Register */
typedef struct {
  __REG32 DPLL_CLKOUT_DIV       : 5;
  __REG32 DPLL_CLKOUT_DIVCHACK  : 1;
  __REG32                       : 2;
  __REG32 DPLL_CLKOUT_GATE_CTRL : 1;
  __REG32 ST_DPLL_CLKOUT        : 1;
  __REG32                       :22;
} __cm_div_m2_dpll_bits;

/* CM_DIV_M2_DPLL_PER Register */
typedef struct {
  __REG32 DPLL_CLKOUT_DIV       : 7;
  __REG32 DPLL_CLKOUT_DIVCHACK  : 1;
  __REG32 DPLL_CLKOUT_GATE_CTRL : 1;
  __REG32 ST_DPLL_CLKOUT        : 1;
  __REG32                       :22;
} __cm_div_m2_dpll_per_bits;

/* CM_DIV_M2_DPLL_PER Register */
typedef struct {
  __REG32 MODULEMODE            : 2;
  __REG32                       :16;
  __REG32 STBYST                : 1;
  __REG32                       :13;
} __cm_wkup_wkup_m3_clkctrl_bits;

/* CM_L4_WKUP_AON_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                    : 2;
  __REG32 CLKACTIVITY_L4_WKUP_AON_GCLK : 1;
  __REG32                              :29;
} __cm_l4_wkup_aon_clkstctrl_bits;

/* CM_DIV_M6_DPLL_CORE Register */
typedef struct {
  __REG32 HSDIVIDER_CLKOUT3_DIV       : 5;
  __REG32 HSDIVIDER_CLKOUT3_DIVCHACK  : 1;
  __REG32                             : 2;
  __REG32 HSDIVIDER_CLKOUT3_GATE_CTRL : 1;
  __REG32 ST_HSDIVIDER_CLKOUT3        : 1;
  __REG32                             : 2;
  __REG32 HSDIVIDER_CLKOUT3_PWDN      : 1;
  __REG32                             :19;
} __cm_div_m6_dpll_core_bits;

/* CLKSEL_TIMER7_CLK Register */
/* CLKSEL_TIMER2_CLK Register */
/* CLKSEL_TIMER3_CLK Register */
/* CLKSEL_TIMER4_CLK Register */
/* CLKSEL_TIMER5_CLK Register */
/* CLKSEL_TIMER6_CLK Register */
typedef struct {
  __REG32 CLKSEL          : 2;
  __REG32                 :30;
} __clksel_timer_clk_bits;

/* CM_MAC_CLKSEL Register */
typedef struct {
  __REG32                 : 2;
  __REG32 MII_CLK_SEL     : 1;
  __REG32                 :29;
} __cm_mac_clksel_bits;

/* CM_CPTS_RFT_CLKSEL Register */
typedef struct {
  __REG32 CLKSEL          : 1;
  __REG32                 :31;
} __cm_cpts_rft_clksel_bits;

/* CLKSEL_TIMER1MS_CLK Register */
typedef struct {
  __REG32 CLKSEL          : 3;
  __REG32                 :29;
} __clksel_timer1ms_clk_bits;

/* CLKSEL_GFX_FCLK Register */
typedef struct {
  __REG32 CLKDIV_SEL_GFX_FCLK : 1;
  __REG32 CLKSEL_GFX_FCLK     : 1;
  __REG32                     :30;
} __clksel_gfx_fclk_bits;

/* CLKSEL_PRU_ICSS_OCP_CLK Register */
typedef struct {
  __REG32 CLKSEL          : 1;
  __REG32                 :31;
} __clksel_pru_icss_ocp_clk_bits;

/* CLKSEL_LCDC_PIXEL_CLK Register */
typedef struct {
  __REG32 CLKSEL          : 2;
  __REG32                 :30;
} __clksel_lcdc_pixel_clk_bits;

/* CLKSEL_WDT1_CLK Register */
typedef struct {
  __REG32 CLKSEL          : 1;
  __REG32                 :31;
} __clksel_wdt1_clk_bits;

/* CLKSEL_GPIO0_DBCLK Register */
typedef struct {
  __REG32 CLKSEL          : 2;
  __REG32                 :30;
} __clksel_gpio0_dbclk_bits;

/* CM_MPU_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL           : 2;
  __REG32 CLKACTIVITY_MPU_CLK : 1;
  __REG32                     :29;
} __cm_mpu_clkstctrl_bits;

/* CM_CLKOUT_CTRL Register */
typedef struct {
  __REG32 CLKOUT2SOURCE       : 3;
  __REG32 CLKOUT2DIV          : 3;
  __REG32                     : 1;
  __REG32 CLKOUT2EN           : 1;
  __REG32                     :24;
} __cm_clkout_ctrl_bits;

/* CM_RTC_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL               : 2;
  __REG32                         : 6;
  __REG32 CLKACTIVITY_L4_RTC_GCLK : 1;
  __REG32 CLKACTIVITY_RTC_32KCLK  : 1;
  __REG32                         :22;
} __cm_rtc_clkstctrl_bits;

/* CM_GFX_L3_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL               : 2;
  __REG32                         : 6;
  __REG32 CLKACTIVITY_GFX_L3_GCLK : 1;
  __REG32 CLKACTIVITY_GFX_FCLK    : 1;
  __REG32                         :22;
} __cm_gfx_l3_clkstctrl_bits;

/* CM_GFX_L4LS_GFX_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                 : 2;
  __REG32                           : 6;
  __REG32 CLKACTIVITY_GFX_FCLK      : 1;
  __REG32                           :23;
} __cm_gfx_l4ls_gfx_clkstctrl_bits;

/* CM_CEFUSE_CLKSTCTRL Register */
typedef struct {
  __REG32 CLKTRCTRL                       : 2;
  __REG32                                 : 6;
  __REG32 CLKACTIVITY_L4_CEFUSE_GICLK     : 1;
  __REG32 CLKACTIVITY_CUST_EFUSE_SYS_CLK  : 1;
  __REG32                                 :22;
} __cm_cefuse_clkstctrl_bits;

/* REVISION_PRM Register */
typedef struct {
  __REG32 REV                 : 8;
  __REG32                     :24;
} __revision_prm_bits;

/* PRM_IRQSTATUS_MPU Register */
/* PRM_IRQSTATUS_M3 Register */
typedef struct {
  __REG32                     : 8;
  __REG32 TRANSITION_ST       : 1;
  __REG32                     : 1;
  __REG32 FORCEWKUP_ST        : 1;
  __REG32 DPLL_MPU_RECAL_ST   : 1;
  __REG32 DPLL_CORE_RECAL_ST  : 1;
  __REG32 DPLL_DISP_RECAL_ST  : 1;
  __REG32 DPLL_DDR_RECAL_ST   : 1;
  __REG32 DPLL_PER_RECAL_ST   : 1;
  __REG32                     :16;
} __prm_irqstatus_bits;

/* PRM_IRQENABLE_MPU Register */
/* PRM_IRQENABLE_M3 Register */
typedef struct {
  __REG32                     : 8;
  __REG32 TRANSITION_EN       : 1;
  __REG32                     : 1;
  __REG32 FORCEWKUP_EN        : 1;
  __REG32 DPLL_MPU_RECAL_EN   : 1;
  __REG32 DPLL_CORE_RECAL_EN  : 1;
  __REG32 DPLL_DISP_RECAL_EN  : 1;
  __REG32 DPLL_DDR_RECAL_EN   : 1;
  __REG32 DPLL_PER_RECAL_EN   : 1;
  __REG32                     :16;
} __prm_irqenable_bits;

/* RM_PER_RSTCTRL Register */
typedef struct {
  __REG32                     : 1;
  __REG32 PRU_ICSS_LRST       : 1;
  __REG32                     :30;
} __rm_per_rstctrl_bits;

/* PM_PER_PWRSTST Register */
typedef struct {
  __REG32 POWERSTATEST        : 2;
  __REG32 LOGICSTATEST        : 1;
  __REG32                     :14;
  __REG32 PER_MEM_STATEST     : 2;
  __REG32                     : 1;
  __REG32 INTRANSITION        : 1;
  __REG32 RAM_MEM_STATEST     : 2;
  __REG32 PRU_ICSS_MEM_STATEST: 2;
  __REG32                     : 7;
} __pm_per_pwrstst_bits;

/* PM_PER_PWRSTCTRL Register */
typedef struct {
  __REG32 POWERSTATE            : 2;
  __REG32                       : 1;
  __REG32 LOGICRETSTATE         : 1;
  __REG32 LOWPOWERSTATECHANGE   : 1;
  __REG32 PRU_ICSS_MEM_ONSTATE  : 2;
  __REG32 PRU_ICSS_MEM_RETSTATE : 1;
  __REG32                       :17;
  __REG32 PER_MEM_ONSTATE       : 2;
  __REG32 RAM_MEM_RETSTATE      : 1;
  __REG32                       : 1;
  __REG32 PER_MEM_RETSTATE      : 1;
  __REG32 RAM_MEM_ONSTATE       : 2;
} __pm_per_pwrstctrl_bits;

/* RM_WKUP_RSTCTRL Register */
typedef struct {
  __REG32                     : 3;
  __REG32 WKUP_M3_LRST        : 1;
  __REG32                     :28;
} __rm_wkup_rstctrl_bits;

/* PM_WKUP_PWRSTCTRL Register */
typedef struct {
  __REG32                     : 3;
  __REG32 LOGICRETSTATE       : 1;
  __REG32 LOWPOWERSTATECHANGE : 1;
  __REG32                     :27;
} __pm_wkup_pwrstctrl_bits;

/* PM_WKUP_PWRSTST Register */
typedef struct {
  __REG32                     : 2;
  __REG32 LOGICSTATEST        : 1;
  __REG32                     :14;
  __REG32 DEBUGSS_MEM_STATEST : 2;
  __REG32                     : 1;
  __REG32 INTRANSITION        : 1;
  __REG32                     :11;
} __pm_wkup_pwrstst_bits;

/* RM_WKUP_RSTST Register */
typedef struct {
  __REG32                     : 5;
  __REG32 WKUP_M3_LRST        : 1;
  __REG32 EMULATION_M3_RST    : 1;
  __REG32 ICECRUSHER_M3_RST   : 1;
  __REG32                     :24;
} __rm_wkup_rsts_bits;

/* PM_MPU_PWRSTCTRL Register */
typedef struct {
  __REG32 POWERSTATE          : 2;
  __REG32 LOGICRETSTATE       : 1;
  __REG32                     : 1;
  __REG32 LOWPOWERSTATECHANGE : 1;
  __REG32                     :11;
  __REG32 MPU_RAM_ONSTATE     : 2;
  __REG32 MPU_L1_ONSTATE      : 2;
  __REG32 MPU_L2_ONSTATE      : 2;
  __REG32 MPU_L1_RETSTATE     : 1;
  __REG32 MPU_L2_RETSTATE     : 1;
  __REG32 MPU_RAM_RETSTATE    : 1;
  __REG32                     : 7;
} __pm_mpu_pwrstctrl_bits;

/* PM_MPU_PWRSTST Register */
typedef struct {
  __REG32 POWERSTATEST        : 2;
  __REG32 LOGICSTATEST        : 1;
  __REG32                     : 1;
  __REG32 MPU_RAM_STATEST     : 2;
  __REG32 MPU_L1_STATEST      : 2;
  __REG32 MPU_L2_STATEST      : 2;
  __REG32                     :10;
  __REG32 INTRANSITION        : 1;
  __REG32                     :11;
} __pm_mpu_pwrstst_bits;

/* RM_MPU_RSTST Register */
typedef struct {
  __REG32                     : 5;
  __REG32 EMULATION_MPU_RST   : 1;
  __REG32 ICECRUSHER_MPU_RST  : 1;
  __REG32                     :25;
} __rm_mpu_rstst_bits;

/* PRM_RSTCTRL Register */
typedef struct {
  __REG32 RST_GLOBAL_WARM_SW  : 1;
  __REG32 RST_GLOBAL_COLD_SW  : 1;
  __REG32                     :30;
} __prm_rstctrl_bits;

/* PRM_RSTTIME Register */
typedef struct {
  __REG32 RSTTIME1            : 8;
  __REG32 RSTTIME2            : 5;
  __REG32                     :19;
} __prm_rsttime_bits;

/* PRM_RSTST Register */
typedef struct {
  __REG32 GLOBAL_COLD_RST     : 1;
  __REG32 GLOBAL_WARM_SW_RST  : 1;
  __REG32                     : 2;
  __REG32 WDT1_RST            : 1;
  __REG32 EXTERNAL_WARM_RST   : 1;
  __REG32                     : 3;
  __REG32 ICEPICK_RST         : 1;
  __REG32                     :22;
} __prm_rstst_bits;

/* PRM_SRAM_COUNT Register */
typedef struct {
  __REG32 PCHARGECNT_VALUE    : 6;
  __REG32                     : 2;
  __REG32 VSETUPCNT_VALUE     : 8;
  __REG32 SLPCNT_VALUE        : 8;
  __REG32 STARTUP_COUNT       : 8;
} __prm_sram_count_bits;

/* PRM_LDO_SRAM_CORE_SETUP Register */
/* PRM_LDO_SRAM_MPU_SETUP Register */
typedef struct {
  __REG32 DISABLE_RTA_EXPORT  : 1;
  __REG32 ABBOFF_ACT_EXPORT   : 1;
  __REG32 ABBOFF_SLEEP_EXPORT : 1;
  __REG32 ENFUNC1_EXPORT      : 1;
  __REG32 ENFUNC2_EXPORT      : 1;
  __REG32 ENFUNC3_EXPORT      : 1;
  __REG32 ENFUNC4             : 1;
  __REG32 ENFUNC5             : 1;
  __REG32 AIPOFF              : 1;
  __REG32                     :23;
} __prm_ldo_sram_setup_bits;

/* PRM_LDO_SRAM_CORE_CTRL Register */
/* PRM_LDO_SRAM_MPU_CTRL Register */
typedef struct {
  __REG32 RETMODE_ENABLE      : 1;
  __REG32                     : 7;
  __REG32 SRAMLDO_STATUS      : 1;
  __REG32 SRAM_IN_TRANSITION  : 1;
  __REG32                     :22;
} __prm_ldo_sram_ctrl_bits;

/* PM_RTC_PWRSTCTRL Register */
typedef struct {
  __REG32                     : 2;
  __REG32 LOGICRETSTATE       : 1;
  __REG32                     : 1;
  __REG32 LOWPOWERSTATECHANGE : 1;
  __REG32                     :27;
} __pm_rtc_pwrstctrl_bits;

/* PM_RTC_PWRSTST Register */
typedef struct {
  __REG32                     : 2;
  __REG32 LOGICSTATEST        : 1;
  __REG32                     :17;
  __REG32 INTRANSITION        : 1;
  __REG32                     :11;
} __pm_rtc_pwrstst_bits;

/* PM_GFX_PWRSTCTRL Register */
typedef struct {
  __REG32 POWERSTATE          : 2;
  __REG32 LOGICRETSTATE       : 1;
  __REG32                     : 1;
  __REG32 LOWPOWERSTATECHANGE : 1;
  __REG32                     : 1;
  __REG32 GFX_MEM_RETSTATE    : 1;
  __REG32                     :10;
  __REG32 GFX_MEM_ONSTATE     : 2;
  __REG32                     :13;
} __pm_gfx_pwrstctrl_bits;

/* RM_GFX_RSTCTRL Register */
typedef struct {
  __REG32 GFX_RST             : 1;
  __REG32                     :31;
} __rm_gfx_rstctrl_bits;

/* PM_GFX_PWRSTST Register */
typedef struct {
  __REG32 POWERSTATEST        : 2;
  __REG32 LOGICSTATEST        : 1;
  __REG32                     : 1;
  __REG32 GFX_MEM_STATEST     : 2;
  __REG32                     :14;
  __REG32 INTRANSITION        : 1;
  __REG32                     :11;
} __pm_gfx_pwrstst_bits;

/* RM_GFX_RSTST Register */
typedef struct {
  __REG32 GFX_RST             : 1;
  __REG32                     :31;
} __rm_gfx_rstst_bits;

/* PM_CEFUSE_PWRSTCTRL Register */
typedef struct {
  __REG32 POWERSTATE          : 2;
  __REG32                     : 2;
  __REG32 LOWPOWERSTATECHANGE : 1;
  __REG32                     :27;
} __pm_cefuse_pwrstctrl_bits;

/* PM_CEFUSE_PWRSTST Register */
typedef struct {
  __REG32 POWERSTATEST          : 2;
  __REG32 LOGICSTATEST          : 1;
  __REG32                       :17;
  __REG32 INTRANSITION          : 1;
  __REG32                       : 3;
  __REG32 LASTPOWERSTATEENTERED : 2;
  __REG32                       : 6;
} __pm_cefuse_pwrstst_bits;

/* CONTROL_REVISION Register */
typedef struct {
  __REG32 IP_REV_MINOR          : 6;
  __REG32 IP_REV_CUSTOM         : 2;
  __REG32 IP_REV_MAJOR          : 3;
  __REG32 IP_REV_RTL            : 5;
  __REG32 IP_REV_FUNC           :12;
  __REG32                       : 2;
  __REG32 IP_REV_SCHEME         : 2;
} __control_revision_bits;

/* CONTROL_SYSCONFIG Register */
typedef struct {
  __REG32                       : 1;
  __REG32 FREEEMU               : 1;
  __REG32 IDLEMODE              : 2;
  __REG32 STANDBY               : 2;
  __REG32                       :26;
} __control_sysconfig_bits;

/* CONTROL_STATUS Register */
typedef struct {
  __REG32 SYSBOOT0              : 8;
  __REG32 DEVTYPE               : 3;
  __REG32                       : 5;
  __REG32 BW                    : 1;
  __REG32 WAITEN                : 1;
  __REG32 ADMUX                 : 2;
  __REG32 TESTMD                : 2;
  __REG32 SYSBOOT1              : 2;
  __REG32                       : 8;
} __control_status_bits;

/* CORTEX_VBBLDO_CTRL Register */
typedef struct {
  __REG32 NOVBGBYR              : 1;
  __REG32 NOCAP                 : 1;
  __REG32 BBSEL                 : 1;
  __REG32 HZ                    : 1;
  __REG32 LOWPWR                : 1;
  __REG32 LDOBYPASSZ            : 1;
  __REG32                       :10;
  __REG32 VSETRBB               : 5;
  __REG32 VSETFBB               : 5;
  __REG32                       : 6;
} __cortex_vbbldo_ctrl_bits;

/* CORE_SLDO_CTRL Register */
/* MPU_SLDO_CTRL Register */
typedef struct {
  __REG32                       :16;
  __REG32 vset                  :10;
  __REG32                       : 6;
} __sldo_ctrl_bits;

/* CLK32KDIVRATIO_CTRL Register */
typedef struct {
  __REG32 CLKDIVOPP50_EN        : 1;
  __REG32                       :31;
} __clk32kdivratio_ctrl_bits;

/* BANDGAP_CTRL Register */
typedef struct {
  __REG32 TSHUT                 : 1;
  __REG32 ECOZ                  : 1;
  __REG32 CONTCONV              : 1;
  __REG32 CLRZ                  : 1;
  __REG32 SOC                   : 1;
  __REG32 TMPSOFF               : 1;
  __REG32 BGROFF                : 1;
  __REG32 CBIASSEL              : 1;
  __REG32 DTEMP                 : 8;
  __REG32                       :16;
} __bandgap_ctrl_bits;

/* BANDGAP_TRIM Register */
typedef struct {
  __REG32 DTRTEMPSC             : 8;
  __REG32 DTRTEMPS              : 8;
  __REG32 DTRBGAPV              : 8;
  __REG32 DTRBGAPC              : 8;
} __bandgap_trim_bits;

/* PLL_CLKINPULOW_CTRL Register */
typedef struct {
  __REG32 MPU_DPLL_CLKINPULOW_SEL : 1;
  __REG32 DISP_PLL_CLKINPULOW_SEL : 1;
  __REG32 DDR_PLL_CLKINPULOW_SEL  : 1;
  __REG32                         :29;
} __pll_clkinpulow_ctrl_bits;

/* MOSC_CTRL Register */
typedef struct {
  __REG32 RESSELECT               : 1;
  __REG32                         :31;
} __mosc_ctrl_bits;

/* RCOSC_CTRL Register */
typedef struct {
  __REG32 STOPOSC                 : 1;
  __REG32                         :31;
} __rcosc_ctrl_bits;

/* DEEPSLEEP_CTRL Register */
typedef struct {
  __REG32 DSCOUNT                 :16;
  __REG32                         : 1;
  __REG32 DSENABLE                : 1;
  __REG32                         :14;
} __deepsleep_ctrl_bits;

/* DEVICE_ID Register */
typedef struct {
  __REG32                         : 1;
  __REG32 MFGR                    :11;
  __REG32 PARTNUM                 :16;
  __REG32 DEVREV                  : 4;
} __device_id_bits;

/* DEV_FEATURE Register */
typedef struct {
  __REG32 PRU_ICSS                : 1;
  __REG32 CPSW                    : 1;
  __REG32                         : 5;
  __REG32 DCAN                    : 1;
  __REG32                         : 8;
  __REG32 PRU_ICSS_FEA0           : 1;
  __REG32 PRU_ICSS_FEA1           : 1;
  __REG32                         :11;
  __REG32 SGX                     : 1;
  __REG32                         : 2;
} __dev_feature_bits;

/* INIT_PRIORITY_0 Register */
typedef struct {
  __REG32 HOST_ARM                : 2;
  __REG32                         : 2;
  __REG32 PRU_ICSS                : 2;
  __REG32 MMU                     : 2;
  __REG32                         : 6;
  __REG32 P1500                   : 2;
  __REG32 TCRD0                   : 2;
  __REG32 TCWR0                   : 2;
  __REG32 TCRD1                   : 2;
  __REG32 TCWR1                   : 2;
  __REG32 TCRD2                   : 2;
  __REG32 TCWR2                   : 2;
  __REG32                         : 4;
} __init_priority_0_bits;

/* INIT_PRIORITY_1 Register */
typedef struct {
  __REG32 CPSW                    : 2;
  __REG32                         : 2;
  __REG32 USB_DMA                 : 2;
  __REG32 USB_QMGR                : 2;
  __REG32                         :12;
  __REG32 SGX                     : 2;
  __REG32 LCD                     : 2;
  __REG32 DEBUG                   : 2;
  __REG32                         : 6;
} __init_priority_1_bits;

/* MMU_CFG Register */
typedef struct {
  __REG32                         : 7;
  __REG32 MMU_DISABLE             : 1;
  __REG32                         : 7;
  __REG32 MMU_ABORT               : 1;
  __REG32                         :16;
} __mmu_cfg_bits;

/* tptc_cfg Register */
typedef struct {
  __REG32 TC0DBS                  : 2;
  __REG32 TC1DBS                  : 2;
  __REG32 TC2DBS                  : 2;
  __REG32                         :26;
} __tptc_cfg_bits;

/* USB_CTRL0 Register */
/* USB_CTRL1 Register */
typedef struct {
  __REG32 CM_PWRDN                : 1;
  __REG32 OTG_PWRDN               : 1;
  __REG32 CHGDET_DIS              : 1;
  __REG32 CHGDET_RSTRT            : 1;
  __REG32 SRCONDM                 : 1;
  __REG32 SINKONDP                : 1;
  __REG32 CHGISINK_EN             : 1;
  __REG32 CHGVSRC_EN              : 1;
  __REG32 DMPULLUP                : 1;
  __REG32 DPPULLUP                : 1;
  __REG32 CDET_EXTCTL             : 1;
  __REG32                         : 1;
  __REG32 GPIOMODE                : 1;
  __REG32 GPIO_SIG_INV            : 1;
  __REG32 GPIO_SIG_CROSS          : 1;
  __REG32                         : 2;
  __REG32 DPGPIO_PD               : 1;
  __REG32 DMGPIO_PD               : 1;
  __REG32 OTGVDET_EN              : 1;
  __REG32 OTGSESSENDEN            : 1;
  __REG32                         : 2;
  __REG32 DATAPOLARITY_INV        : 1;
  __REG32                         : 8;
} __usb_ctrl_bits;

/* USB_STS0 Register */
/* USB_STS1 Register */
typedef struct {
  __REG32 CHGDETDONE              : 1;
  __REG32 CHGDETECT               : 1;
  __REG32 CDET_DATADET            : 1;
  __REG32 CDET_DPDET              : 1;
  __REG32 CDET_DMDET              : 1;
  __REG32 CHGDETSTS               : 3;
  __REG32                         :24;
} __usb_sts_bits;

/* MAC_ID0_LO Register */
/* MAC_ID1_LO Register */
typedef struct {
  __REG32 MACADDR_BYTE1           : 8;
  __REG32 MACADDR_BYTE0           : 8;
  __REG32                         :16;
} __mac_id_lo_bits;

/* MAC_ID0_HI Register */
/* MAC_ID1_HI Register */
typedef struct {
  __REG32 MACADDR_BYTE5           : 8;
  __REG32 MACADDR_BYTE4           : 8;
  __REG32 MACADDR_BYTE3           : 8;
  __REG32 MACADDR_BYTE2           : 8;
} __mac_id_hi_bits;

/* DCAN_RAMINIT Register */
typedef struct {
  __REG32 DCAN0_RAMINIT_START     : 1;
  __REG32 DCAN1_RAMINIT_START     : 1;
  __REG32                         : 6;
  __REG32 DCAN0_RAMINIT_DONE      : 1;
  __REG32 DCAN1_RAMINIT_DONE      : 1;
  __REG32                         :22;
} __dcan_raminit_bits;

/* USB_WKUP_CTRL Register */
typedef struct {
  __REG32 PHY0_WUEN               : 1;
  __REG32                         : 7;
  __REG32 PHY1_WUEN               : 1;
  __REG32                         :23;
} __usb_wkup_ctrl_bits;

/* GMII_SEL Register */
typedef struct {
  __REG32 GMII1_SEL               : 2;
  __REG32 GMII2_SEL               : 2;
  __REG32 RGMII1_IDMODE           : 1;
  __REG32 RGMII2_IDMOE            : 1;
  __REG32 RMII1_IO_CLK_EN         : 1;
  __REG32 RMII2_IO_CLK_EN         : 1;
  __REG32                         :24;
} __gmii_sel_bits;

/* PWMSS_CTRL Register */
typedef struct {
  __REG32 PWMSS0_TBCLKEN          : 1;
  __REG32 PWMSS1_TBCLKEN          : 1;
  __REG32 PWMSS2_TBCLKEN          : 1;
  __REG32                         :29;
} __pwmss_ctrl_bits;

/* MREQPRIO_0 Register */
typedef struct {
  __REG32 SAB_INIT0               : 3;
  __REG32                         : 1;
  __REG32 SAB_INIT1               : 3;
  __REG32                         : 1;
  __REG32 PRU_ICSS1_PRU0          : 3;
  __REG32                         : 5;
  __REG32 CPSW                    : 3;
  __REG32                         : 1;
  __REG32 USB0                    : 3;
  __REG32                         : 1;
  __REG32 USB1                    : 3;
  __REG32                         : 1;
  __REG32 SGX                     : 3;
  __REG32                         : 1;
} __mreqprio_0_bits;

/* MREQPRIO_1 Register */
typedef struct {
  __REG32 EXP                     : 3;
  __REG32                         :29;
} __mreqprio_1_bits;

/* HW_EVENT_SEL_GRP1 Register */
typedef struct {
  __REG32 EVENT1                  : 8;
  __REG32 EVENT2                  : 8;
  __REG32 EVENT3                  : 8;
  __REG32 EVENT4                  : 8;
} __hw_event_sel_grp1_bits;

/* HW_EVENT_SEL_GRP2 Register */
typedef struct {
  __REG32 EVENT5                  : 8;
  __REG32 EVENT6                  : 8;
  __REG32 EVENT7                  : 8;
  __REG32 EVENT8                  : 8;
} __hw_event_sel_grp2_bits;

/* HW_EVENT_SEL_GRP2 Register */
typedef struct {
  __REG32 EVENT9                  : 8;
  __REG32 EVENT10                 : 8;
  __REG32 EVENT11                 : 8;
  __REG32 EVENT12                 : 8;
} __hw_event_sel_grp3_bits;

/* HW_EVENT_SEL_GRP2 Register */
typedef struct {
  __REG32 EVENT13                 : 8;
  __REG32 EVENT14                 : 8;
  __REG32 EVENT15                 : 8;
  __REG32 EVENT16                 : 8;
} __hw_event_sel_grp4_bits;

/* SMRT_CTRL Register */
typedef struct {
  __REG32 SR0_SLEEP               : 1;
  __REG32 SR1_SLEEP               : 1;
  __REG32                         :30;
} __smrt_ctrl_bits;

/* MPUSS_HW_DEBUG_SEL Register */
typedef struct {
  __REG32 HW_DBG_SEL              : 4;
  __REG32                         : 5;
  __REG32 HW_DBG_GATE_EN          : 1;
  __REG32                         :22;
} __mpuss_hw_debug_sel_bits;

/* VDD_MPU_OPP_050 Register */
/* VDD_MPU_OPP_100 Register */
/* VDD_MPU_OPP_120 Register */
/* VDD_MPU_OPP_TURBO Register */
/* VDD_CORE_OPP_050 Register */
/* VDD_CORE_OPP_100 Register */
typedef struct {
  __REG32 NTARGET                 :24;
  __REG32                         : 8;
} __vdd_opp_bits;

/* BB_SCALE Register */
typedef struct {
  __REG32 BBIAS                   : 2;
  __REG32                         : 6;
  __REG32 SCALE                   : 4;
  __REG32                         :20;
} __bb_scale_bits;

/* USB_VID_PID Register */
typedef struct {
  __REG32 USB_PID                 :16;
  __REG32 USB_VID                 :16;
} __usb_vid_pid_bits;

/* CONF_MODULE_PIN Registers */
typedef struct {
  __REG32 MMODE                   : 3;
  __REG32 PUDEN                   : 1;
  __REG32 PUTYPESEL               : 1;
  __REG32 RXACTIVE                : 1;
  __REG32 SLEWCTRL                : 1;
  __REG32                         :25;
} __conf_module_pin_bits;

/* CQDETECT_STATUS Registers */
typedef struct {
  __REG32 CQSTAT_GPMC         : 1;
  __REG32 CQSTAT_MMCSD_A      : 1;
  __REG32 CQSTAT_MMCSD_B      : 1;
  __REG32 CQSTAT_GEMAC_A      : 1;
  __REG32 CQSTAT_GEMAC_B      : 1;
  __REG32 CQSTAT_GENERAL      : 1;
  __REG32                     : 2;
  __REG32 CQERR_GPMC          : 1;
  __REG32 CQERR_MMCSD_A       : 1;
  __REG32 CQERR_MMCSD_B       : 1;
  __REG32 CQERR_GEMAC_A       : 1;
  __REG32 CQERR_GEMAC_B       : 1;
  __REG32 CQERR_GENERAL       : 1;
  __REG32                     :18;
} __cqdetect_status_bits;

/* DDR_IO_CTRL Registers */
typedef struct {
  __REG32                     :28;
  __REG32 MDDR_SEL            : 1;
  __REG32                     : 1;
  __REG32 DDR_WUCLK_DISABLE   : 1;
  __REG32 DDR3_RST_DEF_VAL    : 1;
} __ddr_io_ctrl_bits;

/* VTP_CTRL Registers */
typedef struct {
  __REG32 CLRZ                : 1;
  __REG32 FILTER              : 3;
  __REG32 LOCK                : 1;
  __REG32 READY               : 1;
  __REG32 ENABLE              : 1;
  __REG32                     : 1;
  __REG32 NCIN                : 7;
  __REG32                     : 1;
  __REG32 PCIN                : 7;
  __REG32                     : 9;
} __vtp_ctrl_bits;

/* VREF_CTRL Registers */
typedef struct {
  __REG32 DDR_VREF_EN         : 1;
  __REG32 DDR_VREF_TAP        : 2;
  __REG32 DDR_VREF_CCAP       : 2;
  __REG32                     :27;
} __vref_ctrl_bits;

/* TPCC_EVT_MUX_0_3 Registers */
typedef struct {
  __REG32 EVT_MUX_0           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_1           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_2           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_3           : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_0_3_bits;

/* TPCC_EVT_MUX_4_7 Registers */
typedef struct {
  __REG32 EVT_MUX_4           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_5           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_6           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_7           : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_4_7_bits;

/* TPCC_EVT_MUX_8_11 Registers */
typedef struct {
  __REG32 EVT_MUX_8           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_9           : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_10          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_11          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_8_11_bits;

/* TPCC_EVT_MUX_12_15 Registers */
typedef struct {
  __REG32 EVT_MUX_12          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_13          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_14          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_15          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_12_15_bits;

/* TPCC_EVT_MUX_16_19 Registers */
typedef struct {
  __REG32 EVT_MUX_16          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_17          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_18          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_19          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_16_19_bits;

/* TPCC_EVT_MUX_20_23 Registers */
typedef struct {
  __REG32 EVT_MUX_20          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_21          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_22          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_23          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_20_23_bits;

/* TPCC_EVT_MUX_24_27 Registers */
typedef struct {
  __REG32 EVT_MUX_24          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_25          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_26          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_27          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_24_27_bits;

/* TPCC_EVT_MUX_28_31 Registers */
typedef struct {
  __REG32 EVT_MUX_28          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_29          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_30          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_31          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_28_31_bits;

/* TPCC_EVT_MUX_32_35 Registers */
typedef struct {
  __REG32 EVT_MUX_32          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_33          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_34          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_35          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_32_35_bits;

/* TPCC_EVT_MUX_36_39 Registers */
typedef struct {
  __REG32 EVT_MUX_36          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_37          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_38          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_39          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_36_39_bits;

/* TPCC_EVT_MUX_40_43 Registers */
typedef struct {
  __REG32 EVT_MUX_40          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_41          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_42          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_43          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_40_43_bits;

/* TPCC_EVT_MUX_44_47 Registers */
typedef struct {
  __REG32 EVT_MUX_44          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_45          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_46          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_47          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_44_47_bits;

/* TPCC_EVT_MUX_48_51 Registers */
typedef struct {
  __REG32 EVT_MUX_20          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_21          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_22          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_23          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_48_51_bits;

/* TPCC_EVT_MUX_52_55 Registers */
typedef struct {
  __REG32 EVT_MUX_20          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_21          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_22          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_23          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_52_55_bits;

/* TPCC_EVT_MUX_56_59 Registers */
typedef struct {
  __REG32 EVT_MUX_56          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_57          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_58          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_59          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_56_59_bits;

/* TPCC_EVT_MUX_60_63 Registers */
typedef struct {
  __REG32 EVT_MUX_60          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_61          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_62          : 6;
  __REG32                     : 2;
  __REG32 EVT_MUX_63          : 6;
  __REG32                     : 2;
} __tpcc_evt_mux_60_63_bits;

/* TIMER_EVT_CAPT Registers */
typedef struct {
  __REG32 TIMER5_EVTCAPT      : 5;
  __REG32                     : 3;
  __REG32 TIMER6_EVTCAPT      : 5;
  __REG32                     : 3;
  __REG32 TIMER7_EVTCAPT      : 5;
  __REG32                     :11;
} __timer_evt_capt_bits;

/* ECAP_EVT_CAPT Registers */
typedef struct {
  __REG32 ECAP0_EVTCAPT       : 5;
  __REG32                     : 3;
  __REG32 ECAP1_EVTCAPT       : 5;
  __REG32                     : 3;
  __REG32 ECAP2_EVTCAPT       : 5;
  __REG32                     :11;
} __ecap_evt_capt_bits;

/* ADC_EVT_CAPT Registers */
typedef struct {
  __REG32 ADC_EVTCAPT         : 4;
  __REG32                     :28;
} __adc_evt_capt_bits;

/* RESET_ISO Registers */
typedef struct {
  __REG32 ISO_CONTROL         : 1;
  __REG32                     :31;
} __reset_iso_bits;

/* DDR_CKE_CTRL Registers */
typedef struct {
  __REG32 DDR_CKE_CTRL        : 1;
  __REG32                     :31;
} __ddr_cke_ctrl_bits;

/* M3_TXEV_EOI Registers */
typedef struct {
  __REG32 M3_TXEV_EOI         : 1;
  __REG32                     :31;
} __m3_txev_eoi_bits;

/* DDR_CMD0_IOCTRL Registers */
/* DDR_CMD1_IOCTRL Registers */
/* DDR_CMD2_IOCTRL Registers */
typedef struct {
  __REG32 IO_CONFIG_I         : 3;
  __REG32 IO_CONFIG_SR        : 2;
  __REG32 IO_CONFIG_I_CLK     : 3;
  __REG32 IO_CONFIG_SR_CLK    : 2;
  __REG32 IO_CONFIG_GP_WD0    :11;
  __REG32 IO_CONFIG_GP_WD1    :11;
} __ddr_cmd_ioctrl_bits;

/* ddr_data0_ioctrl Registers */
/* ddr_data1_ioctrl Registers */
typedef struct {
  __REG32 IO_CONFIG_I         : 3;
  __REG32 IO_CONFIG_SR        : 2;
  __REG32 IO_CONFIG_I_CLK     : 3;
  __REG32 IO_CONFIG_SR_CLK    : 2;
  __REG32 IO_CONFIG_WD0_DQ    : 8;
  __REG32 IO_CONFIG_WD0_DM    : 1;
  __REG32 IO_CONFIG_WD0_DQS   : 1;
  __REG32 IO_CONFIG_WD1_DQ    : 8;
  __REG32 IO_CONFIG_WD1_DM    : 1;
  __REG32 IO_CONFIG_WD1_DQS   : 1;
  __REG32                     : 2;
} __ddr_data_ioctrl_bits;

/* EDMA3CC Configuration Register (CCCFG) */
typedef struct {
  __REG32 NUM_DMACH               : 3;
  __REG32                         : 1;
  __REG32 NUM_QDMACH              : 3;
  __REG32                         : 1;
  __REG32 NUM_INTCH               : 3;
  __REG32                         : 1;
  __REG32 NUM_PAENTRY             : 3;
  __REG32                         : 1;
  __REG32 NUM_EVQUE               : 3;
  __REG32                         : 1;
  __REG32 NUM_REGN                : 2;
  __REG32                         : 2;
  __REG32 CHMAP_EXIST             : 1;
  __REG32 MP_EXIST                : 1;
  __REG32                         : 6;
} __edmacc_cccfg_bits;

/* DMA Channel Map n Registers (DCHMAPn) */
typedef struct {
  __REG32                         : 5;
  __REG32 PAENTRY                 : 9;
  __REG32                         :18;
} __edmacc_dchmap_bits;

/* QDMA Channel n Mapping Register (QCHMAPn) */
typedef struct {
  __REG32                         : 2;
  __REG32 TRWORD                  : 3;
  __REG32 PAENTRY                 : 9;
  __REG32                         :18;
} __edmacc_qchmap_bits;

/* DMA Channel Queue Number Register 0 (DMAQNUM0) */
typedef struct {
  __REG32 E0                      : 3;
  __REG32                         : 1;
  __REG32 E1                      : 3;
  __REG32                         : 1;
  __REG32 E2                      : 3;
  __REG32                         : 1;
  __REG32 E3                      : 3;
  __REG32                         : 1;
  __REG32 E4                      : 3;
  __REG32                         : 1;
  __REG32 E5                      : 3;
  __REG32                         : 1;
  __REG32 E6                      : 3;
  __REG32                         : 1;
  __REG32 E7                      : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum0_bits;

/* DMA Channel Queue Number Register 1 (DMAQNUM1) */
typedef struct {
  __REG32 E8                      : 3;
  __REG32                         : 1;
  __REG32 E9                      : 3;
  __REG32                         : 1;
  __REG32 E10                     : 3;
  __REG32                         : 1;
  __REG32 E11                     : 3;
  __REG32                         : 1;
  __REG32 E12                     : 3;
  __REG32                         : 1;
  __REG32 E13                     : 3;
  __REG32                         : 1;
  __REG32 E14                     : 3;
  __REG32                         : 1;
  __REG32 E15                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum1_bits;

/* DMA Channel Queue Number Register 2 (DMAQNUM2) */
typedef struct {
  __REG32 E16                     : 3;
  __REG32                         : 1;
  __REG32 E17                     : 3;
  __REG32                         : 1;
  __REG32 E18                     : 3;
  __REG32                         : 1;
  __REG32 E19                     : 3;
  __REG32                         : 1;
  __REG32 E20                     : 3;
  __REG32                         : 1;
  __REG32 E21                     : 3;
  __REG32                         : 1;
  __REG32 E22                     : 3;
  __REG32                         : 1;
  __REG32 E23                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum2_bits;

/* DMA Channel Queue Number Register 3 (DMAQNUM3) */
typedef struct {
  __REG32 E24                     : 3;
  __REG32                         : 1;
  __REG32 E25                     : 3;
  __REG32                         : 1;
  __REG32 E26                     : 3;
  __REG32                         : 1;
  __REG32 E27                     : 3;
  __REG32                         : 1;
  __REG32 E28                     : 3;
  __REG32                         : 1;
  __REG32 E29                     : 3;
  __REG32                         : 1;
  __REG32 E30                     : 3;
  __REG32                         : 1;
  __REG32 E31                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum3_bits;

/* DMA Channel Queue Number Register 4 (DMAQNUM4) */
typedef struct {
  __REG32 E32                     : 3;
  __REG32                         : 1;
  __REG32 E33                     : 3;
  __REG32                         : 1;
  __REG32 E34                     : 3;
  __REG32                         : 1;
  __REG32 E35                     : 3;
  __REG32                         : 1;
  __REG32 E36                     : 3;
  __REG32                         : 1;
  __REG32 E37                     : 3;
  __REG32                         : 1;
  __REG32 E38                     : 3;
  __REG32                         : 1;
  __REG32 E39                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum4_bits;

/* DMA Channel Queue Number Register 5 (DMAQNUM5) */
typedef struct {
  __REG32 E40                     : 3;
  __REG32                         : 1;
  __REG32 E41                     : 3;
  __REG32                         : 1;
  __REG32 E42                     : 3;
  __REG32                         : 1;
  __REG32 E43                     : 3;
  __REG32                         : 1;
  __REG32 E44                     : 3;
  __REG32                         : 1;
  __REG32 E45                     : 3;
  __REG32                         : 1;
  __REG32 E46                     : 3;
  __REG32                         : 1;
  __REG32 E47                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum5_bits;

/* DMA Channel Queue Number Register 6 (DMAQNUM6) */
typedef struct {
  __REG32 E48                     : 3;
  __REG32                         : 1;
  __REG32 E49                     : 3;
  __REG32                         : 1;
  __REG32 E50                     : 3;
  __REG32                         : 1;
  __REG32 E51                     : 3;
  __REG32                         : 1;
  __REG32 E52                     : 3;
  __REG32                         : 1;
  __REG32 E53                     : 3;
  __REG32                         : 1;
  __REG32 E54                     : 3;
  __REG32                         : 1;
  __REG32 E55                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum6_bits;

/* DMA Channel Queue Number Register 7 (DMAQNUM7) */
typedef struct {
  __REG32 E56                     : 3;
  __REG32                         : 1;
  __REG32 E57                     : 3;
  __REG32                         : 1;
  __REG32 E58                     : 3;
  __REG32                         : 1;
  __REG32 E59                     : 3;
  __REG32                         : 1;
  __REG32 E60                     : 3;
  __REG32                         : 1;
  __REG32 E61                     : 3;
  __REG32                         : 1;
  __REG32 E62                     : 3;
  __REG32                         : 1;
  __REG32 E63                     : 3;
  __REG32                         : 1;
} __edmacc_dmaqnum7_bits;

/* QDMA Channel Queue Number Register (QDMAQNUM) */
typedef struct {
  __REG32 E0                      : 3;
  __REG32                         : 1;
  __REG32 E1                      : 3;
  __REG32                         : 1;
  __REG32 E2                      : 3;
  __REG32                         : 1;
  __REG32 E3                      : 3;
  __REG32                         : 1;
  __REG32 E4                      : 3;
  __REG32                         : 1;
  __REG32 E5                      : 3;
  __REG32                         : 1;
  __REG32 E6                      : 3;
  __REG32                         : 1;
  __REG32 E7                      : 3;
  __REG32                         : 1;
} __edmacc_qdmaqnum_bits;

/* Queue Priority Register (QUEPRI) */
typedef struct {
  __REG32 PRIQ0                   : 3;
  __REG32                         : 1;
  __REG32 PRIQ1                   : 3;
  __REG32                         : 1;
  __REG32 PRIQ2                   : 3;
  __REG32                         : 1;
  __REG32 PRIQ3                   : 3;
  __REG32                         :17;
} __edmacc_quepri_bits;

/* Event Missed Registers (EMR) */
/* DMA Region Access Enable for Region m (DRAEm) */
/* Event Register (ER) */
/* Chained Event Register (CER) */
/* Event Enable Register (EER) */
/* Secondary Event Register (SER) */
typedef struct {
  __REG32 E0                      : 1;
  __REG32 E1                      : 1;
  __REG32 E2                      : 1;
  __REG32 E3                      : 1;
  __REG32 E4                      : 1;
  __REG32 E5                      : 1;
  __REG32 E6                      : 1;
  __REG32 E7                      : 1;
  __REG32 E8                      : 1;
  __REG32 E9                      : 1;
  __REG32 E10                     : 1;
  __REG32 E11                     : 1;
  __REG32 E12                     : 1;
  __REG32 E13                     : 1;
  __REG32 E14                     : 1;
  __REG32 E15                     : 1;
  __REG32 E16                     : 1;
  __REG32 E17                     : 1;
  __REG32 E18                     : 1;
  __REG32 E19                     : 1;
  __REG32 E20                     : 1;
  __REG32 E21                     : 1;
  __REG32 E22                     : 1;
  __REG32 E23                     : 1;
  __REG32 E24                     : 1;
  __REG32 E25                     : 1;
  __REG32 E26                     : 1;
  __REG32 E27                     : 1;
  __REG32 E28                     : 1;
  __REG32 E29                     : 1;
  __REG32 E30                     : 1;
  __REG32 E31                     : 1;
} __edmacc_emr_bits;

/* Event Missed Registers (EMRH) */
/* DMA Region Access Enable High Register for Region m (DRAEHm) */
/* Event Register High (ERH) */
/* Chained Event Register (CER) */
typedef struct {
  __REG32 E32                     : 1;
  __REG32 E33                     : 1;
  __REG32 E34                     : 1;
  __REG32 E35                     : 1;
  __REG32 E36                     : 1;
  __REG32 E37                     : 1;
  __REG32 E38                     : 1;
  __REG32 E39                     : 1;
  __REG32 E40                     : 1;
  __REG32 E41                     : 1;
  __REG32 E42                     : 1;
  __REG32 E43                     : 1;
  __REG32 E44                     : 1;
  __REG32 E45                     : 1;
  __REG32 E46                     : 1;
  __REG32 E47                     : 1;
  __REG32 E48                     : 1;
  __REG32 E49                     : 1;
  __REG32 E50                     : 1;
  __REG32 E51                     : 1;
  __REG32 E52                     : 1;
  __REG32 E53                     : 1;
  __REG32 E54                     : 1;
  __REG32 E55                     : 1;
  __REG32 E56                     : 1;
  __REG32 E57                     : 1;
  __REG32 E58                     : 1;
  __REG32 E59                     : 1;
  __REG32 E60                     : 1;
  __REG32 E61                     : 1;
  __REG32 E62                     : 1;
  __REG32 E63                     : 1;
} __edmacc_emrh_bits;

#define EDMACC_EVENT0_BIT      (0x1UL<<0)
#define EDMACC_EVENT1_BIT      (0x1UL<<1)
#define EDMACC_EVENT2_BIT      (0x1UL<<2)
#define EDMACC_EVENT3_BIT      (0x1UL<<3)
#define EDMACC_EVENT4_BIT      (0x1UL<<4)
#define EDMACC_EVENT5_BIT      (0x1UL<<5)
#define EDMACC_EVENT6_BIT      (0x1UL<<6)
#define EDMACC_EVENT7_BIT      (0x1UL<<7)
#define EDMACC_EVENT8_BIT      (0x1UL<<8)
#define EDMACC_EVENT9_BIT      (0x1UL<<9)
#define EDMACC_EVENT10_BIT     (0x1UL<<10)
#define EDMACC_EVENT11_BIT     (0x1UL<<11)
#define EDMACC_EVENT12_BIT     (0x1UL<<12)
#define EDMACC_EVENT13_BIT     (0x1UL<<13)
#define EDMACC_EVENT14_BIT     (0x1UL<<14)
#define EDMACC_EVENT15_BIT     (0x1UL<<15)
#define EDMACC_EVENT16_BIT     (0x1UL<<16)
#define EDMACC_EVENT17_BIT     (0x1UL<<17)
#define EDMACC_EVENT18_BIT     (0x1UL<<18)
#define EDMACC_EVENT19_BIT     (0x1UL<<19)
#define EDMACC_EVENT20_BIT     (0x1UL<<20)
#define EDMACC_EVENT21_BIT     (0x1UL<<21)
#define EDMACC_EVENT22_BIT     (0x1UL<<22)
#define EDMACC_EVENT23_BIT     (0x1UL<<23)
#define EDMACC_EVENT24_BIT     (0x1UL<<24)
#define EDMACC_EVENT25_BIT     (0x1UL<<25)
#define EDMACC_EVENT26_BIT     (0x1UL<<26)
#define EDMACC_EVENT27_BIT     (0x1UL<<27)
#define EDMACC_EVENT28_BIT     (0x1UL<<28)
#define EDMACC_EVENT29_BIT     (0x1UL<<29)
#define EDMACC_EVENT30_BIT     (0x1UL<<30)
#define EDMACC_EVENT31_BIT     (0x1UL<<31)
#define EDMACC_EVENT32_BIT     (0x1UL<<0)
#define EDMACC_EVENT33_BIT     (0x1UL<<1)
#define EDMACC_EVENT34_BIT     (0x1UL<<2)
#define EDMACC_EVENT35_BIT     (0x1UL<<3)
#define EDMACC_EVENT36_BIT     (0x1UL<<4)
#define EDMACC_EVENT37_BIT     (0x1UL<<5)
#define EDMACC_EVENT38_BIT     (0x1UL<<6)
#define EDMACC_EVENT39_BIT     (0x1UL<<7)
#define EDMACC_EVENT40_BIT     (0x1UL<<8)
#define EDMACC_EVENT41_BIT     (0x1UL<<9)
#define EDMACC_EVENT42_BIT     (0x1UL<<10)
#define EDMACC_EVENT43_BIT     (0x1UL<<11)
#define EDMACC_EVENT44_BIT     (0x1UL<<12)
#define EDMACC_EVENT45_BIT     (0x1UL<<13)
#define EDMACC_EVENT46_BIT     (0x1UL<<14)
#define EDMACC_EVENT47_BIT     (0x1UL<<15)
#define EDMACC_EVENT48_BIT     (0x1UL<<16)
#define EDMACC_EVENT49_BIT     (0x1UL<<17)
#define EDMACC_EVENT50_BIT     (0x1UL<<18)
#define EDMACC_EVENT51_BIT     (0x1UL<<19)
#define EDMACC_EVENT52_BIT     (0x1UL<<20)
#define EDMACC_EVENT53_BIT     (0x1UL<<21)
#define EDMACC_EVENT54_BIT     (0x1UL<<22)
#define EDMACC_EVENT55_BIT     (0x1UL<<23)
#define EDMACC_EVENT56_BIT     (0x1UL<<24)
#define EDMACC_EVENT57_BIT     (0x1UL<<25)
#define EDMACC_EVENT58_BIT     (0x1UL<<26)
#define EDMACC_EVENT59_BIT     (0x1UL<<27)
#define EDMACC_EVENT60_BIT     (0x1UL<<28)
#define EDMACC_EVENT61_BIT     (0x1UL<<29)
#define EDMACC_EVENT62_BIT     (0x1UL<<30)
#define EDMACC_EVENT63_BIT     (0x1UL<<31)

/* QDMA Event Missed Register (QEMR) */
/* QDMA Region Access Enable Registers (QRAEm) */
/* QDMA Event Register (QER) */
/* QDMA Event Enable Register (QEER) */
/* QDMA Event Enable Set Register (QEESR) */
/* QDMA Secondary Event Register (QSER) */
typedef struct {
  __REG32 E0                      : 1;
  __REG32 E1                      : 1;
  __REG32 E2                      : 1;
  __REG32 E3                      : 1;
  __REG32 E4                      : 1;
  __REG32 E5                      : 1;
  __REG32 E6                      : 1;
  __REG32 E7                      : 1;
  __REG32                         :24;
} __edmacc_qemr_bits;

#define EDMACC_QEVENT0_BIT     (0x1UL<<0)
#define EDMACC_QEVENT1_BIT     (0x1UL<<1)
#define EDMACC_QEVENT2_BIT     (0x1UL<<2)
#define EDMACC_QEVENT3_BIT     (0x1UL<<3)
#define EDMACC_QEVENT4_BIT     (0x1UL<<4)
#define EDMACC_QEVENT5_BIT     (0x1UL<<5)
#define EDMACC_QEVENT6_BIT     (0x1UL<<6)
#define EDMACC_QEVENT7_BIT     (0x1UL<<7)

/* EDMA3CC Error Register (CCERR) */
typedef struct {
  __REG32 QTHRXCD0                : 1;
  __REG32 QTHRXCD1                : 1;
  __REG32 QTHRXCD2                : 1;
  __REG32 QTHRXCD3                : 1;
  __REG32                         :12;
  __REG32 TCCERR                  : 1;
  __REG32                         :15;
} __edmacc_ccerr_bits;

#define EDMACC_CCERR_QTHRXCD0     (0x1UL<<0)
#define EDMACC_CCERR_QTHRXCD1     (0x1UL<<1)
#define EDMACC_CCERR_QTHRXCD2     (0x1UL<<2)
#define EDMACC_CCERR_QTHRXCD3     (0x1UL<<3)
#define EDMACC_CCERR_TCCERR       (0x1UL<<16)

#define EDMACC_EEVAL_EVAL         (0x1UL<<0)

/* Event Queue Entry Registers (QxEy) */
typedef struct {
  __REG32 ENUM                    : 6;
  __REG32 ETYPE                   : 2;
  __REG32                         :24;
} __edmacc_qe_bits;

/* Queue n Status Registers (QSTATn) */
typedef struct {
  __REG32 STRTPTR                 : 4;
  __REG32                         : 4;
  __REG32 NUMVAL                  : 5;
  __REG32                         : 3;
  __REG32 WM                      : 5;
  __REG32                         : 3;
  __REG32 THRXCD                  : 1;
  __REG32                         : 7;
} __edmacc_qstat_bits;

/* Queue Watermark Threshold A Register (QWMTHRA) */
typedef struct {
  __REG32 Q0                      : 5;
  __REG32                         : 3;
  __REG32 Q1                      : 5;
  __REG32                         : 3;
  __REG32 Q2                      : 5;
  __REG32                         : 3;
  __REG32 Q3                      : 5;
  __REG32                         : 3;
} __edmacc_qwmthra_bits;

/* EDMA3CC Status Register (CCSTAT) */
typedef struct {
  __REG32 EVTACTV                 : 1;
  __REG32 QEVTACTV                : 1;
  __REG32 TRACTV                  : 1;
  __REG32                         : 1;
  __REG32 ACTV                    : 1;
  __REG32                         : 3;
  __REG32 COMPACTV                : 6;
  __REG32                         : 2;
  __REG32 QUEACTV0                : 1;
  __REG32 QUEACTV1                : 1;
  __REG32 QUEACTV2                : 1;
  __REG32 QUEACTV3                : 1;
  __REG32                         :12;
} __edmacc_ccstat_bits;

/* EDMA3CC Memory Protection Fault Status Register (MPFSR) */
typedef struct {
  __REG32 UXE                     : 1;
  __REG32 UWE                     : 1;
  __REG32 URE                     : 1;
  __REG32 SXE                     : 1;
  __REG32 SWE                     : 1;
  __REG32 SRE                     : 1;
  __REG32                         : 3;
  __REG32 FID                     : 4;
  __REG32                         :19;
} __edmacc_mpfsr_bits;

#define EDMACC_MPFCR_MPFCLR       (0x1UL<<0)

/* EDMA3CC Memory Protection Page Attribute Register (MPPAn) */
typedef struct {
  __REG32 UX                      : 1;
  __REG32 UW                      : 1;
  __REG32 UR                      : 1;
  __REG32 SX                      : 1;
  __REG32 SW                      : 1;
  __REG32 SR                      : 1;
  __REG32                         : 3;
  __REG32 EXT                     : 1;
  __REG32 AID0                    : 1;
  __REG32 AID1                    : 1;
  __REG32 AID2                    : 1;
  __REG32 AID3                    : 1;
  __REG32 AID4                    : 1;
  __REG32 AID5                    : 1;
  __REG32                         :16;
} __edmacc_mppa_bits;

/* Interrupt Enable Registers (IER) */
/* Interrupt Enable Clear Register (IECR) */
/* Interrupt Enable Set Register (IESR) */
/* Interrupt Pending Register (IPR) */
/* Interrupt Clear Register (ICR) */
typedef struct {
  __REG32 I0                      : 1;
  __REG32 I1                      : 1;
  __REG32 I2                      : 1;
  __REG32 I3                      : 1;
  __REG32 I4                      : 1;
  __REG32 I5                      : 1;
  __REG32 I6                      : 1;
  __REG32 I7                      : 1;
  __REG32 I8                      : 1;
  __REG32 I9                      : 1;
  __REG32 I10                     : 1;
  __REG32 I11                     : 1;
  __REG32 I12                     : 1;
  __REG32 I13                     : 1;
  __REG32 I14                     : 1;
  __REG32 I15                     : 1;
  __REG32 I16                     : 1;
  __REG32 I17                     : 1;
  __REG32 I18                     : 1;
  __REG32 I19                     : 1;
  __REG32 I20                     : 1;
  __REG32 I21                     : 1;
  __REG32 I22                     : 1;
  __REG32 I23                     : 1;
  __REG32 I24                     : 1;
  __REG32 I25                     : 1;
  __REG32 I26                     : 1;
  __REG32 I27                     : 1;
  __REG32 I28                     : 1;
  __REG32 I29                     : 1;
  __REG32 I30                     : 1;
  __REG32 I31                     : 1;
} __edmacc_ier_bits;

/* Interrupt Enable Registers (IERH) */
/* Interrupt Enable Clear Register (IECHR) */
/* Interrupt Enable Set Register (IESHR) */
/* Interrupt Pending Register (IPHR) */
/* Interrupt Clear Register (ICHR) */
typedef struct {
  __REG32 I32                     : 1;
  __REG32 I33                     : 1;
  __REG32 I34                     : 1;
  __REG32 I35                     : 1;
  __REG32 I36                     : 1;
  __REG32 I37                     : 1;
  __REG32 I38                     : 1;
  __REG32 I39                     : 1;
  __REG32 I40                     : 1;
  __REG32 I41                     : 1;
  __REG32 I42                     : 1;
  __REG32 I43                     : 1;
  __REG32 I44                     : 1;
  __REG32 I45                     : 1;
  __REG32 I46                     : 1;
  __REG32 I47                     : 1;
  __REG32 I48                     : 1;
  __REG32 I49                     : 1;
  __REG32 I50                     : 1;
  __REG32 I51                     : 1;
  __REG32 I52                     : 1;
  __REG32 I53                     : 1;
  __REG32 I54                     : 1;
  __REG32 I55                     : 1;
  __REG32 I56                     : 1;
  __REG32 I57                     : 1;
  __REG32 I58                     : 1;
  __REG32 I59                     : 1;
  __REG32 I60                     : 1;
  __REG32 I61                     : 1;
  __REG32 I62                     : 1;
  __REG32 I63                     : 1;
} __edmacc_ierh_bits;

/* Interrupt Evaluate Register (IEVAL) */
typedef struct {
  __REG32 EVAL                    : 1;
  __REG32                         :31;
} __edmacc_ieval_bits;

/* EDMA3TC Configuration Register (TCCFG) */
typedef struct {
  __REG32 FIFOSIZE                : 3;
  __REG32                         : 1;
  __REG32 BUSWIDTH                : 2;
  __REG32                         : 2;
  __REG32 DREGDEPTH               : 2;
  __REG32                         :22;
} __edmatc_tccfg_bits;

/* EDMA3TC Channel Status Register (TCSTAT) */
typedef struct {
  __REG32 PROGBUSY                : 1;
  __REG32 SRCACTV                 : 1;
  __REG32 WSACTV                  : 1;
  __REG32                         : 1;
  __REG32 DSTACTV                 : 3;
  __REG32                         : 5;
  __REG32 DFSTRTPTR               : 2;
  __REG32                         :18;
} __edmatc_tcstat_bits;

/* Error Status Register (ERRSTAT) */
/* Error Enable Register (ERREN) */
/* Error Clear Register (ERRCLR) */
typedef struct {
  __REG32 BUSERR                  : 1;
  __REG32                         : 1;
  __REG32 TRERR                   : 1;
  __REG32 MMRAERR                 : 1;
  __REG32                         :28;
} __edmatc_errstat_bits;

#define EDMATC_ERRCLR_BUSERR      (0x1UL<<0)
#define EDMATC_ERRCLR_TRERR       (0x1UL<<2)
#define EDMATC_ERRCLR_MMRAERR     (0x1UL<<3)

/* Error Details Register (ERRDET) */
typedef struct {
  __REG32 STAT                    : 4;
  __REG32                         : 4;
  __REG32 TCC                     : 6;
  __REG32                         : 2;
  __REG32 TCINTEN                 : 1;
  __REG32 TCCHEN                  : 1;
  __REG32                         :14;
} __edmatc_errdet_bits;

#define EDMATC_ERRCMD_EVAL        (0x1UL<<0)

/* Read Command Rate Register (RDRATE) */
typedef struct {
  __REG32 RDRATE                  : 3;
  __REG32                         :29;
} __edmatc_rdrate_bits;

/* Source Active Options Register (SAOPT) */
/* Destination FIFO Options Register n (DFOPTn) */
typedef struct {
  __REG32 SAM                     : 1;
  __REG32 DAM                     : 1;
  __REG32                         : 2;
  __REG32 PRI                     : 3;
  __REG32                         : 1;
  __REG32 FWID                    : 3;
  __REG32                         : 1;
  __REG32 TCC                     : 6;
  __REG32                         : 2;
  __REG32 TCINTEN                 : 1;
  __REG32                         : 1;
  __REG32 TCCHEN                  : 1;
  __REG32                         : 9;
} __edmatc_saopt_bits;

/* Source Active Count Register (SACNT) */
typedef struct {
  __REG32 ACNT                    :16;
  __REG32 BCNT                    :16;
} __edmatc_sacnt_bits;

/* Source Active B-Index Register (SABIDX) */
typedef struct {
  __REG32 SBIDX                   :16;
  __REG32 DBIDX                   :16;
} __edmatc_sabidx_bits;

/* Source Active Memory Protection Proxy Register (SAMPPRXY) */
typedef struct {
  __REG32 PRIVID                  : 4;
  __REG32                         : 4;
  __REG32 PRIV                    : 1;
  __REG32                         :23;
} __edmatc_sampprxy_bits;

/* Source Active Count Reload Register (SACNTRLD) */
/* Destination FIFO Set Count Reload Register (DFCNTRLD) */
typedef struct {
  __REG32 ACNTRLD                 :16;
  __REG32                         :16;
} __edmatc_sacntrld_bits;

/* Destination FIFO Options Register n (DFOPTn) */
typedef struct {
  __REG32 SAM                     : 1;
  __REG32 DAM                     : 1;
  __REG32                         : 2;
  __REG32 PRI                     : 3;
  __REG32                         : 1;
  __REG32 FWID                    : 3;
  __REG32                         : 1;
  __REG32 TCC                     : 6;
  __REG32                         : 2;
  __REG32 TCINTEN                 : 1;
  __REG32                         : 1;
  __REG32 TCCHEN                  : 1;
  __REG32                         : 9;
} __edmatc_dfopt_bits;

/* TSC_ADC_SS REVISION Register */
typedef struct {
  __REG32 Y_MINOR                 : 6;
  __REG32 CUSTOM                  : 2;
  __REG32 X_MAJOR                 : 3;
  __REG32 R_RTL                   : 5;
  __REG32 FUNC                    :12;
  __REG32                         : 2;
  __REG32 SCHEME                  : 2;
} __tsc_adc_ss_revision_bits;

/* TSC_ADC_SS SYSCONFIG Register */
typedef struct {
  __REG32                         : 2;
  __REG32 IDLEMODE                : 2;
  __REG32                         :28;
} __tsc_adc_ss_sysconfig_bits;

/* TSC_ADC_SS IRQ_EOI Register */
typedef struct {
  __REG32 LINE_NUMBER             : 1;
  __REG32                         :31;
} __tsc_adc_ss_irq_eoi_bits;

/* TSC_ADC_SS IRQSTATUS_RAW Register */
/* TSC_ADC_SS IRQSTATUS Register */
typedef struct {
  __REG32 HW_PEN_EVENT_ASYNCHRONOUS   : 1;
  __REG32 END_OF_SEQUENCE             : 1;
  __REG32 FIFO0_THRESHOLD             : 1;
  __REG32 FIFO0_OVERRUN               : 1;
  __REG32 FIFO0_UNDERFLOW             : 1;
  __REG32 FIFO1_THRESHOLD             : 1;
  __REG32 FIFO1_OVERRUN               : 1;
  __REG32 FIFO1_UNDERFLOW             : 1;
  __REG32 OUT_OF_RANGE                : 1;
  __REG32 PEN_UP_EVENT                : 1;
  __REG32 HW_PEN_EVENT_SYNCHRONOUS    : 1;
  __REG32                             :21;
} __tsc_adc_ss_irq_bits;

#define   TSC_ADC_SS_IRQ_HW_PEN_EVENT_ASYNCHRONOUS  (0x1UL<<0)
#define   TSC_ADC_SS_IRQ_END_OF_SEQUENCE            (0x1UL<<1)
#define   TSC_ADC_SS_IRQ_FIFO0_THRESHOLD            (0x1UL<<2)
#define   TSC_ADC_SS_IRQ_FIFO0_OVERRUN              (0x1UL<<3)
#define   TSC_ADC_SS_IRQ_FIFO0_UNDERFLOW            (0x1UL<<4)
#define   TSC_ADC_SS_IRQ_FIFO1_THRESHOLD            (0x1UL<<5)
#define   TSC_ADC_SS_IRQ_FIFO1_OVERRUN              (0x1UL<<6)
#define   TSC_ADC_SS_IRQ_FIFO1_UNDERFLOW            (0x1UL<<7)
#define   TSC_ADC_SS_IRQ_OUT_OF_RANGE               (0x1UL<<8)
#define   TSC_ADC_SS_IRQ_PEN_UP_EVENT               (0x1UL<<9)
#define   TSC_ADC_SS_IRQ_HW_PEN_EVENT_SYNCHRONOUS   (0x1UL<<10)

/* TSC_ADC_SS IRQWAKEUP Register */
typedef struct {
  __REG32 WAKEEN0                     : 1;
  __REG32                             :31;
} __tsc_adc_ss_irqwakeup_bits;

/* TSC_ADC_SS DMAENABLE_SET Register */
/* TSC_ADC_SS DMAENABLE_CLR Register */
typedef struct {
  __REG32 ENABLE_0                    : 1;
  __REG32 ENABLE_1                    : 1;
  __REG32                             :30;
} __tsc_adc_ss_dmaenable_bits;

#define   TSC_ADC_SS_DMA_ENABLE0      (0x1UL<<0)
#define   TSC_ADC_SS_DMA_ENABLE1      (0x1UL<<1)

/* TSC_ADC_SS CTRL Register */
typedef struct {
  __REG32 ENABLE                      : 1;
  __REG32 STEP_ID_TAG                 : 1;
  __REG32 STEPCONFIG_WRITEPROTECT     : 1;
  __REG32 ADC_BIAS_SELECT             : 1;
  __REG32 POWER_DOWN                  : 1;
  __REG32 AFE_PEN_CTRL                : 2;
  __REG32 TOUCH_SCREEN_ENABLE         : 1;
  __REG32 HW_EVENT_MAPPING            : 1;
  __REG32 HW_PREEMPT                  : 1;
  __REG32                             :22;
} __tsc_adc_ss_ctrl_bits;

/* TSC_ADC_SS ADCSTAT Register */
typedef struct {
  __REG32 STEP_ID                     : 5;
  __REG32 FSM_BUSY                    : 1;
  __REG32 PEN_IRQ0                    : 1;
  __REG32 PEN_IRQ1                    : 1;
  __REG32                             :24;
} __tsc_adc_ss_adcstat_bits;

/* TSC_ADC_SS ADCRANGE Register */
typedef struct {
  __REG32 LOW_RANGE_DATA              :12;
  __REG32 HIGH_RANGE_DATA             :16;
  __REG32                             : 4;
} __tsc_adc_ss_adcrange_bits;

/* TSC_ADC_SS ADC_CLKDIV Register */
typedef struct {
  __REG32 ADC_CLKDIV                  :16;
  __REG32                             :16;
} __tsc_adc_ss_adc_clkdiv_bits;

/* TSC_ADC_SS ADC_MISC Register */
typedef struct {
  __REG32 AFE_SPARE_INPUT             : 4;
  __REG32 AFE_SPARE_OUTPUT            : 4;
  __REG32                             :24;
} __tsc_adc_ss_adc_misc_bits;

/* TSC_ADC_SS STEPENABLE Register */
typedef struct {
  __REG32 TS_CHARGE                   : 1;
  __REG32 STEP1                       : 1;
  __REG32 STEP2                       : 1;
  __REG32 STEP3                       : 1;
  __REG32 STEP4                       : 1;
  __REG32 STEP5                       : 1;
  __REG32 STEP6                       : 1;
  __REG32 STEP7                       : 1;
  __REG32 STEP8                       : 1;
  __REG32 STEP9                       : 1;
  __REG32 STEP10                      : 1;
  __REG32 STEP11                      : 1;
  __REG32 STEP12                      : 1;
  __REG32 STEP13                      : 1;
  __REG32 STEP14                      : 1;
  __REG32 STEP15                      : 1;
  __REG32 STEP16                      : 1;
  __REG32                             :15;
} __tsc_adc_ss_stepenable_bits;

/* TSC_ADC_SS IDLECONFIG Register */
/* TSC_ADC_SS TS_CHARGE_STEPCONFIG Register */
typedef struct {
  __REG32                             : 5;
  __REG32 XPPSW_SWC                   : 1;
  __REG32 XNNSW_SWC                   : 1;
  __REG32 YPPSW_SWC                   : 1;
  __REG32 YNNSW_SWC                   : 1;
  __REG32 XNPSW_SWC                   : 1;
  __REG32 YPNSW_SWC                   : 1;
  __REG32 WPNSW_SWC                   : 1;
  __REG32 SEL_RFP_SWC_2_0             : 3;
  __REG32 SEL_INM_SWC_3_0             : 4;
  __REG32 SEL_INP_SWC_3_0             : 4;
  __REG32 SEL_RFM_SWC_1_0             : 2;
  __REG32 DIFF_CNTRL                  : 1;
  __REG32                             : 6;
} __tsc_adc_ss_idleconfig_bits;

/* TSC_ADC_SS TS_CHARGE_DELAY Register */
typedef struct {
  __REG32 OPENDELAY                   :18;
  __REG32                             :14;
} __tsc_adc_ss_ts_charge_delay_bits;

/* TSC_ADC_SS STEPCONFIGx Register */
typedef struct {
  __REG32 MODE                        : 2;
  __REG32 AVERAGING                   : 3;
  __REG32 XPPSW_SWC                   : 1;
  __REG32 XNNSW_SWC                   : 1;
  __REG32 YPPSW_SWC                   : 1;
  __REG32 YNNSW_SWC                   : 1;
  __REG32 XNPSW_SWC                   : 1;
  __REG32 YPNSW_SWC                   : 1;
  __REG32 WPNSW_SWC                   : 1;
  __REG32 SEL_RFP_SWC_2_0             : 3;
  __REG32 SEL_INM_SWC_3_0             : 4;
  __REG32 SEL_INP_SWC_3_0             : 4;
  __REG32 SEL_RFM_SWC_1_0             : 2;
  __REG32 DIFF_CNTRL                  : 1;
  __REG32 FIFO_SELECT                 : 1;
  __REG32 RANGE_CHECK                 : 1;
  __REG32                             : 4;
} __tsc_adc_ss_stepconfig_bits;

/* TSC_ADC_SS STEPDELAYx Register */
typedef struct {
  __REG32 OPENDELAY                   :18;
  __REG32                             : 6;
  __REG32 SAMPLEDELAY                 : 8;
} __tsc_adc_ss_stepdelay_bits;

/* TSC_ADC_SS FIFOxCOUNT Register */
typedef struct {
  __REG32 WORDS_IN_FIFO               : 7;
  __REG32                             :25;
} __tsc_adc_ss_fifocount_bits;

/* TSC_ADC_SS FIFOx THRESHOLD Register */
typedef struct {
  __REG32 FIFO_THRESHOLD_LEVEL        : 6;
  __REG32                             :26;
} __tsc_adc_ss_fifothreshold_bits;

/* TSC_ADC_SS DMAxREQ Register */
typedef struct {
  __REG32 DMA_REQUEST_LEVEL           : 6;
  __REG32                             :26;
} __tsc_adc_ss_dmareq_bits;

/* TSC_ADC_SS FIFOxDATA Register */
typedef struct {
  __REG32 ADCDATA                     :12;
  __REG32                             : 4;
  __REG32 ADCCHNLID                   : 4;
  __REG32                             :12;
} __tsc_adc_ss_fifodata_bits;

/* LCD PID Register */
typedef struct {
  __REG32 MINOR                   : 6;
  __REG32 CUSTOM                  : 2;
  __REG32 MAJOR                   : 3;
  __REG32 RTL                     : 5;
  __REG32 FUNC                    :12;
  __REG32                         : 2;
  __REG32 SCHEME                  : 2;
} __lcdc_pid_bits;

/* LCD Control Register (LCD_CTRL) */
typedef struct {
  __REG32 MODESEL                 : 1;
  __REG32 AUTO_UFLOW_RESTART      : 1;
  __REG32                         : 6;
  __REG32 CLKDIV                  : 8;
  __REG32                         :16;
} __lcdc_ctrl_bits;

/*  LCD LIDD Control Register (LIDD_CTRL) */
typedef struct {
  __REG32 LIDD_MODE_SEL           : 3;
  __REG32 ALEPOL                  : 1;
  __REG32 RS_EN_POL               : 1;
  __REG32 WS_DIR_POL              : 1;
  __REG32 CS0_E0_POL              : 1;
  __REG32 CS1_E1_POL              : 1;
  __REG32 LIDD_DMA_EN             : 1;
  __REG32 DMA_CS0_CS1             : 1;
  __REG32                         :22;
} __lidd_ctrl_bits;

/* LCD LIDD CSn Configuration Registers (LIDD_CS0_CONF and LIDD_CS1_CONF) */
typedef struct {
  __REG32 TA                      : 2;
  __REG32 R_HOLD                  : 4;
  __REG32 R_STROBE                : 6;
  __REG32 R_SU                    : 5;
  __REG32 W_HOLD                  : 4;
  __REG32 W_STROBE                : 6;
  __REG32 W_SU                    : 5;
} __lcdc_cs_conf_bits;

/* LCD LIDD CSn Address Read/Write Registers (LIDD_CS0_ADDR and LIDD_CS1_ADDR) */
typedef struct {
  __REG32 ADR_INDX                :16;
  __REG32                         :16;
} __lcdc_cs_addr_bits;

/* LCD LIDD CSn Data Read/Write Register (LIDD_CSn_DATA) */
typedef struct {
  __REG32 DATA                    :16;
  __REG32                         :16;
} __lcdc_cs_data_bits;

/* LCD Raster Control Register (RASTER_CTRL) */
typedef struct {
  __REG32 LCDEN                   : 1;
  __REG32 LCDBW                   : 1;
  __REG32                         : 5;
  __REG32 LCDTFT                  : 1;
  __REG32 RDORDE                  : 1;
  __REG32 NONO8B                  : 1;
  __REG32                         : 2;
  __REG32 REQDLY                  : 8;
  __REG32 PALMODE                 : 2;
  __REG32 NIBMODE                 : 1;
  __REG32 TFTMAP                  : 1;
  __REG32 STN565                  : 1;
  __REG32 TFT24                   : 1;
  __REG32 TFT24UNPACKED           : 1;
  __REG32                         : 5;
} __lcdc_raster_ctrl_bits;

/* LCD Raster Timing Register 0 (RASTER_TIMING_0) */
typedef struct {
  __REG32                         : 3;
  __REG32 PPLMSB                  : 1;
  __REG32 PPLLSB                  : 6;
  __REG32 HSW                     : 6;
  __REG32 HFP                     : 8;
  __REG32 HBP                     : 8;
} __lcdc_raster_timing_0_bits;

/* LCD Raster Timing Register 1 (RASTER_TIMING_1) */
typedef struct {
  __REG32 LPP                     :10;
  __REG32 VSW                     : 6;
  __REG32 VFP                     : 8;
  __REG32 VBP                     : 8;
} __lcdc_raster_timing_1_bits;

/* LCD Raster Timing Register 2 (RASTER_TIMING_2) */
typedef struct {
  __REG32 HFP_HIGHBITS            : 2;
  __REG32                         : 2;
  __REG32 HBP_HIGHBITS            : 2;
  __REG32                         : 2;
  __REG32 ACB                     : 8;
  __REG32 ACBI                    : 4;
  __REG32 IVS                     : 1;
  __REG32 IHS                     : 1;
  __REG32 IPC                     : 1;
  __REG32 IEO                     : 1;
  __REG32 PHSVS_RF                : 1;
  __REG32 PHSVS_ON_OFF            : 1;
  __REG32 LPP_B10                 : 1;
  __REG32 HSW_HIGHBITS            : 4;
  __REG32                         : 1;
} __lcdc_raster_timing_2_bits;

/* LCD Raster Subpanel Display Register (RASTER_SUBPANEL) */
typedef struct {
  __REG32 DPDLSB                  :16;
  __REG32 LPPT                    :10;
  __REG32                         : 3;
  __REG32 HOLS                    : 1;
  __REG32                         : 1;
  __REG32 SPEN                    : 1;
} __lcdc_raster_subpanel_bits;

/* LCD Raster Subpanel Display Register 2 (RASTER_SUBPANEL2) */
typedef struct {
  __REG32 DPDMSB                  : 8;
  __REG32 LPPT_B10                : 1;
  __REG32                         :23;
} __lcdc_raster_subpanel2_bits;

/* LCD DMA Control Register (LCDDMA_CTRL) */
typedef struct {
  __REG32 FRAME_MODE              : 1;
  __REG32 BIGENDIAN               : 1;
  __REG32                         : 1;
  __REG32 BYTE_SWAP               : 1;
  __REG32 BURST_SIZE              : 3;
  __REG32                         : 1;
  __REG32 TH_FIFO_READY           : 3;
  __REG32                         : 5;
  __REG32 DMA_MASTER_PRIO         : 3;
  __REG32                         :13;
} __lcdc_dma_ctrl_bits;

/* LCD SYSCONFIG Register (LCDC_SYSCONFIG) */
typedef struct {
  __REG32                         : 2;
  __REG32 IDLEMODE                : 2;
  __REG32 STANDBYMODE             : 2;
  __REG32                         :26;
} __lcdc_sysconfig_bits;

/* LCD IRQSTATUS_RAW Register (LCDC_IRQSTATUS_RAW) */
typedef struct {
  __REG32 DONE_RAW_SET            : 1;
  __REG32                         : 1;
  __REG32 SYNC_RAW_SET            : 1;
  __REG32 ACB_RAW_SET             : 1;
  __REG32                         : 1;
  __REG32 FUF_RAW_SET             : 1;
  __REG32 PL_RAW_SET              : 1;
  __REG32                         : 1;
  __REG32 EOF0_RAW_SET            : 1;
  __REG32 EOF1_RAW_SET            : 1;
  __REG32                         :22;
} __lcdc_irqstatus_raw_bits;

/* LCD IRQSTATUS Register (LCDC_IRQSTATUS) */
typedef struct {
  __REG32 DONE_EN_CLR             : 1;
  __REG32                         : 1;
  __REG32 SYNC_EN_CLR             : 1;
  __REG32 ACB_EN_CLR              : 1;
  __REG32                         : 1;
  __REG32 FUF_EN_CLR              : 1;
  __REG32 PL_EN_CLR               : 1;
  __REG32                         : 1;
  __REG32 EOF0_EN_CLR             : 1;
  __REG32 EOF1_EN_CLR             : 1;
  __REG32                         :22;
} __lcdc_irqstatus_bits;

/* LCD IRQENABLE_SET Register (LCDC_IRQENABLE_SET) */
typedef struct {
  __REG32 DONE_EN_SET             : 1;
  __REG32                         : 1;
  __REG32 SYNC_EN_SET             : 1;
  __REG32 ACB_EN_SET              : 1;
  __REG32                         : 1;
  __REG32 FUF_EN_SET              : 1;
  __REG32 PL_EN_SET               : 1;
  __REG32                         : 1;
  __REG32 EOF0_EN_SET             : 1;
  __REG32 EOF1_EN_SET             : 1;
  __REG32                         :22;
} __lcdc_irqenable_set_bits;

#define LCDC_IRQ_DONE             (0x1UL<<0)
#define LCDC_IRQ_SYNC             (0x1UL<<2)
#define LCDC_IRQ_ACB              (0x1UL<<3)
#define LCDC_IRQ_FUF              (0x1UL<<5)
#define LCDC_IRQ_PL               (0x1UL<<6)
#define LCDC_IRQ_EOF0             (0x1UL<<8)
#define LCDC_IRQ_EOF1             (0x1UL<<9)

/* LCD CLKC_ENABLE Register (LCDC_CLKC_ENABLE) */
typedef struct {
  __REG32 CORE_CLK_EN             : 1;
  __REG32 LIDD_CLK_EN             : 1;
  __REG32 DMA_CLK_EN              : 1;
  __REG32                         :29;
} __lcdc_clkc_enable_bits;

/* LCD CLKC_RESET Register (LCDC_CLKC_RESET) */
typedef struct {
  __REG32 CORE_RST                : 1;
  __REG32 LIDD_RST                : 1;
  __REG32 DMA_RST                 : 1;
  __REG32 MAIN_RST                : 1;
  __REG32                         :28;
} __lcdc_clkc_reset_bits;

/* Ethernet CPSW_ALE_IDVER Register */
typedef struct {
  __REG32 MINOR_VER               : 8;
  __REG32 MAJ_VER                 : 8;
  __REG32 IDENT                   :16;
} __cpsw_ale_idver_bits;

/* Ethernet CPSW_ALE_CONTROL Register */
typedef struct {
  __REG32 ENABLE_RATE_LIMIT       : 1;
  __REG32 ENABLE_AUTH_MODE        : 1;
  __REG32 VLAN_AWARE              : 1;
  __REG32 RATE_LIMIT_TX           : 1;
  __REG32 BYPASS                  : 1;
  __REG32 ENABLE_OUI_DENY         : 1;
  __REG32 EN_VID0_MODE            : 1;
  __REG32 LEARN_NO_VID            : 1;
  __REG32 EN_P0_UNI_FLOOD         : 1;
  __REG32                         :20;
  __REG32 AGE_OUT_NOW             : 1;
  __REG32 CLEAR_TABLE             : 1;
  __REG32 ENABLE_ALE              : 1;
} __cpsw_ale_control_bits;

/* Ethernet CPSW_ALE_PRESCALE Register */
typedef struct {
  __REG32 PRESCALE                :20;
  __REG32                         :12;
} __cpsw_ale_prescale_bits;

/* Ethernet CPSW_ALE_UNKNOWN_VLAN Register */
typedef struct {
  __REG32 UNKNOWN_VLAN_MEMBER_LIST      : 6;
  __REG32                               : 2;
  __REG32 UNKNOWN_MCAST_FLOOD_MASK      : 6;
  __REG32                               : 2;
  __REG32 UNKNOWN_REG_MCAST_FLOOD_MASK  : 6;
  __REG32                               : 2;
  __REG32 UNKNOWN_FORCE_UNTAGGED_EGRESS : 6;
  __REG32                               : 2;
} __cpsw_ale_unknown_vlan_bits;

/* Ethernet CPSW_ALE_TBLCTL Register */
typedef struct {
  __REG32 ENTRY_POINTER                 :10;
  __REG32                               :21;
  __REG32 WRITE_RDZ                     : 1;
} __cpsw_ale_tblctl_bits;

/* Ethernet CPSW_ALE_PORTCTLx Register */
typedef struct {
  __REG32 PORT_STATE                    : 2;
  __REG32 DROP_UNTAGGED                 : 1;
  __REG32 VID_INGRESS_CHECK             : 1;
  __REG32 NO_LEARN                      : 1;
  __REG32 NO_SA_UPDATE                  : 1;
  __REG32                               :10;
  __REG32 MCAST_LIMIT                   : 8;
  __REG32 BCAST_LIMIT                   : 8;
} __cpsw_ale_portctl_bits;

/* Ethernet CPSW_CPDMA_TX_IDVER Register */
typedef struct {
  __REG32 TX_MINOR_VER                  : 8;
  __REG32 TX_MAJOR_VER                  : 8;
  __REG32 TX_IDENT                      :16;
} __cpsw_cpdma_tx_idver_bits;

/* Ethernet CPSW_CPDMA_TX_CONTROL Register */
typedef struct {
  __REG32 TX_EN                         : 1;
  __REG32                               :31;
} __cpsw_cpdam_tx_control_bits;

/* Ethernet CPSW_CPDMA_TX_TEARDOWN Register */
typedef struct {
  __REG32 TX_TDN_CH                     : 3;
  __REG32                               :28;
  __REG32 TX_TDN_RDY                    : 1;
} __cpsw_cpdam_tx_teardown_bits;

/* Ethernet CPSW_CPDMA_RX_IDVER Register */
typedef struct {
  __REG32 RX_MINOR_VER                  : 8;
  __REG32 RX_MAJOR_VER                  : 8;
  __REG32 RX_IDENT                      :16;
} __cpsw_cpdam_rx_idver_bits;

/* Ethernet CPSW_CPDMA_RX_CONTROL Register */
typedef struct {
  __REG32 RX_EN                         : 1;
  __REG32                               :31;
} __cpsw_cpdam_rx_control_bits;

/* Ethernet CPSW_CPDMA_RX_TEARDOWN Register */
typedef struct {
  __REG32 RX_TDN_CH                     : 3;
  __REG32                               :28;
  __REG32 RX_TDN_RDY                    : 1;
} __cpsw_cpdam_rx_teardown_bits;

/* Ethernet CPSW_CPDMA_SOFT_RESET Register */
typedef struct {
  __REG32 SOFT_RESET                    : 1;
  __REG32                               :31;
} __cpsw_cpdma_soft_reset_bits;

/* Ethernet CPSW_DMACONTROL Register */
typedef struct {
  __REG32 TX_PTYPE                      : 1;
  __REG32 RX_OWNERSHIP                  : 1;
  __REG32 RX_OFFLEN_BLOCK               : 1;
  __REG32 CMD_IDLE                      : 1;
  __REG32 RX_CEF                        : 1;
  __REG32                               : 3;
  __REG32 TX_RLIM                       : 8;
  __REG32                               :16;
} __cpsw_cpdma_dmacontrol_bits;

/* Ethernet CPSW_DMASTATUS Register */
typedef struct {
  __REG32                               : 8;
  __REG32 RX_ERR_CH                     : 3;
  __REG32                               : 1;
  __REG32 RX_HOST_ERR_CODE              : 4;
  __REG32 TX_ERR_CH                     : 3;
  __REG32                               : 1;
  __REG32 TX_HOST_ERR_CODE              : 4;
  __REG32                               : 7;
  __REG32 IDLE                          : 1;
} __cpsw_cpdam_dmastatus_bits;

/* Ethernet CPSW_RX_BUFFER_OFFSET Register */
typedef struct {
  __REG32 RX_BUFFER_OFFSET              :16;
  __REG32                               :16;
} __cpsw_cpdam_rx_buffer_offset_bits;

/* Ethernet CPSW_EMCONTROL Register */
typedef struct {
  __REG32 FREE                          : 1;
  __REG32 SOFT                          : 1;
  __REG32                               :30;
} __cpsw_cpdam_emcontrol_bits;

/* Ethernet CPSW_TX_PRIx_RATE Register */
typedef struct {
  __REG32 PRIN_SEND_CNT                 :14;
  __REG32                               : 2;
  __REG32 PRIN_IDLE_CNT                 :14;
  __REG32                               : 2;
} __cpsw_cpdam_tx_pri_rate_bits;

/* Ethernet CPSW_TX_INTSTAT_RAW Register */
/* Ethernet CPSW_TX_INTSTAT_MASKED Register */
typedef struct {
  __REG32 TX0_PEND                      : 1;
  __REG32 TX1_PEND                      : 1;
  __REG32 TX2_PEND                      : 1;
  __REG32 TX3_PEND                      : 1;
  __REG32 TX4_PEND                      : 1;
  __REG32 TX5_PEND                      : 1;
  __REG32 TX6_PEND                      : 1;
  __REG32 TX7_PEND                      : 1;
  __REG32                               :24;
} __cpsw_cpdam_tx_intstat_bits;

/* Ethernet CPSW_TX_INTMASK_SET Register */
/* Ethernet CPSW_TX_INTMASK_CLEAR Register */
typedef struct {
  __REG32 TX0_MASK                      : 1;
  __REG32 TX1_MASK                      : 1;
  __REG32 TX2_MASK                      : 1;
  __REG32 TX3_MASK                      : 1;
  __REG32 TX4_MASK                      : 1;
  __REG32 TX5_MASK                      : 1;
  __REG32 TX6_MASK                      : 1;
  __REG32 TX7_MASK                      : 1;
  __REG32                               :24;
} __cpsw_cpdam_tx_intmask_bits;

#define   CPSW_TX_INT_TX0               (0x1UL<<0)
#define   CPSW_TX_INT_TX1               (0x1UL<<1)
#define   CPSW_TX_INT_TX2               (0x1UL<<2)
#define   CPSW_TX_INT_TX3               (0x1UL<<3)
#define   CPSW_TX_INT_TX4               (0x1UL<<4)
#define   CPSW_TX_INT_TX5               (0x1UL<<5)
#define   CPSW_TX_INT_TX6               (0x1UL<<6)
#define   CPSW_TX_INT_TX7               (0x1UL<<7)

/* Ethernet CPSW_CPDMA_EOI_VECTOR Register */
typedef struct {
  __REG32 DMA_EOI_VECTOR                : 5;
  __REG32                               :27;
} __cpsw_cpdma_eoi_vector_bits;

/* Ethernet CPSW_RX_INTSTAT_RAW Register */
/* Ethernet CPSW_RX_INTSTAT_MASKED Register */
typedef struct {
  __REG32 RX0_PEND                      : 1;
  __REG32 RX1_PEND                      : 1;
  __REG32 RX2_PEND                      : 1;
  __REG32 RX3_PEND                      : 1;
  __REG32 RX4_PEND                      : 1;
  __REG32 RX5_PEND                      : 1;
  __REG32 RX6_PEND                      : 1;
  __REG32 RX7_PEND                      : 1;
  __REG32 RX0_THRESH_PEND               : 1;
  __REG32 RX1_THRESH_PEND               : 1;
  __REG32 RX2_THRESH_PEND               : 1;
  __REG32 RX3_THRESH_PEND               : 1;
  __REG32 RX4_THRESH_PEND               : 1;
  __REG32 RX5_THRESH_PEND               : 1;
  __REG32 RX6_THRESH_PEND               : 1;
  __REG32 RX7_THRESH_PEND               : 1;
  __REG32                               :16;
} __cpsw_cpdam_rx_intstat_bits;

#define   CPSW_RX_INT_RX0               (0x1UL<<0)
#define   CPSW_RX_INT_RX1               (0x1UL<<1)
#define   CPSW_RX_INT_RX2               (0x1UL<<2)
#define   CPSW_RX_INT_RX3               (0x1UL<<3)
#define   CPSW_RX_INT_RX4               (0x1UL<<4)
#define   CPSW_RX_INT_RX5               (0x1UL<<5)
#define   CPSW_RX_INT_RX6               (0x1UL<<6)
#define   CPSW_RX_INT_RX7               (0x1UL<<7)
#define   CPSW_RX_INT_RX0_THRESH        (0x1UL<<8)
#define   CPSW_RX_INT_RX1_THRESH        (0x1UL<<9)
#define   CPSW_RX_INT_RX2_THRESH        (0x1UL<<10)
#define   CPSW_RX_INT_RX3_THRESH        (0x1UL<<11)
#define   CPSW_RX_INT_RX4_THRESH        (0x1UL<<12)
#define   CPSW_RX_INT_RX5_THRESH        (0x1UL<<13)
#define   CPSW_RX_INT_RX6_THRESH        (0x1UL<<14)
#define   CPSW_RX_INT_RX7_THRESH        (0x1UL<<15)

/* Ethernet CPSW_DMA_INTSTAT_RAW Register */
/* Ethernet CPSW_DMA_INTSTAT_MASKED Register */
typedef struct {
  __REG32 STAT_PEND                     : 1;
  __REG32 HOST_PEND                     : 1;
  __REG32                               :30;
} __cpsw_cpdam_dma_intstat_bits;

#define   CPSW_DMA_INT_STAT             (0x1UL<<0)
#define   CPSW_DMA_INT_HOST             (0x1UL<<1)

/* Ethernet CPSW_RXn_PENDTHRESH Register */
typedef struct {
  __REG32 RX_PENDTHRESH                 : 8;
  __REG32                               :24;
} __cpsw_cpdam_rx_pendthresh_bits;

/* Ethernet CPSW_RXn_FREEBUFFER Register */
typedef struct {
  __REG32 RX_FREEBUFFER                 :16;
  __REG32                               :16;
} __cpsw_cpdam_rx_freebuffer_bits;

/* Ethernet CPSW_CPTS_IDVER Register */
typedef struct {
  __REG32 MINOR_VER                     : 8;
  __REG32 MAJOR_VER                     : 3;
  __REG32 RTL_VER                       : 5;
  __REG32 TX_IDENT                      :16;
} __cpsw_cpts_idver_bits;

/* Ethernet CPSW_CPTS_CONTROL Register */
typedef struct {
  __REG32 CPTS_EN                       : 1;
  __REG32 INT_TEST                      : 1;
  __REG32                               : 6;
  __REG32 HW1_TS_PUSH_EN                : 1;
  __REG32 HW2_TS_PUSH_EN                : 1;
  __REG32 HW3_TS_PUSH_EN                : 1;
  __REG32 HW4_TS_PUSH_EN                : 1;
  __REG32                               :20;
} __cpsw_cpts_control_bits;

#define CPSW_CPTS_TS_PUSH_TS_PUSH       (0x1UL<<0)

#define CPSW_CPTS_TS_LOAD_EN_TS_LOAD_EN (0x1UL<<0)

/* Ethernet CPSW_CPTS_INTSTAT_RAW Register */
typedef struct {
  __REG32 TS_PEND_RAW                   : 1;
  __REG32                               :31;
} __cpsw_cpts_intstat_raw_bits;

/* Ethernet CPSW_CPTS_INTSTAT_MASKED Register */
typedef struct {
  __REG32 TS_PEND                       : 1;
  __REG32                               :31;
} __cpsw_cpts_intstat_masked_bits;

/* Ethernet CPSW_CPTS_INT_ENABLE Register */
typedef struct {
  __REG32 TS_PEND_EN                    : 1;
  __REG32                               :31;
} __cpsw_cpts_int_enable_bits;

#define CPSW_CPTS_EVENT_POP_EVENT_POP   (0x1UL<<0)

/* Ethernet CPSW_CPTS_EVENT_LOW Register */
typedef struct {
  __REG32 TIME_STAMP                    :32;
} __cpsw_cpts_event_low_bits;

/* Ethernet CPSW_CPTS_EVENT_HIGH Register */
typedef struct {
  __REG32 SEQUENCE_ID                   :16;
  __REG32 MESSAGE_TYPE                  : 4;
  __REG32 EVENT_TYPE                    : 4;
  __REG32 PORT_NUMBER                   : 5;
  __REG32                               : 3;
} __cpsw_cpts_event_high_bits;

/* Ethernet CPSW_P0_CONTROL Register */
typedef struct {
  __REG32                               :16;
  __REG32 P0_DSCP_PRI_EN                : 1;
  __REG32                               : 3;
  __REG32 P0_VLAN_LTYPE1_EN             : 1;
  __REG32 P0_VLAN_LTYPE2_EN             : 1;
  __REG32                               : 2;
  __REG32 P0_PASS_PRI_TAGGED            : 1;
  __REG32                               : 3;
  __REG32 P0_DLR_CPDMA_CH               : 3;
  __REG32                               : 1;
} __cpsw_p0_control_bits;

/* Ethernet CPSW_P0_MAX_BLKS Register */
typedef struct {
  __REG32 P0_RX_MAX_BLKS                : 4;
  __REG32 P0_TX_MAX_BLKS                : 5;
  __REG32                               :23;
} __cpsw_p0_max_blks_bits;

/* Ethernet CPSW_P0_BLK_CNT Register */
typedef struct {
  __REG32 P0_RX_BLK_CNT                 : 4;
  __REG32 P0_TX_BLK_CNT                 : 5;
  __REG32                               :23;
} __cpsw_p0_blk_cnt_bits;

/* Ethernet CPSW_P0_TX_IN_CTL Register */
typedef struct {
  __REG32 TX_PRI_WDS                    :10;
  __REG32                               : 2;
  __REG32 TX_BLKS_REM                   : 4;
  __REG32 TX_IN_SEL                     : 2;
  __REG32                               : 2;
  __REG32 TX_RATE_EN                    : 4;
  __REG32                               : 8;
} __cpsw_p0_tx_in_ctl_bits;

/* Ethernet CPSW_P0_PORT_VLAN Register */
/* Ethernet CPSW_P1_PORT_VLAN Register */
typedef struct {
  __REG32 PORT_VID                      :12;
  __REG32 PORT_CFI                      : 1;
  __REG32 PORT_PRI                      : 3;
  __REG32                               :16;
} __cpsw_p_port_vlan_bits;

/* Ethernet CPSW_P0_TX_PRI_MAP Register */
/* Ethernet CPSW_P1_TX_PRI_MAP Register */
typedef struct {
  __REG32 PRI0                          : 2;
  __REG32                               : 2;
  __REG32 PRI1                          : 2;
  __REG32                               : 2;
  __REG32 PRI2                          : 2;
  __REG32                               : 2;
  __REG32 PRI3                          : 2;
  __REG32                               : 2;
  __REG32 PRI4                          : 2;
  __REG32                               : 2;
  __REG32 PRI5                          : 2;
  __REG32                               : 2;
  __REG32 PRI6                          : 2;
  __REG32                               : 2;
  __REG32 PRI7                          : 2;
  __REG32                               : 2;
} __cpsw_p_tx_pri_map_bits;

/* Ethernet CPSW_P0_CPDMA_TX_PRI_MAP Register */
typedef struct {
  __REG32 PRI0                          : 3;
  __REG32                               : 1;
  __REG32 PRI1                          : 3;
  __REG32                               : 1;
  __REG32 PRI2                          : 3;
  __REG32                               : 1;
  __REG32 PRI3                          : 3;
  __REG32                               : 1;
  __REG32 PRI4                          : 3;
  __REG32                               : 1;
  __REG32 PRI5                          : 3;
  __REG32                               : 1;
  __REG32 PRI6                          : 3;
  __REG32                               : 1;
  __REG32 PRI7                          : 3;
  __REG32                               : 1;
} __cpsw_p0_cpdma_tx_pri_map_bits;

/* Ethernet CPSW_P0_CPDMA_RX_CH_MAP Register */
typedef struct {
  __REG32 P1_PRI0                       : 3;
  __REG32                               : 1;
  __REG32 P1_PRI1                       : 3;
  __REG32                               : 1;
  __REG32 P1_PRI2                       : 3;
  __REG32                               : 1;
  __REG32 P1_PRI3                       : 3;
  __REG32                               : 1;
  __REG32 P2_PRI0                       : 3;
  __REG32                               : 1;
  __REG32 P2_PRI1                       : 3;
  __REG32                               : 1;
  __REG32 P2_PRI2                       : 3;
  __REG32                               : 1;
  __REG32 P2_PRI3                       : 3;
  __REG32                               : 1;
} __cpsw_p0_cpdma_rx_ch_map_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP0 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP0 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP0 Register */
typedef struct {
  __REG32 PRI0                          : 3;
  __REG32                               : 1;
  __REG32 PRI1                          : 3;
  __REG32                               : 1;
  __REG32 PRI2                          : 3;
  __REG32                               : 1;
  __REG32 PRI3                          : 3;
  __REG32                               : 1;
  __REG32 PRI4                          : 3;
  __REG32                               : 1;
  __REG32 PRI5                          : 3;
  __REG32                               : 1;
  __REG32 PRI6                          : 3;
  __REG32                               : 1;
  __REG32 PRI7                          : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map0_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP1 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP1 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP1 Register */
typedef struct {
  __REG32 PRI8                          : 3;
  __REG32                               : 1;
  __REG32 PRI9                          : 3;
  __REG32                               : 1;
  __REG32 PRI10                         : 3;
  __REG32                               : 1;
  __REG32 PRI11                         : 3;
  __REG32                               : 1;
  __REG32 PRI12                         : 3;
  __REG32                               : 1;
  __REG32 PRI13                         : 3;
  __REG32                               : 1;
  __REG32 PRI14                         : 3;
  __REG32                               : 1;
  __REG32 PRI15                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map1_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP2 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP2 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP2 Register */
typedef struct {
  __REG32 PRI16                         : 3;
  __REG32                               : 1;
  __REG32 PRI17                         : 3;
  __REG32                               : 1;
  __REG32 PRI18                         : 3;
  __REG32                               : 1;
  __REG32 PRI19                         : 3;
  __REG32                               : 1;
  __REG32 PRI20                         : 3;
  __REG32                               : 1;
  __REG32 PRI21                         : 3;
  __REG32                               : 1;
  __REG32 PRI22                         : 3;
  __REG32                               : 1;
  __REG32 PRI23                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map2_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP3 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP3 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP3 Register */
typedef struct {
  __REG32 PRI24                         : 3;
  __REG32                               : 1;
  __REG32 PRI25                         : 3;
  __REG32                               : 1;
  __REG32 PRI26                         : 3;
  __REG32                               : 1;
  __REG32 PRI27                         : 3;
  __REG32                               : 1;
  __REG32 PRI28                         : 3;
  __REG32                               : 1;
  __REG32 PRI29                         : 3;
  __REG32                               : 1;
  __REG32 PRI30                         : 3;
  __REG32                               : 1;
  __REG32 PRI31                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map3_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP4 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP4 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP4 Register */
typedef struct {
  __REG32 PRI32                         : 3;
  __REG32                               : 1;
  __REG32 PRI33                         : 3;
  __REG32                               : 1;
  __REG32 PRI34                         : 3;
  __REG32                               : 1;
  __REG32 PRI35                         : 3;
  __REG32                               : 1;
  __REG32 PRI36                         : 3;
  __REG32                               : 1;
  __REG32 PRI37                         : 3;
  __REG32                               : 1;
  __REG32 PRI38                         : 3;
  __REG32                               : 1;
  __REG32 PRI39                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map4_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP5 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP5 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP5 Register */
typedef struct {
  __REG32 PRI40                         : 3;
  __REG32                               : 1;
  __REG32 PRI41                         : 3;
  __REG32                               : 1;
  __REG32 PRI42                         : 3;
  __REG32                               : 1;
  __REG32 PRI43                         : 3;
  __REG32                               : 1;
  __REG32 PRI44                         : 3;
  __REG32                               : 1;
  __REG32 PRI45                         : 3;
  __REG32                               : 1;
  __REG32 PRI46                         : 3;
  __REG32                               : 1;
  __REG32 PRI47                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map5_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP6 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP6 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP6 Register */
typedef struct {
  __REG32 PRI48                         : 3;
  __REG32                               : 1;
  __REG32 PRI49                         : 3;
  __REG32                               : 1;
  __REG32 PRI50                         : 3;
  __REG32                               : 1;
  __REG32 PRI51                         : 3;
  __REG32                               : 1;
  __REG32 PRI52                         : 3;
  __REG32                               : 1;
  __REG32 PRI53                         : 3;
  __REG32                               : 1;
  __REG32 PRI54                         : 3;
  __REG32                               : 1;
  __REG32 PRI55                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map6_bits;

/* Ethernet CPSW_P0_RX_DSCP_PRI_MAP7 Register */
/* Ethernet CPSW_P1_RX_DSCP_PRI_MAP7 Register */
/* Ethernet CPSW_P2_RX_DSCP_PRI_MAP7 Register */
typedef struct {
  __REG32 PRI56                         : 3;
  __REG32                               : 1;
  __REG32 PRI57                         : 3;
  __REG32                               : 1;
  __REG32 PRI58                         : 3;
  __REG32                               : 1;
  __REG32 PRI59                         : 3;
  __REG32                               : 1;
  __REG32 PRI60                         : 3;
  __REG32                               : 1;
  __REG32 PRI61                         : 3;
  __REG32                               : 1;
  __REG32 PRI62                         : 3;
  __REG32                               : 1;
  __REG32 PRI63                         : 3;
  __REG32                               : 1;
} __cpsw_p_rx_dscp_pri_map7_bits;

/* Ethernet CPSW_P1_CONTROL Register */
typedef struct {
  __REG32 P1_TS_RX_EN                   : 1;
  __REG32 P1_TS_TX_EN                   : 1;
  __REG32 P1_TS_LTYPE1_EN               : 1;
  __REG32 P1_TS_LTYPE2_EN               : 1;
  __REG32 P1_TS_ANNEX_D_EN              : 1;
  __REG32                               : 4;
  __REG32 P1_TS_129                     : 1;
  __REG32 P1_TS_130                     : 1;
  __REG32 P1_TS_131                     : 1;
  __REG32 P1_TS_132                     : 1;
  __REG32 P1_TS_319                     : 1;
  __REG32 P1_TS_320                     : 1;
  __REG32                               : 1;
  __REG32 P1_DSCP_PRI_EN                : 1;
  __REG32                               : 3;
  __REG32 P1_VLAN_LTYPE1_EN             : 1;
  __REG32 P1_VLAN_LTYPE2_EN             : 1;
  __REG32                               : 2;
  __REG32 P1_PASS_PRI_TAGGED            : 1;
  __REG32                               : 7;
} __cpsw_p1_control_bits;

/* Ethernet CPSW_P1_MAX_BLKS Register */
typedef struct {
  __REG32 P1_RX_MAX_BLKS                : 4;
  __REG32 P1_TX_MAX_BLKS                : 5;
  __REG32                               :23;
} __cpsw_p1_max_blks_bits;

/* Ethernet CPSW_P1_BLK_CNT Register */
typedef struct {
  __REG32 P1_RX_BLK_CNT                 : 4;
  __REG32 P1_TX_BLK_CNT                 : 5;
  __REG32                               :23;
} __cpsw_p1_blk_cnt_bits;

/* Ethernet CPSW_P1_TX_IN_CTL Register */
typedef struct {
  __REG32 TX_PRI_WDS                    :10;
  __REG32                               : 2;
  __REG32 TX_BLKS_REM                   : 4;
  __REG32 TX_IN_SEL                     : 2;
  __REG32                               : 2;
  __REG32 TX_RATE_EN                    : 4;
  __REG32 HOST_BLKS_REM                 : 4;
  __REG32                               : 4;
} __cpsw_p1_tx_in_ctl_bits;

/* Ethernet CPSW_P1_TS_SEQ_MTYPE Register */
typedef struct {
  __REG32 P1_TS_MSG_TYPE_EN             :16;
  __REG32 P1_TS_SEQ_ID_OFFSET           : 6;
  __REG32                               :10;
} __cpsw_p1_ts_seq_mtype_bits;

/* Ethernet CPSW_P1_SA_LO Register */
/* Ethernet CPSW_P2_SA_LO Register */
typedef struct {
  __REG32 MACSRCADDR_BYTE1              : 8;
  __REG32 MACSRCADDR_BYTE0              : 8;
  __REG32                               :16;
} __cpsw_p_sa_lo_bits;

/* Ethernet CPSW_P1_SA_HI Register */
/* Ethernet CPSW_P2_SA_HI Register */
typedef struct {
  __REG32 MACSRCADDR_BYTE2              : 8;
  __REG32 MACSRCADDR_BYTE3              : 8;
  __REG32 MACSRCADDR_BYTE4              : 8;
  __REG32 MACSRCADDR_BYTE5              : 8;
} __cpsw_p_sa_hi_bits;

/* Ethernet CPSW_P1_SEND_PERCENT Register */
typedef struct {
  __REG32 PRI1_SEND_PERCENT             : 7;
  __REG32                               : 1;
  __REG32 PRI2_SEND_PERCENT             : 7;
  __REG32                               : 1;
  __REG32 PRI3_SEND_PERCENT             : 7;
  __REG32                               : 9;
} __cpsw_p_send_percent_bits;

/* Ethernet CPSW_P2_CONTROL Register */
typedef struct {
  __REG32 P2_TS_RX_EN                   : 1;
  __REG32 P2_TS_TX_EN                   : 1;
  __REG32 P2_TS_LTYPE1_EN               : 1;
  __REG32 P2_TS_LTYPE2_EN               : 1;
  __REG32 P2_TS_ANNEX_D_EN              : 1;
  __REG32                               : 5;
  __REG32 P2_TS_130                     : 1;
  __REG32 P2_TS_131                     : 1;
  __REG32 P2_TS_132                     : 1;
  __REG32 P2_TS_319                     : 1;
  __REG32 P2_TS_320                     : 1;
  __REG32                               : 1;
  __REG32 P2_DSCP_PRI_EN                : 1;
  __REG32                               : 2;
  __REG32 P2_TS_129                     : 1;
  __REG32 P2_VLAN_LTYPE1_EN             : 1;
  __REG32 P2_VLAN_LTYPE2_EN             : 1;
  __REG32                               : 2;
  __REG32 P2_PASS_PRI_TAGGED            : 1;
  __REG32                               : 7;
} __cpsw_p2_control_bits;

/* Ethernet CPSW_P2_MAX_BLKS Register */
typedef struct {
  __REG32 P2_RX_MAX_BLKS                : 4;
  __REG32 P2_TX_MAX_BLKS                : 5;
  __REG32                               :23;
} __cpsw_p2_max_blks_bits;

/* Ethernet CPSW_P2_BLK_CNT Register */
typedef struct {
  __REG32 P2_RX_BLK_CNT                 : 4;
  __REG32 P2_TX_BLK_CNT                 : 5;
  __REG32                               :23;
} __cpsw_p2_blk_cnt_bits;

/* Ethernet CPSW_P2_TX_IN_CTL Register */
typedef struct {
  __REG32 TX_PRI_WDS                    :10;
  __REG32                               : 2;
  __REG32 TX_BLKS_REM                   : 4;
  __REG32 TX_IN_SEL                     : 2;
  __REG32                               : 2;
  __REG32 TX_RATE_EN                    : 4;
  __REG32 HOST_BLKS_REM                 : 4;
  __REG32                               : 4;
} __cpsw_p2_tx_in_ctl_bits;

/* Ethernet CPSW_P2_TS_SEQ_MTYPE Register */
typedef struct {
  __REG32 P2_TS_MSG_TYPE_EN             :16;
  __REG32 P2_TS_SEQ_ID_OFFSET           : 6;
  __REG32                               :10;
} __cpsw_p2_ts_seq_mtype_bits;

/* Ethernet CPSW_SL_IDVER Register */
typedef struct {
  __REG32 Y                             : 8;
  __REG32 X                             : 3;
  __REG32 Z                             : 5;
  __REG32 IDENT                         :16;
} __cpsw_sl_idver_bits;

/* Ethernet CPSW_MACCONTROL Register */
typedef struct {
  __REG32 FULLDUPLEX                    : 1;
  __REG32 LOOPBACK                      : 1;
  __REG32 MTEST                         : 1;
  __REG32 RX_FLOW_EN                    : 1;
  __REG32 TX_FLOW_EN                    : 1;
  __REG32 GMII_EN                       : 1;
  __REG32 TX_PACE                       : 1;
  __REG32 GIG                           : 1;
  __REG32                               : 2;
  __REG32 TX_SHORT_GAP_EN               : 1;
  __REG32 CMD_IDLE                      : 1;
  __REG32                               : 3;
  __REG32 IFCTL_A                       : 1;
  __REG32 IFCTL_B                       : 1;
  __REG32 GIG_FORCE                     : 1;
  __REG32 EXT_EN                        : 1;
  __REG32                               : 2;
  __REG32 TX_SHORT_GAP_LIM_EN           : 1;
  __REG32 RX_CEF_EN                     : 1;
  __REG32 RX_CSF_EN                     : 1;
  __REG32 RX_CMF_EN                     : 1;
  __REG32                               : 7;
} __cpsw_sl_maccontrol_bits;

/* Ethernet CPSW_MACSTATUS Register */
typedef struct {
  __REG32 TX_FLOW_ACT                   : 1;
  __REG32 RX_FLOW_ACT                   : 1;
  __REG32                               : 1;
  __REG32 EXT_FULLDUPLEX                : 1;
  __REG32 EXT_GIG                       : 1;
  __REG32                               :26;
  __REG32 IDLE                          : 1;
} __cpsw_sl_macstatus_bits;

/* Ethernet CPSW_SOFT_RESET Register */
typedef struct {
  __REG32 SOFT_RESET                    : 1;
  __REG32                               :31;
} __cpsw_sl_soft_reset_bits;

/* Ethernet CPSW_RX_MAXLEN Register */
typedef struct {
  __REG32 RX_MAXLEN                     :14;
  __REG32                               :18;
} __cpsw_sl_rx_maxlen_bits;

/* Ethernet CPSW_BOFFTEST Register */
typedef struct {
  __REG32 TX_BACKOFF                    :10;
  __REG32                               : 2;
  __REG32 COLL_COUNT                    : 4;
  __REG32 RNDNUM                        :10;
  __REG32 PACEVAL                       : 5;
  __REG32                               : 1;
} __cpsw_sl_bofftest_bits;

/* Ethernet CPSW_RX_PAUSE Register */
typedef struct {
  __REG32                               :16;
  __REG32 RX_PAUSETIMER                 :16;
} __cpsw_sl_rx_pause_bits;

/* Ethernet CPSW_TX_PAUSE Register */
typedef struct {
  __REG32                               :16;
  __REG32 TX_PAUSETIMER                 :16;
} __cpsw_sl_tx_pause_bits;

/* Ethernet CPSW_EMCONTROL Register */
typedef struct {
  __REG32 FREE                          : 1;
  __REG32 SOF                           : 1;
  __REG32                               :30;
} __cpsw_sl_emcontrol_bits;

/* Ethernet CPSW_RX_PRI_MAP Register */
typedef struct {
  __REG32 PRI0                          : 3;
  __REG32                               : 1;
  __REG32 PRI1                          : 3;
  __REG32                               : 1;
  __REG32 PRI2                          : 3;
  __REG32                               : 1;
  __REG32 PRI3                          : 3;
  __REG32                               : 1;
  __REG32 PRI4                          : 3;
  __REG32                               : 1;
  __REG32 PRI5                          : 3;
  __REG32                               : 1;
  __REG32 PRI6                          : 3;
  __REG32                               : 1;
  __REG32 PRI7                          : 3;
  __REG32                               : 1;
} __cpsw_sl_rx_pri_map_bits;

/* Ethernet CPSW_TX_GAP Register */
typedef struct {
  __REG32 TX_GAP                        : 9;
  __REG32                               :23;
} __cpsw_sl_tx_gap_bits;

/* Ethernet CPSW_SS_ID_VER Register */
typedef struct {
  __REG32 CPSW_3G_MINOR_VER             : 8;
  __REG32 CPSW_3G_MAJ_VER               : 3;
  __REG32 CPSW_3G_RTL_VER               : 5;
  __REG32 CPSW_3G_IDENT                 :16;
} __cpsw_ss_idver_bits;

/* Ethernet CPSW_SS_CONTROL Register */
typedef struct {
  __REG32 FIFO_LOOPBACK                 : 1;
  __REG32 VLAN_AWARE                    : 1;
  __REG32 RX_VLAN_ENCAP                 : 1;
  __REG32 DLR_EN                        : 1;
  __REG32                               :28;
} __cpsw_ss_control_bits;

/* Ethernet CPSW_SS_SOFT_RESET Register */
typedef struct {
  __REG32 SOFT_RESET                    : 1;
  __REG32                               :31;
} __cpsw_ss_soft_reset_bits;

/* Ethernet CPSW_SS_STAT_PORT_EN Register */
typedef struct {
  __REG32 P0_STAT_EN                    : 1;
  __REG32 P1_STAT_EN                    : 1;
  __REG32 P2_STAT_EN                    : 1;
  __REG32                               :29;
} __cpsw_ss_stat_port_en_bits;

/* Ethernet CPSW_SS_PTYPE Register */
typedef struct {
  __REG32 ESC_PRI_LD_VAL                : 5;
  __REG32                               : 3;
  __REG32 P0_PTYPE_ESC                  : 1;
  __REG32 P1_PTYPE_ESC                  : 1;
  __REG32 P2_PTYPE_ESC                  : 1;
  __REG32                               : 5;
  __REG32 P1_PRI1_SHAPE_EN              : 1;
  __REG32 P1_PRI2_SHAPE_EN              : 1;
  __REG32 P1_PRI3_SHAPE_EN              : 1;
  __REG32 P2_PRI1_SHAPE_EN              : 1;
  __REG32 P2_PRI2_SHAPE_EN              : 1;
  __REG32 P2_PRI3_SHAPE_EN              : 1;
  __REG32                               :10;
} __cpsw_ss_ptype_bits;

/* Ethernet CPSW_SS_SOFT_IDLE Register */
typedef struct {
  __REG32 SOFT_IDLE                     : 1;
  __REG32                               :31;
} __cpsw_ss_soft_idle_bits;

/* Ethernet CPSW_SS_THRU_RATE Register */
typedef struct {
  __REG32 CPDMA_THRU_RATE               : 4;
  __REG32                               : 8;
  __REG32 SL_RX_THRU_RATE               : 4;
  __REG32                               :16;
} __cpsw_ss_thru_rate_bits;

/* Ethernet CPSW_SS_GAP_THRESH Register */
typedef struct {
  __REG32 GAP_THRESH                    : 5;
  __REG32                               :27;
} __cpsw_ss_gap_thresh_bits;

/* Ethernet CPSW_SS_TX_START_WDS Register */
typedef struct {
  __REG32 TX_START_WDS                  :11;
  __REG32                               :21;
} __cpsw_ss_tx_start_wds_bits;

/* Ethernet CPSW_SS_FLOW_CONTROL Register */
typedef struct {
  __REG32 P0_FLOW_EN                    : 1;
  __REG32 P1_FLOW_EN                    : 1;
  __REG32 P2_FLOW_EN                    : 1;
  __REG32                               :29;
} __cpsw_ss_flow_control_bits;

/* Ethernet CPSW_SS_VLAN_LTYPE Register */
typedef struct {
  __REG32 VLAN_LTYPE1                   :16;
  __REG32 VLAN_LTYPE2                   :16;
} __cpsw_ss_vlan_ltype_bits;

/* Ethernet CPSW_SS_TS_LTYPE Register */
typedef struct {
  __REG32 TS_LTYPE1                     :16;
  __REG32 TS_LTYPE2                     : 6;
  __REG32                               :10;
} __cpsw_ss_ts_ltype_bits;

/* Ethernet CPSW_SS_DLR_LTYPE Register */
typedef struct {
  __REG32 DLR_LTYPE                     :16;
  __REG32                               :16;
} __cpsw_ss_dlr_ltype_bits;

/* Ethernet CPSW_WR_IDVER Register */
typedef struct {
  __REG32 MINOR                         : 6;
  __REG32 CUSTOM                        : 2;
  __REG32 MAJOR                         : 3;
  __REG32 RTL                           : 5;
  __REG32 FUNCTION                      :12;
  __REG32                               : 2;
  __REG32 SCHEME                        : 2;
} __cpsw_wr_idver_bits;

/* Ethernet CPSW_WR_SOFT_RESET Register */
typedef struct {
  __REG32 SOFT_RESET                    : 1;
  __REG32                               :31;
} __cpsw_wr_soft_reset_bits;

/* Ethernet CPSW_WR_CONTROL Register */
typedef struct {
  __REG32 MMR_IDLEMODE                  : 2;
  __REG32 MMR_STDBYMODE                 : 2;
  __REG32                               :28;
} __cpsw_wr_control_bits;

/* Ethernet CPSW_WR_INT_CONTROL Register */
typedef struct {
  __REG32 INT_PRESCALE                  :12;
  __REG32                               : 4;
  __REG32 INT_PACE_EN0                  : 1;
  __REG32 INT_PACE_EN1                  : 1;
  __REG32 INT_PACE_EN2                  : 1;
  __REG32 INT_PACE_EN3                  : 1;
  __REG32 INT_PACE_EN4                  : 1;
  __REG32 INT_PACE_EN5                  : 1;
  __REG32                               : 9;
  __REG32 INT_TEST                      : 1;
} __cpsw_wr_int_control_bits;

/* Ethernet CPSW_WR_C0_RX_THRESH_EN Register */
typedef struct {
  __REG32 C0_RX_THRESH_EN0              : 1;
  __REG32 C0_RX_THRESH_EN1              : 1;
  __REG32 C0_RX_THRESH_EN2              : 1;
  __REG32 C0_RX_THRESH_EN3              : 1;
  __REG32 C0_RX_THRESH_EN4              : 1;
  __REG32 C0_RX_THRESH_EN5              : 1;
  __REG32 C0_RX_THRESH_EN6              : 1;
  __REG32 C0_RX_THRESH_EN7              : 1;
  __REG32                               :24;
} __cpsw_wr_c0_rx_thresh_en_bits;

/* Ethernet CPSW_WR_C0_RX_EN Register */
typedef struct {
  __REG32 C0_RX_EN0                     : 1;
  __REG32 C0_RX_EN1                     : 1;
  __REG32 C0_RX_EN2                     : 1;
  __REG32 C0_RX_EN3                     : 1;
  __REG32 C0_RX_EN4                     : 1;
  __REG32 C0_RX_EN5                     : 1;
  __REG32 C0_RX_EN6                     : 1;
  __REG32 C0_RX_EN7                     : 1;
  __REG32                               :24;
} __cpsw_wr_c0_rx_en_bits;

/* Ethernet CPSW_WR_C0_TX_EN Register */
typedef struct {
  __REG32 C0_TX_EN0                     : 1;
  __REG32 C0_TX_EN1                     : 1;
  __REG32 C0_TX_EN2                     : 1;
  __REG32 C0_TX_EN3                     : 1;
  __REG32 C0_TX_EN4                     : 1;
  __REG32 C0_TX_EN5                     : 1;
  __REG32 C0_TX_EN6                     : 1;
  __REG32 C0_TX_EN7                     : 1;
  __REG32                               :24;
} __cpsw_wr_c0_tx_en_bits;

/* Ethernet CPSW_WR_C0_MISC_EN Register */
typedef struct {
  __REG32 C0_MISC_EN0                   : 1;
  __REG32 C0_MISC_EN1                   : 1;
  __REG32 C0_MISC_EN2                   : 1;
  __REG32 C0_MISC_EN3                   : 1;
  __REG32 C0_MISC_EN4                   : 1;
  __REG32                               :27;
} __cpsw_wr_c0_misc_en_bits;

/* Ethernet CPSW_WR_C1_RX_THRESH_EN Register */
typedef struct {
  __REG32 C1_RX_THRESH_EN0              : 1;
  __REG32 C1_RX_THRESH_EN1              : 1;
  __REG32 C1_RX_THRESH_EN2              : 1;
  __REG32 C1_RX_THRESH_EN3              : 1;
  __REG32 C1_RX_THRESH_EN4              : 1;
  __REG32 C1_RX_THRESH_EN5              : 1;
  __REG32 C1_RX_THRESH_EN6              : 1;
  __REG32 C1_RX_THRESH_EN7              : 1;
  __REG32                               :24;
} __cpsw_wr_c1_rx_thresh_en_bits;

/* Ethernet CPSW_WR_C1_RX_EN Register */
typedef struct {
  __REG32 C1_RX_EN0                     : 1;
  __REG32 C1_RX_EN1                     : 1;
  __REG32 C1_RX_EN2                     : 1;
  __REG32 C1_RX_EN3                     : 1;
  __REG32 C1_RX_EN4                     : 1;
  __REG32 C1_RX_EN5                     : 1;
  __REG32 C1_RX_EN6                     : 1;
  __REG32 C1_RX_EN7                     : 1;
  __REG32                               :24;
} __cpsw_wr_c1_rx_en_bits;

/* Ethernet CPSW_WR_C1_TX_EN Register */
typedef struct {
  __REG32 C1_TX_EN0                     : 1;
  __REG32 C1_TX_EN1                     : 1;
  __REG32 C1_TX_EN2                     : 1;
  __REG32 C1_TX_EN3                     : 1;
  __REG32 C1_TX_EN4                     : 1;
  __REG32 C1_TX_EN5                     : 1;
  __REG32 C1_TX_EN6                     : 1;
  __REG32 C1_TX_EN7                     : 1;
  __REG32                               :24;
} __cpsw_wr_c1_tx_en_bits;

/* Ethernet CPSW_WR_C1_MISC_EN Register */
typedef struct {
  __REG32 C1_MISC_EN0                   : 1;
  __REG32 C1_MISC_EN1                   : 1;
  __REG32 C1_MISC_EN2                   : 1;
  __REG32 C1_MISC_EN3                   : 1;
  __REG32 C1_MISC_EN4                   : 1;
  __REG32                               :27;
} __cpsw_wr_c1_misc_en_bits;

/* Ethernet CPSW_WR_C2_RX_THRESH_EN Register */
typedef struct {
  __REG32 C2_RX_THRESH_EN0              : 1;
  __REG32 C2_RX_THRESH_EN1              : 1;
  __REG32 C2_RX_THRESH_EN2              : 1;
  __REG32 C2_RX_THRESH_EN3              : 1;
  __REG32 C2_RX_THRESH_EN4              : 1;
  __REG32 C2_RX_THRESH_EN5              : 1;
  __REG32 C2_RX_THRESH_EN6              : 1;
  __REG32 C2_RX_THRESH_EN7              : 1;
  __REG32                               :24;
} __cpsw_wr_c2_rx_thresh_en_bits;

/* Ethernet CPSW_WR_C2_RX_EN Register */
typedef struct {
  __REG32 C2_RX_EN0                     : 1;
  __REG32 C2_RX_EN1                     : 1;
  __REG32 C2_RX_EN2                     : 1;
  __REG32 C2_RX_EN3                     : 1;
  __REG32 C2_RX_EN4                     : 1;
  __REG32 C2_RX_EN5                     : 1;
  __REG32 C2_RX_EN6                     : 1;
  __REG32 C2_RX_EN7                     : 1;
  __REG32                               :24;
} __cpsw_wr_c2_rx_en_bits;

/* Ethernet CPSW_WR_C2_TX_EN Register */
typedef struct {
  __REG32 C2_TX_EN0                     : 1;
  __REG32 C2_TX_EN1                     : 1;
  __REG32 C2_TX_EN2                     : 1;
  __REG32 C2_TX_EN3                     : 1;
  __REG32 C2_TX_EN4                     : 1;
  __REG32 C2_TX_EN5                     : 1;
  __REG32 C2_TX_EN6                     : 1;
  __REG32 C2_TX_EN7                     : 1;
  __REG32                               :24;
} __cpsw_wr_c2_tx_en_bits;

/* Ethernet CPSW_WR_C2_MISC_EN Register */
typedef struct {
  __REG32 C2_MISC_EN0                   : 1;
  __REG32 C2_MISC_EN1                   : 1;
  __REG32 C2_MISC_EN2                   : 1;
  __REG32 C2_MISC_EN3                   : 1;
  __REG32 C2_MISC_EN4                   : 1;
  __REG32                               :27;
} __cpsw_wr_c2_misc_en_bits;

/* Ethernet CPSW_WR_C0_RX_THRESH_STAT Register */
typedef struct {
  __REG32 C0_RX_THRESH_STAT0            : 1;
  __REG32 C0_RX_THRESH_STAT1            : 1;
  __REG32 C0_RX_THRESH_STAT2            : 1;
  __REG32 C0_RX_THRESH_STAT3            : 1;
  __REG32 C0_RX_THRESH_STAT4            : 1;
  __REG32 C0_RX_THRESH_STAT5            : 1;
  __REG32 C0_RX_THRESH_STAT6            : 1;
  __REG32 C0_RX_THRESH_STAT7            : 1;
  __REG32                               :24;
} __cpsw_wr_c0_rx_thresh_stat_bits;

/* Ethernet CPSW_WR_C0_RX_STAT Register */
typedef struct {
  __REG32 C0_RX_STAT0                   : 1;
  __REG32 C0_RX_STAT1                   : 1;
  __REG32 C0_RX_STAT2                   : 1;
  __REG32 C0_RX_STAT3                   : 1;
  __REG32 C0_RX_STAT4                   : 1;
  __REG32 C0_RX_STAT5                   : 1;
  __REG32 C0_RX_STAT6                   : 1;
  __REG32 C0_RX_STAT7                   : 1;
  __REG32                               :24;
} __cpsw_wr_c0_rx_stat_bits;

/* Ethernet CPSW_WR_C0_TX_STAT Register */
typedef struct {
  __REG32 C0_TX_STAT0                   : 1;
  __REG32 C0_TX_STAT1                   : 1;
  __REG32 C0_TX_STAT2                   : 1;
  __REG32 C0_TX_STAT3                   : 1;
  __REG32 C0_TX_STAT4                   : 1;
  __REG32 C0_TX_STAT5                   : 1;
  __REG32 C0_TX_STAT6                   : 1;
  __REG32 C0_TX_STAT7                   : 1;
  __REG32                               :24;
} __cpsw_wr_c0_tx_stat_bits;

/* Ethernet CPSW_WR_C0_MISC_STAT Register */
typedef struct {
  __REG32 C0_MISC_STAT0                 : 1;
  __REG32 C0_MISC_STAT1                 : 1;
  __REG32 C0_MISC_STAT2                 : 1;
  __REG32 C0_MISC_STAT3                 : 1;
  __REG32 C0_MISC_STAT4                 : 1;
  __REG32                               :27;
} __cpsw_wr_c0_misc_stat_bits;

/* Ethernet CPSW_WR_C1_RX_THRESH_STAT Register */
typedef struct {
  __REG32 C1_RX_THRESH_STAT0            : 1;
  __REG32 C1_RX_THRESH_STAT1            : 1;
  __REG32 C1_RX_THRESH_STAT2            : 1;
  __REG32 C1_RX_THRESH_STAT3            : 1;
  __REG32 C1_RX_THRESH_STAT4            : 1;
  __REG32 C1_RX_THRESH_STAT5            : 1;
  __REG32 C1_RX_THRESH_STAT6            : 1;
  __REG32 C1_RX_THRESH_STAT7            : 1;
  __REG32                               :24;
} __cpsw_wr_c1_rx_thresh_stat_bits;

/* Ethernet CPSW_WR_C1_RX_STAT Register */
typedef struct {
  __REG32 C1_RX_STAT0                   : 1;
  __REG32 C1_RX_STAT1                   : 1;
  __REG32 C1_RX_STAT2                   : 1;
  __REG32 C1_RX_STAT3                   : 1;
  __REG32 C1_RX_STAT4                   : 1;
  __REG32 C1_RX_STAT5                   : 1;
  __REG32 C1_RX_STAT6                   : 1;
  __REG32 C1_RX_STAT7                   : 1;
  __REG32                               :24;
} __cpsw_wr_c1_rx_stat_bits;

/* Ethernet CPSW_WR_C1_TX_STAT Register */
typedef struct {
  __REG32 C1_TX_STAT0                   : 1;
  __REG32 C1_TX_STAT1                   : 1;
  __REG32 C1_TX_STAT2                   : 1;
  __REG32 C1_TX_STAT3                   : 1;
  __REG32 C1_TX_STAT4                   : 1;
  __REG32 C1_TX_STAT5                   : 1;
  __REG32 C1_TX_STAT6                   : 1;
  __REG32 C1_TX_STAT7                   : 1;
  __REG32                               :24;
} __cpsw_wr_c1_tx_stat_bits;

/* Ethernet CPSW_WR_C1_MISC_STAT Register */
typedef struct {
  __REG32 C1_MISC_STAT0                 : 1;
  __REG32 C1_MISC_STAT1                 : 1;
  __REG32 C1_MISC_STAT2                 : 1;
  __REG32 C1_MISC_STAT3                 : 1;
  __REG32 C1_MISC_STAT4                 : 1;
  __REG32                               :27;
} __cpsw_wr_c1_misc_stat_bits;

/* Ethernet CPSW_WR_C2_RX_THRESH_STAT Register */
typedef struct {
  __REG32 C2_RX_THRESH_STAT0            : 1;
  __REG32 C2_RX_THRESH_STAT1            : 1;
  __REG32 C2_RX_THRESH_STAT2            : 1;
  __REG32 C2_RX_THRESH_STAT3            : 1;
  __REG32 C2_RX_THRESH_STAT4            : 1;
  __REG32 C2_RX_THRESH_STAT5            : 1;
  __REG32 C2_RX_THRESH_STAT6            : 1;
  __REG32 C2_RX_THRESH_STAT7            : 1;
  __REG32                               :24;
} __cpsw_wr_c2_rx_thresh_stat_bits;

/* Ethernet CPSW_WR_C2_RX_STAT Register */
typedef struct {
  __REG32 C2_RX_STAT0                   : 1;
  __REG32 C2_RX_STAT1                   : 1;
  __REG32 C2_RX_STAT2                   : 1;
  __REG32 C2_RX_STAT3                   : 1;
  __REG32 C2_RX_STAT4                   : 1;
  __REG32 C2_RX_STAT5                   : 1;
  __REG32 C2_RX_STAT6                   : 1;
  __REG32 C2_RX_STAT7                   : 1;
  __REG32                               :24;
} __cpsw_wr_c2_rx_stat_bits;

/* Ethernet CPSW_WR_C2_TX_STAT Register */
typedef struct {
  __REG32 C2_TX_STAT0                   : 1;
  __REG32 C2_TX_STAT1                   : 1;
  __REG32 C2_TX_STAT2                   : 1;
  __REG32 C2_TX_STAT3                   : 1;
  __REG32 C2_TX_STAT4                   : 1;
  __REG32 C2_TX_STAT5                   : 1;
  __REG32 C2_TX_STAT6                   : 1;
  __REG32 C2_TX_STAT7                   : 1;
  __REG32                               :24;
} __cpsw_wr_c2_tx_stat_bits;

/* Ethernet CPSW_WR_C2_MISC_STAT Register */
typedef struct {
  __REG32 C2_MISC_STAT0                 : 1;
  __REG32 C2_MISC_STAT1                 : 1;
  __REG32 C2_MISC_STAT2                 : 1;
  __REG32 C2_MISC_STAT3                 : 1;
  __REG32 C2_MISC_STAT4                 : 1;
  __REG32                               :27;
} __cpsw_wr_c2_misc_stat_bits;

/* Ethernet CPSW_WR_C0_RX_IMAX Register */
typedef struct {
  __REG32 C0_RX_IMAX                    : 6;
  __REG32                               :26;
} __cpsw_wr_c0_rx_imax_bits;

/* Ethernet CPSW_WR_C0_TX_IMAX Register */
typedef struct {
  __REG32 C0_TX_IMAX                    : 6;
  __REG32                               :26;
} __cpsw_wr_c0_tx_imax_bits;

/* Ethernet CPSW_WR_C1_RX_IMAX Register */
typedef struct {
  __REG32 C1_RX_IMAX                    : 6;
  __REG32                               :26;
} __cpsw_wr_c1_rx_imax_bits;

/* Ethernet CPSW_WR_C1_TX_IMAX Register */
typedef struct {
  __REG32 C1_TX_IMAX                    : 6;
  __REG32                               :26;
} __cpsw_wr_c1_tx_imax_bits;

/* Ethernet CPSW_WR_C2_RX_IMAX Register */
typedef struct {
  __REG32 C2_RX_IMAX                    : 6;
  __REG32                               :26;
} __cpsw_wr_c2_rx_imax_bits;

/* Ethernet CPSW_WR_C2_TX_IMAX Register */
typedef struct {
  __REG32 C2_TX_IMAX                    : 6;
  __REG32                               :26;
} __cpsw_wr_c2_tx_imax_bits;

/* Ethernet CPSW_WR_RGMII_CTL Register */
typedef struct {
  __REG32 RGMII1_LINK                   : 1;
  __REG32 RGMII1_SPEED                  : 2;
  __REG32 RGMII1_FULLDUPLEX             : 1;
  __REG32 RGMII2_LINK                   : 1;
  __REG32 RGMII2_SPEED                  : 2;
  __REG32 RGMII2_FULLDUPLEX             : 1;
  __REG32                               :24;
} __cpsw_wr_rgmii_ctl_bits;

/* MDIO Version Register (MDIOVER) */
typedef struct {
  __REG32 REVMIN                        : 8;
  __REG32 REVMAJ                        : 8;
  __REG32 MODID                         :16;
} __mdio_ver_bits;

/* MDIO Control Register (MDIOCONTROL) */
typedef struct {
  __REG32 CLKDIV                        :16;
  __REG32                               : 1;
  __REG32 INTTESTENB                    : 1;
  __REG32 FAULTENB                      : 1;
  __REG32 FAULT                         : 1;
  __REG32 PREAMBLE                      : 1;
  __REG32                               : 3;
  __REG32 HIGHEST_USER_CHANNEL          : 5;
  __REG32                               : 1;
  __REG32 ENABLE                        : 1;
  __REG32 IDLE                          : 1;
} __mdio_control_bits;

/* PHY Acknowledge Status Register (MDIOALIVE) */
typedef struct {
  __REG32 ALIVE0                        : 1;
  __REG32 ALIVE1                        : 1;
  __REG32 ALIVE2                        : 1;
  __REG32 ALIVE3                        : 1;
  __REG32 ALIVE4                        : 1;
  __REG32 ALIVE5                        : 1;
  __REG32 ALIVE6                        : 1;
  __REG32 ALIVE7                        : 1;
  __REG32 ALIVE8                        : 1;
  __REG32 ALIVE9                        : 1;
  __REG32 ALIVE10                       : 1;
  __REG32 ALIVE11                       : 1;
  __REG32 ALIVE12                       : 1;
  __REG32 ALIVE13                       : 1;
  __REG32 ALIVE14                       : 1;
  __REG32 ALIVE15                       : 1;
  __REG32 ALIVE16                       : 1;
  __REG32 ALIVE17                       : 1;
  __REG32 ALIVE18                       : 1;
  __REG32 ALIVE19                       : 1;
  __REG32 ALIVE20                       : 1;
  __REG32 ALIVE21                       : 1;
  __REG32 ALIVE22                       : 1;
  __REG32 ALIVE23                       : 1;
  __REG32 ALIVE24                       : 1;
  __REG32 ALIVE25                       : 1;
  __REG32 ALIVE26                       : 1;
  __REG32 ALIVE27                       : 1;
  __REG32 ALIVE28                       : 1;
  __REG32 ALIVE29                       : 1;
  __REG32 ALIVE30                       : 1;
  __REG32 ALIVE31                       : 1;
} __mdio_alive_bits;

#define MDIOALIVE_ALIVE0                (0x1UL<<0)
#define MDIOALIVE_ALIVE1                (0x1UL<<1)
#define MDIOALIVE_ALIVE2                (0x1UL<<2)
#define MDIOALIVE_ALIVE3                (0x1UL<<3)
#define MDIOALIVE_ALIVE4                (0x1UL<<4)
#define MDIOALIVE_ALIVE5                (0x1UL<<5)
#define MDIOALIVE_ALIVE6                (0x1UL<<6)
#define MDIOALIVE_ALIVE7                (0x1UL<<7)
#define MDIOALIVE_ALIVE8                (0x1UL<<8)
#define MDIOALIVE_ALIVE9                (0x1UL<<9)
#define MDIOALIVE_ALIVE10               (0x1UL<<10)
#define MDIOALIVE_ALIVE11               (0x1UL<<11)
#define MDIOALIVE_ALIVE12               (0x1UL<<12)
#define MDIOALIVE_ALIVE13               (0x1UL<<13)
#define MDIOALIVE_ALIVE14               (0x1UL<<14)
#define MDIOALIVE_ALIVE15               (0x1UL<<15)
#define MDIOALIVE_ALIVE16               (0x1UL<<16)
#define MDIOALIVE_ALIVE17               (0x1UL<<17)
#define MDIOALIVE_ALIVE18               (0x1UL<<18)
#define MDIOALIVE_ALIVE19               (0x1UL<<19)
#define MDIOALIVE_ALIVE20               (0x1UL<<20)
#define MDIOALIVE_ALIVE21               (0x1UL<<21)
#define MDIOALIVE_ALIVE22               (0x1UL<<22)
#define MDIOALIVE_ALIVE23               (0x1UL<<23)
#define MDIOALIVE_ALIVE24               (0x1UL<<24)
#define MDIOALIVE_ALIVE25               (0x1UL<<25)
#define MDIOALIVE_ALIVE26               (0x1UL<<26)
#define MDIOALIVE_ALIVE27               (0x1UL<<27)
#define MDIOALIVE_ALIVE28               (0x1UL<<28)
#define MDIOALIVE_ALIVE29               (0x1UL<<29)
#define MDIOALIVE_ALIVE30               (0x1UL<<30)
#define MDIOALIVE_ALIVE31               (0x1UL<<31)

/* PHY Link Status Register (MDIOLINK) */
typedef struct {
  __REG32 LINK0                         : 1;
  __REG32 LINK1                         : 1;
  __REG32 LINK2                         : 1;
  __REG32 LINK3                         : 1;
  __REG32 LINK4                         : 1;
  __REG32 LINK5                         : 1;
  __REG32 LINK6                         : 1;
  __REG32 LINK7                         : 1;
  __REG32 LINK8                         : 1;
  __REG32 LINK9                         : 1;
  __REG32 LINK10                        : 1;
  __REG32 LINK11                        : 1;
  __REG32 LINK12                        : 1;
  __REG32 LINK13                        : 1;
  __REG32 LINK14                        : 1;
  __REG32 LINK15                        : 1;
  __REG32 LINK16                        : 1;
  __REG32 LINK17                        : 1;
  __REG32 LINK18                        : 1;
  __REG32 LINK19                        : 1;
  __REG32 LINK20                        : 1;
  __REG32 LINK21                        : 1;
  __REG32 LINK22                        : 1;
  __REG32 LINK23                        : 1;
  __REG32 LINK24                        : 1;
  __REG32 LINK25                        : 1;
  __REG32 LINK26                        : 1;
  __REG32 LINK27                        : 1;
  __REG32 LINK28                        : 1;
  __REG32 LINK29                        : 1;
  __REG32 LINK30                        : 1;
  __REG32 LINK31                        : 1;
} __mdio_link_bits;

/* MDIO Link Status Change Interrupt Register (MDIOLINKINTRAW) */
typedef struct {
  __REG32 LINKINTRAW0                   : 1;
  __REG32 LINKINTRAW1                   : 1;
  __REG32                               :30;
} __mdio_linkintraw_bits;

/* MDIO Link Status Change Interrupt Register (Masked Value) (MDIOLINKINTMASKED) */
typedef struct {
  __REG32 LINKINTMASKED0                : 1;
  __REG32 LINKINTMASKED1                : 1;
  __REG32                               :30;
} __mdio_linkintmasked_bits;

#define MDIOLINKINT_LINKINT0            (0x1UL<<0)
#define MDIOLINKINT_LINKINT1            (0x1UL<<1)

/* MDIO User Command Complete Interrupt Register (Raw Value) (MDIOUSERINTRAW) */
typedef struct {
  __REG32 USERINTRAW0                   : 1;
  __REG32 USERINTRAW1                   : 1;
  __REG32                               :30;
} __mdio_userintraw_bits;

/* MDIO User Command Complete Interrupt Register (Masked Value) (MDIOUSERINTMASKED) */
typedef struct {
  __REG32 USERINTMASKED0                : 1;
  __REG32 USERINTMASKED1                : 1;
  __REG32                               :30;
} __mdio_userintmasked_bits;

/* MDIO User Command Complete Interrupt Mask Set Register (MDIOUSERINTMASKSET) */
typedef struct {
  __REG32 USERINTMASKSET0               : 1;
  __REG32 USERINTMASKSET1               : 1;
  __REG32                               :30;
} __mdio_userintmaskset_bits;

/* MDIO User Command Complete Interrupt Mask Clear Register (MDIOUSERINTMASKCLR) */
typedef struct {
  __REG32 USERINTMASKCLEAR0             : 1;
  __REG32 USERINTMASKCLEAR1             : 1;
  __REG32                               :30;
} __mdio_userintmaskclr_bits;

#define MDIOUSERINT_USERINT0            (0x1UL<<0)
#define MDIOUSERINT_USERINT1            (0x1UL<<1)

/* MDIO User Access Register 0 (MDIOUSERACCESS0) */
/* MDIO User Access Register 1 (MDIOUSERACCESS1) */
typedef struct {
  __REG32 DATA                          :16;
  __REG32 PHYADR                        : 5;
  __REG32 REGADR                        : 5;
  __REG32                               : 3;
  __REG32 ACK                           : 1;
  __REG32 WRITE                         : 1;
  __REG32 GO                            : 1;
} __mdio_useraccess_bits;

/* MDIO User PHY Select Register 0 (MDIOUSERPHYSEL0) */
/* MDIO User PHY Select Register 1 (MDIOUSERPHYSEL1) */
typedef struct {
  __REG32 PHYADDRMON                    : 5;
  __REG32                               : 1;
  __REG32 LINKINTENB                    : 1;
  __REG32 LINKSEL                       : 1;
  __REG32                               :24;
} __mdio_userphysel_bits;

/* PWMSS IP Revision Register (IDVER) */
typedef struct {
  __REG32 Y_MINOR                       : 6;
  __REG32 CUSTOM                        : 2;
  __REG32 X_MAJOR                       : 3;
  __REG32 R_RTL                         : 5;
  __REG32 FUNC                          :12;
  __REG32                               : 2;
  __REG32 SCHEME                        : 2;
} __pwmss_idver_bits;

/* PWMSS System Configuration Register (SYSCONFIG) */
typedef struct {
  __REG32 SOFTRESET                     : 1;
  __REG32 FREEEMU                       : 1;
  __REG32 IDLEMODE                      : 2;
  __REG32 STANDBYMODE                   : 2;
  __REG32                               :26;
} __pwmss_sysconfig_bits;

/* PWMSS Clock Configuration Register (CLKCONFG) */
typedef struct {
  __REG32 ECAPCLK_EN                    : 1;
  __REG32 ECAPCLKSTOP_REQ               : 1;
  __REG32                               : 2;
  __REG32 EQEPCLK_EN                    : 1;
  __REG32 EQEPCLKSTOP_REQ               : 1;
  __REG32                               : 2;
  __REG32 EPWMCLK_EN                    : 1;
  __REG32 EPWMCLKSTOP_REQ               : 1;
  __REG32                               :22;
} __pwmss_clkconfig_bits;

/* PWMSS Clock Status Register (CLKSTATUS) */
typedef struct {
  __REG32 ECAPCLK_EN_ACK                : 1;
  __REG32 ECAPCLKSTOP_ACK               : 1;
  __REG32                               : 2;
  __REG32 EQEPCLK_EN_ACK                : 1;
  __REG32 EQEPCLKSTOP_ACK               : 1;
  __REG32                               : 2;
  __REG32 EPWMCLK_EN_ACK                : 1;
  __REG32 EPWMCLKSTOP_ACK               : 1;
  __REG32                               :22;
} __pwmss_clkstatus_bits;

/* eHRPWM Time-Base Control Register (TBCTL) */
typedef struct {
  __REG16 CTRMODE                 : 2;
  __REG16 PHSEN                   : 1;
  __REG16 PRDLD                   : 1;
  __REG16 SYNCOSEL                : 2;
  __REG16 SWFSYNC                 : 1;
  __REG16 HSPCLKDIV               : 3;
  __REG16 CLKDIV                  : 3;
  __REG16 PHSDIR                  : 1;
  __REG16 FREE_SOFT               : 2;
} __ehrpwm_tbctl_bits;

/* eHRPWM Time-Base Status Register (TBSTS) */
typedef struct {
  __REG16 CTRDIR                  : 1;
  __REG16 SYNCI                   : 1;
  __REG16 CTRMAX                  : 1;
  __REG16                         :13;
} __ehrpwm_tbsts_bits;

#define EPWM_TBSTS_SYNCI          (0x1UL<<1)
#define EPWM_TBSTS_CTRMAX         (0x1UL<<2)

/* eHRPWM Counter-Compare Control Register (CMPCTL) */
typedef struct {
  __REG16 LOADAMODE               : 2;
  __REG16 LOADBMODE               : 2;
  __REG16 SHDWAMODE               : 1;
  __REG16                         : 1;
  __REG16 SHDWBMODE               : 1;
  __REG16                         : 1;
  __REG16 SHDWAFULL               : 1;
  __REG16 SHDWBFULL               : 1;
  __REG16                         : 6;
} __ehrpwm_cmpctl_bits;

/* eHRPWM Action-Qualifier Output A Control Register (AQCTLA) */
/* eHRPWM Action-Qualifier Output B Control Register (AQCTLB) */
typedef struct {
  __REG16 ZRO                     : 2;
  __REG16 PRD                     : 2;
  __REG16 CAU                     : 2;
  __REG16 CAD                     : 2;
  __REG16 CBU                     : 2;
  __REG16 CBD                     : 2;
  __REG16                         : 4;
} __ehrpwm_aqctla_bits;

/* eHRPWM Action-Qualifier Software Force Register (AQSFRC) */
typedef struct {
  __REG16 ACTSFA                  : 2;
  __REG16 OTSFA                   : 1;
  __REG16 ACTSFB                  : 2;
  __REG16 OTSFB                   : 1;
  __REG16 RLDCSF                  : 2;
  __REG16                         : 8;
} __ehrpwm_aqsfrc_bits;

/* eHRPWM Action-Qualifier Continuous Software Force Register (AQCSFRC) */
typedef struct {
  __REG16 CSFA                    : 2;
  __REG16 CSFB                    : 2;
  __REG16                         :12;
} __ehrpwm_aqcsfrc_bits;

/* eHRPWM Dead-Band Generator Control Register (DBCTL) */
typedef struct {
  __REG16 OUT_MODE                : 2;
  __REG16 POLSEL                  : 2;
  __REG16 IN_MODE                 : 2;
  __REG16                         :10;
} __ehrpwm_dbctl_bits;

/* eHRPWM Dead-Band Generator Rising Edge Delay Register (DBRED) */
/* eHRPWM Dead-Band Generator Falling Edge Delay Register (DBFED) */
typedef struct {
  __REG16 DEL                     :10;
  __REG16                         : 6;
} __ehrpwm_dbred_bits;

/* eHRPWM PWM-Chopper Submodule Register */
typedef struct {
  __REG16 CHPEN                   : 1;
  __REG16 OSHTWTH                 : 4;
  __REG16 CHPFREQ                 : 3;
  __REG16 CHPDUTY                 : 3;
  __REG16                         : 5;
} __ehrpwm_pcctl_bits;

/* eHRPWM Trip-Zone Select Register (TZSEL) */
typedef struct {
  __REG16 CBC1                    : 1;
  __REG16                         : 7;
  __REG16 OSHT1                   : 1;
  __REG16                         : 7;
} __ehrpwm_tzsel_bits;

/* eHRPWM Trip-Zone Control Register (TZCTL) */
typedef struct {
  __REG16 TZA                     : 2;
  __REG16 TZB                     : 2;
  __REG16                         :12;
} __ehrpwm_tzctl_bits;

/* eHRPWM Trip-Zone Enable Interrupt Register (TZEINT) */
/* eHRPWM Trip-Zone Force Register (TZFRC) */
typedef struct {
  __REG16                         : 1;
  __REG16 CBC                     : 1;
  __REG16 OST                     : 1;
  __REG16                         :13;
} __ehrpwm_tzeint_bits;

/* eHRPWM Trip-Zone Flag Register (TZFLG) */
/* eHRPWM Trip-Zone Clear Register (TZCLR) */
typedef struct {
  __REG16 INT                     : 1;
  __REG16 CBC                     : 1;
  __REG16 OST                     : 1;
  __REG16                         :13;
} __ehrpwm_tzflg_bits;

/* eHRPWM Event-Trigger Selection Register (ETSEL) */
typedef struct {
  __REG16 INTSEL                  : 3;
  __REG16 INTEN                   : 1;
  __REG16                         :12;
} __ehrpwm_etsel_bits;

/* eHRPWM Event-Trigger Prescale Register (ETPS) */
typedef struct {
  __REG16 INTPRD                  : 2;
  __REG16 INTCNT                  : 2;
  __REG16                         :12;
} __ehrpwm_etps_bits;

/* eHRPWM Event-Trigger Prescale Register (ETPS) */
/* eHRPWM Event-Trigger Clear Register (ETCLR) */
/* eHRPWM Event-Trigger Force Register (ETFRC) */
typedef struct {
  __REG16 INT                     : 1;
  __REG16                         :15;
} __ehrpwm_etflg_bits;

/* eHRPWM Time-Base Phase High-Resolution Register (TBPHSHR) */
typedef struct {
  __REG16                         : 8;
  __REG16 TBPHSH                  : 8;
} __ehrpwm_tbphshr_bits;

/* eHRPWM Counter-Compare A High-Resolution Register (CMPAHR) */
typedef struct {
  __REG16                         : 8;
  __REG16 CMPAHR                  : 8;
} __ehrpwm_cmpahr_bits;

/* eHRPWM Control Register (HRCTL) */
typedef struct {
  __REG16 DELMODE                 : 2;
  __REG16 DELBUSSEL               : 1;
  __REG16 PULSESEL                : 1;
  __REG16                         :12;
} __ehrpwm_hrctl_bits;

/* QEP Decoder Control Register (QDECCTL) */
typedef struct {
  __REG16                         : 5;
  __REG16 QSP                     : 1;
  __REG16 QIP                     : 1;
  __REG16 QBP                     : 1;
  __REG16 QAP                     : 1;
  __REG16 IGATE                   : 1;
  __REG16 SWAP                    : 1;
  __REG16 XCR                     : 1;
  __REG16 SPSEL                   : 1;
  __REG16 SOEN                    : 1;
  __REG16 QSRC                    : 2;
} __eqep_qdecctl_bits;

/* eQEP Control Register (QEPCTL) */
typedef struct {
  __REG16 WDE                     : 1;
  __REG16 UTE                     : 1;
  __REG16 QCLM                    : 1;
  __REG16 PHEN                    : 1;
  __REG16 IEL                     : 2;
  __REG16 SEL                     : 1;
  __REG16 SWI                     : 1;
  __REG16 IEI                     : 2;
  __REG16 SEI                     : 2;
  __REG16 PCRM                    : 2;
  __REG16 FREE_SOFT               : 2;
} __eqep_qepctl_bits;

/* eQEP Capture Control Register (QCAPCTL) */
typedef struct {
  __REG16 UPPS                    : 4;
  __REG16 CCPS                    : 3;
  __REG16                         : 8;
  __REG16 CEN                     : 1;
} __eqep_qcapctl_bits;

/* eQEP Position-Compare Control Register (QPOSCTL) */
typedef struct {
  __REG16 PCSPW                   :12;
  __REG16 PCE                     : 1;
  __REG16 PCPOL                   : 1;
  __REG16 PCLOAD                  : 1;
  __REG16 PCSHDW                  : 1;
} __eqep_qposctl_bits;

/* eQEP Interrupt Enable Register (QEINT) */
/* eQEP Interrupt Force Register (QFRC) */
typedef struct {
  __REG16                         : 1;
  __REG16 PCE                     : 1;
  __REG16 PHE                     : 1;
  __REG16 QDC                     : 1;
  __REG16 WTO                     : 1;
  __REG16 PCU                     : 1;
  __REG16 PCO                     : 1;
  __REG16 PCR                     : 1;
  __REG16 PCM                     : 1;
  __REG16 SEL                     : 1;
  __REG16 IEL                     : 1;
  __REG16 UTO                     : 1;
  __REG16                         : 4;
} __eqep_qeint_bits;

/* eQEP Interrupt Flag Register (QFLG) */
/* eQEP Interrupt Clear Register (QCLR) */
typedef struct {
  __REG16 INT                     : 1;
  __REG16 PCE                     : 1;
  __REG16 PHE                     : 1;
  __REG16 QDC                     : 1;
  __REG16 WTO                     : 1;
  __REG16 PCU                     : 1;
  __REG16 PCO                     : 1;
  __REG16 PCR                     : 1;
  __REG16 PCM                     : 1;
  __REG16 SEL                     : 1;
  __REG16 IEL                     : 1;
  __REG16 UTO                     : 1;
  __REG16                         : 4;
} __eqep_qflg_bits;

/* eQEP Status Register (QEPSTS) */
typedef struct {
  __REG16 PCEF                    : 1;
  __REG16 FIMF                    : 1;
  __REG16 CDEF                    : 1;
  __REG16 COEF                    : 1;
  __REG16 QDLF                    : 1;
  __REG16 QDF                     : 1;
  __REG16 FIDF                    : 1;
  __REG16 UPEVNT                  : 1;
  __REG16                         : 8;
} __eqep_qepsts_bits;

#define EQEP_QEPSTS_FIMF          (0x1UL<<1)
#define EQEP_QEPSTS_CDEF          (0x1UL<<2)
#define EQEP_QEPSTS_COEF          (0x1UL<<3)

/* USBSS REVREG Register */
typedef struct {
  __REG32 Y_MINOR                 : 6;
  __REG32 CUSTOM                  : 2;
  __REG32 X_MAJOR                 : 3;
  __REG32 R_RTL                   : 5;
  __REG32 FUNC                    :12;
  __REG32                         : 2;
  __REG32 SCHEME                  : 2;
} __usbss_revreg_bits;

/* USBSS SYSCONFIG Register */
typedef struct {
  __REG32 SOFT_RESET              : 1;
  __REG32 FREEEMU                 : 1;
  __REG32 IDLEMODE                : 2;
  __REG32 STANDBY_MODE            : 2;
  __REG32                         : 2;
  __REG32 PHY1_UTMI_EN_N          : 1;
  __REG32 USB1_OCP_EN_N           : 1;
  __REG32 PHY0_UTMI_EN_N          : 1;
  __REG32 USB0_OCP_EN_N           : 1;
  __REG32                         :20;
} __usbss_sysconfig_bits;

/* USBSS EOI Register */
typedef struct {
  __REG32 EOI_VECTOR              : 1;
  __REG32                         :31;
} __usbss_eoi_bits;

/* USBSS IRQSTATRAW Register */
/* USBSS IRQSTAT Register */
/* USBSS IRQENABLER Register */
/* USBSS IRQCLEARR Register */
typedef struct {
  __REG32 RX_SOP_STARVATION       : 1;
  __REG32 RX_MOP_STARVATION       : 1;
  __REG32 PD_CMP_FLAG             : 1;
  __REG32                         : 5;
  __REG32 TX_PKT_CMP_0            : 1;
  __REG32 RX_PKT_CMP_0            : 1;
  __REG32 TX_PKT_CMP_1            : 1;
  __REG32 RX_PKT_CMP_1            : 1;
  __REG32                         :20;
} __usbss_irq_bits;

/* USBSS IRQDMATHOLDTX00 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 DMA_THRES_TX0_1         : 8;
  __REG32 DMA_THRES_TX0_2         : 8;
  __REG32 DMA_THRES_TX0_3         : 8;
} __usbss_irqdmatholdtx00_bits;

/* USBSS IRQDMATHOLDTX01 Register */
typedef struct {
  __REG32 DMA_THRES_TX0_4         : 8;
  __REG32 DMA_THRES_TX0_5         : 8;
  __REG32 DMA_THRES_TX0_6         : 8;
  __REG32 DMA_THRES_TX0_7         : 8;
} __usbss_irqdmatholdtx01_bits;

/* USBSS IRQDMATHOLDTX02 Register */
typedef struct {
  __REG32 DMA_THRES_TX0_8         : 8;
  __REG32 DMA_THRES_TX0_9         : 8;
  __REG32 DMA_THRES_TX0_10        : 8;
  __REG32 DMA_THRES_TX0_11        : 8;
} __usbss_irqdmatholdtx02_bits;

/* USBSS IRQDMATHOLDTX03 Register */
typedef struct {
  __REG32 DMA_THRES_TX0_12        : 8;
  __REG32 DMA_THRES_TX0_13        : 8;
  __REG32 DMA_THRES_TX0_14        : 8;
  __REG32 DMA_THRES_TX0_15        : 8;
} __usbss_irqdmatholdtx03_bits;

/* USBSS IRQDMATHOLDRX00 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 DMA_THRES_RX0_1         : 8;
  __REG32 DMA_THRES_RX0_2         : 8;
  __REG32 DMA_THRES_RX0_3         : 8;
} __usbss_irqdmatholdrx00_bits;

/* USBSS IRQDMATHOLDRX01 Register */
typedef struct {
  __REG32 DMA_THRES_RX0_4         : 8;
  __REG32 DMA_THRES_RX0_5         : 8;
  __REG32 DMA_THRES_RX0_6         : 8;
  __REG32 DMA_THRES_RX0_7         : 8;
} __usbss_irqdmatholdrx01_bits;

/* USBSS IRQDMATHOLDRX02 Register */
typedef struct {
  __REG32 DMA_THRES_RX0_8         : 8;
  __REG32 DMA_THRES_RX0_9         : 8;
  __REG32 DMA_THRES_RX0_10        : 8;
  __REG32 DMA_THRES_RX0_11        : 8;
} __usbss_irqdmatholdrx02_bits;

/* USBSS IRQDMATHOLDRX03 Register */
typedef struct {
  __REG32 DMA_THRES_RX0_12        : 8;
  __REG32 DMA_THRES_RX0_13        : 8;
  __REG32 DMA_THRES_RX0_14        : 8;
  __REG32 DMA_THRES_RX0_15        : 8;
} __usbss_irqdmatholdrx03_bits;

/* USBSS IRQDMATHOLDTX10 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 DMA_THRES_TX1_1         : 8;
  __REG32 DMA_THRES_TX1_2         : 8;
  __REG32 DMA_THRES_TX1_3         : 8;
} __usbss_irqdmatholdtx10_bits;

/* USBSS IRQDMATHOLDTX11 Register */
typedef struct {
  __REG32 DMA_THRES_TX1_4         : 8;
  __REG32 DMA_THRES_TX1_5         : 8;
  __REG32 DMA_THRES_TX1_6         : 8;
  __REG32 DMA_THRES_TX1_7         : 8;
} __usbss_irqdmatholdtx11_bits;

/* USBSS IRQDMATHOLDTX12 Register */
typedef struct {
  __REG32 DMA_THRES_TX1_8         : 8;
  __REG32 DMA_THRES_TX1_9         : 8;
  __REG32 DMA_THRES_TX1_10        : 8;
  __REG32 DMA_THRES_TX1_11        : 8;
} __usbss_irqdmatholdtx12_bits;

/* USBSS IRQDMATHOLDTX13 Register */
typedef struct {
  __REG32 DMA_THRES_TX1_12        : 8;
  __REG32 DMA_THRES_TX1_13        : 8;
  __REG32 DMA_THRES_TX1_14        : 8;
  __REG32 DMA_THRES_TX1_15        : 8;
} __usbss_irqdmatholdtx13_bits;

/* USBSS IRQDMATHOLDRX10 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 DMA_THRES_RX1_1         : 8;
  __REG32 DMA_THRES_RX1_2         : 8;
  __REG32 DMA_THRES_RX1_3         : 8;
} __usbss_irqdmatholdrx10_bits;

/* USBSS IRQDMATHOLDRX11 Register */
typedef struct {
  __REG32 DMA_THRES_RX1_4         : 8;
  __REG32 DMA_THRES_RX1_5         : 8;
  __REG32 DMA_THRES_RX1_6         : 8;
  __REG32 DMA_THRES_RX1_7         : 8;
} __usbss_irqdmatholdrx11_bits;

/* USBSS IRQDMATHOLDRX12 Register */
typedef struct {
  __REG32 DMA_THRES_RX1_8         : 8;
  __REG32 DMA_THRES_RX1_9         : 8;
  __REG32 DMA_THRES_RX1_10        : 8;
  __REG32 DMA_THRES_RX1_11        : 8;
} __usbss_irqdmatholdrx12_bits;

/* USBSS IRQDMATHOLDRX13 Register */
typedef struct {
  __REG32 DMA_THRES_RX1_12        : 8;
  __REG32 DMA_THRES_RX1_13        : 8;
  __REG32 DMA_THRES_RX1_14        : 8;
  __REG32 DMA_THRES_RX1_15        : 8;
} __usbss_irqdmatholdrx13_bits;

/* USBSS IRQDMAENABLE0 Register */
typedef struct {
  __REG32 DMA_EN_TX0_1            : 1;
  __REG32                         : 1;
  __REG32 DMA_EN_TX0_2            : 1;
  __REG32                         :12;
  __REG32 DMA_EN_TX0_15           : 1;
  __REG32                         : 1;
  __REG32 DMA_EN_RX0_1            : 1;
  __REG32                         :13;
  __REG32 DMA_EN_RX0_15           : 1;
} __usbss_irqdmaenable0_bits;

/* USBSS IRQDMAENABLE1 Register */
typedef struct {
  __REG32                         : 1;
  __REG32 DMA_EN_TX1_1            : 1;
  __REG32                         :13;
  __REG32 DMA_EN_TX1_15           : 1;
  __REG32                         : 1;
  __REG32 DMA_EN_RX1_1            : 1;
  __REG32                         :13;
  __REG32 DMA_EN_RX1_15           : 1;
} __usbss_irqdmaenable1_bits;

/* USBSS IRQFRAMETHOLDTX00 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 FRAME_THRES_TX0_1       : 8;
  __REG32 FRAME_THRES_TX0_2       : 8;
  __REG32 FRAME_THRES_TX0_3       : 8;
} __usbss_irqframetholdtx00_bits;

/* USBSS IRQFRAMETHOLDTX01 Register */
typedef struct {
  __REG32 FRAME_THRES_TX0_4       : 8;
  __REG32 FRAME_THRES_TX0_5       : 8;
  __REG32 FRAME_THRES_TX0_6       : 8;
  __REG32 FRAME_THRES_TX0_7       : 8;
} __usbss_irqframetholdtx01_bits;

/* USBSS IRQFRAMETHOLDTX02 Register */
typedef struct {
  __REG32 FRAME_THRES_TX0_8       : 8;
  __REG32 FRAME_THRES_TX0_9       : 8;
  __REG32 FRAME_THRES_TX0_10      : 8;
  __REG32 FRAME_THRES_TX0_11      : 8;
} __usbss_irqframetholdtx02_bits;

/* USBSS IRQFRAMETHOLDTX03 Register */
typedef struct {
  __REG32 FRAME_THRES_TX0_12      : 8;
  __REG32 FRAME_THRES_TX0_13      : 8;
  __REG32 FRAME_THRES_TX0_14      : 8;
  __REG32 FRAME_THRES_TX0_15      : 8;
} __usbss_irqframetholdtx03_bits;

/* USBSS IRQFRAMETHOLDRX00 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 FRAME_THRES_RX0_1       : 8;
  __REG32 FRAME_THRES_RX0_2       : 8;
  __REG32 FRAME_THRES_RX0_3       : 8;
} __usbss_irqframetholdrx00_bits;

/* USBSS IRQFRAMETHOLDRX01 Register */
typedef struct {
  __REG32 FRAME_THRES_RX0_4       : 8;
  __REG32 FRAME_THRES_RX0_5       : 8;
  __REG32 FRAME_THRES_RX0_6       : 8;
  __REG32 FRAME_THRES_RX0_7       : 8;
} __usbss_irqframetholdrx01_bits;

/* USBSS IRQFRAMETHOLDRX02 Register */
typedef struct {
  __REG32 FRAME_THRES_RX0_8       : 8;
  __REG32 FRAME_THRES_RX0_9       : 8;
  __REG32 FRAME_THRES_RX0_10      : 8;
  __REG32 FRAME_THRES_RX0_11      : 8;
} __usbss_irqframetholdrx02_bits;

/* USBSS IRQFRAMETHOLDRX03 Register */
typedef struct {
  __REG32 FRAME_THRES_RX0_12      : 8;
  __REG32 FRAME_THRES_RX0_13      : 8;
  __REG32 FRAME_THRES_RX0_14      : 8;
  __REG32 FRAME_THRES_RX0_15      : 8;
} __usbss_irqframetholdrx03_bits;

/* USBSS IRQFRAMETHOLDTX10 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 FRAME_THRES_TX1_1       : 8;
  __REG32 FRAME_THRES_TX1_2       : 8;
  __REG32 FRAME_THRES_TX1_3       : 8;
} __usbss_irqframetholdtx10_bits;

/* USBSS IRQFRAMETHOLDTX11 Register */
typedef struct {
  __REG32 FRAME_THRES_TX1_4       : 8;
  __REG32 FRAME_THRES_TX1_5       : 8;
  __REG32 FRAME_THRES_TX1_6       : 8;
  __REG32 FRAME_THRES_TX1_7       : 8;
} __usbss_irqframetholdtx11_bits;

/* USBSS IRQFRAMETHOLDTX12 Register */
typedef struct {
  __REG32 FRAME_THRES_TX1_8       : 8;
  __REG32 FRAME_THRES_TX1_9       : 8;
  __REG32 FRAME_THRES_TX1_10      : 8;
  __REG32 FRAME_THRES_TX1_11      : 8;
} __usbss_irqframetholdtx12_bits;

/* USBSS IRQFRAMETHOLDTX13 Register */
typedef struct {
  __REG32 FRAME_THRES_TX1_12      : 8;
  __REG32 FRAME_THRES_TX1_13      : 8;
  __REG32 FRAME_THRES_TX1_14      : 8;
  __REG32 FRAME_THRES_TX1_15      : 8;
} __usbss_irqframetholdtx13_bits;

/* USBSS IRQFRAMETHOLDRX10 Register */
typedef struct {
  __REG32                         : 8;
  __REG32 FRAME_THRES_RX1_1       : 8;
  __REG32 FRAME_THRES_RX1_2       : 8;
  __REG32 FRAME_THRES_RX1_3       : 8;
} __usbss_irqframetholdrx10_bits;

/* USBSS IRQFRAMETHOLDRX11 Register */
typedef struct {
  __REG32 FRAME_THRES_RX1_4       : 8;
  __REG32 FRAME_THRES_RX1_5       : 8;
  __REG32 FRAME_THRES_RX1_6       : 8;
  __REG32 FRAME_THRES_RX1_7       : 8;
} __usbss_irqframetholdrx11_bits;

/* USBSS IRQFRAMETHOLDRX12 Register */
typedef struct {
  __REG32 FRAME_THRES_RX1_8       : 8;
  __REG32 FRAME_THRES_RX1_9       : 8;
  __REG32 FRAME_THRES_RX1_10      : 8;
  __REG32 FRAME_THRES_RX1_11      : 8;
} __usbss_irqframetholdrx12_bits;

/* USBSS IRQFRAMETHOLDRX13 Register */
typedef struct {
  __REG32 FRAME_THRES_RX1_12      : 8;
  __REG32 FRAME_THRES_RX1_13      : 8;
  __REG32 FRAME_THRES_RX1_14      : 8;
  __REG32 FRAME_THRES_RX1_15      : 8;
} __usbss_irqframetholdrx13_bits;

/* USBSS IRQFRAMEENABLE0 Register */
typedef struct {
  __REG32                         : 1;
  __REG32 FRAME_EN_TX0_1          : 1;
  __REG32                         :13;
  __REG32 FRAME_EN_TX0_15         : 1;
  __REG32                         : 1;
  __REG32 FRAME_EN_RX0_1          : 1;
  __REG32                         :13;
  __REG32 FRAME_EN_RX0_15         : 1;
} __usbss_irqframeenable0_bits;

/* USBSS IRQFRAMEENABLE1 Register */
typedef struct {
  __REG32                         : 1;
  __REG32 FRAME_EN_TX1_1          : 1;
  __REG32                         :13;
  __REG32 FRAME_EN_TX1_15         : 1;
  __REG32                         : 1;
  __REG32 FRAME_EN_RX1_1          : 1;
  __REG32                         :13;
  __REG32 FRAME_EN_RX1_15         : 1;
} __usbss_irqframeenable1_bits;

/* USBxREV Register */
typedef struct {
  __REG32 Y_MINOR                 : 6;
  __REG32 CUSTOM                  : 2;
  __REG32 X_MAJOR                 : 3;
  __REG32 R_RTL                   : 5;
  __REG32 FUNC                    :12;
  __REG32                         : 2;
  __REG32 SCHEME                  : 2;
} __usbrev_bits;

/* USBxCTRL Register */
typedef struct {
  __REG32 SOFT_RESET              : 1;
  __REG32 CLKFACK                 : 1;
  __REG32                         : 1;
  __REG32 UINT                    : 1;
  __REG32 RNDIS                   : 1;
  __REG32 SOFT_RESET_ISOLATION    : 1;
  __REG32                         :24;
  __REG32 DIS_SRP                 : 1;
  __REG32 DIS_DEB                 : 1;
} __usbctrl_bits;

/* USBxSTAT Register */
typedef struct {
  __REG32 DRVVBUS                 : 1;
  __REG32                         :31;
} __usbstat_bits;

/* USBxIRQMSTAT Register */
typedef struct {
  __REG32 BANK0                   : 1;
  __REG32 BANK1                   : 1;
  __REG32                         :30;
} __usbirqmstat_bits;

/* USBxIRQEOI Register */
typedef struct {
  __REG32 EOI                     : 1;
  __REG32                         :31;
} __usbirqeoi_bits;

/* USBxIRQSTATRAW0 Register */
/* USBxIRQSTAT0 Register */
/* USBxIRQENABLESET0 Register */
/* USBxIRQENABLECLR0 Register */
typedef struct {
  __REG32 TX_EP_0                 : 1;
  __REG32 TX_EP_1                 : 1;
  __REG32 TX_EP_2                 : 1;
  __REG32 TX_EP_3                 : 1;
  __REG32 TX_EP_4                 : 1;
  __REG32 TX_EP_5                 : 1;
  __REG32 TX_EP_6                 : 1;
  __REG32 TX_EP_7                 : 1;
  __REG32 TX_EP_8                 : 1;
  __REG32 TX_EP_9                 : 1;
  __REG32 TX_EP_10                : 1;
  __REG32 TX_EP_11                : 1;
  __REG32 TX_EP_12                : 1;
  __REG32 TX_EP_13                : 1;
  __REG32 TX_EP_14                : 1;
  __REG32 TX_EP_15                : 1;
  __REG32                         : 1;
  __REG32 RX_EP_1                 : 1;
  __REG32 RX_EP_2                 : 1;
  __REG32 RX_EP_3                 : 1;
  __REG32 RX_EP_4                 : 1;
  __REG32 RX_EP_5                 : 1;
  __REG32 RX_EP_6                 : 1;
  __REG32 RX_EP_7                 : 1;
  __REG32 RX_EP_8                 : 1;
  __REG32 RX_EP_9                 : 1;
  __REG32 RX_EP_10                : 1;
  __REG32 RX_EP_11                : 1;
  __REG32 RX_EP_12                : 1;
  __REG32 RX_EP_13                : 1;
  __REG32 RX_EP_14                : 1;
  __REG32 RX_EP_15                : 1;
} __usbirq0_bits;

/* USBxIRQSTATRAW1 Register */
/* USBxIRQSTAT1 Register */
/* USBxIRQENABLESET1 Register */
/* USBxIRQENABLECLR1 Register */
typedef struct {
  __REG32 USB_0                   : 1;
  __REG32 USB_1                   : 1;
  __REG32 USB_2                   : 1;
  __REG32 USB_3                   : 1;
  __REG32 USB_4                   : 1;
  __REG32 USB_5                   : 1;
  __REG32 USB_6                   : 1;
  __REG32 USB_7                   : 1;
  __REG32 USB_8                   : 1;
  __REG32 USB_9                   : 1;
  __REG32                         : 6;
  __REG32 TX_FIFO_0               : 1;
  __REG32 TX_FIFO_1               : 1;
  __REG32 TX_FIFO_2               : 1;
  __REG32 TX_FIFO_3               : 1;
  __REG32 TX_FIFO_4               : 1;
  __REG32 TX_FIFO_5               : 1;
  __REG32 TX_FIFO_6               : 1;
  __REG32 TX_FIFO_7               : 1;
  __REG32 TX_FIFO_8               : 1;
  __REG32 TX_FIFO_9               : 1;
  __REG32 TX_FIFO_10              : 1;
  __REG32 TX_FIFO_11              : 1;
  __REG32 TX_FIFO_12              : 1;
  __REG32 TX_FIFO_13              : 1;
  __REG32 TX_FIFO_14              : 1;
  __REG32 TX_FIFO_15              : 1;
} __usbirq1_bits;

#define USBIRQ0_TX_EP_0           (0x1UL<<0)
#define USBIRQ0_TX_EP_1           (0x1UL<<1)
#define USBIRQ0_TX_EP_2           (0x1UL<<2)
#define USBIRQ0_TX_EP_3           (0x1UL<<3)
#define USBIRQ0_TX_EP_4           (0x1UL<<4)
#define USBIRQ0_TX_EP_5           (0x1UL<<5)
#define USBIRQ0_TX_EP_6           (0x1UL<<6)
#define USBIRQ0_TX_EP_7           (0x1UL<<7)
#define USBIRQ0_TX_EP_8           (0x1UL<<8)
#define USBIRQ0_TX_EP_9           (0x1UL<<9)
#define USBIRQ0_TX_EP_10          (0x1UL<<10)
#define USBIRQ0_TX_EP_11          (0x1UL<<11)
#define USBIRQ0_TX_EP_12          (0x1UL<<12)
#define USBIRQ0_TX_EP_13          (0x1UL<<13)
#define USBIRQ0_TX_EP_14          (0x1UL<<14)
#define USBIRQ0_TX_EP_15          (0x1UL<<15)
#define USBIRQ0_RX_EP_1           (0x1UL<<17)
#define USBIRQ0_RX_EP_2           (0x1UL<<18)
#define USBIRQ0_RX_EP_3           (0x1UL<<19)
#define USBIRQ0_RX_EP_4           (0x1UL<<20)
#define USBIRQ0_RX_EP_5           (0x1UL<<21)
#define USBIRQ0_RX_EP_6           (0x1UL<<22)
#define USBIRQ0_RX_EP_7           (0x1UL<<23)
#define USBIRQ0_RX_EP_8           (0x1UL<<24)
#define USBIRQ0_RX_EP_9           (0x1UL<<25)
#define USBIRQ0_RX_EP_10          (0x1UL<<26)
#define USBIRQ0_RX_EP_11          (0x1UL<<27)
#define USBIRQ0_RX_EP_12          (0x1UL<<28)
#define USBIRQ0_RX_EP_13          (0x1UL<<29)
#define USBIRQ0_RX_EP_14          (0x1UL<<30)
#define USBIRQ0_RX_EP_15          (0x1UL<<31)

#define USBIRQ1_USB_0             (0x1UL<<0)
#define USBIRQ1_USB_1             (0x1UL<<1)
#define USBIRQ1_USB_2             (0x1UL<<2)
#define USBIRQ1_USB_3             (0x1UL<<3)
#define USBIRQ1_USB_4             (0x1UL<<4)
#define USBIRQ1_USB_5             (0x1UL<<5)
#define USBIRQ1_USB_6             (0x1UL<<6)
#define USBIRQ1_USB_7             (0x1UL<<7)
#define USBIRQ1_USB_8             (0x1UL<<8)
#define USBIRQ1_USB_9             (0x1UL<<9)
#define USBIRQ0_TX_FIFO_0         (0x1UL<<16)
#define USBIRQ0_TX_FIFO_1         (0x1UL<<17)
#define USBIRQ0_TX_FIFO_2         (0x1UL<<18)
#define USBIRQ0_TX_FIFO_3         (0x1UL<<19)
#define USBIRQ0_TX_FIFO_4         (0x1UL<<20)
#define USBIRQ0_TX_FIFO_5         (0x1UL<<21)
#define USBIRQ0_TX_FIFO_6         (0x1UL<<22)
#define USBIRQ0_TX_FIFO_7         (0x1UL<<23)
#define USBIRQ0_TX_FIFO_8         (0x1UL<<24)
#define USBIRQ0_TX_FIFO_9         (0x1UL<<25)
#define USBIRQ0_TX_FIFO_10        (0x1UL<<26)
#define USBIRQ0_TX_FIFO_11        (0x1UL<<27)
#define USBIRQ0_TX_FIFO_12        (0x1UL<<28)
#define USBIRQ0_TX_FIFO_13        (0x1UL<<29)
#define USBIRQ0_TX_FIFO_14        (0x1UL<<30)
#define USBIRQ0_TX_FIFO_15        (0x1UL<<31)

/* USBxTXMODE Register */
typedef struct {
  __REG32 TX1_MODE                : 2;
  __REG32 TX2_MODE                : 2;
  __REG32 TX3_MODE                : 2;
  __REG32 TX4_MODE                : 2;
  __REG32 TX5_MODE                : 2;
  __REG32 TX6_MODE                : 2;
  __REG32 TX7_MODE                : 2;
  __REG32 TX8_MODE                : 2;
  __REG32 TX9_MODE                : 2;
  __REG32 TX10_MODE               : 2;
  __REG32 TX11_MODE               : 2;
  __REG32 TX12_MODE               : 2;
  __REG32 TX13_MODE               : 2;
  __REG32 TX14_MODE               : 2;
  __REG32 TX15_MODE               : 2;
  __REG32                         : 2;
} __usbtxmode_bits;

/* USBxRXMODE Register */
typedef struct {
  __REG32 RX1_MODE                : 2;
  __REG32 RX2_MODE                : 2;
  __REG32 RX3_MODE                : 2;
  __REG32 RX4_MODE                : 2;
  __REG32 RX5_MODE                : 2;
  __REG32 RX6_MODE                : 2;
  __REG32 RX7_MODE                : 2;
  __REG32 RX8_MODE                : 2;
  __REG32 RX9_MODE                : 2;
  __REG32 RX10_MODE               : 2;
  __REG32 RX11_MODE               : 2;
  __REG32 RX12_MODE               : 2;
  __REG32 RX13_MODE               : 2;
  __REG32 RX14_MODE               : 2;
  __REG32 RX15_MODE               : 2;
  __REG32                         : 2;
} __usbrxmode_bits;

/* USBxGENRNDISEP1 Register */
typedef struct {
  __REG32 EP1_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep1_bits;

/* USBxGENRNDISEP2 Register */
typedef struct {
  __REG32 EP2_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep2_bits;

/* USBxGENRNDISEP3 Register */
typedef struct {
  __REG32 EP3_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep3_bits;

/* USBxGENRNDISEP4 Register */
typedef struct {
  __REG32 EP4_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep4_bits;

/* USBxGENRNDISEP5 Register */
typedef struct {
  __REG32 EP5_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep5_bits;

/* USBxGENRNDISEP6 Register */
typedef struct {
  __REG32 EP6_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep6_bits;

/* USBxGENRNDISEP7 Register */
typedef struct {
  __REG32 EP7_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep7_bits;

/* USBxGENRNDISEP8 Register */
typedef struct {
  __REG32 EP8_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep8_bits;

/* USBxGENRNDISEP9 Register */
typedef struct {
  __REG32 EP9_SIZE                :16;
  __REG32                         :16;
} __usbgenrndisep9_bits;

/* USBxGENRNDISEP10 Register */
typedef struct {
  __REG32 EP10_SIZE               :16;
  __REG32                         :16;
} __usbgenrndisep10_bits;

/* USBxGENRNDISEP11 Register */
typedef struct {
  __REG32 EP11_SIZE               :16;
  __REG32                         :16;
} __usbgenrndisep11_bits;

/* USBxGENRNDISEP12 Register */
typedef struct {
  __REG32 EP12_SIZE               :16;
  __REG32                         :16;
} __usbgenrndisep12_bits;

/* USBxGENRNDISEP13 Register */
typedef struct {
  __REG32 EP13_SIZE               :16;
  __REG32                         :16;
} __usbgenrndisep13_bits;

/* USBxGENRNDISEP14 Register */
typedef struct {
  __REG32 EP14_SIZE               :16;
  __REG32                         :16;
} __usbgenrndisep14_bits;

/* USBxGENRNDISEP15 Register */
typedef struct {
  __REG32 EP15_SIZE               :16;
  __REG32                         :16;
} __usbgenrndisep15_bits;

/* USBxAUTOREQ Register */
typedef struct {
  __REG32 RX1_AUTOREQ             : 2;
  __REG32 RX2_AUTOREQ             : 2;
  __REG32 RX3_AUTOREQ             : 2;
  __REG32 RX4_AUTOREQ             : 2;
  __REG32 RX5_AUTOREQ             : 2;
  __REG32 RX6_AUTOREQ             : 2;
  __REG32 RX7_AUTOREQ             : 2;
  __REG32 RX8_AUTOREQ             : 2;
  __REG32 RX9_AUTOREQ             : 2;
  __REG32 RX10_AUTOREQ            : 2;
  __REG32 RX11_AUTOREQ            : 2;
  __REG32 RX12_AUTOREQ            : 2;
  __REG32 RX13_AUTOREQ            : 2;
  __REG32 RX14_AUTOREQ            : 2;
  __REG32 RX15_AUTOREQ            : 2;
  __REG32                         : 2;
} __usbautoreq_bits;

/* USBx_TDOWN Register */
typedef struct {
  __REG32                         : 1;
  __REG32 RX1_TDOWN               : 1;
  __REG32 RX2_TDOWN               : 1;
  __REG32 RX3_TDOWN               : 1;
  __REG32 RX4_TDOWN               : 1;
  __REG32 RX5_TDOWN               : 1;
  __REG32 RX6_TDOWN               : 1;
  __REG32 RX7_TDOWN               : 1;
  __REG32 RX8_TDOWN               : 1;
  __REG32 RX9_TDOWN               : 1;
  __REG32 RX10_TDOWN              : 1;
  __REG32 RX11_TDOWN              : 1;
  __REG32 RX12_TDOWN              : 1;
  __REG32 RX13_TDOWN              : 1;
  __REG32 RX14_TDOWN              : 1;
  __REG32 RX15_TDOWN              : 1;
  __REG32                         : 1;
  __REG32 TX1_TDOWN               : 1;
  __REG32 TX2_TDOWN               : 1;
  __REG32 TX3_TDOWN               : 1;
  __REG32 TX4_TDOWN               : 1;
  __REG32 TX5_TDOWN               : 1;
  __REG32 TX6_TDOWN               : 1;
  __REG32 TX7_TDOWN               : 1;
  __REG32 TX8_TDOWN               : 1;
  __REG32 TX9_TDOWN               : 1;
  __REG32 TX10_TDOWN              : 1;
  __REG32 TX11_TDOWN              : 1;
  __REG32 TX12_TDOWN              : 1;
  __REG32 TX13_TDOWN              : 1;
  __REG32 TX14_TDOWN              : 1;
  __REG32 TX15_TDOWN              : 1;
} __usbtdown_bits;

/* USBxUTMI Register */
typedef struct {
  __REG32 FSSE0EXT                : 1;
  __REG32 FSDATAEXT               : 1;
  __REG32 WORDINTERFACE           : 1;
  __REG32                         : 5;
  __REG32 DATAINH                 : 8;
  __REG32 TXVALIDH                : 1;
  __REG32 FSXCVROWNER             : 1;
  __REG32 TXENABLEN               : 1;
  __REG32 VBUSVLDEXT              : 1;
  __REG32 VBUSVLDEXTSEL           : 1;
  __REG32 OTGDISABLE              : 1;
  __REG32 TXBITSTUFFENH           : 1;
  __REG32 TXBITSTUFFEN            : 1;
  __REG32                         : 8;
} __usbutmi_bits;

/* USBxMGCUTMILB Register */
typedef struct {
  __REG32                         : 2;
  __REG32 LINESTATE               : 2;
  __REG32                         : 2;
  __REG32 RXERROR                 : 1;
  __REG32 VBUSVALID               : 1;
  __REG32 AVALID                  : 1;
  __REG32 SESSEND                 : 1;
  __REG32 HOSTDISCON              : 1;
  __REG32 IDDIG                   : 1;
  __REG32                         : 4;
  __REG32 IDPULLUP                : 1;
  __REG32 DMPULLDOWN              : 1;
  __REG32 DPPULLDOWN              : 1;
  __REG32 DISCHRGVBUS             : 1;
  __REG32 CHRGVBUS                : 1;
  __REG32 DRVVBUS                 : 1;
  __REG32 TERMSEL                 : 1;
  __REG32 XCVRSEL                 : 2;
  __REG32 TXVALID                 : 1;
  __REG32 OPMODE                  : 2;
  __REG32 SUSPENDM                : 1;
  __REG32                         : 3;
} __usbutmilb_bits;

/* USBxMODE Register */
typedef struct {
  __REG32 LOOPBACK                : 1;
  __REG32 PHY_TEST                : 1;
  __REG32                         : 6;
  __REG32 IDDIG                   : 1;
  __REG32                         :23;
} __usbmode_bits;

/* USB_PHY Termination_control Register */
typedef struct {
  __REG32 RTERM_CAL               : 7;
  __REG32 USE_RTERM_CAL_REG       : 1;
  __REG32 DISABLE_TEMP_TRACK      : 1;
  __REG32 RESTART_RTERM_CAL       : 1;
  __REG32 RTERM_COMP_OUT          : 1;
  __REG32 HS_CODE_SEL             : 3;
  __REG32 RTERM_RMX               : 7;
  __REG32 USE_RTERM_RMX_REG       : 1;
  __REG32                         : 2;
  __REG32 FS_CODE_SEL             : 4;
  __REG32 RTERM_CAL_DONE          : 1;
  __REG32 ALWAYS_UPDATE           : 1;
  __REG32                         : 2;
} __usb_phy_term_ctrl_bits;

/* USB_PHY RX_CALIB Register */
typedef struct {
  __REG32 RESTART_SQ_CAL          : 1;
  __REG32 SQ_CAL_DONE             : 1;
  __REG32 SQ_COMP_OUT             : 1;
  __REG32 SQ_OFF_CODE_DAC3        : 5;
  __REG32 USE_SQ_OFF_DAC3         : 1;
  __REG32 SQ_OFF_CODE_DAC2        : 5;
  __REG32 USE_SQ_OFF_DAC2         : 1;
  __REG32 SQ_OFF_CODE_DAC1        : 6;
  __REG32 USE_SQ_OFF_DAC1         : 1;
  __REG32 HSRX_CAL_DONE           : 1;
  __REG32 HSRX_COMP_OUT           : 1;
  __REG32 HS_OFF_CODE             : 6;
  __REG32 USE_HS_OFF_REG          : 1;
  __REG32 RESTART_HSRX_CAL        : 1;
} __usb_phy_rx_calib_bits;

/* USB_PHY DLLHS_2 Register */
typedef struct {
  __REG32 LINESTATE_DEBOUNCE_CNTL : 4;
  __REG32 LINESTATE_DEBOUNCE_EN   : 1;
  __REG32                         :11;
  __REG32 DLLHS_STATUS_LDO        : 8;
  __REG32 DLLHS_CNTRL_LDO         : 8;
} __usb_phy_dllhs_2_bits;

/* USB_PHY RX_TEST_2 Register */
typedef struct {
  __REG32 CDR_TESTOUT             : 8;
  __REG32                         : 3;
  __REG32 INTDATAOUTREG           :16;
  __REG32 USEINTDATAOUT           : 1;
  __REG32 RXPIDERR                : 1;
  __REG32 PHYCLKOUTINVERSION      : 1;
  __REG32 HSOSBITINVERSION        : 1;
  __REG32 HSOSREVERSAL            : 1;
} __usb_phy_rx_test_2_bits;

/* USB_PHY CHRG_DET Register */
typedef struct {
  __REG32 FOR_CE                  : 1;
  __REG32 CHG_DET_VCTRL           : 2;
  __REG32 CHG_DET_ICTRL           : 2;
  __REG32                         : 2;
  __REG32 CHG_DET_TIMER           : 6;
  __REG32 CHG_DET_OSC_CNTRL       : 2;
  __REG32 COMP_DM                 : 1;
  __REG32 COMP_DP                 : 1;
  __REG32 CHG_VSRC_EN             : 1;
  __REG32 CHG_ISINK_EN            : 1;
  __REG32                         : 2;
  __REG32 DATA_DET                : 1;
  __REG32 CHG_DETECTED            : 1;
  __REG32 CHG_DET_DONE            : 1;
  __REG32 RESTART_CHG_DET         : 1;
  __REG32 CHG_DET_EXT_CTL         : 1;
  __REG32 SINK_ON_DP              : 1;
  __REG32 SRC_ON_DM               : 1;
  __REG32 DIS_CHG_DET             : 1;
  __REG32 USE_CHG_DET_REG         : 1;
  __REG32                         : 2;
} __usb_phy_chrg_det_bits;

/* USB_PHY PWR_CNTL Register */
typedef struct {
  __REG32                         : 1;
  __REG32 PD                      : 1;
  __REG32 USE_PD_REG              : 1;
  __REG32 DATAPOLARITYN           : 1;
  __REG32 USE_DATAPOLARITYN_REG   : 1;
  __REG32 USEPLLLOCK              : 1;
  __REG32 PLLLOCK                 : 1;
  __REG32                         : 2;
  __REG32 FORCEPLLON              : 1;
  __REG32 FORCELDOON              : 1;
  __REG32 FORCEPLLSLOWCLK         : 1;
  __REG32 LDOPWRCOUNTER           :15;
  __REG32 RESETDONE_CHGDET        : 1;
  __REG32 RESETDONEMCLK           : 1;
  __REG32 VMAIN_GLOBAL_RESET_DONE : 1;
  __REG32 RESET_DONE_VMAIN        : 1;
  __REG32 RESETDONETCLK           : 1;
} __usb_phy_pwr_cntl_bits;

/* USB_PHY UTMI_INTERFACE_CNTL_1 Register */
typedef struct {
  __REG32 TXVALIDH                : 1;
  __REG32 TXVALID                 : 1;
  __REG32 USETXVALIDREG           : 1;
  __REG32 XCVRSEL                 : 2;
  __REG32 OVERRIDEXCVRSEL         : 1;
  __REG32 UTMIRESET               : 1;
  __REG32 SUSPENDM                : 1;
  __REG32 OVERRIDESUSRESET        : 1;
  __REG32 OPMODE                  : 2;
  __REG32 USEOPMODEREG            : 1;
  __REG32 DATABUS16OR8            : 1;
  __REG32 USEDATABUSREG           : 1;
  __REG32                         : 1;
  __REG32 UTMIDATAIN              :16;
  __REG32 USEUTMIDATAREG          : 1;
} __usb_phy_utmi_interface_cntl_1_bits;

/* USB_PHY UTMI_INTERFACE_CNTL_2 Register */
typedef struct {
  __REG32 SIG_BYPASS_SUSPENDMPULSE_INCR : 1;
  __REG32                               : 4;
  __REG32 TXENABLEN                     : 1;
  __REG32 FSLSSERIALMODE                : 1;
  __REG32 TXDAT                         : 1;
  __REG32 TXSE0                         : 1;
  __REG32 USEREGSERIALMODE              : 1;
  __REG32                               : 3;
  __REG32 DMPULLDOWN                    : 1;
  __REG32 DPPULLDOWN                    : 1;
  __REG32 TERMSEL                       : 1;
  __REG32 USETERMCONTROLREG             : 1;
  __REG32 TXBITSTUFFENABLEH             : 1;
  __REG32 TXBITSTUFFENABLE              : 1;
  __REG32 USEBITSTUFFREG                : 1;
  __REG32 UTMIRESETDONE                 : 1;
  __REG32 TXREADY                       : 1;
  __REG32 RXERROR                       : 1;
  __REG32 RXACTIVE                      : 1;
  __REG32 RXVALIDH                      : 1;
  __REG32 RXVALID                       : 1;
  __REG32 LINESTATE                     : 2;
  __REG32 HOSTDISCONNECT                : 1;
  __REG32 RXDM                          : 1;
  __REG32 RXDP                          : 1;
  __REG32 RXRCV                         : 1;
} __usb_phy_utmi_interface_cntl_2_bits;

/* USB_PHY BIST Register */
typedef struct {
  __REG32 USE_BIST_TX_PHASES            : 1;
  __REG32 HS_ALL_ONES_TEST              : 1;
  __REG32 INTER_PKT_DELAY_TEST          : 1;
  __REG32                               : 1;
  __REG32 RX_TEST_MODE                  : 1;
  __REG32 OP_CODE                       : 2;
  __REG32                               : 3;
  __REG32 BIST_BUSY                     : 1;
  __REG32 BIST_PASS                     : 1;
  __REG32 SWEEP_MODE                    : 3;
  __REG32 SWEEP_EN                      : 1;
  __REG32 BIST_OP_PHASE_SEL             : 3;
  __REG32 LOOPBACK_EN                   : 1;
  __REG32 BIST_PKT_LENGTH               : 9;
  __REG32 BIST_CRC_CALC_EN              : 1;
  __REG32 REDUCED_SWING                 : 1;
  __REG32 BIST_START                    : 1;
} __usb_phy_bist_bits;

/* USB_PHY CDR_BIST2 Register */
typedef struct {
  __REG32 BIST_END_ADDR                 : 6;
  __REG32 BIST_START_ADDR               : 6;
  __REG32                               :13;
  __REG32 NUM_DECISIONS                 : 3;
  __REG32 CDR_EXE_MODE                  : 3;
  __REG32 CDR_EXE_EN                    : 1;
} __usb_phy_cdr_bist2_bits;

/* USB_PHY GPIO Register */
typedef struct {
  __REG32                               :18;
  __REG32 DPGPIOPIPD                    : 1;
  __REG32 DMGPIOPIPD                    : 1;
  __REG32 GPIOCONFIG                    : 3;
  __REG32 GPIO1P8VCONFIG                : 1;
  __REG32 DMGPIOY                       : 1;
  __REG32 DPGPIOY                       : 1;
  __REG32 DMGPIOA                       : 1;
  __REG32 DPGPIOA                       : 1;
  __REG32 DMGPIOGZ                      : 1;
  __REG32 DPGPIOGZ                      : 1;
  __REG32 GPIOMODE                      : 1;
  __REG32 USEGPIOMODEREG                : 1;
} __usb_phy_gpio_bits;

/* USB_PHY DLLHS Register */
typedef struct {
  __REG32                               : 1;
  __REG32 DLL_FREEZE                    : 1;
  __REG32 DLL_IDLE                      : 1;
  __REG32 DLL_CDR_MODE                  : 1;
  __REG32 DLL_FILT                      : 2;
  __REG32 DLL_RATE                      : 2;
  __REG32 FORCE_DLL_CODE                : 1;
  __REG32 DLL_FORCED_CODE               : 6;
  __REG32 DLL_PHS0_8                    : 1;
  __REG32 DLL_SEL_COD                   : 3;
  __REG32 DLL_LOCKCHK                   : 2;
  __REG32 DLL_SEL_CODE_PHS              : 1;
  __REG32 DLLHS_GENERATED_CODE          : 6;
  __REG32 DLLHS_LOCK                    : 1;
  __REG32                               : 3;
} __usb_phy_dllhs_bits;

/* USB_PHY CM_TRIM Register */
typedef struct {
  __REG32                               : 4;
  __REG32 NWELLTRIM_CODE                : 3;
  __REG32 USE_NWELLTRIM_REG             : 1;
  __REG32 SWTRIM                        : 7;
  __REG32 USE_SW_TRIM                   : 1;
  __REG32 BGTRIM                        :15;
  __REG32 USEBGTRIM                     : 1;
} __usb_phy_cm_trim_bits;

/* USB_PHY CM_CONFIG Register */
typedef struct {
  __REG32 LDOSTATUS                     : 2;
  __REG32 LDOCONFIG                     :16;
  __REG32 CMSTATUS                      : 6;
  __REG32 CONFIGURECM                   : 8;
} __usb_phy_cm_config_bits;

/* USB_PHY USBOTG Register */
typedef struct {
  __REG32                               : 6;
  __REG32 TESTOTGSTATUS                 :10;
  __REG32 TESTOTGCONFIG                 :16;
} __usb_phy_usbotg_bits;

/* USB_PHY AD_INTERFACE_REG1 Register */
typedef struct {
  __REG32 SQUELCH                       : 1;
  __REG32 LSFS_RX_DATA                  : 1;
  __REG32 LSHOSTMODE                    : 1;
  __REG32 USE_LSHOST_REG                : 1;
  __REG32 DISCON_DETECT                 : 1;
  __REG32 DLL_RX_DATA                   : 1;
  __REG32 RTERM_CAL_EN                  : 1;
  __REG32 TEST_RTERM_CAL_CONTROL        : 1;
  __REG32 SQ_CAL_EN2                    : 1;
  __REG32 SQ_CAL_EN1                    : 1;
  __REG32 SQ_CAL_EN3                    : 1;
  __REG32 TEST_SQ_CAL_CONTROL           : 1;
  __REG32                               : 1;
  __REG32 FSTX_PRE_EN                   : 1;
  __REG32 FSTX_GZ                       : 1;
  __REG32 TEST_FS_MODE                  : 1;
  __REG32 HS_CHIRP                      : 1;
  __REG32 HS_HV_SW                      : 1;
  __REG32 TEST_HS_MODE                  : 1;
  __REG32 HS_RX_EN                      : 1;
  __REG32 SQ_EN                         : 1;
  __REG32                               : 1;
  __REG32 FS_RX_EN                      : 1;
  __REG32 HS_TX_EN                      : 1;
  __REG32 TEST_EN_CNTRL                 : 1;
  __REG32 HS_RX_PRE_EN                  : 1;
  __REG32 HS_TX_PRE_EN                  : 1;
  __REG32 SQ_PRE_EN                     : 1;
  __REG32 TEST_PRE_EN_CNTRL             : 1;
  __REG32 FS_TX_DATA                    : 1;
  __REG32 HS_TX_DATA                    : 1;
  __REG32 USE_AD_DATA_REG               : 1;
} __usb_phy_ad_interface_reg1_bits;

/* USB_PHY AD_INTERFACE_REG2 Register */
typedef struct {
  __REG32 PORZ                          : 1;
  __REG32 SPARE_IN_CORE                 : 5;
  __REG32 DP_DM_5V_SHORT                : 1;
  __REG32 DM_PULLDOWN_EN_CORE           : 1;
  __REG32 DP_PULLDOWN_EN_CORE           : 1;
  __REG32 RPU_DM_SW2_EN_CORE            : 1;
  __REG32 RPU_DM_SW1_EN_CORE            : 1;
  __REG32 RPU_DP_SW2_EN_CORE            : 1;
  __REG32 RPU_DP_SW1_EN_CORE            : 1;
  __REG32 USE_RPU_RPD_REG               : 1;
  __REG32 HSRX_CAL_EN                   : 1;
  __REG32 USE_HSRX_CAL_EN_REG           : 1;
  __REG32 SERX_DM_CORE                  : 1;
  __REG32 SERX_DP_CORE                  : 1;
  __REG32 SPARE_OUT_CORE                : 5;
  __REG32                               : 1;
  __REG32 DISCON_PRE_EN                 : 1;
  __REG32 DISCON_EN                     : 1;
  __REG32 USE_DISCON_REG                : 1;
  __REG32 SUS_DRV_DM_EN                 : 1;
  __REG32 SUS_DRV_DM_DATA               : 1;
  __REG32 SUS_DRV_DP_EN                 : 1;
  __REG32 SUS_DRV_DP_DATA               : 1;
  __REG32 USE_SUSP_DRV_REG              : 1;
} __usb_phy_ad_interface_reg2_bits;

/* USB_PHY AD_INTERFACE_REG3 Register */
typedef struct {
  __REG32 FARCORE                       : 1;
  __REG32 USE_FARCORE_REG               : 1;
  __REG32 SPARE_OUT_LDO                 : 8;
  __REG32 SPARE_IN_LDO                  : 8;
  __REG32 HS_TERM_RES                   : 1;
  __REG32 USE_HS_TERM_RES_REG           : 1;
  __REG32 FSTX_SE0                      : 1;
  __REG32 FSTX_MODE                     : 1;
  __REG32 USE_FS_REG3                   : 1;
  __REG32 HSOS_DATA                     : 8;
  __REG32 USE_HSOS_DATA_REG             : 1;
} __usb_phy_ad_interface_reg3_bits;

/* USB_PHY ANA_CONFIG1 Register */
typedef struct {
  __REG32 PROTECT_TEST                  : 5;
  __REG32 HSFSTX_TEST                   : 7;
  __REG32 HS_PRE_EMP_CNTRL              : 2;
  __REG32 FS_SLEW                       : 3;
  __REG32 SQ_CTRL_REG                   :15;
} __usb_phy_ana_config1_bits;

/* USB_PHY ANA_CONFIG2 Register */
typedef struct {
  __REG32 SQ_LPMODEZ                    : 1;
  __REG32 SERX_HYST_CNTRL               : 2;
  __REG32 SERX_TEST                     : 3;
  __REG32 HSRX_TEST                     : 5;
  __REG32 DISCON_TEST                   : 4;
  __REG32 FSRX_TEST                     : 5;
  __REG32 REF_GEN_TEST                  : 7;
  __REG32 RTERM_CAL_TEST                : 5;
} __usb_phy_ana_config2_bits;

/* CPPIDMA DMAREVID Register */
typedef struct {
  __REG32 REVMIN                        : 8;
  __REG32 REVMAJ                        : 3;
  __REG32 REVRTL                        : 5;
  __REG32 MODID                         :14;
  __REG32                               : 2;
} __cppi_dma_dmarevid_bits;

/* CPPIDMA TDFDQ Register */
typedef struct {
  __REG32 TD_DESC_QNUM                  :12;
  __REG32 TD_DESC_QMGR                  : 2;
  __REG32                               :18;
} __cppi_dma_tdfdq_bits;

/* CPPIDMA DMAEMU Register */
typedef struct {
  __REG32 FREE                          : 1;
  __REG32 SOFT                          : 1;
  __REG32                               :30;
} __cppi_dma_dmaemu_bits;

/* CPPIDMA TXGCRx Register */
typedef struct {
  __REG32 TX_DEFAULT_QNUM               :12;
  __REG32 TX_DEFAULT_QMGR               : 2;
  __REG32                               :16;
  __REG32 TX_TEARDOWN                   : 1;
  __REG32 TX_ENABLE                     : 1;
} __cppi_dma_txgcr_bits;

/* CPPIDMA RXGCRx Register */
typedef struct {
  __REG32 RX_DEFAULT_RQ_QNUM            :12;
  __REG32 RX_DEFAULT_RQ_QMGR            : 2;
  __REG32 RX_DEFAULT_DESC_TYPE          : 2;
  __REG32 RX_SOP_OFFSET                 : 8;
  __REG32 RX_ERROR_HANDLING             : 1;
  __REG32                               : 4;
  __REG32 RX_PAUSE                      : 1;
  __REG32 RX_TEARDOWN                   : 1;
  __REG32 RX_ENABLE                     : 1;
} __cppi_dma_rxgcr_bits;

#define CPPIDMA_RXHPCRA_RX_HOST_FDQ0_QNUM       (0xFFFUL<<0)
#define CPPIDMA_RXHPCRA_RX_HOST_FDQ0_QMGR       (0x3UL<<12)
#define CPPIDMA_RXHPCRA_RX_HOST_FDQ1_QNUM       (0xFFFUL<<16)
#define CPPIDMA_RXHPCRA_RX_HOST_FDQ1_QMGR       (0x3UL<<28)

#define CPPIDMA_RXHPCRA_RX_HOST_FDQ2_QNUM       (0xFFFUL<<0)
#define CPPIDMA_RXHPCRA_RX_HOST_FDQ2_QMGR       (0x3UL<<12)
#define CPPIDMA_RXHPCRA_RX_HOST_FDQ3_QNUM       (0xFFFUL<<16)
#define CPPIDMA_RXHPCRA_RX_HOST_FDQ3_QMGR       (0x3UL<<28)

/* CPPI DMA_SCHED_CTRL Register */
typedef struct {
  __REG32 LAST_ENTRY                    : 8;
  __REG32                               :23;
  __REG32 ENABLE                        : 1;
} __cppi_dma_sched_ctrl_bits;

#define CPPI_DMA_SCHED_WORD_ENTRY0_RXTX         (0x1UL<<7)
#define CPPI_DMA_SCHED_WORD_ENTRY1_RXTX         (0x1UL<<15)
#define CPPI_DMA_SCHED_WORD_ENTRY2_RXTX         (0x1UL<<23)
#define CPPI_DMA_SCHED_WORD_ENTRY3_RXTX         (0x1UL<<31)

/* USB QMGR REVID Register */
typedef struct {
  __REG32 REVMIN                        : 6;
  __REG32 REVCUSTOM                     : 2;
  __REG32 REVMAJ                        : 3;
  __REG32 REVRTL                        : 5;
  __REG32 FUNCTION                      :12;
  __REG32                               : 2;
  __REG32 SCHEME                        : 2;
} __usb_qmgr_revid_bits;

/* USB QMGR RST Register */
#define USB_QMGR_RST_SOURCE_QNUM        (0x3FFFUL<<0)
#define USB_QMGR_RST_DEST_QNUM          (0x3FFFUL<<16)
#define USB_QMGR_RST_HEAD_TAIL          (0x1UL<<31)

/* USB QMGR FDBSC0 Register */
typedef struct {
  __REG32 FDBQ0_STARVE_CNT              : 8;
  __REG32 FDBQ1_STARVE_CNT              : 8;
  __REG32 FDBQ2_STARVE_CNT              : 8;
  __REG32 FDBQ3_STARVE_CNT              : 8;
} __usb_qmgr_fdbsc0_bits;

/* USB QMGR FDBSC1 Register */
typedef struct {
  __REG32 FDBQ4_STARVE_CNT              : 8;
  __REG32 FDBQ5_STARVE_CNT              : 8;
  __REG32 FDBQ6_STARVE_CNT              : 8;
  __REG32 FDBQ7_STARVE_CNT              : 8;
} __usb_qmgr_fdbsc1_bits;

/* USB QMGR FDBSC2 Register */
typedef struct {
  __REG32 FDBQ8_STARVE_CNT              : 8;
  __REG32 FDBQ9_STARVE_CNT              : 8;
  __REG32 FDBQ10_STARVE_CNT             : 8;
  __REG32 FDBQ11_STARVE_CNT             : 8;
} __usb_qmgr_fdbsc2_bits;

/* USB QMGR FDBSC3 Register */
typedef struct {
  __REG32 FDBQ12_STARVE_CNT             : 8;
  __REG32 FDBQ13_STARVE_CNT             : 8;
  __REG32 FDBQ14_STARVE_CNT             : 8;
  __REG32 FDBQ15_STARVE_CNT             : 8;
} __usb_qmgr_fdbsc3_bits;

/* USB QMGR FDBSC4 Register */
typedef struct {
  __REG32 FDBQ16_STARVE_CNT             : 8;
  __REG32 FDBQ17_STARVE_CNT             : 8;
  __REG32 FDBQ18_STARVE_CNT             : 8;
  __REG32 FDBQ19_STARVE_CNT             : 8;
} __usb_qmgr_fdbsc4_bits;

/* USB QMGR FDBSC5 Register */
typedef struct {
  __REG32 FDBQ20_STARVE_CNT             : 8;
  __REG32 FDBQ21_STARVE_CNT             : 8;
  __REG32 FDBQ22_STARVE_CNT             : 8;
  __REG32 FDBQ23_STARVE_CNT             : 8;
} __usb_qmgr_fdbsc5_bits;

/* USB QMGR FDBSC6 Register */
typedef struct {
  __REG32 FDBQ24_STARVE_CNT             : 8;
  __REG32 FDBQ25_STARVE_CNT             : 8;
  __REG32 FDBQ26_STARVE_CNT             : 8;
  __REG32 FDBQ27_STARVE_CNT             : 8;
} __usb_qmgr_fdbsc6_bits;

/* USB QMGR FDBSC7 Register */
typedef struct {
  __REG32 FDBQ28_STARVE_CNT             : 8;
  __REG32 FDBQ29_STARVE_CNT             : 8;
  __REG32 FDBQ30_STARVE_CNT             : 8;
  __REG32 FDBQ31_STARVE_CNT             : 8;
} __usb_qmgr_fdbsc7_bits;

/* USB QMGR LRAM0SIZE Register */
typedef struct {
  __REG32 REGION0_SIZE                  :14;
  __REG32                               :18;
} __usb_qmgr_lramsize_bits;

/* USB QMGR PEND0 Register */
typedef struct {
  __REG32 QPEND0                        : 1;
  __REG32 QPEND1                        : 1;
  __REG32 QPEND2                        : 1;
  __REG32 QPEND3                        : 1;
  __REG32 QPEND4                        : 1;
  __REG32 QPEND5                        : 1;
  __REG32 QPEND6                        : 1;
  __REG32 QPEND7                        : 1;
  __REG32 QPEND8                        : 1;
  __REG32 QPEND9                        : 1;
  __REG32 QPEND10                       : 1;
  __REG32 QPEND11                       : 1;
  __REG32 QPEND12                       : 1;
  __REG32 QPEND13                       : 1;
  __REG32 QPEND14                       : 1;
  __REG32 QPEND15                       : 1;
  __REG32 QPEND16                       : 1;
  __REG32 QPEND17                       : 1;
  __REG32 QPEND18                       : 1;
  __REG32 QPEND19                       : 1;
  __REG32 QPEND20                       : 1;
  __REG32 QPEND21                       : 1;
  __REG32 QPEND22                       : 1;
  __REG32 QPEND23                       : 1;
  __REG32 QPEND24                       : 1;
  __REG32 QPEND25                       : 1;
  __REG32 QPEND26                       : 1;
  __REG32 QPEND27                       : 1;
  __REG32 QPEND28                       : 1;
  __REG32 QPEND29                       : 1;
  __REG32 QPEND30                       : 1;
  __REG32 QPEND31                       : 1;
} __usb_qmgr_pend0_bits;

/* USB QMGR PEND1 Register */
typedef struct {
  __REG32 QPEND32                       : 1;
  __REG32 QPEND33                       : 1;
  __REG32 QPEND34                       : 1;
  __REG32 QPEND35                       : 1;
  __REG32 QPEND36                       : 1;
  __REG32 QPEND37                       : 1;
  __REG32 QPEND38                       : 1;
  __REG32 QPEND39                       : 1;
  __REG32 QPEND40                       : 1;
  __REG32 QPEND41                       : 1;
  __REG32 QPEND42                       : 1;
  __REG32 QPEND43                       : 1;
  __REG32 QPEND44                       : 1;
  __REG32 QPEND45                       : 1;
  __REG32 QPEND46                       : 1;
  __REG32 QPEND47                       : 1;
  __REG32 QPEND48                       : 1;
  __REG32 QPEND49                       : 1;
  __REG32 QPEND50                       : 1;
  __REG32 QPEND51                       : 1;
  __REG32 QPEND52                       : 1;
  __REG32 QPEND53                       : 1;
  __REG32 QPEND54                       : 1;
  __REG32 QPEND55                       : 1;
  __REG32 QPEND56                       : 1;
  __REG32 QPEND57                       : 1;
  __REG32 QPEND58                       : 1;
  __REG32 QPEND59                       : 1;
  __REG32 QPEND60                       : 1;
  __REG32 QPEND61                       : 1;
  __REG32 QPEND62                       : 1;
  __REG32 QPEND63                       : 1;
} __usb_qmgr_pend1_bits;

/* USB QMGR PEND1 Register */
typedef struct {
  __REG32 QPEND64                       : 1;
  __REG32 QPEND65                       : 1;
  __REG32 QPEND66                       : 1;
  __REG32 QPEND67                       : 1;
  __REG32 QPEND68                       : 1;
  __REG32 QPEND69                       : 1;
  __REG32 QPEND70                       : 1;
  __REG32 QPEND71                       : 1;
  __REG32 QPEND72                       : 1;
  __REG32 QPEND73                       : 1;
  __REG32 QPEND74                       : 1;
  __REG32 QPEND75                       : 1;
  __REG32 QPEND76                       : 1;
  __REG32 QPEND77                       : 1;
  __REG32 QPEND78                       : 1;
  __REG32 QPEND79                       : 1;
  __REG32 QPEND80                       : 1;
  __REG32 QPEND81                       : 1;
  __REG32 QPEND82                       : 1;
  __REG32 QPEND83                       : 1;
  __REG32 QPEND84                       : 1;
  __REG32 QPEND85                       : 1;
  __REG32 QPEND86                       : 1;
  __REG32 QPEND87                       : 1;
  __REG32 QPEND88                       : 1;
  __REG32 QPEND89                       : 1;
  __REG32 QPEND90                       : 1;
  __REG32 QPEND91                       : 1;
  __REG32 QPEND92                       : 1;
  __REG32 QPEND93                       : 1;
  __REG32 QPEND94                       : 1;
  __REG32 QPEND95                       : 1;
} __usb_qmgr_pend2_bits;

/* USB QMGR PEND1 Register */
typedef struct {
  __REG32 QPEND96                       : 1;
  __REG32 QPEND97                       : 1;
  __REG32 QPEND98                       : 1;
  __REG32 QPEND99                       : 1;
  __REG32 QPEND100                      : 1;
  __REG32 QPEND101                      : 1;
  __REG32 QPEND102                      : 1;
  __REG32 QPEND103                      : 1;
  __REG32 QPEND104                      : 1;
  __REG32 QPEND105                      : 1;
  __REG32 QPEND106                      : 1;
  __REG32 QPEND107                      : 1;
  __REG32 QPEND108                      : 1;
  __REG32 QPEND109                      : 1;
  __REG32 QPEND110                      : 1;
  __REG32 QPEND111                      : 1;
  __REG32 QPEND112                      : 1;
  __REG32 QPEND113                      : 1;
  __REG32 QPEND114                      : 1;
  __REG32 QPEND115                      : 1;
  __REG32 QPEND116                      : 1;
  __REG32 QPEND117                      : 1;
  __REG32 QPEND118                      : 1;
  __REG32 QPEND119                      : 1;
  __REG32 QPEND120                      : 1;
  __REG32 QPEND121                      : 1;
  __REG32 QPEND122                      : 1;
  __REG32 QPEND123                      : 1;
  __REG32 QPEND124                      : 1;
  __REG32 QPEND125                      : 1;
  __REG32 QPEND126                      : 1;
  __REG32 QPEND127                      : 1;
} __usb_qmgr_pend3_bits;

/* USB QMGR PEND1 Register */
typedef struct {
  __REG32 QPEND128                       : 1;
  __REG32 QPEND129                       : 1;
  __REG32 QPEND130                      : 1;
  __REG32 QPEND131                      : 1;
  __REG32 QPEND132                      : 1;
  __REG32 QPEND133                      : 1;
  __REG32 QPEND134                      : 1;
  __REG32 QPEND135                      : 1;
  __REG32 QPEND136                      : 1;
  __REG32 QPEND137                      : 1;
  __REG32 QPEND138                      : 1;
  __REG32 QPEND139                      : 1;
  __REG32 QPEND140                      : 1;
  __REG32 QPEND141                      : 1;
  __REG32 QPEND142                      : 1;
  __REG32 QPEND143                      : 1;
  __REG32 QPEND144                      : 1;
  __REG32 QPEND145                      : 1;
  __REG32 QPEND146                      : 1;
  __REG32 QPEND147                      : 1;
  __REG32 QPEND148                      : 1;
  __REG32 QPEND149                      : 1;
  __REG32 QPEND150                      : 1;
  __REG32 QPEND151                      : 1;
  __REG32 QPEND152                      : 1;
  __REG32 QPEND153                      : 1;
  __REG32 QPEND154                      : 1;
  __REG32 QPEND155                      : 1;
  __REG32 QPEND156                      : 1;
  __REG32 QPEND157                      : 1;
  __REG32 QPEND158                      : 1;
  __REG32 QPEND159                      : 1;
} __usb_qmgr_pend4_bits;

/* USB QMGR QMEMCTRLx Register */
typedef struct {
  __REG32 REG_SIZE                      : 3;
  __REG32                               : 5;
  __REG32 DESC_SIZE                     : 4;
  __REG32                               : 4;
  __REG32 START_INDEX                   :14;
  __REG32                               : 2;
} __usb_qmgr_qmemctrl_bits;

/* USB QMGR QUEUE_x_A Register */
/* USB QMGR QUEUE_x_STATUS_A Register */
typedef struct {
  __REG32 QUEUE_ENTRY_COUNT             :14;
  __REG32                               :18;
} __usb_qmgr_queue_a_bits;

/* USB QMGR QUEUE_x_B Register */
/* USB QMGR QUEUE_x_STATUS_B Register */
typedef struct {
  __REG32 QUEUE_BYTE_COUNT              :28;
  __REG32                               : 4;
} __usb_qmgr_queue_b_bits;

/* USB QMGR QUEUE_x_C Register */
typedef struct {
  __REG32 PACKET_SIZE                   :14;
  __REG32                               :17;
  __REG32 HEAD_TAIL                     : 1;
} __usb_qmgr_queue_c_bits;

/* USB QMGR QUEUE_x_D Register */
typedef struct {
  __REG32 DESC_SIZE                     : 5;
  __REG32 DESC_PTR                      :27;
} __usb_qmgr_queue_d_bits;

/* USB QMGR QUEUE_x_STATUS_C Register */
typedef struct {
  __REG32 PACKET_SIZE                   :14;
  __REG32                               :18;
} __usb_qmgr_queue_status_c_bits;

/* MAILBOX_REVISION */
typedef struct {
   __REG32 MINOR                     : 6;
   __REG32 CUSTOM                    : 2;
   __REG32 MAJOR                     : 3;
   __REG32 RTL                       : 5;
   __REG32 FUNC                      :12;
   __REG32                           : 2;
   __REG32 SCHEME                    : 2;
} __mailbox_revision_bits;

/* MAILBOX_SYSCONFIG */
typedef struct {
   __REG32 SOFTRESET                 : 1;
   __REG32                           : 1;
   __REG32 SIDLEMODE                 : 2;
   __REG32                           :28;
} __mailbox_sysconfig_bits;

/* MAILBOX_FIFOSTATUS_m */
typedef struct {
   __REG32 FIFOFULLMBM               : 1;
   __REG32                           :31;
} __mailbox_fifostatus_bits;

/* MAILBOX_MSGSTATUS_m */
typedef struct {
   __REG32 NBOFMSGMBM                : 3;
   __REG32                           :29;
} __mailbox_msgstatus_bits;

/* MAILBOX_IRQSTATUS_0 */
typedef struct {
   __REG32 NEWMSGSTATUSU0MB0         : 1;
   __REG32 NOTFULLSTATUSU0MB0        : 1;
   __REG32 NEWMSGSTATUSU0MB1         : 1;
   __REG32 NOTFULLSTATUSU0MB1        : 1;
   __REG32 NEWMSGSTATUSU0MB2         : 1;
   __REG32 NOTFULLSTATUSU0MB2        : 1;
   __REG32 NEWMSGSTATUSU0MB3         : 1;
   __REG32 NOTFULLSTATUSU0MB3        : 1;
   __REG32 NEWMSGSTATUSU0MB4         : 1;
   __REG32 NOTFULLSTATUSU0MB4        : 1;
   __REG32 NEWMSGSTATUSU0MB5         : 1;
   __REG32 NOTFULLSTATUSU0MB5        : 1;
   __REG32 NEWMSGSTATUSU0MB6         : 1;
   __REG32 NOTFULLSTATUSU0MB6        : 1;
   __REG32 NEWMSGSTATUSU0MB7         : 1;
   __REG32 NOTFULLSTATUSU0MB7        : 1;
   __REG32                           :16;
} __mailbox_irqstatus_0_bits;

/* MAILBOX_IRQSTATUS_1 */
typedef struct {
   __REG32 NEWMSGSTATUSU1MB0         : 1;
   __REG32 NOTFULLSTATUSU1MB0        : 1;
   __REG32 NEWMSGSTATUSU1MB1         : 1;
   __REG32 NOTFULLSTATUSU1MB1        : 1;
   __REG32 NEWMSGSTATUSU1MB2         : 1;
   __REG32 NOTFULLSTATUSU1MB2        : 1;
   __REG32 NEWMSGSTATUSU1MB3         : 1;
   __REG32 NOTFULLSTATUSU1MB3        : 1;
   __REG32 NEWMSGSTATUSU1MB4         : 1;
   __REG32 NOTFULLSTATUSU1MB4        : 1;
   __REG32 NEWMSGSTATUSU1MB5         : 1;
   __REG32 NOTFULLSTATUSU1MB5        : 1;
   __REG32 NEWMSGSTATUSU1MB6         : 1;
   __REG32 NOTFULLSTATUSU1MB6        : 1;
   __REG32 NEWMSGSTATUSU1MB7         : 1;
   __REG32 NOTFULLSTATUSU1MB7        : 1;
   __REG32                           :16;
} __mailbox_irqstatus_1_bits;

/* MAILBOX_IRQSTATUS_2 */
typedef struct {
   __REG32 NEWMSGSTATUSU2MB0         : 1;
   __REG32 NOTFULLSTATUSU2MB0        : 1;
   __REG32 NEWMSGSTATUSU2MB1         : 1;
   __REG32 NOTFULLSTATUSU2MB1        : 1;
   __REG32 NEWMSGSTATUSU2MB2         : 1;
   __REG32 NOTFULLSTATUSU2MB2        : 1;
   __REG32 NEWMSGSTATUSU2MB3         : 1;
   __REG32 NOTFULLSTATUSU2MB3        : 1;
   __REG32 NEWMSGSTATUSU2MB4         : 1;
   __REG32 NOTFULLSTATUSU2MB4        : 1;
   __REG32 NEWMSGSTATUSU2MB5         : 1;
   __REG32 NOTFULLSTATUSU2MB5        : 1;
   __REG32 NEWMSGSTATUSU2MB6         : 1;
   __REG32 NOTFULLSTATUSU2MB6        : 1;
   __REG32 NEWMSGSTATUSU2MB7         : 1;
   __REG32 NOTFULLSTATUSU2MB7        : 1;
   __REG32                           :16;
} __mailbox_irqstatus_2_bits;

/* MAILBOX_IRQSTATUS_3 */
typedef struct {
   __REG32 NEWMSGSTATUSU3MB0         : 1;
   __REG32 NOTFULLSTATUSU3MB0        : 1;
   __REG32 NEWMSGSTATUSU3MB1         : 1;
   __REG32 NOTFULLSTATUSU3MB1        : 1;
   __REG32 NEWMSGSTATUSU3MB2         : 1;
   __REG32 NOTFULLSTATUSU3MB2        : 1;
   __REG32 NEWMSGSTATUSU3MB3         : 1;
   __REG32 NOTFULLSTATUSU3MB3        : 1;
   __REG32 NEWMSGSTATUSU3MB4         : 1;
   __REG32 NOTFULLSTATUSU3MB4        : 1;
   __REG32 NEWMSGSTATUSU3MB5         : 1;
   __REG32 NOTFULLSTATUSU3MB5        : 1;
   __REG32 NEWMSGSTATUSU3MB6         : 1;
   __REG32 NOTFULLSTATUSU3MB6        : 1;
   __REG32 NEWMSGSTATUSU3MB7         : 1;
   __REG32 NOTFULLSTATUSU3MB7        : 1;
   __REG32                           :16;
} __mailbox_irqstatus_3_bits;

/* MAILBOX_IRQENABLE_0 */
typedef struct {
   __REG32 NEWMSGENABLEU0MB0         : 1;
   __REG32 NOTFULLENABLEU0MB0        : 1;
   __REG32 NEWMSGENABLEU0MB1         : 1;
   __REG32 NOTFULLENABLEU0MB1        : 1;
   __REG32 NEWMSGENABLEU0MB2         : 1;
   __REG32 NOTFULLENABLEU0MB2        : 1;
   __REG32 NEWMSGENABLEU0MB3         : 1;
   __REG32 NOTFULLENABLEU0MB3        : 1;
   __REG32 NEWMSGENABLEU0MB4         : 1;
   __REG32 NOTFULLENABLEU0MB4        : 1;
   __REG32 NEWMSGENABLEU0MB5         : 1;
   __REG32 NOTFULLENABLEU0MB5        : 1;
   __REG32 NEWMSGENABLEU0MB6         : 1;
   __REG32 NOTFULLENABLEU0MB6        : 1;
   __REG32 NEWMSGENABLEU0MB7         : 1;
   __REG32 NOTFULLENABLEU0MB7        : 1;
   __REG32                           :16;
} __mailbox_irqenable_0_bits;

/* MAILBOX_IRQENABLE_1 */
typedef struct {
   __REG32 NEWMSGENABLEU1MB0         : 1;
   __REG32 NOTFULLENABLEU1MB0        : 1;
   __REG32 NEWMSGENABLEU1MB1         : 1;
   __REG32 NOTFULLENABLEU1MB1        : 1;
   __REG32 NEWMSGENABLEU1MB2         : 1;
   __REG32 NOTFULLENABLEU1MB2        : 1;
   __REG32 NEWMSGENABLEU1MB3         : 1;
   __REG32 NOTFULLENABLEU1MB3        : 1;
   __REG32 NEWMSGENABLEU1MB4         : 1;
   __REG32 NOTFULLENABLEU1MB4        : 1;
   __REG32 NEWMSGENABLEU1MB5         : 1;
   __REG32 NOTFULLENABLEU1MB5        : 1;
   __REG32 NEWMSGENABLEU1MB6         : 1;
   __REG32 NOTFULLENABLEU1MB6        : 1;
   __REG32 NEWMSGENABLEU1MB7         : 1;
   __REG32 NOTFULLENABLEU1MB7        : 1;
   __REG32                           :16;
} __mailbox_irqenable_1_bits;

/* MAILBOX_IRQENABLE_2 */
typedef struct {
   __REG32 NEWMSGENABLEU2MB0         : 1;
   __REG32 NOTFULLENABLEU2MB0        : 1;
   __REG32 NEWMSGENABLEU2MB1         : 1;
   __REG32 NOTFULLENABLEU2MB1        : 1;
   __REG32 NEWMSGENABLEU2MB2         : 1;
   __REG32 NOTFULLENABLEU2MB2        : 1;
   __REG32 NEWMSGENABLEU2MB3         : 1;
   __REG32 NOTFULLENABLEU2MB3        : 1;
   __REG32 NEWMSGENABLEU2MB4         : 1;
   __REG32 NOTFULLENABLEU2MB4        : 1;
   __REG32 NEWMSGENABLEU2MB5         : 1;
   __REG32 NOTFULLENABLEU2MB5        : 1;
   __REG32 NEWMSGENABLEU2MB6         : 1;
   __REG32 NOTFULLENABLEU2MB6        : 1;
   __REG32 NEWMSGENABLEU2MB7         : 1;
   __REG32 NOTFULLENABLEU2MB7        : 1;
   __REG32                           :16;
} __mailbox_irqenable_2_bits;

/* MAILBOX_IRQENABLE_3 */
typedef struct {
   __REG32 NEWMSGENABLEU3MB0         : 1;
   __REG32 NOTFULLENABLEU3MB0        : 1;
   __REG32 NEWMSGENABLEU3MB1         : 1;
   __REG32 NOTFULLENABLEU3MB1        : 1;
   __REG32 NEWMSGENABLEU3MB2         : 1;
   __REG32 NOTFULLENABLEU3MB2        : 1;
   __REG32 NEWMSGENABLEU3MB3         : 1;
   __REG32 NOTFULLENABLEU3MB3        : 1;
   __REG32 NEWMSGENABLEU3MB4         : 1;
   __REG32 NOTFULLENABLEU3MB4        : 1;
   __REG32 NEWMSGENABLEU3MB5         : 1;
   __REG32 NOTFULLENABLEU3MB5        : 1;
   __REG32 NEWMSGENABLEU3MB6         : 1;
   __REG32 NOTFULLENABLEU3MB6        : 1;
   __REG32 NEWMSGENABLEU3MB7         : 1;
   __REG32 NOTFULLENABLEU3MB7        : 1;
   __REG32                           :16;
} __mailbox_irqenable_3_bits;

#define MAILBOX_IRQ_NEWMSG_MB0       (0x1UL<<0)
#define MAILBOX_IRQ_NOTFULL_MB0      (0x1UL<<1)
#define MAILBOX_IRQ_NEWMSG_MB1       (0x1UL<<2)
#define MAILBOX_IRQ_NOTFULL_MB1      (0x1UL<<3)
#define MAILBOX_IRQ_NEWMSG_MB2       (0x1UL<<4)
#define MAILBOX_IRQ_NOTFULL_MB2      (0x1UL<<5)
#define MAILBOX_IRQ_NEWMSG_MB3       (0x1UL<<6)
#define MAILBOX_IRQ_NOTFULL_MB3      (0x1UL<<7)
#define MAILBOX_IRQ_NEWMSG_MB4       (0x1UL<<8)
#define MAILBOX_IRQ_NOTFULL_MB4      (0x1UL<<9)
#define MAILBOX_IRQ_NEWMSG_MB5       (0x1UL<<10)
#define MAILBOX_IRQ_NOTFULL_MB5      (0x1UL<<11)
#define MAILBOX_IRQ_NEWMSG_MB6       (0x1UL<<12)
#define MAILBOX_IRQ_NOTFULL_MB6      (0x1UL<<13)
#define MAILBOX_IRQ_NEWMSG_MB7       (0x1UL<<14)
#define MAILBOX_IRQ_NOTFULL_MB7      (0x1UL<<15)

/* SPINLOCK SYSCONFIG Register*/
typedef struct {
   __REG32 AUTOGATING                : 1;
   __REG32 SOFTRESET                 : 1;
   __REG32 ENWAKEUP                  : 1;
   __REG32 SIDLEMODE                 : 2;
   __REG32                           : 3;
   __REG32 CLOCKACTIVITY             : 1;
   __REG32                           :23;
} __spinlock_sysconfig_bits;

/* SPINLOCK SYSTATUS Register*/
typedef struct {
   __REG32 RESETDONE                 : 1;
   __REG32                           : 7;
   __REG32 IU0                       : 1;
   __REG32 IU1                       : 1;
   __REG32 IU2                       : 1;
   __REG32 IU3                       : 1;
   __REG32 IU4                       : 1;
   __REG32 IU5                       : 1;
   __REG32 IU6                       : 1;
   __REG32 IU7                       : 1;
   __REG32                           : 8;
   __REG32 NUMLOCKS                  : 8;
} __spinlock_systatus_bits;

/* SPINLOCK LOCK_REG_x Register*/
typedef struct {
   __REG32 TAKEN                     : 1;
   __REG32                           :31;
} __spinlock_lock_reg_bits;

/* MMCHSn_SYSCONFIG */
typedef struct {
   __REG32 AUTOIDLE                  : 1;
   __REG32 SOFTRESET                 : 1;
   __REG32 ENAWAKEUP                 : 1;
   __REG32 SIDLEMODE                 : 2;
   __REG32                           : 3;
   __REG32 CLOCKACTIVITY             : 2;
   __REG32                           : 2;
   __REG32 STANDBYMODE               : 2;
   __REG32                           :18;
} __mmchs_sysconfig_bits;

/* MMCHSn_SYSSTATUS */
typedef struct {
   __REG32 RESETDONE                 : 1;
   __REG32                           :31;
} __mmchs_sysstatus_bits;

/* MMCHSn_SYSTEST */
typedef struct {
   __REG32 MCKD                      : 1;
   __REG32 CDIR                      : 1;
   __REG32 CDAT                      : 1;
   __REG32 DDIR                      : 1;
   __REG32 D0D                       : 1;
   __REG32 D1D                       : 1;
   __REG32 D2D                       : 1;
   __REG32 D3D                       : 1;
   __REG32 D4D                       : 1;
   __REG32 D5D                       : 1;
   __REG32 D6D                       : 1;
   __REG32 D7D                       : 1;
   __REG32 SSB                       : 1;
   __REG32 WAKD                      : 1;
   __REG32 SDWP                      : 1;
   __REG32 SDCD                      : 1;
   __REG32 OBI                       : 1;
   __REG32                           :15;
} __mmchs_systest_bits;

/* MMCHSn_CON */
typedef struct {
   __REG32 OD                        : 1;
   __REG32 INIT                      : 1;
   __REG32 HR                        : 1;
   __REG32 STR                       : 1;
   __REG32 MODE                      : 1;
   __REG32 DW8                       : 1;
   __REG32 MIT                       : 1;
   __REG32 CDP                       : 1;
   __REG32 WPP                       : 1;
   __REG32 DVAL                      : 2;
   __REG32 CTPL                      : 1;
   __REG32 CEATA                     : 1;
   __REG32                           : 2;
   __REG32 PADEN                     : 1;
   __REG32 CLKEXTFREE                : 1;
   __REG32 BOOT_ACK                  : 1;
   __REG32 BOOT_CF0                  : 1;
   __REG32 DDR                       : 1;
   __REG32 DMA_MNS                   : 1;
   __REG32 SDMA_LNE                  : 1;
   __REG32                           :10;
} __mmchs_con_bits;

/* MMCHSn_PWCNT */
typedef struct {
   __REG32 PWRCNT                    :16;
   __REG32                           :16;
} __mmchs_pwcnt_bits;

/* MMCHSn_BLK */
typedef struct {
   __REG32 BLEN                      :12;
   __REG32                           : 4;
   __REG32 NBLK                      :16;
} __mmchs_blk_bits;

/* MMCHSn_CMD */
typedef struct {
   __REG32 DE                        : 1;
   __REG32 BCE                       : 1;
   __REG32 ACEN                      : 1;
   __REG32                           : 1;
   __REG32 DDIR                      : 1;
   __REG32 MSBS                      : 1;
   __REG32                           :10;
   __REG32 RSP_TYPE                  : 2;
   __REG32                           : 1;
   __REG32 CCCE                      : 1;
   __REG32 CICE                      : 1;
   __REG32 DP                        : 1;
   __REG32 CMD_TYPE                  : 2;
   __REG32 INDX                      : 6;
   __REG32                           : 2;
} __mmchs_cmd_bits;

/* MMCHSn_RSP10 */
typedef struct {
   __REG32 RSP0                      :16;
   __REG32 RSP1                      :16;
} __mmchs_rsp10_bits;

/* MMCHSn_RSP32 */
typedef struct {
   __REG32 RSP2                      :16;
   __REG32 RSP3                      :16;
} __mmchs_rsp32_bits;

/* MMCHSn_RSP54 */
typedef struct {
   __REG32 RSP4                      :16;
   __REG32 RSP5                      :16;
} __mmchs_rsp54_bits;

/* MMCHSn_RSP76 */
typedef struct {
   __REG32 RSP6                      :16;
   __REG32 RSP7                      :16;
} __mmchs_rsp76_bits;

/* MMCHSn_PSTATE */
typedef struct {
   __REG32 CMDI                      : 1;
   __REG32 DATI                      : 1;
   __REG32 DLA                       : 1;
   __REG32                           : 5;
   __REG32 WTA                       : 1;
   __REG32 RTA                       : 1;
   __REG32 BWE                       : 1;
   __REG32 BRE                       : 1;
   __REG32                           : 4;
   __REG32 CI                        : 1;
   __REG32 CSS                       : 1;
   __REG32 CDPL                      : 1;
   __REG32 WP                        : 1;
   __REG32 DLEV                      : 4;
   __REG32 CLEV                      : 1;
   __REG32                           : 7;
} __mmchs_pstate_bits;

/* MMCHSn_HCTL */
typedef struct {
   __REG32                           : 1;
   __REG32 DTW                       : 1;
   __REG32 HSPE                      : 1;
   __REG32 DMAS                      : 2;
   __REG32                           : 1;
   __REG32 CDTL                      : 1;
   __REG32 CDSS                      : 1;
   __REG32 SDBP                      : 1;
   __REG32 SDVS                      : 3;
   __REG32                           : 4;
   __REG32 SBGR                      : 1;
   __REG32 CR                        : 1;
   __REG32 RWC                       : 1;
   __REG32 IBG                       : 1;
   __REG32                           : 4;
   __REG32 IWE                       : 1;
   __REG32 INS                       : 1;
   __REG32 REM                       : 1;
   __REG32 OBWE                      : 1;
   __REG32                           : 4;
} __mmchs_hctl_bits;

/* MMCHSn_SYSCTL */
typedef struct {
   __REG32 ICE                       : 1;
   __REG32 ICS                       : 1;
   __REG32 CEN                       : 1;
   __REG32                           : 3;
   __REG32 CLKD                      :10;
   __REG32 DTO                       : 4;
   __REG32                           : 4;
   __REG32 SRA                       : 1;
   __REG32 SRC                       : 1;
   __REG32 SRD                       : 1;
   __REG32                           : 5;
} __mmchs_sysctl_bits;

/* MMCHSn_STAT */
typedef struct {
   __REG32 CC                        : 1;
   __REG32 TC                        : 1;
   __REG32 BGE                       : 1;
   __REG32 DMA                       : 1;
   __REG32 BWR                       : 1;
   __REG32 BRR                       : 1;
   __REG32 CINS                      : 1;
   __REG32 CREM                      : 1;
   __REG32 CIRQ                      : 1;
   __REG32 OBI                       : 1;
   __REG32 BSR                       : 1;
   __REG32                           : 4;
   __REG32 ERRI                      : 1;
   __REG32 CTO                       : 1;
   __REG32 CCRC                      : 1;
   __REG32 CEB                       : 1;
   __REG32 CIE                       : 1;
   __REG32 DTO                       : 1;
   __REG32 DCRC                      : 1;
   __REG32 DEB                       : 1;
   __REG32                           : 1;
   __REG32 ACE                       : 1;
   __REG32 ADMAE                     : 1;
   __REG32                           : 2;
   __REG32 CERR                      : 1;
   __REG32 BADA                      : 1;
   __REG32                           : 2;
} __mmchs_stat_bits;

/* MMCHSn_IE */
typedef struct {
   __REG32 CC_ENABLE                 : 1;
   __REG32 TC_ENABLE                 : 1;
   __REG32 BGE_ENABLE                : 1;
   __REG32 DMA_ENABLE                : 1;
   __REG32 BWR_ENABLE                : 1;
   __REG32 BRR_ENABLE                : 1;
   __REG32 CINS_ENABLE               : 1;
   __REG32 CREM_ENABLE               : 1;
   __REG32 CIRQ_ENABLE               : 1;
   __REG32 OBI_ENABLE                : 1;
   __REG32 BSR_ENABLE                : 1;
   __REG32                           : 5;
   __REG32 CTO_ENABLE                : 1;
   __REG32 CCRC_ENABLE               : 1;
   __REG32 CEB_ENABLE                : 1;
   __REG32 CIE_ENABLE                : 1;
   __REG32 DTO_ENABLE                : 1;
   __REG32 DCRC_ENABLE               : 1;
   __REG32 DEB_ENABLE                : 1;
   __REG32                           : 1;
   __REG32 ACE_ENABLE                : 1;
   __REG32 ADMA_ENABLE               : 1;
   __REG32                           : 2;
   __REG32 CERR_ENABLE               : 1;
   __REG32 BADA_ENABLE               : 1;
   __REG32                           : 2;
} __mmchs_ie_bits;

/* MMCHSn_ISE */
typedef struct {
   __REG32 CC_SIGEN                  : 1;
   __REG32 TC_SIGEN                  : 1;
   __REG32 BGE_SIGEN                 : 1;
   __REG32 DMA_SIGEN                 : 1;
   __REG32 BWR_SIGEN                 : 1;
   __REG32 BRR_SIGEN                 : 1;
   __REG32 CINS_SIGEN                : 1;
   __REG32 CREM_SIGEN                : 1;
   __REG32 CIRQ_SIGEN                : 1;
   __REG32 OBI_SIGEN                 : 1;
   __REG32 BSR_SIGEN                 : 1;
   __REG32                           : 5;
   __REG32 CTO_SIGEN                 : 1;
   __REG32 CCRC_SIGEN                : 1;
   __REG32 CEB_SIGEN                 : 1;
   __REG32 CIE_SIGEN                 : 1;
   __REG32 DTO_SIGEN                 : 1;
   __REG32 DCRC_SIGEN                : 1;
   __REG32 DEB_SIGEN                 : 1;
   __REG32                           : 1;
   __REG32 ACE_SIGEN                 : 1;
   __REG32 ADMA_SIGEN                : 1;
   __REG32                           : 2;
   __REG32 CERR_SIGEN                : 1;
   __REG32 BADA_SIGEN                : 1;
   __REG32                           : 2;
} __mmchs_ise_bits;

/* MMCHSn_AC12 */
typedef struct {
   __REG32 ACNE                      : 1;
   __REG32 ACTO                      : 1;
   __REG32 ACCE                      : 1;
   __REG32 ACEB                      : 1;
   __REG32 ACIE                      : 1;
   __REG32                           : 2;
   __REG32 CNI                       : 1;
   __REG32                           :24;
} __mmchs_ac12_bits;

/* MMCHSn_CAPA */
typedef struct {
   __REG32 TCF                       : 6;
   __REG32                           : 1;
   __REG32 TCU                       : 1;
   __REG32 BCF                       : 6;
   __REG32                           : 2;
   __REG32 MBL                       : 2;
   __REG32                           : 1;
   __REG32 AD2S                      : 1;
   __REG32                           : 1;
   __REG32 HSS                       : 1;
   __REG32 DS                        : 1;
   __REG32 SRS                       : 1;
   __REG32 VS33                      : 1;
   __REG32 VS30                      : 1;
   __REG32 VS18                      : 1;
   __REG32                           : 1;
   __REG32 _64BIT                    : 1;
   __REG32                           : 3;
} __mmchs_capa_bits;

/* MMCHSn_CUR_CAPA */
typedef struct {
   __REG32 CUR_3V3                   : 8;
   __REG32 CUR_3V0                   : 8;
   __REG32 CUR_1V8                   : 8;
   __REG32                           : 8;
} __mmchs_cur_capa_bits;

/* MMCHSn_FE */
#define MMCHS_FE_FE_ACNE             (0x1UL<<0)
#define MMCHS_FE_FE_ACTO             (0x1UL<<1)
#define MMCHS_FE_FE_ACCE             (0x1UL<<2)
#define MMCHS_FE_FE_ACEB             (0x1UL<<3)
#define MMCHS_FE_FE_ACIE             (0x1UL<<4)
#define MMCHS_FE_FE_CNI              (0x1UL<<7)
#define MMCHS_FE_FE_CTO              (0x1UL<<16)
#define MMCHS_FE_FE_CCRC             (0x1UL<<17)
#define MMCHS_FE_FE_CEB              (0x1UL<<18)
#define MMCHS_FE_FE_CIE              (0x1UL<<19)
#define MMCHS_FE_FE_DTO              (0x1UL<<20)
#define MMCHS_FE_FE_DCRC             (0x1UL<<21)
#define MMCHS_FE_FE_DEB              (0x1UL<<22)
#define MMCHS_FE_FE_ACE              (0x1UL<<24)
#define MMCHS_FE_FE_ADMAE            (0x1UL<<25)
#define MMCHS_FE_FE_CERR             (0x1UL<<28)
#define MMCHS_FE_FE_BADA             (0x1UL<<29)

/* MMCHSn_ADMAES */
typedef struct {
   __REG32 AES                       : 2;
   __REG32 LME                       : 1;
   __REG32                           :29;
} __mmchs_admaes_bits;

/* MMCHSn_REV */
typedef struct {
   __REG32 SIS                       : 1;
   __REG32                           :15;
   __REG32 SREV                      : 8;
   __REG32 VREV                      : 8;
} __mmchs_rev_bits;

/* UARTn_IER */
typedef union {
  /* UARTx_IER */
  struct {
   __REG8  RHR_IT                : 1;
   __REG8  THR_IT                : 1;
   __REG8  LINE_STS_IT           : 1;
   __REG8  MODEM_STS_IT          : 1;
   __REG8  SLEEP_MODE            : 1;
   __REG8  XOFF_IT               : 1;
   __REG8  RTS_IT                : 1;
   __REG8  CTS_IT                : 1;
   };
  /* UARTx_IrDA_IER */
  struct {
   __REG8  RHR_IT                : 1;
   __REG8  THR_IT                : 1;
   __REG8  LAST_RX_BYTE_IT       : 1;
   __REG8  RX_OVERRUN_IT         : 1;
   __REG8  STS_FIFO_TRIG_IT      : 1;
   __REG8  TX_STATUS_IT          : 1;
   __REG8  LINE_STS_IT_I         : 1;
   __REG8  EOF_IT                : 1;
   } IrDA;
  /* UARTx_CIR_IER */
  struct {
   __REG8  RHR_IT                : 1;
   __REG8  THR_IT                : 1;
   __REG8  RX_STOP_IT            : 1;
   __REG8  RX_OVERRUN_IT         : 1;
   __REG8                        : 1;
   __REG8  TX_STATUS_IT          : 1;
   __REG8                        : 2;
   } CIR;
} __uart_ier_bits;

/* UARTn_IIR */
typedef union {
  /* UARTx_IIR */
  struct {
   __REG8  IT_PENDING            : 1;
   __REG8  IT_TYPE               : 5;
   __REG8  FCR_MIRROR            : 2;
   };
  /* UARTx_IrDA_IIR */
  struct {
   __REG8  RHR_IT                : 1;
   __REG8  THR_IT                : 1;
   __REG8  RX_FIFO_LB_IT         : 1;
   __REG8  RX_OE_IT              : 1;
   __REG8  STS_FIFO_IT           : 1;
   __REG8  TX_STATUS_IT          : 1;
   __REG8  LINE_STS_IT           : 1;
   __REG8  EOF_IT                : 1;
   } IrDA;
  /* UARTx_CIR_IIR */
  struct {
   __REG8  RHR_IT                : 1;
   __REG8  THR_IT                : 1;
   __REG8  RX_STOP_IT            : 1;
   __REG8  RX_OE_IT              : 1;
   __REG8                        : 1;
   __REG8  TX_STATUS_IT          : 1;
   __REG8                        : 2;
   } CIR;
  /* UARTx_EFR */
  struct {
   __REG8  SW_FLOW_CONTROL       : 4;
   __REG8  ENHANCED_EN           : 1;
   __REG8  SPEC_CHAR             : 1;
   __REG8  AUTO_RTS_EN           : 1;
   __REG8  AUTO_CTS_EN           : 1;
   };
} __uart_iir_bits;

/* UARTx_FCR */
#define UART_FCR_FIFO_EN               (0x1<<0)
#define UART_FCR_RX_FIFO_CLEAR         (0x1<<1)
#define UART_FCR_TX_FIFO_CLEAR         (0x1<<2)
#define UART_FCR_DMA_MODE              (0x1<<3)
#define UART_FCR_TX_FIFO_TRIG          (0x3<<4)
#define UART_FCR_RX_FIFO_TRIG          (0x3<<6)

/* UARTn_LCR */
typedef struct {
   __REG8  CHAR_LENGTH           : 2;
   __REG8  NB_STOP               : 1;
   __REG8  PARITY_EN             : 1;
   __REG8  PARITY_TYPE1          : 1;
   __REG8  PARITY_TYPE2          : 1;
   __REG8  BREAK_EN              : 1;
   __REG8  DIV_EN                : 1;
} __uart_lcr_bits;

/* UARTn_MCR */
typedef struct {
   __REG8  DTR                   : 1;
   __REG8  RTS                   : 1;
   __REG8  RI_STS_CH             : 1;
   __REG8  CD_STS_CH             : 1;
   __REG8  LOOPBACK_EN           : 1;
   __REG8  XON_EN                : 1;
   __REG8  TCR_TLR               : 1;
   __REG8                        : 1;
} __uart_mcr_bits;

/* UARTn_LSR */
typedef union {
  /* UARTx_LSR */
  struct {
   __REG8  RX_FIFO_E              : 1;
   __REG8  RX_OE                  : 1;
   __REG8  RX_PE                  : 1;
   __REG8  RX_FE                  : 1;
   __REG8  RX_BI                  : 1;
   __REG8  TX_FIFO_E              : 1;
   __REG8  TX_SR_E                : 1;
   __REG8  RX_FIFO_STS            : 1;
   };
  /* UARTx_LSR_IrDA */
  struct {
   __REG8  RX_FIFO_E             : 1;
   __REG8  STS_FIFO_E            : 1;
   __REG8  CRC                   : 1;
   __REG8  ABORT                 : 1;
   __REG8  FRAME_TOO_LONG        : 1;
   __REG8  RX_LAST_BYTE          : 1;
   __REG8  STS_FIFO_FUL          : 1;
   __REG8  THR_EMPTY             : 1;
   } IrDA;
  /* UARTx_LSR_CIR */
  struct {
   __REG8  RX_FIFO_E             : 1;
   __REG8                        : 4;
   __REG8  RX_STOP               : 1;
   __REG8                        : 1;
   __REG8  THR_EMPTY             : 1;
   } CIR;
} __uart_lsr_bits;

/* UARTx_TCR */
typedef union {
  /* UARTx_TCR */
  struct {
   __REG8  RX_FIFO_TRIG_HALT     : 4;
   __REG8  RX_FIFO_TRIG_START    : 4;
   };
  /* UARTx_MSR */
  struct {
   __REG8  CTS_STS               : 1;
   __REG8  DSR_STS               : 1;
   __REG8  RI_STS                : 1;
   __REG8  DCD_STS               : 1;
   __REG8  NCTS_STS              : 1;
   __REG8  NDSR_STS              : 1;
   __REG8  NRI_STS               : 1;
   __REG8  NCD_STS               : 1;
   };
} __uart_tcr_bits;

/* UARTx_TLR */
typedef struct {
   __REG8  TX_FIFO_TRIG_DMA      : 4;
   __REG8  RX_FIFO_TRIG_DMA      : 4;
} __uart_tlr_bits;

/* UARTn_MDR1 */
typedef struct {
   __REG8  MODE_SELECT           : 3;
   __REG8  IR_SLEEP              : 1;
   __REG8  SET_TXIR              : 1;
   __REG8  SCT                   : 1;
   __REG8  SIP_MODE              : 1;
   __REG8  FRAME_END_MODE        : 1;
} __uart_mdr1_bits;

/* UARTn_MDR2 */
typedef struct {
   __REG8  IRTX_UNDERRUN         : 1;
   __REG8  STS_FIFO_TRIG         : 2;
   __REG8  UART_PULSE            : 1;
   __REG8  CIR_PULSE_MODE        : 2;
   __REG8  IRRXINVERT            : 1;
   __REG8  SET_TXIR_ALT          : 1;
} __uart_mdr2_bits;

/* UARTx_SFLSR */
typedef struct {
   __REG8                        : 1;
   __REG8  CRC_ERROR             : 1;
   __REG8  ABORT_DETECT          : 1;
   __REG8  FTL_ERROR             : 1;
   __REG8  OE_ERROR              : 1;
   __REG8                        : 3;
} __uart_sflsr_bits;

/* UARTx_TXFLH */
typedef struct {
   __REG8  TXFLH                 : 5;
   __REG8                        : 3;
} __uart_txflh_bits;

/* UARTx_SFREGH */
typedef union {
  /* UARTx_RXFLH */
  struct {
   __REG8  RXFLH                 : 4;
   __REG8                        : 4;
   };
  /* UARTx_SFREGH */
  struct {
   __REG8  SFREGH                : 4;
   __REG8                        : 4;
   };
} __uart_sfregh_bits;

/* UARTn_BLR */
typedef union {
  /* UARTx_BLR */
  struct {
   __REG8                        : 6;
   __REG8  XBOF_TYPE             : 1;
   __REG8  STS_FIFO_RESET        : 1;
   };
  /* UARTx_UASR */
  struct {
   __REG8  SPEED                 : 5;
   __REG8  BIT_BY_CHAR           : 1;
   __REG8  PARITY_TYPE           : 2;
   };
} __uart_uasr_bits;

/* UARTn_ACREG */
typedef struct {
   __REG8  EOT_EN                : 1;
   __REG8  ABORT_EN              : 1;
   __REG8  SCTX_EN               : 1;
   __REG8  SEND_SIP              : 1;
   __REG8  DIS_TX_UNDERRUN       : 1;
   __REG8  DIS_IR_RX             : 1;
   __REG8  SD_MOD                : 1;
   __REG8  PULSE_TYPE            : 1;
} __uart_acreg_bits;

/* UARTn_SCR */
typedef struct {
   __REG8  DMA_MODE_CTL          : 1;
   __REG8  DMA_MODE_2            : 2;
   __REG8  TX_EMPTY_CTL_IT       : 1;
   __REG8  RX_CTS_WU_EN          : 1;
   __REG8  DSR_IT                : 1;
   __REG8  TX_TRIG_GRANU1        : 1;
   __REG8  RX_TRIG_GRANU1        : 1;
} __uart_scr_bits;

/* UARTn_SSR */
typedef struct {
   __REG8  TX_FIFO_FULL          : 1;
   __REG8  RX_CTS_DSR_WU_STS     : 1;
   __REG8  DMA_COUNTER_RST       : 1;
   __REG8                        : 5;
} __uart_ssr_bits;

/* UARTn_MVR */
typedef struct {
   __REG8  MINOR_REV             : 4;
   __REG8  MAJOR_REV             : 4;
} __uart_mvr_bits;

/* UARTn_SYSC */
typedef struct {
   __REG8  AUTOIDLE              : 1;
   __REG8  SOFTRESET             : 1;
   __REG8  ENAWAKEUP             : 1;
   __REG8  IDLEMODE              : 2;
   __REG8                        : 3;
} __uart_sysc_bits;

/* UARTn_SYSS */
typedef struct {
   __REG8  RESETDONE             : 1;
   __REG8                        : 7;
} __uart_syss_bits;

/* UARTn_WER */
typedef struct {
   __REG8  CTS_ACTIVITY          : 1;
   __REG8  DSR_ACTIVITY          : 1;
   __REG8  RI_ACTIVITY           : 1;
   __REG8  DCD_ACTIVITY          : 1;
   __REG8  RX_ACTIVITY           : 1;
   __REG8  RHR_INTERRUPT         : 1;
   __REG8  RLS_INTERRUPT         : 1;
   __REG8  TX_WAKEUP_EN          : 1;
} __uart_wer_bits;

/* DMTIMER TIDR */
typedef struct {
   __REG32 Y_MINOR                   : 6;
   __REG32 CUSTOM                    : 2;
   __REG32 X_MAJOR                   : 3;
   __REG32 R_RTL                     : 5;
   __REG32 FUNC                      :12;
   __REG32                           : 2;
   __REG32 SCHEME                    : 2;
} __dmt_tidr_bits;

/* DMTIMER TIOCP_CFG  */
typedef struct {
   __REG32 SOFTRESET                 : 1;
   __REG32 EMUFREE                   : 1;
   __REG32 IDLEMODE                  : 2;
   __REG32                           :28;
} __dmt_tiocp_cfg_bits;

/* DMTIMER IRQ_EOI  */
typedef struct {
   __REG32 LINE_NUMBER               : 1;
   __REG32                           :31;
} __dmt_irq_eoi_bits;

/* DMTIMER IRQSTATUS_RAW  */
/* DMTIMER IRQSTATUS  */
typedef struct {
   __REG32 MAT_IT_FLAG               : 1;
   __REG32 OVF_IT_FLAG               : 1;
   __REG32 TCAR_IT_FLAG              : 1;
   __REG32                           :29;
} __dmt_irqstatus_bits;

/* DMTIMER IRQENABLE_SET  */
/* DMTIMER IRQENABLE_CLR  */
typedef struct {
   __REG32 MAT_EN_FLAG               : 1;
   __REG32 OVF_EN_FLAG               : 1;
   __REG32 TCAR_EN_FLAG              : 1;
   __REG32                           :29;
} __dmt_irqenable_bits;

#define DMTIMER_IRQ_MAT_FLAG         (0x1UL<<0)
#define DMTIMER_IRQ_OVF_FLAG         (0x1UL<<1)
#define DMTIMER_IRQ_TCAR_FLAG        (0x1UL<<2)

/* DMTIMER IRQWAKEEN */
typedef struct {
   __REG32 MAT_WUP_ENA               : 1;
   __REG32 OVF_WUP_ENA               : 1;
   __REG32 TCAR_WUP_ENA              : 1;
   __REG32                           :29;
} __dmt_irqwakeen_bits;

/* DMTIMER TCLR */
typedef struct {
   __REG32 ST                        : 1;
   __REG32 AR                        : 1;
   __REG32 PTV                       : 3;
   __REG32 PRE                       : 1;
   __REG32 CE                        : 1;
   __REG32 SCPWM                     : 1;
   __REG32 TCM                       : 2;
   __REG32 TRG                       : 2;
   __REG32 PT                        : 1;
   __REG32 CAPT_MODE                 : 1;
   __REG32 GPO_CFG                   : 1;
   __REG32                           :17;
} __dmt_tclr_bits;

/* DMTIMER TWPS */
typedef struct {
   __REG32 W_PEND_TCLR               : 1;
   __REG32 W_PEND_TCRR               : 1;
   __REG32 W_PEND_TLDR               : 1;
   __REG32 W_PEND_TTGR               : 1;
   __REG32 W_PEND_TMAR               : 1;
   __REG32                           :27;
} __dmt_twps_bits;

/* DMTIMER TSICR */
typedef struct {
   __REG32                           : 1;
   __REG32 SFT                       : 1;
   __REG32 POSTED                    : 1;
   __REG32                           :29;
} __dmt_tsicr_bits;

/* DMTIMER1_1MS_TIDR */
typedef struct {
   __REG32 TID_REV                   : 8;
   __REG32                           :24;
} __dmt_1ms_tidr_bits;

/* DMTIMER1_1MS_TIOCP_CFG */
typedef struct {
   __REG32 AUTOIDLE                  : 1;
   __REG32 SOFTRESET                 : 1;
   __REG32 ENAWAKEUP                 : 1;
   __REG32 IDLEMODE                  : 2;
   __REG32 EMUFREE                   : 1;
   __REG32                           : 2;
   __REG32 CLOCKACTIVITY             : 2;
   __REG32                           :22;
} __dmt_1ms_tiocp_cfg_bits;

/* DMTIMER1_1MS_TISTAT */
typedef struct {
   __REG32 RESETDONE                 : 1;
   __REG32                           :31;
} __dmt_1ms_tistat_bits;

/* DMTIMER1_1MS_TISR */
typedef struct {
   __REG32 MAT_IT_FLAG               : 1;
   __REG32 OVF_IT_FLAG               : 1;
   __REG32 TCAR_IT_FLAG              : 1;
   __REG32                           :29;
} __dmt_1ms_tisr_bits;

/* DMTIMER1_1MS_TIER */
typedef struct {
   __REG32 MAT_IT_ENA                : 1;
   __REG32 OVF_IT_ENA                : 1;
   __REG32 TCAR_IT_ENA                : 1;
   __REG32                           :29;
} __dmt_1ms_tier_bits;

/* DMTIMER1_1MS_TWER */
typedef struct {
   __REG32 MAT_WUP_ENA               : 1;
   __REG32 OVF_WUP_ENA               : 1;
   __REG32 TCAR_WUP_ENA              : 1;
   __REG32                           :29;
} __dmt_1ms_twer_bits;

/* DMTIMER1_1MS_TCLR */
typedef struct {
   __REG32 ST                        : 1;
   __REG32 AR                        : 1;
   __REG32 PTV                       : 3;
   __REG32 PRE                       : 1;
   __REG32 CE                        : 1;
   __REG32 SCPWM                     : 1;
   __REG32 TCM                       : 2;
   __REG32 TRG                       : 2;
   __REG32 PT                        : 1;
   __REG32 CAPT_MODE                 : 1;
   __REG32 GPO_CFG                   : 1;
   __REG32                           :17;
} __dmt_1ms_tclr_bits;

/* DMTIMER1_1MS_TWPS */
typedef struct {
   __REG32 W_PEND_TCLR               : 1;
   __REG32 W_PEND_TCRR               : 1;
   __REG32 W_PEND_TLDR               : 1;
   __REG32 W_PEND_TTGR               : 1;
   __REG32 W_PEND_TMAR               : 1;
   __REG32 W_PEND_TPIR               : 1;
   __REG32 W_PEND_TNIR               : 1;
   __REG32 W_PEND_TCVR               : 1;
   __REG32 W_PEND_TOCR               : 1;
   __REG32 W_PEND_TOWR               : 1;
   __REG32                           :22;
} __dmt_1ms_twps_bits;

/* DMTIMER1_1MS_TSICR */
typedef struct {
   __REG32                           : 1;
   __REG32 SFT                       : 1;
   __REG32 POSTED                    : 1;
   __REG32                           :29;
} __dmt_1ms_tsicr_bits;

/* DMTIMER1_1MS_TOCR */
typedef struct {
   __REG32 OVF_COUNTER_VALUE         :24;
   __REG32                           : 8;
} __dmt_1ms_tocr_bits;

/* DMTIMER1_1MS_TOWR */
typedef struct {
   __REG32 OVF_WRAPPING_VALUE        :24;
   __REG32                           : 8;
} __dmt_1ms_towr_bits;

/* RTC Second Register (SECOND) */
/* RTC Alarm Second Register (ALARMSECOND) */
typedef struct {
  __REG32 SEC0                    : 4;
  __REG32 SEC1                    : 3;
  __REG32                         :25;
} __rtc_second_bits;

/* RTC Minute Register (MINUTE) */
/* RTC Alarm Minute Register (ALARMMINUTE) */
typedef struct {
  __REG32 MIN0                    : 4;
  __REG32 MIN1                    : 3;
  __REG32                         :25;
} __rtc_minute_bits;

/* RTC Hour Register (HOUR) */
/* RTC Alarm Hour Register (ALARMHOUR) */
typedef struct {
  __REG32 HOUR0                   : 4;
  __REG32 HOUR1                   : 2;
  __REG32                         : 1;
  __REG32 PM_NAM                  : 1;
  __REG32                         :24;
} __rtc_hour_bits;

/* RTC Day of the Month Register (DAY) */
/* RTC Alarm Day of the Month Register (ALARMDAY) */
typedef struct {
  __REG32 DAY0                    : 4;
  __REG32 DAY1                    : 2;
  __REG32                         :26;
} __rtc_day_bits;

/* RTC Month Register (MONTH) */
/* RTC Alarm Month Register (ALARMMONTH) */
typedef struct {
  __REG32 MONTH0                  : 4;
  __REG32 MONTH1                  : 1;
  __REG32                         :27;
} __rtc_month_bits;

/* RTC Year Register (YEAR) */
/* RTC Alarm Year Register (ALARMYEAR) */
typedef struct {
  __REG32 YEAR0                   : 4;
  __REG32 YEAR1                   : 4;
  __REG32                         :24;
} __rtc_year_bits;

/* RTC Day of the Week Register (DOTW) */
typedef struct {
  __REG32 WEEK                    : 3;
  __REG32                         :29;
} __rtc_dotw_bits;

/* RTC Control Register (CTRL) */
typedef struct {
  __REG32 STOP_RTC                : 1;
  __REG32 ROUND_30S               : 1;
  __REG32 AUTO_COMP               : 1;
  __REG32 MODE_12_24              : 1;
  __REG32 TEST_MODE               : 1;
  __REG32 SET_32_COUNTER          : 1;
  __REG32 RTC_DISABLE             : 1;
  __REG32                         :25;
} __rtc_ctrl_bits;

/* RTC Status Register (STATUS) */
typedef struct {
  __REG32 BUSY                    : 1;
  __REG32 RUN                     : 1;
  __REG32 _1S_EVENT               : 1;
  __REG32 _1M_EVENT               : 1;
  __REG32 _1H_EVENT               : 1;
  __REG32 _1D_EVENT               : 1;
  __REG32 ALARM                   : 1;
  __REG32 ALARM2                  : 1;
  __REG32                         :24;
} __rtc_status_bits;

/* RTC Interrupt Register (INTERRUPT) */
typedef struct {
  __REG32 EVERY                   : 2;
  __REG32 IT_TIMER                : 1;
  __REG32 IT_ALARM                : 1;
  __REG32 IT_ALARM2               : 1;
  __REG32                         :27;
} __rtc_interrupt_bits;

/* RTC Compensation (LSB) Register (COMPLSB) */
typedef struct {
  __REG32 RTC_COMP_LSB            : 8;
  __REG32                         :24;
} __rtc_complsb_bits;

/* RTC Compensation (MSB) Register (COMPMSB) */
typedef struct {
  __REG32 RTC_COMP_MSB            : 8;
  __REG32                         :24;
} __rtc_compmsb_bits;

/* RTC Oscillator Register (OSC) */
typedef struct {
  __REG32 SW1                     : 1;
  __REG32 SW2                     : 1;
  __REG32 RES_SELECT              : 1;
  __REG32 _32KCLK_SEL             : 1;
  __REG32 OSC32K_GZ               : 1;
  __REG32                         : 1;
  __REG32 _32KCLK_EN              : 1;
  __REG32                         :25;
} __rtc_osc_bits;

/* RTC Revision Register (RTC_REVISION) */
typedef struct {
  __REG32 Y_MINOR                 : 6;
  __REG32 CUSTOM                  : 2;
  __REG32 X_MAJOR                 : 5;
  __REG32 R_RTL                   : 3;
  __REG32 FUNC                    :12;
  __REG32                         : 2;
  __REG32 SCHEME                  : 2;
} __rtc_revision_bits;

/*System Configuration Register (RTC_SYSCONFIG) */
typedef struct {
  __REG32 IDLEMODE                : 2;
  __REG32                         :30;
} __rtc_sysconfig_bits;

/*Wakeup Enable Register (RTC_IRQWAKEEN) */
typedef struct {
  __REG32 TIMER_WAKEEN            : 1;
  __REG32 ALARM_WAKEEN            : 1;
  __REG32                         :30;
} __rtc_irqwakeen_bits;

/*RTC PMIC Register (RTC_PMIC) */
typedef struct {
  __REG32 EXT_WAKEUP_EN0          : 1;
  __REG32 EXT_WAKEUP_EN1          : 1;
  __REG32 EXT_WAKEUP_EN2          : 1;
  __REG32 EXT_WAKEUP_EN3          : 1;
  __REG32 EXT_WAKEUP_POL0         : 1;
  __REG32 EXT_WAKEUP_POL1         : 1;
  __REG32 EXT_WAKEUP_POL2         : 1;
  __REG32 EXT_WAKEUP_POL3         : 1;
  __REG32 EXT_WAKEUP_DB_EN0       : 1;
  __REG32 EXT_WAKEUP_DB_EN1       : 1;
  __REG32 EXT_WAKEUP_DB_EN2       : 1;
  __REG32 EXT_WAKEUP_DB_EN3       : 1;
  __REG32 EXT_WAKEUP_STATUS0      : 1;
  __REG32 EXT_WAKEUP_STATUS1      : 1;
  __REG32 EXT_WAKEUP_STATUS2      : 1;
  __REG32 EXT_WAKEUP_STATUS3      : 1;
  __REG32 PWR_ENABLE_EN           : 1;
  __REG32 PWR_ENABLE_SM           : 2;
  __REG32                         :13;
} __rtc_pmic_bits;

#define RTC_PMIC_EXT_WAKEUP_STATUS0 (0x1UL<<12)
#define RTC_PMIC_EXT_WAKEUP_STATUS1 (0x1UL<<13)
#define RTC_PMIC_EXT_WAKEUP_STATUS2 (0x1UL<<14)
#define RTC_PMIC_EXT_WAKEUP_STATUS3 (0x1UL<<15)

/*RTC Debounce Register (RTC_DEBOUNCE) */
typedef struct {
  __REG32 DEBOUNCE_REG            : 8;
  __REG32                         :24;
} __rtc_debounce_bits;

/* WDTn_WDSC */
typedef struct {
   __REG32                        : 1;
   __REG32 SOFTRESET              : 1;
   __REG32                        : 3;
   __REG32 EMUFREE                : 1;
   __REG32                        : 2;
   __REG32 CLOCKACTIVITY          : 2;
   __REG32                        :22;
} __wdt_wdsc_bits;

/* WDTn_WDST */
typedef struct {
   __REG32 RESETDONE              : 1;
   __REG32                        :31;
} __wdt_wdst_bits;

/* WDTn_WISR */
typedef struct {
   __REG32 OVF_IT_FLAG            : 1;
   __REG32 DLY_IT_FLAG            : 1;
   __REG32                        :30;
} __wdt_wisr_bits;

/* WDTn_WIER */
typedef struct {
   __REG32 OVF_IT_ENA             : 1;
   __REG32 DLY_IT_ENA             : 1;
   __REG32                        :30;
} __wdt_wier_bits;

/* WDTn_WCLR */
typedef struct {
   __REG32                        : 2;
   __REG32 PTV                    : 3;
   __REG32 PRE                    : 1;
   __REG32                        :26;
} __wdt_wclr_bits;

/* WDTn_WWPS */
typedef struct {
   __REG32 W_PEND_WCLR            : 1;
   __REG32 W_PEND_WCRR            : 1;
   __REG32 W_PEND_WLDR            : 1;
   __REG32 W_PEND_WTGR            : 1;
   __REG32 W_PEND_WSPR            : 1;
   __REG32 W_PEND_WDLY            : 1;
   __REG32                        :26;
} __wdt_wwps_bits;

/* WDTn_WIRQSTATRAW */
/* WDTn_WIRQSTAT */
typedef struct {
   __REG32 EVENT_OVF              : 1;
   __REG32 EVENT_DLY              : 1;
   __REG32                        :30;
} __wdt_wirqstat_bits;

/* WDTn_WIRQENSET */
/* WDTn_WIRQENCLR */
typedef struct {
   __REG32 EVENT_OVF              : 1;
   __REG32 EVENT_DLY              : 1;
   __REG32                        :30;
} __wdt_wirqen_bits;

#define WDT_WIRQ_EVENT_OVF        (0x1UL<<0)
#define WDT_WIRQ_EVENT_DLY        (0x1UL<<1)

/* I2C_REVNB_LO */
typedef struct {
   __REG32 MINOR                  : 6;
   __REG32 CUSTOM                 : 2;
   __REG32 MAJOR                  : 3;
   __REG32 RTL                    : 5;
   __REG32                        :16;
} __i2c_revnb_lo_bits;

/* I2C_REVNB_HI */
typedef struct {
   __REG32 FUNC                   :12;
   __REG32 CUSTOM                 : 2;
   __REG32 SCHEME                 : 2;
   __REG32                        :16;
} __i2c_revnb_hi_bits;

/* I2C_SYSC */
typedef struct {
   __REG32 AUTOIDLE               : 1;
   __REG32 SRST                   : 1;
   __REG32 ENAWAKEUP              : 1;
   __REG32 IDLEMODE               : 2;
   __REG32                        : 3;
   __REG32 CLOCKACTIVITY          : 2;
   __REG32                        :22;
} __i2c_sysc_bits;

/* I2C_EOI */
typedef struct {
   __REG32 LINE_NUMBER            : 1;
   __REG32                        :31;
} __i2c_eoi_bits;

/* I2C_IRQSTATUS_RAW */
/* I2C_IRQSTATUS */
typedef struct {
   __REG32 AL                     : 1;
   __REG32 NACK                   : 1;
   __REG32 ARDY                   : 1;
   __REG32 RRDY                   : 1;
   __REG32 XRDY                   : 1;
   __REG32 GC                     : 1;
   __REG32 STC                    : 1;
   __REG32 AERR                   : 1;
   __REG32 BF                     : 1;
   __REG32 AAS                    : 1;
   __REG32 XUDF                   : 1;
   __REG32 ROVR                   : 1;
   __REG32 BB                     : 1;
   __REG32 RDR                    : 1;
   __REG32 XDR                    : 1;
   __REG32                        :17;
} __i2c_irqstatus_bits;

/* I2C_IRQENABLE_SET */
/* I2C_IRQENABLE_CLR */
typedef struct {
   __REG32 AL_IE                  : 1;
   __REG32 NACK_IE                : 1;
   __REG32 ARDY_IE                : 1;
   __REG32 RRDY_IE                : 1;
   __REG32 XRDY_IE                : 1;
   __REG32 GC_IE                  : 1;
   __REG32 STC_IE                 : 1;
   __REG32 AERR_IE                : 1;
   __REG32 BF_IE                  : 1;
   __REG32 AAS_IE                 : 1;
   __REG32 XUDF_IE                : 1;
   __REG32 ROVR_IE                : 1;
   __REG32                        : 1;
   __REG32 RDR_IE                 : 1;
   __REG32 XDR_IE                 : 1;
   __REG32                        :17;
} __i2c_irqenable_bits;

#define I2C_IRQ_AL                (0x1UL<<0)
#define I2C_IRQ_NACK              (0x1UL<<1)
#define I2C_IRQ_ARDY              (0x1UL<<2)
#define I2C_IRQ_RRDY              (0x1UL<<3)
#define I2C_IRQ_XRDY              (0x1UL<<4)
#define I2C_IRQ_GC                (0x1UL<<5)
#define I2C_IRQ_STC               (0x1UL<<6)
#define I2C_IRQ_AERR              (0x1UL<<7)
#define I2C_IRQ_BF                (0x1UL<<8)
#define I2C_IRQ_AAS               (0x1UL<<9)
#define I2C_IRQ_XUDF              (0x1UL<<10)
#define I2C_IRQ_ROVR              (0x1UL<<11)
#define I2C_IRQ_BB                (0x1UL<<12)
#define I2C_IRQ_RDR               (0x1UL<<13)
#define I2C_IRQ_XDR               (0x1UL<<14)

/* I2C_WE */
typedef struct {
   __REG32 AL_WE                  : 1;
   __REG32 NACK_WE                : 1;
   __REG32 ARDY_WE                : 1;
   __REG32 DRDY_WE                : 1;
   __REG32                        : 1;
   __REG32 GC_WE                  : 1;
   __REG32 STC_WE                 : 1;
   __REG32                        : 1;
   __REG32 BF_WE                  : 1;
   __REG32 AAS_WE                 : 1;
   __REG32 XUDF_WE                : 1;
   __REG32 ROVR_WE                : 1;
   __REG32                        : 1;
   __REG32 RDR_WE                 : 1;
   __REG32 XDR_WE                 : 1;
   __REG32                        :17;
} __i2c_we_bits;

/* I2C_WE */
typedef struct {
   __REG32 DMARX_ENABLE_SET       : 1;
   __REG32                        :31;
} __i2c_dmarxenable_set_bits;

/* I2C_WE */
typedef struct {
   __REG32 DMATX_ENABLE_SET       : 1;
   __REG32                        :31;
} __i2c_dmatxenable_set_bits;

/* I2C_WE */
typedef struct {
   __REG32 DMARX_ENABLE_CLEAR     : 1;
   __REG32                        :31;
} __i2c_dmarxenable_clr_bits;

/* I2C_WE */
typedef struct {
   __REG32 DMARX_ENABLE_CLEAR     : 1;
   __REG32                        :31;
} __i2c_dmatxenable_clr_bits;

#define I2C_DMATX_ENABLE          (0x1UL<<0)
#define I2C_DMARX_ENABLE          (0x1UL<<0)

/* I2C_DMATXWAKE_EN */
/* I2C_DMARXWAKE_EN */
typedef struct {
   __REG32 AL                     : 1;
   __REG32 NACK                   : 1;
   __REG32 ARDY                   : 1;
   __REG32 DRDY                   : 1;
   __REG32                        : 1;
   __REG32 GC                     : 1;
   __REG32 STC                    : 1;
   __REG32                        : 1;
   __REG32 BF                     : 1;
   __REG32 AAS                    : 1;
   __REG32 XUDF                   : 1;
   __REG32 ROVR                   : 1;
   __REG32                        : 1;
   __REG32 RDR                    : 1;
   __REG32 XDR                    : 1;
   __REG32                        :17;
} __i2c_dmawake_en_bits;

/* I2C_SYSS */
typedef struct {
   __REG32 RDONE                  : 1;
   __REG32                        :31;
} __i2c_syss_bits;

/* I2C_BUF */
typedef struct {
   __REG32 TXTRSH                 : 6;
   __REG32 TXFIFO_CLR             : 1;
   __REG32 XDMA_EN                : 1;
   __REG32 RXTRSH                 : 6;
   __REG32 RXFIFO_CLR             : 1;
   __REG32 RDMA_EN                : 1;
   __REG32                        :16;
} __i2c_buf_bits;

/* I2C_BUF */
typedef struct {
   __REG32 DCOUNT                 :16;
   __REG32                        :16;
} __i2c_cnt_bits;

/* I2C_DATA */
typedef struct {
   __REG32 DATA                   : 8;
   __REG32                        :24;
} __i2c_data_bits;

/* I2C_CON */
typedef struct {
   __REG32 STT                    : 1;
   __REG32 STP                    : 1;
   __REG32                        : 2;
   __REG32 XOA3                   : 1;
   __REG32 XOA2                   : 1;
   __REG32 XOA1                   : 1;
   __REG32 XOA0                   : 1;
   __REG32 XSA                    : 1;
   __REG32 TRX                    : 1;
   __REG32 MST                    : 1;
   __REG32 STB                    : 1;
   __REG32 OPMODE                 : 2;
   __REG32                        : 1;
   __REG32 I2C_EN                 : 1;
   __REG32                        :16;
} __i2c_con_bits;

/* I2C_OA */
typedef struct {
   __REG32 OA                     :10;
   __REG32                        :22;
} __i2c_oa_bits;

/* I2C_SA */
typedef struct {
   __REG32 SA                     :10;
   __REG32                        :22;
} __i2c_sa_bits;

/* I2C_PSC */
typedef struct {
   __REG32 PSC                    : 8;
   __REG32                        :24;
} __i2c_psc_bits;

/* I2C_SCLL */
typedef struct {
   __REG32 SCLL                   : 8;
   __REG32                        :24;
} __i2c_scll_bits;

/* I2C_SCLH */
typedef struct {
   __REG32 SCLH                   : 8;
   __REG32                        :24;
} __i2c_sclh_bits;

/* I2C_SYSTEST */
typedef struct {
   __REG32 SDA_O                  : 1;
   __REG32 SDA_I                  : 1;
   __REG32 SCL_O                  : 1;
   __REG32 SCL_I                  : 1;
   __REG32                        : 1;
   __REG32 SDA_O_FUNC             : 1;
   __REG32 SDA_I_FUNC             : 1;
   __REG32 SCL_O_FUNC             : 1;
   __REG32 SCL_I_FUNC             : 1;
   __REG32                        : 2;
   __REG32 SSB                    : 1;
   __REG32 TMODE                  : 2;
   __REG32 FREE                   : 1;
   __REG32 ST_EN                  : 1;
   __REG32                        :16;
} __i2c_systest_bits;

/* I2C_BUFSTAT */
typedef struct {
   __REG32 TXSTAT                 : 6;
   __REG32                        : 2;
   __REG32 RXSTAT                 : 6;
   __REG32 FIFODEPTH              : 2;
   __REG32                        :16;
} __i2c_bufstat_bits;

/* I2C_OA1 */
typedef struct {
   __REG32 OA1                    :10;
   __REG32                        :22;
} __i2c_oa1_bits;

/* I2C_OA2 */
typedef struct {
   __REG32 OA2                    :10;
   __REG32                        :22;
} __i2c_oa2_bits;

/* I2C_OA3 */
typedef struct {
   __REG32 OA3                    :10;
   __REG32                        :22;
} __i2c_oa3_bits;

/* I2C_ACTOA */
typedef struct {
   __REG32 OA0_ACT                : 1;
   __REG32 OA1_ACT                : 1;
   __REG32 OA2_ACT                : 1;
   __REG32 OA3_ACT                : 1;
   __REG32                        :28;
} __i2c_actoa_bits;

/* I2Cn_SBLOCK */
typedef struct {
   __REG32 OA0_EN                 : 1;
   __REG32 OA1_EN                 : 1;
   __REG32 OA2_EN                 : 1;
   __REG32 OA3_EN                 : 1;
   __REG32                        :28;
} __i2c_sblock_bits;

/* McASP Pin Function Register (PFUNC) */
typedef struct {
  __REG32 AXR0                    : 1;
  __REG32 AXR1                    : 1;
  __REG32 AXR2                    : 1 ;
  __REG32 AXR3                    : 1;
  __REG32 AXR4                    : 1;
  __REG32 AXR5                    : 1;
  __REG32                         :19;
  __REG32 AMUTE                   : 1;
  __REG32 ACLKX                   : 1;
  __REG32 AHCLKX                  : 1;
  __REG32 AFSX                    : 1;
  __REG32 ACLKR                   : 1;
  __REG32 AHCLKR                  : 1;
  __REG32 AFSR                    : 1;
} __mcasp_pfunc_bits;

/* McASP Global Control Register (GBLCTL) */
typedef struct {
  __REG32 RCLKRST                 : 1;
  __REG32 RHCLKRST                : 1;
  __REG32 RSRCLR                  : 1;
  __REG32 RSMRST                  : 1;
  __REG32 RFRST                   : 1;
  __REG32                         : 3;
  __REG32 XCLKRST                 : 1;
  __REG32 XHCLKRST                : 1;
  __REG32 XSRCLR                  : 1;
  __REG32 XSMRST                  : 1;
  __REG32 XFRST                   : 1;
  __REG32                         :19;
} __mcasp_gblctl_bits;

/* McASP Audio Mute Control Register (AMUTE) */
typedef struct {
  __REG32 MUTEN                   : 2;
  __REG32 INPOL                   : 1;
  __REG32 INEN                    : 1;
  __REG32 INSTAT                  : 1;
  __REG32 ROVRN                   : 1;
  __REG32 XUNDRN                  : 1;
  __REG32 RSYNCERR                : 1;
  __REG32 XSYNCERR                : 1;
  __REG32 RCKFAIL                 : 1;
  __REG32 XCKFAIL                 : 1;
  __REG32 RDMAERR                 : 1;
  __REG32 XDMAERR                 : 1;
  __REG32                         :19;
} __mcasp_amute_bits;

/* McASP Digital Loopback Control Register (DLBCTL) */
typedef struct {
  __REG32 DLBEN                   : 1;
  __REG32 ORD                     : 1;
  __REG32 MODE                    : 2;
  __REG32                         :28;
} __mcasp_dlbctl_bits;

/* McASP Digital Mode Control Register (DITCTL) */
typedef struct {
  __REG32 DITEN                   : 1;
  __REG32                         : 1;
  __REG32 VA                      : 1;
  __REG32 VB                      : 1;
  __REG32                         :28;
} __mcasp_ditctl_bits;

/* McASP Receiver Global Control Register (RGBLCTL) */
typedef struct {
  __REG32 RCLKRST                 : 1;
  __REG32 RHCLKRST                : 1;
  __REG32 RSRCLR                  : 1;
  __REG32 RSMRST                  : 1;
  __REG32 RFRST                   : 1;
  __REG32                         : 3;
  __REG32 XCLKRST                 : 1;
  __REG32 XHCLKRST                : 1;
  __REG32 XSRCLR                  : 1;
  __REG32 XSMRST                  : 1;
  __REG32 XFRST                   : 1;
  __REG32                         :19;
} __mcasp_rgblctl_bits;

/* McASP Receive Format Unit Bit Mask Register (RMASK) */
typedef struct {
  __REG32 RMASK0                  : 1;
  __REG32 RMASK1                  : 1;
  __REG32 RMASK2                  : 1;
  __REG32 RMASK3                  : 1;
  __REG32 RMASK4                  : 1;
  __REG32 RMASK5                  : 1;
  __REG32 RMASK6                  : 1;
  __REG32 RMASK7                  : 1;
  __REG32 RMASK8                  : 1;
  __REG32 RMASK9                  : 1;
  __REG32 RMASK10                 : 1;
  __REG32 RMASK11                 : 1;
  __REG32 RMASK12                 : 1;
  __REG32 RMASK13                 : 1;
  __REG32 RMASK14                 : 1;
  __REG32 RMASK15                 : 1;
  __REG32 RMASK16                 : 1;
  __REG32 RMASK17                 : 1;
  __REG32 RMASK18                 : 1;
  __REG32 RMASK19                 : 1;
  __REG32 RMASK20                 : 1;
  __REG32 RMASK21                 : 1;
  __REG32 RMASK22                 : 1;
  __REG32 RMASK23                 : 1;
  __REG32 RMASK24                 : 1;
  __REG32 RMASK25                 : 1;
  __REG32 RMASK26                 : 1;
  __REG32 RMASK27                 : 1;
  __REG32 RMASK28                 : 1;
  __REG32 RMASK29                 : 1;
  __REG32 RMASK30                 : 1;
  __REG32 RMASK31                 : 1;
} __mcasp_rmask_bits;

/* McASP Receive Bit Stream Format Register (RFMT) */
typedef struct {
  __REG32 RROT                    : 3;
  __REG32 RBUSEL                  : 1;
  __REG32 RSSZ                    : 4;
  __REG32 RPBIT                   : 5;
  __REG32 RPAD                    : 2;
  __REG32 RRVRS                   : 1;
  __REG32 RDATDLY                 : 2;
  __REG32                         :14;
} __mcasp_rfmt_bits;

/* Receive Frame Sync Control Register (AFSRCTL) */
typedef struct {
  __REG32 FSRP                    : 1;
  __REG32 FSRM                    : 1;
  __REG32                         : 2;
  __REG32 FRWID                   : 1;
  __REG32                         : 2;
  __REG32 RMOD                    : 9;
  __REG32                         :16;
} __mcasp_afsrctl_bits;

/* McASP Receive Clock Control Register (ACLKRCTL) */
typedef struct {
  __REG32 CLKRDIV                 : 5;
  __REG32 CLKRM                   : 1;
  __REG32                         : 1;
  __REG32 CLKRP                   : 1;
  __REG32                         :24;
} __mcasp_aclkrctl_bits;

/* McASP Receive Clock Control Register (ACLKRCTL) */
typedef struct {
  __REG32 HCLKRDIV                :12;
  __REG32                         : 2;
  __REG32 HCLKRP                  : 1;
  __REG32 HCLKRM                  : 1;
  __REG32                         :16;
} __mcasp_ahclkrctl_bits;

/* McASP Receive TDM Time Slot Register (RTDM) */
typedef struct {
  __REG32 RTDMS0                  : 1;
  __REG32 RTDMS1                  : 1;
  __REG32 RTDMS2                  : 1;
  __REG32 RTDMS3                  : 1;
  __REG32 RTDMS4                  : 1;
  __REG32 RTDMS5                  : 1;
  __REG32 RTDMS6                  : 1;
  __REG32 RTDMS7                  : 1;
  __REG32 RTDMS8                  : 1;
  __REG32 RTDMS9                  : 1;
  __REG32 RTDMS10                 : 1;
  __REG32 RTDMS11                 : 1;
  __REG32 RTDMS12                 : 1;
  __REG32 RTDMS13                 : 1;
  __REG32 RTDMS14                 : 1;
  __REG32 RTDMS15                 : 1;
  __REG32 RTDMS16                 : 1;
  __REG32 RTDMS17                 : 1;
  __REG32 RTDMS18                 : 1;
  __REG32 RTDMS19                 : 1;
  __REG32 RTDMS20                 : 1;
  __REG32 RTDMS21                 : 1;
  __REG32 RTDMS22                 : 1;
  __REG32 RTDMS23                 : 1;
  __REG32 RTDMS24                 : 1;
  __REG32 RTDMS25                 : 1;
  __REG32 RTDMS26                 : 1;
  __REG32 RTDMS27                 : 1;
  __REG32 RTDMS28                 : 1;
  __REG32 RTDMS29                 : 1;
  __REG32 RTDMS30                 : 1;
  __REG32 RTDMS31                 : 1;
} __mcasp_rtdm_bits;

/* McASP Receiver Interrupt Control Register (RINTCTL) */
typedef struct {
  __REG32 ROVRN                   : 1;
  __REG32 RSYNCERR                : 1;
  __REG32 RCKFAIL                 : 1;
  __REG32 RDMAERR                 : 1;
  __REG32 RLAST                   : 1;
  __REG32 RDATA                   : 1;
  __REG32                         : 1;
  __REG32 RSTAFRM                 : 1;
  __REG32                         :24;
} __mcasp_rintctl_bits;

/* McASP Receiver Status Register (RSTAT) */
typedef struct {
  __REG32 ROVRN                   : 1;
  __REG32 RSYNCERR                : 1;
  __REG32 RCKFAIL                 : 1;
  __REG32 RTDMSLOT                : 1;
  __REG32 RLAST                   : 1;
  __REG32 RDATA                   : 1;
  __REG32 RSTAFRM                 : 1;
  __REG32 RDMAERR                 : 1;
  __REG32 RERR                    : 1;
  __REG32                         :23;
} __mcasp_rstat_bits;

/* McASP Current Receive TDM Time Slot Registers (RSLOT) */
typedef struct {
  __REG32 RSLOTCNT                : 9;
  __REG32                         :23;
} __mcasp_rslot_bits;

/* McASP Receive Clock Check Control Register (RCLKCHK) */
typedef struct {
  __REG32 RPS                     : 4;
  __REG32                         : 4;
  __REG32 RMIN                    : 8;
  __REG32 RMAX                    : 8;
  __REG32 RCNT                    : 8;
} __mcasp_rclkchk_bits;

/* McASP Receiver DMA Event Control Register (REVTCTL) */
typedef struct {
  __REG32 RDATDMA                 : 1;
  __REG32                         :31;
} __mcasp_revtctl_bits;

/* McASP Transmitter Global Control Register (XGBLCTL) */
typedef struct {
  __REG32 RCLKRST                 : 1;
  __REG32 RHCLKRST                : 1;
  __REG32 RSRCLR                  : 1;
  __REG32 RSMRST                  : 1;
  __REG32 RFRST                   : 1;
  __REG32                         : 3;
  __REG32 XCLKRST                 : 1;
  __REG32 XHCLKRST                : 1;
  __REG32 XSRCLR                  : 1;
  __REG32 XSMRST                  : 1;
  __REG32 XFRST                   : 1;
  __REG32                         :19;
} __mcasp_xgblctl_bits;

/* McASP Transmit Format Unit Bit Mask Register (XMASK) */
typedef struct {
  __REG32 XMASK0                  : 1;
  __REG32 XMASK1                  : 1;
  __REG32 XMASK2                  : 1;
  __REG32 XMASK3                  : 1;
  __REG32 XMASK4                  : 1;
  __REG32 XMASK5                  : 1;
  __REG32 XMASK6                  : 1;
  __REG32 XMASK7                  : 1;
  __REG32 XMASK8                  : 1;
  __REG32 XMASK9                  : 1;
  __REG32 XMASK10                 : 1;
  __REG32 XMASK11                 : 1;
  __REG32 XMASK12                 : 1;
  __REG32 XMASK13                 : 1;
  __REG32 XMASK14                 : 1;
  __REG32 XMASK15                 : 1;
  __REG32 XMASK16                 : 1;
  __REG32 XMASK17                 : 1;
  __REG32 XMASK18                 : 1;
  __REG32 XMASK19                 : 1;
  __REG32 XMASK20                 : 1;
  __REG32 XMASK21                 : 1;
  __REG32 XMASK22                 : 1;
  __REG32 XMASK23                 : 1;
  __REG32 XMASK24                 : 1;
  __REG32 XMASK25                 : 1;
  __REG32 XMASK26                 : 1;
  __REG32 XMASK27                 : 1;
  __REG32 XMASK28                 : 1;
  __REG32 XMASK29                 : 1;
  __REG32 XMASK30                 : 1;
  __REG32 XMASK31                 : 1;
} __mcasp_xmask_bits;

/* McASP Transmit Bit Stream Format Register (XFMT) */
typedef struct {
  __REG32 XROT                    : 3;
  __REG32 XBUSEL                  : 1;
  __REG32 XSSZ                    : 4;
  __REG32 XPBIT                   : 5;
  __REG32 XPAD                    : 2;
  __REG32 XRVRS                   : 1;
  __REG32 XDATDLY                 : 2;
  __REG32                         :14;
} __mcasp_xfmt_bits;

/* McASP Transmit Frame Sync Control Register (AFSXCTL) */
typedef struct {
  __REG32 FSXP                    : 1;
  __REG32 FSXM                    : 1;
  __REG32                         : 2;
  __REG32 FXWID                   : 1;
  __REG32                         : 2;
  __REG32 XMOD                    : 9;
  __REG32                         :16;
} __mcasp_afsxctl_bits;

/* McASP Transmit Clock Control Register (ACLKXCTL) */
typedef struct {
  __REG32 CLKXDIV                 : 5;
  __REG32 CLKXM                   : 1;
  __REG32 ASYNC                   : 1;
  __REG32 CLKXP                   : 1;
  __REG32                         :24;
} __mcasp_aclkxctl_bits;

/* McASP Transmit High-Frequency Clock Control Register (AHCLKXCTL) */
typedef struct {
  __REG32 HCLKXDIV                :12;
  __REG32                         : 2;
  __REG32 HCLKXP                  : 1;
  __REG32 HCLKXM                  : 1;
  __REG32                         :16;
} __mcasp_ahclkxctl_bits;

/* McASP Transmit TDM Time Slot Register (XTDM) */
typedef struct {
  __REG32 XTDMS0                  : 1;
  __REG32 XTDMS1                  : 1;
  __REG32 XTDMS2                  : 1;
  __REG32 XTDMS3                  : 1;
  __REG32 XTDMS4                  : 1;
  __REG32 XTDMS5                  : 1;
  __REG32 XTDMS6                  : 1;
  __REG32 XTDMS7                  : 1;
  __REG32 XTDMS8                  : 1;
  __REG32 XTDMS9                  : 1;
  __REG32 XTDMS10                 : 1;
  __REG32 XTDMS11                 : 1;
  __REG32 XTDMS12                 : 1;
  __REG32 XTDMS13                 : 1;
  __REG32 XTDMS14                 : 1;
  __REG32 XTDMS15                 : 1;
  __REG32 XTDMS16                 : 1;
  __REG32 XTDMS17                 : 1;
  __REG32 XTDMS18                 : 1;
  __REG32 XTDMS19                 : 1;
  __REG32 XTDMS20                 : 1;
  __REG32 XTDMS21                 : 1;
  __REG32 XTDMS22                 : 1;
  __REG32 XTDMS23                 : 1;
  __REG32 XTDMS24                 : 1;
  __REG32 XTDMS25                 : 1;
  __REG32 XTDMS26                 : 1;
  __REG32 XTDMS27                 : 1;
  __REG32 XTDMS28                 : 1;
  __REG32 XTDMS29                 : 1;
  __REG32 XTDMS30                 : 1;
  __REG32 XTDMS31                 : 1;
} __mcasp_xtdm_bits;

/* McASP Transmitter Interrupt Control Register (XINTCTL) */
typedef struct {
  __REG32 XUNDRN                  : 1;
  __REG32 XSYNCERR                : 1;
  __REG32 XCKFAIL                 : 1;
  __REG32 XDMAERR                 : 1;
  __REG32 XLAST                   : 1;
  __REG32 XDATA                   : 1;
  __REG32                         : 1;
  __REG32 XSTAFRM                 : 1;
  __REG32                         :24;
} __mcasp_xintctl_bits;

/* McASP Transmitter Status Register (XSTAT) */
typedef struct {
  __REG32 XUNDRN                  : 1;
  __REG32 XSYNCERR                : 1;
  __REG32 XCKFAIL                 : 1;
  __REG32 XTDMSLOT                : 1;
  __REG32 XLAST                   : 1;
  __REG32 XDATA                   : 1;
  __REG32 XSTAFRM                 : 1;
  __REG32 XDMAERR                 : 1;
  __REG32 XERR                    : 1;
  __REG32                         :23;
} __mcasp_xstat_bits;

/* McASP Current Transmit TDM Time Slot Register (XSLOT) */
typedef struct {
  __REG32 XSLOTCNT                :10;
  __REG32                         :22;
} __mcasp_xslot_bits;

/* McASP Transmit Clock Check Control Register (XCLKCHK) */
typedef struct {
  __REG32 XPS                     : 4;
  __REG32                         : 4;
  __REG32 XMIN                    : 8;
  __REG32 XMAX                    : 8;
  __REG32 XCNT                    : 8;
} __mcasp_xclkchk_bits;

/* McASP Transmitter DMA Event Control Register (XEVTCTL) */
typedef struct {
  __REG32 XDATDMA                 : 1;
  __REG32                         :31;
} __mcasp_xevtctl_bits;

/* McASP Serializer Control Registers (SRCTLn) */
typedef struct {
  __REG32 SRMOD                   : 2;
  __REG32 DISMOD                  : 2;
  __REG32 XRDY                    : 1;
  __REG32 RRDY                    : 1;
  __REG32                         :26;
} __mcasp_srctl_bits;

/* McASP Write FIFO Control Register (WFIFOCTL) */
typedef struct {
  __REG32 WNUMDMA                 : 8;
  __REG32 WNUMEVT                 : 8;
  __REG32 WENA                    : 1;
  __REG32                         :15;
} __mcasp_wfifoctl_bits;

/* McASP Write FIFO Status Register (WFIFOSTS) */
typedef struct {
  __REG32 WLVL                    : 8;
  __REG32                         :24;
} __mcasp_wfifosts_bits;

/* McASP Read FIFO Control Register (RFIFOCTL) */
typedef struct {
  __REG32 RNUMDMA                 : 8;
  __REG32 RNUMEVT                 : 8;
  __REG32 RENA                    : 1;
  __REG32                         :15;
} __mcasp_rfifoctl_bits;

/* McASP Read FIFO Status Register (RFIFOSTS) */
typedef struct {
  __REG32 RLVL                    : 8;
  __REG32                         :24;
} __mcasp_rfifosts_bits;

/* CAN Control Register (DCAN CTL) */
typedef struct {
  __REG32 Init                : 1;
  __REG32 IE0                 : 1;
  __REG32 SIE                 : 1;
  __REG32 EIE                 : 1;
  __REG32                     : 1;
  __REG32 DAR                 : 1;
  __REG32 CCE                 : 1;
  __REG32 Test                : 1;
  __REG32 IDS                 : 1;
  __REG32 ABO                 : 1;
  __REG32 PMD                 : 4;
  __REG32                     : 1;
  __REG32 SWR                 : 1;
  __REG32 InitDbg             : 1;
  __REG32 IE1                 : 1;
  __REG32 DE1                 : 1;
  __REG32 DE2                 : 1;
  __REG32 DE3                 : 1;
  __REG32                     : 3;
  __REG32 PDR                 : 1;
  __REG32 WUBA                : 1;
  __REG32                     : 6;
} __dcanctl_bits;

/* CAN Error and Status Register (DCAN ES) */
typedef struct {
  __REG32 LEC                 : 3;
  __REG32 TxOK                : 1;
  __REG32 RxOK                : 1;
  __REG32 EPass               : 1;
  __REG32 EWarn               : 1;
  __REG32 BOff                : 1;
  __REG32 PER                 : 1;
  __REG32 WakeUpPnd           : 1;
  __REG32 PDA                 : 1;
  __REG32                     :21;
} __dcanes_bits;

#define DCAN_PARITYERR_EOI_PARITYERR_EOI  (0x1UL<<8)

/* CAN Error Counter Register (DCAN ERRC) */
typedef struct {
  __REG32 TEC                 : 8;
  __REG32 REC                 : 7;
  __REG32 RP                  : 1;
  __REG32                     :16;
} __dcanerrc_bits;

/* CAN Bit Timing Register (DCAN BTR) */
typedef struct {
  __REG32 BRP                 : 6;
  __REG32 SJW                 : 2;
  __REG32 TSeg1               : 4;
  __REG32 TSeg2               : 3;
  __REG32                     : 1;
  __REG32 BRPE                : 4;
  __REG32                     :12;
} __dcanbtr_bits;

/* CAN Interrupt Register (DCANINT) */
typedef struct {
  __REG32 Int0ID              :16;
  __REG32 Int1ID              : 8;
  __REG32                     : 8;
} __dcanint_bits;

/* CAN Test Register (DCAN TEST) */
typedef struct {
  __REG32                     : 3;
  __REG32 Silent              : 1;
  __REG32 LBack               : 1;
  __REG32 Tx                  : 2;
  __REG32 Rx                  : 1;
  __REG32 EXL                 : 1;
  __REG32 RDA                 : 1;
  __REG32                     :22;
} __dcantest_bits;

/* CAN Parity Error Code Register (DCAN PERR) */
typedef struct {
  __REG32 MessageNumber       : 8;
  __REG32 WordNumber          : 3;
  __REG32                     :21;
} __dcanperr_bits;

/* CAN Transmission Request X Register (DCAN TXRQ X) */
typedef struct {
  __REG32 TxRqstReg1          : 2;
  __REG32 TxRqstReg2          : 2;
  __REG32 TxRqstReg3          : 2;
  __REG32 TxRqstReg4          : 2;
  __REG32 TxRqstReg5          : 2;
  __REG32 TxRqstReg6          : 2;
  __REG32 TxRqstReg7          : 2;
  __REG32 TxRqstReg8          : 2;
  __REG32                     :16;
} __dcantxrqx_bits;

/* CAN Transmission Request Registers (DCAN TXRQ12) */
typedef struct {
  __REG32 TxRqst1           : 1;
  __REG32 TxRqst2           : 1;
  __REG32 TxRqst3           : 1;
  __REG32 TxRqst4           : 1;
  __REG32 TxRqst5           : 1;
  __REG32 TxRqst6           : 1;
  __REG32 TxRqst7           : 1;
  __REG32 TxRqst8           : 1;
  __REG32 TxRqst9           : 1;
  __REG32 TxRqst10          : 1;
  __REG32 TxRqst11          : 1;
  __REG32 TxRqst12          : 1;
  __REG32 TxRqst13          : 1;
  __REG32 TxRqst14          : 1;
  __REG32 TxRqst15          : 1;
  __REG32 TxRqst16          : 1;
  __REG32 TxRqst17          : 1;
  __REG32 TxRqst18          : 1;
  __REG32 TxRqst19          : 1;
  __REG32 TxRqst20          : 1;
  __REG32 TxRqst21          : 1;
  __REG32 TxRqst22          : 1;
  __REG32 TxRqst23          : 1;
  __REG32 TxRqst24          : 1;
  __REG32 TxRqst25          : 1;
  __REG32 TxRqst26          : 1;
  __REG32 TxRqst27          : 1;
  __REG32 TxRqst28          : 1;
  __REG32 TxRqst29          : 1;
  __REG32 TxRqst30          : 1;
  __REG32 TxRqst31          : 1;
  __REG32 TxRqst32          : 1;
} __dcantxrq12_bits;

/* CAN Transmission Request Registers (DCAN TXRQ34) */
typedef struct {
  __REG32 TxRqst33          : 1;
  __REG32 TxRqst34          : 1;
  __REG32 TxRqst35          : 1;
  __REG32 TxRqst36          : 1;
  __REG32 TxRqst37          : 1;
  __REG32 TxRqst38          : 1;
  __REG32 TxRqst39          : 1;
  __REG32 TxRqst40          : 1;
  __REG32 TxRqst41          : 1;
  __REG32 TxRqst42          : 1;
  __REG32 TxRqst43          : 1;
  __REG32 TxRqst44          : 1;
  __REG32 TxRqst45          : 1;
  __REG32 TxRqst46          : 1;
  __REG32 TxRqst47          : 1;
  __REG32 TxRqst48          : 1;
  __REG32 TxRqst49          : 1;
  __REG32 TxRqst50          : 1;
  __REG32 TxRqst51          : 1;
  __REG32 TxRqst52          : 1;
  __REG32 TxRqst53          : 1;
  __REG32 TxRqst54          : 1;
  __REG32 TxRqst55          : 1;
  __REG32 TxRqst56          : 1;
  __REG32 TxRqst57          : 1;
  __REG32 TxRqst58          : 1;
  __REG32 TxRqst59          : 1;
  __REG32 TxRqst60          : 1;
  __REG32 TxRqst61          : 1;
  __REG32 TxRqst62          : 1;
  __REG32 TxRqst63          : 1;
  __REG32 TxRqst64          : 1;
} __dcantxrq34_bits;

/* CAN Transmission Request Registers (DCAN TXRQ56) */
typedef struct {
  __REG32 TxRqst65          : 1;
  __REG32 TxRqst66          : 1;
  __REG32 TxRqst67          : 1;
  __REG32 TxRqst68          : 1;
  __REG32 TxRqst69          : 1;
  __REG32 TxRqst70          : 1;
  __REG32 TxRqst71          : 1;
  __REG32 TxRqst72          : 1;
  __REG32 TxRqst73          : 1;
  __REG32 TxRqst74          : 1;
  __REG32 TxRqst75          : 1;
  __REG32 TxRqst76          : 1;
  __REG32 TxRqst77          : 1;
  __REG32 TxRqst78          : 1;
  __REG32 TxRqst79          : 1;
  __REG32 TxRqst80          : 1;
  __REG32 TxRqst81          : 1;
  __REG32 TxRqst82          : 1;
  __REG32 TxRqst83          : 1;
  __REG32 TxRqst84          : 1;
  __REG32 TxRqst85          : 1;
  __REG32 TxRqst86          : 1;
  __REG32 TxRqst87          : 1;
  __REG32 TxRqst88          : 1;
  __REG32 TxRqst89          : 1;
  __REG32 TxRqst90          : 1;
  __REG32 TxRqst91          : 1;
  __REG32 TxRqst92          : 1;
  __REG32 TxRqst93          : 1;
  __REG32 TxRqst94          : 1;
  __REG32 TxRqst95          : 1;
  __REG32 TxRqst96          : 1;
} __dcantxrq56_bits;

/* CAN Transmission Request Registers (DCAN TXRQ78) */
typedef struct {
  __REG32 TxRqst97          : 1;
  __REG32 TxRqst98          : 1;
  __REG32 TxRqst99          : 1;
  __REG32 TxRqst100         : 1;
  __REG32 TxRqst101         : 1;
  __REG32 TxRqst102         : 1;
  __REG32 TxRqst103         : 1;
  __REG32 TxRqst104         : 1;
  __REG32 TxRqst105         : 1;
  __REG32 TxRqst106         : 1;
  __REG32 TxRqst107         : 1;
  __REG32 TxRqst108         : 1;
  __REG32 TxRqst109         : 1;
  __REG32 TxRqst110         : 1;
  __REG32 TxRqst111         : 1;
  __REG32 TxRqst112         : 1;
  __REG32 TxRqst113         : 1;
  __REG32 TxRqst114         : 1;
  __REG32 TxRqst115         : 1;
  __REG32 TxRqst116         : 1;
  __REG32 TxRqst117         : 1;
  __REG32 TxRqst118         : 1;
  __REG32 TxRqst119         : 1;
  __REG32 TxRqst120         : 1;
  __REG32 TxRqst121         : 1;
  __REG32 TxRqst122         : 1;
  __REG32 TxRqst123         : 1;
  __REG32 TxRqst124         : 1;
  __REG32 TxRqst125         : 1;
  __REG32 TxRqst126         : 1;
  __REG32 TxRqst127         : 1;
  __REG32 TxRqst128         : 1;
} __dcantxrq78_bits;

/* CAN New Data X Register (DCAN NWDAT X) */
typedef struct {
  __REG32 NewDatReg1          : 2;
  __REG32 NewDatReg2          : 2;
  __REG32 NewDatReg3          : 2;
  __REG32 NewDatReg4          : 2;
  __REG32 NewDatReg5          : 2;
  __REG32 NewDatReg6          : 2;
  __REG32 NewDatReg7          : 2;
  __REG32 NewDatReg8          : 2;
  __REG32                     :16;
} __dcannwdatx_bits;

/* CAN New Data Registers (DCAN NWDAT12) */
typedef struct {
  __REG32 NewDat1           : 1;
  __REG32 NewDat2           : 1;
  __REG32 NewDat3           : 1;
  __REG32 NewDat4           : 1;
  __REG32 NewDat5           : 1;
  __REG32 NewDat6           : 1;
  __REG32 NewDat7           : 1;
  __REG32 NewDat8           : 1;
  __REG32 NewDat9           : 1;
  __REG32 NewDat10          : 1;
  __REG32 NewDat11          : 1;
  __REG32 NewDat12          : 1;
  __REG32 NewDat13          : 1;
  __REG32 NewDat14          : 1;
  __REG32 NewDat15          : 1;
  __REG32 NewDat16          : 1;
  __REG32 NewDat17          : 1;
  __REG32 NewDat18          : 1;
  __REG32 NewDat19          : 1;
  __REG32 NewDat20          : 1;
  __REG32 NewDat21          : 1;
  __REG32 NewDat22          : 1;
  __REG32 NewDat23          : 1;
  __REG32 NewDat24          : 1;
  __REG32 NewDat25          : 1;
  __REG32 NewDat26          : 1;
  __REG32 NewDat27          : 1;
  __REG32 NewDat28          : 1;
  __REG32 NewDat29          : 1;
  __REG32 NewDat30          : 1;
  __REG32 NewDat31          : 1;
  __REG32 NewDat32          : 1;
} __dcannwdat12_bits;

/* CAN New Data Registers (DCAN NWDAT34) */
typedef struct {
  __REG32 NewDat33          : 1;
  __REG32 NewDat34          : 1;
  __REG32 NewDat35          : 1;
  __REG32 NewDat36          : 1;
  __REG32 NewDat37          : 1;
  __REG32 NewDat38          : 1;
  __REG32 NewDat39          : 1;
  __REG32 NewDat40          : 1;
  __REG32 NewDat41          : 1;
  __REG32 NewDat42          : 1;
  __REG32 NewDat43          : 1;
  __REG32 NewDat44          : 1;
  __REG32 NewDat45          : 1;
  __REG32 NewDat46          : 1;
  __REG32 NewDat47          : 1;
  __REG32 NewDat48          : 1;
  __REG32 NewDat49          : 1;
  __REG32 NewDat50          : 1;
  __REG32 NewDat51          : 1;
  __REG32 NewDat52          : 1;
  __REG32 NewDat53          : 1;
  __REG32 NewDat54          : 1;
  __REG32 NewDat55          : 1;
  __REG32 NewDat56          : 1;
  __REG32 NewDat57          : 1;
  __REG32 NewDat58          : 1;
  __REG32 NewDat59          : 1;
  __REG32 NewDat60          : 1;
  __REG32 NewDat61          : 1;
  __REG32 NewDat62          : 1;
  __REG32 NewDat63          : 1;
  __REG32 NewDat64          : 1;
} __dcannwdat34_bits;

/* CAN New Data Registers (DCAN NWDAT56) */
typedef struct {
  __REG32 NewDat65          : 1;
  __REG32 NewDat66          : 1;
  __REG32 NewDat67          : 1;
  __REG32 NewDat68          : 1;
  __REG32 NewDat69          : 1;
  __REG32 NewDat70          : 1;
  __REG32 NewDat71          : 1;
  __REG32 NewDat72          : 1;
  __REG32 NewDat73          : 1;
  __REG32 NewDat74          : 1;
  __REG32 NewDat75          : 1;
  __REG32 NewDat76          : 1;
  __REG32 NewDat77          : 1;
  __REG32 NewDat78          : 1;
  __REG32 NewDat79          : 1;
  __REG32 NewDat80          : 1;
  __REG32 NewDat81          : 1;
  __REG32 NewDat82          : 1;
  __REG32 NewDat83          : 1;
  __REG32 NewDat84          : 1;
  __REG32 NewDat85          : 1;
  __REG32 NewDat86          : 1;
  __REG32 NewDat87          : 1;
  __REG32 NewDat88          : 1;
  __REG32 NewDat89          : 1;
  __REG32 NewDat90          : 1;
  __REG32 NewDat91          : 1;
  __REG32 NewDat92          : 1;
  __REG32 NewDat93          : 1;
  __REG32 NewDat94          : 1;
  __REG32 NewDat95          : 1;
  __REG32 NewDat96          : 1;
} __dcannwdat56_bits;

/* CAN New Data Registers (DCAN NWDAT78) */
typedef struct {
  __REG32 NewDat97          : 1;
  __REG32 NewDat98          : 1;
  __REG32 NewDat99          : 1;
  __REG32 NewDat100         : 1;
  __REG32 NewDat101         : 1;
  __REG32 NewDat102         : 1;
  __REG32 NewDat103         : 1;
  __REG32 NewDat104         : 1;
  __REG32 NewDat105         : 1;
  __REG32 NewDat106         : 1;
  __REG32 NewDat107         : 1;
  __REG32 NewDat108         : 1;
  __REG32 NewDat109         : 1;
  __REG32 NewDat110         : 1;
  __REG32 NewDat111         : 1;
  __REG32 NewDat112         : 1;
  __REG32 NewDat113         : 1;
  __REG32 NewDat114         : 1;
  __REG32 NewDat115         : 1;
  __REG32 NewDat116         : 1;
  __REG32 NewDat117         : 1;
  __REG32 NewDat118         : 1;
  __REG32 NewDat119         : 1;
  __REG32 NewDat120         : 1;
  __REG32 NewDat121         : 1;
  __REG32 NewDat122         : 1;
  __REG32 NewDat123         : 1;
  __REG32 NewDat124         : 1;
  __REG32 NewDat125         : 1;
  __REG32 NewDat126         : 1;
  __REG32 NewDat127         : 1;
  __REG32 NewDat128         : 1;
} __dcannwdat78_bits;

/* CAN Interrupt Pending X Register (DCAN INTPND X) */
typedef struct {
  __REG32 IntPndReg1          : 2;
  __REG32 IntPndReg2          : 2;
  __REG32 IntPndReg3          : 2;
  __REG32 IntPndReg4          : 2;
  __REG32 IntPndReg5          : 2;
  __REG32 IntPndReg6          : 2;
  __REG32 IntPndReg7          : 2;
  __REG32 IntPndReg8          : 2;
  __REG32                     :16;
} __dcanintpndx_bits;

/* CAN Interrupt Pending Registers (DCAN INTPND12) */
typedef struct {
  __REG32 IntPnd1           : 1;
  __REG32 IntPnd2           : 1;
  __REG32 IntPnd3           : 1;
  __REG32 IntPnd4           : 1;
  __REG32 IntPnd5           : 1;
  __REG32 IntPnd6           : 1;
  __REG32 IntPnd7           : 1;
  __REG32 IntPnd8           : 1;
  __REG32 IntPnd9           : 1;
  __REG32 IntPnd10          : 1;
  __REG32 IntPnd11          : 1;
  __REG32 IntPnd12          : 1;
  __REG32 IntPnd13          : 1;
  __REG32 IntPnd14          : 1;
  __REG32 IntPnd15          : 1;
  __REG32 IntPnd16          : 1;
  __REG32 IntPnd17          : 1;
  __REG32 IntPnd18          : 1;
  __REG32 IntPnd19          : 1;
  __REG32 IntPnd20          : 1;
  __REG32 IntPnd21          : 1;
  __REG32 IntPnd22          : 1;
  __REG32 IntPnd23          : 1;
  __REG32 IntPnd24          : 1;
  __REG32 IntPnd25          : 1;
  __REG32 IntPnd26          : 1;
  __REG32 IntPnd27          : 1;
  __REG32 IntPnd28          : 1;
  __REG32 IntPnd29          : 1;
  __REG32 IntPnd30          : 1;
  __REG32 IntPnd31          : 1;
  __REG32 IntPnd32          : 1;
} __dcanintpnd12_bits;

/* CAN Interrupt Pending Registers (DCAN INTPND34) */
typedef struct {
  __REG32 IntPnd33          : 1;
  __REG32 IntPnd34          : 1;
  __REG32 IntPnd35          : 1;
  __REG32 IntPnd36          : 1;
  __REG32 IntPnd37          : 1;
  __REG32 IntPnd38          : 1;
  __REG32 IntPnd39          : 1;
  __REG32 IntPnd40          : 1;
  __REG32 IntPnd41          : 1;
  __REG32 IntPnd42          : 1;
  __REG32 IntPnd43          : 1;
  __REG32 IntPnd44          : 1;
  __REG32 IntPnd45          : 1;
  __REG32 IntPnd46          : 1;
  __REG32 IntPnd47          : 1;
  __REG32 IntPnd48          : 1;
  __REG32 IntPnd49          : 1;
  __REG32 IntPnd50          : 1;
  __REG32 IntPnd51          : 1;
  __REG32 IntPnd52          : 1;
  __REG32 IntPnd53          : 1;
  __REG32 IntPnd54          : 1;
  __REG32 IntPnd55          : 1;
  __REG32 IntPnd56          : 1;
  __REG32 IntPnd57          : 1;
  __REG32 IntPnd58          : 1;
  __REG32 IntPnd59          : 1;
  __REG32 IntPnd60          : 1;
  __REG32 IntPnd61          : 1;
  __REG32 IntPnd62          : 1;
  __REG32 IntPnd63          : 1;
  __REG32 IntPnd64          : 1;
} __dcanintpnd34_bits;

/* CAN Interrupt Pending Registers (DCAN INTPND56) */
typedef struct {
  __REG32 IntPnd65          : 1;
  __REG32 IntPnd66          : 1;
  __REG32 IntPnd67          : 1;
  __REG32 IntPnd68          : 1;
  __REG32 IntPnd69          : 1;
  __REG32 IntPnd70          : 1;
  __REG32 IntPnd71          : 1;
  __REG32 IntPnd72          : 1;
  __REG32 IntPnd73          : 1;
  __REG32 IntPnd74          : 1;
  __REG32 IntPnd75          : 1;
  __REG32 IntPnd76          : 1;
  __REG32 IntPnd77          : 1;
  __REG32 IntPnd78          : 1;
  __REG32 IntPnd79          : 1;
  __REG32 IntPnd80          : 1;
  __REG32 IntPnd81          : 1;
  __REG32 IntPnd82          : 1;
  __REG32 IntPnd83          : 1;
  __REG32 IntPnd84          : 1;
  __REG32 IntPnd85          : 1;
  __REG32 IntPnd86          : 1;
  __REG32 IntPnd87          : 1;
  __REG32 IntPnd88          : 1;
  __REG32 IntPnd89          : 1;
  __REG32 IntPnd90          : 1;
  __REG32 IntPnd91          : 1;
  __REG32 IntPnd92          : 1;
  __REG32 IntPnd93          : 1;
  __REG32 IntPnd94          : 1;
  __REG32 IntPnd95          : 1;
  __REG32 IntPnd96          : 1;
} __dcanintpnd56_bits;

/* CAN Interrupt Pending Registers (DCAN INTPND78) */
typedef struct {
  __REG32 IntPnd97          : 1;
  __REG32 IntPnd98          : 1;
  __REG32 IntPnd99          : 1;
  __REG32 IntPnd100         : 1;
  __REG32 IntPnd101         : 1;
  __REG32 IntPnd102         : 1;
  __REG32 IntPnd103         : 1;
  __REG32 IntPnd104         : 1;
  __REG32 IntPnd105         : 1;
  __REG32 IntPnd106         : 1;
  __REG32 IntPnd107         : 1;
  __REG32 IntPnd108         : 1;
  __REG32 IntPnd109         : 1;
  __REG32 IntPnd110         : 1;
  __REG32 IntPnd111         : 1;
  __REG32 IntPnd112         : 1;
  __REG32 IntPnd113         : 1;
  __REG32 IntPnd114         : 1;
  __REG32 IntPnd115         : 1;
  __REG32 IntPnd116         : 1;
  __REG32 IntPnd117         : 1;
  __REG32 IntPnd118         : 1;
  __REG32 IntPnd119         : 1;
  __REG32 IntPnd120         : 1;
  __REG32 IntPnd121         : 1;
  __REG32 IntPnd122         : 1;
  __REG32 IntPnd123         : 1;
  __REG32 IntPnd124         : 1;
  __REG32 IntPnd125         : 1;
  __REG32 IntPnd126         : 1;
  __REG32 IntPnd127         : 1;
  __REG32 IntPnd128         : 1;
} __dcanintpnd78_bits;

/* CAN Message Valid X Register (DCAN MSGVAL X) */
typedef struct {
  __REG32 MsgValReg1          : 2;
  __REG32 MsgValReg2          : 2;
  __REG32 MsgValReg3          : 2;
  __REG32 MsgValReg4          : 2;
  __REG32 MsgValReg5          : 2;
  __REG32 MsgValReg6          : 2;
  __REG32 MsgValReg7          : 2;
  __REG32 MsgValReg8          : 2;
  __REG32                     :16;
} __dcanmsgvalx_bits;

/* CAN Message Valid Registers (DCAN MSGVAL12) */
typedef struct {
  __REG32 MsgVal1           : 1;
  __REG32 MsgVal2           : 1;
  __REG32 MsgVal3           : 1;
  __REG32 MsgVal4           : 1;
  __REG32 MsgVal5           : 1;
  __REG32 MsgVal6           : 1;
  __REG32 MsgVal7           : 1;
  __REG32 MsgVal8           : 1;
  __REG32 MsgVal9           : 1;
  __REG32 MsgVal10          : 1;
  __REG32 MsgVal11          : 1;
  __REG32 MsgVal12          : 1;
  __REG32 MsgVal13          : 1;
  __REG32 MsgVal14          : 1;
  __REG32 MsgVal15          : 1;
  __REG32 MsgVal16          : 1;
  __REG32 MsgVal17          : 1;
  __REG32 MsgVal18          : 1;
  __REG32 MsgVal19          : 1;
  __REG32 MsgVal20          : 1;
  __REG32 MsgVal21          : 1;
  __REG32 MsgVal22          : 1;
  __REG32 MsgVal23          : 1;
  __REG32 MsgVal24          : 1;
  __REG32 MsgVal25          : 1;
  __REG32 MsgVal26          : 1;
  __REG32 MsgVal27          : 1;
  __REG32 MsgVal28          : 1;
  __REG32 MsgVal29          : 1;
  __REG32 MsgVal30          : 1;
  __REG32 MsgVal31          : 1;
  __REG32 MsgVal32          : 1;
} __dcanmsgval12_bits;

/* CAN Message Valid Registers (DCAN MSGVAL34) */
typedef struct {
  __REG32 MsgVal33          : 1;
  __REG32 MsgVal34          : 1;
  __REG32 MsgVal35          : 1;
  __REG32 MsgVal36          : 1;
  __REG32 MsgVal37          : 1;
  __REG32 MsgVal38          : 1;
  __REG32 MsgVal39          : 1;
  __REG32 MsgVal40          : 1;
  __REG32 MsgVal41          : 1;
  __REG32 MsgVal42          : 1;
  __REG32 MsgVal43          : 1;
  __REG32 MsgVal44          : 1;
  __REG32 MsgVal45          : 1;
  __REG32 MsgVal46          : 1;
  __REG32 MsgVal47          : 1;
  __REG32 MsgVal48          : 1;
  __REG32 MsgVal49          : 1;
  __REG32 MsgVal50          : 1;
  __REG32 MsgVal51          : 1;
  __REG32 MsgVal52          : 1;
  __REG32 MsgVal53          : 1;
  __REG32 MsgVal54          : 1;
  __REG32 MsgVal55          : 1;
  __REG32 MsgVal56          : 1;
  __REG32 MsgVal57          : 1;
  __REG32 MsgVal58          : 1;
  __REG32 MsgVal59          : 1;
  __REG32 MsgVal60          : 1;
  __REG32 MsgVal61          : 1;
  __REG32 MsgVal62          : 1;
  __REG32 MsgVal63          : 1;
  __REG32 MsgVal64          : 1;
} __dcanmsgval34_bits;

/* CAN Message Valid Registers (DCAN MSGVAL56) */
typedef struct {
  __REG32 MsgVal65          : 1;
  __REG32 MsgVal66          : 1;
  __REG32 MsgVal67          : 1;
  __REG32 MsgVal68          : 1;
  __REG32 MsgVal69          : 1;
  __REG32 MsgVal70          : 1;
  __REG32 MsgVal71          : 1;
  __REG32 MsgVal72          : 1;
  __REG32 MsgVal73          : 1;
  __REG32 MsgVal74          : 1;
  __REG32 MsgVal75          : 1;
  __REG32 MsgVal76          : 1;
  __REG32 MsgVal77          : 1;
  __REG32 MsgVal78          : 1;
  __REG32 MsgVal79          : 1;
  __REG32 MsgVal80          : 1;
  __REG32 MsgVal81          : 1;
  __REG32 MsgVal82          : 1;
  __REG32 MsgVal83          : 1;
  __REG32 MsgVal84          : 1;
  __REG32 MsgVal85          : 1;
  __REG32 MsgVal86          : 1;
  __REG32 MsgVal87          : 1;
  __REG32 MsgVal88          : 1;
  __REG32 MsgVal89          : 1;
  __REG32 MsgVal90          : 1;
  __REG32 MsgVal91          : 1;
  __REG32 MsgVal92          : 1;
  __REG32 MsgVal93          : 1;
  __REG32 MsgVal94          : 1;
  __REG32 MsgVal95          : 1;
  __REG32 MsgVal96          : 1;
} __dcanmsgval56_bits;

/* CAN Message Valid Registers (DCAN MSGVAL78) */
typedef struct {
  __REG32 MsgVal97          : 1;
  __REG32 MsgVal98          : 1;
  __REG32 MsgVal99          : 1;
  __REG32 MsgVal100         : 1;
  __REG32 MsgVal101         : 1;
  __REG32 MsgVal102         : 1;
  __REG32 MsgVal103         : 1;
  __REG32 MsgVal104         : 1;
  __REG32 MsgVal105         : 1;
  __REG32 MsgVal106         : 1;
  __REG32 MsgVal107         : 1;
  __REG32 MsgVal108         : 1;
  __REG32 MsgVal109         : 1;
  __REG32 MsgVal110         : 1;
  __REG32 MsgVal111         : 1;
  __REG32 MsgVal112         : 1;
  __REG32 MsgVal113         : 1;
  __REG32 MsgVal114         : 1;
  __REG32 MsgVal115         : 1;
  __REG32 MsgVal116         : 1;
  __REG32 MsgVal117         : 1;
  __REG32 MsgVal118         : 1;
  __REG32 MsgVal119         : 1;
  __REG32 MsgVal120         : 1;
  __REG32 MsgVal121         : 1;
  __REG32 MsgVal122         : 1;
  __REG32 MsgVal123         : 1;
  __REG32 MsgVal124         : 1;
  __REG32 MsgVal125         : 1;
  __REG32 MsgVal126         : 1;
  __REG32 MsgVal127         : 1;
  __REG32 MsgVal128         : 1;
} __dcanmsgval78_bits;

/* CAN Interrupt Multiplexer Registers (DCAN INTMUX12) */
typedef struct {
  __REG32 IntMux1           : 1;
  __REG32 IntMux2           : 1;
  __REG32 IntMux3           : 1;
  __REG32 IntMux4           : 1;
  __REG32 IntMux5           : 1;
  __REG32 IntMux6           : 1;
  __REG32 IntMux7           : 1;
  __REG32 IntMux8           : 1;
  __REG32 IntMux9           : 1;
  __REG32 IntMux10          : 1;
  __REG32 IntMux11          : 1;
  __REG32 IntMux12          : 1;
  __REG32 IntMux13          : 1;
  __REG32 IntMux14          : 1;
  __REG32 IntMux15          : 1;
  __REG32 IntMux16          : 1;
  __REG32 IntMux17          : 1;
  __REG32 IntMux18          : 1;
  __REG32 IntMux19          : 1;
  __REG32 IntMux20          : 1;
  __REG32 IntMux21          : 1;
  __REG32 IntMux22          : 1;
  __REG32 IntMux23          : 1;
  __REG32 IntMux24          : 1;
  __REG32 IntMux25          : 1;
  __REG32 IntMux26          : 1;
  __REG32 IntMux27          : 1;
  __REG32 IntMux28          : 1;
  __REG32 IntMux29          : 1;
  __REG32 IntMux30          : 1;
  __REG32 IntMux31          : 1;
  __REG32 IntMux32          : 1;
} __dcanintmux12_bits;

/* CAN Interrupt Multiplexer Registers (DCAN INTMUX34) */
typedef struct {
  __REG32 IntMux33          : 1;
  __REG32 IntMux34          : 1;
  __REG32 IntMux35          : 1;
  __REG32 IntMux36          : 1;
  __REG32 IntMux37          : 1;
  __REG32 IntMux38          : 1;
  __REG32 IntMux39          : 1;
  __REG32 IntMux40          : 1;
  __REG32 IntMux41          : 1;
  __REG32 IntMux42          : 1;
  __REG32 IntMux43          : 1;
  __REG32 IntMux44          : 1;
  __REG32 IntMux45          : 1;
  __REG32 IntMux46          : 1;
  __REG32 IntMux47          : 1;
  __REG32 IntMux48          : 1;
  __REG32 IntMux49          : 1;
  __REG32 IntMux50          : 1;
  __REG32 IntMux51          : 1;
  __REG32 IntMux52          : 1;
  __REG32 IntMux53          : 1;
  __REG32 IntMux54          : 1;
  __REG32 IntMux55          : 1;
  __REG32 IntMux56          : 1;
  __REG32 IntMux57          : 1;
  __REG32 IntMux58          : 1;
  __REG32 IntMux59          : 1;
  __REG32 IntMux60          : 1;
  __REG32 IntMux61          : 1;
  __REG32 IntMux62          : 1;
  __REG32 IntMux63          : 1;
  __REG32 IntMux64          : 1;
} __dcanintmux34_bits;

/* CAN Interrupt Multiplexer Registers (DCAN INTMUX56) */
typedef struct {
  __REG32 IntMux65          : 1;
  __REG32 IntMux66          : 1;
  __REG32 IntMux67          : 1;
  __REG32 IntMux68          : 1;
  __REG32 IntMux69          : 1;
  __REG32 IntMux70          : 1;
  __REG32 IntMux71          : 1;
  __REG32 IntMux72          : 1;
  __REG32 IntMux73          : 1;
  __REG32 IntMux74          : 1;
  __REG32 IntMux75          : 1;
  __REG32 IntMux76          : 1;
  __REG32 IntMux77          : 1;
  __REG32 IntMux78          : 1;
  __REG32 IntMux79          : 1;
  __REG32 IntMux80          : 1;
  __REG32 IntMux81          : 1;
  __REG32 IntMux82          : 1;
  __REG32 IntMux83          : 1;
  __REG32 IntMux84          : 1;
  __REG32 IntMux85          : 1;
  __REG32 IntMux86          : 1;
  __REG32 IntMux87          : 1;
  __REG32 IntMux88          : 1;
  __REG32 IntMux89          : 1;
  __REG32 IntMux90          : 1;
  __REG32 IntMux91          : 1;
  __REG32 IntMux92          : 1;
  __REG32 IntMux93          : 1;
  __REG32 IntMux94          : 1;
  __REG32 IntMux95          : 1;
  __REG32 IntMux96          : 1;
} __dcanintmux56_bits;

/* CAN Interrupt Multiplexer Registers (DCAN INTMUX78) */
typedef struct {
  __REG32 IntMux97          : 1;
  __REG32 IntMux98          : 1;
  __REG32 IntMux99          : 1;
  __REG32 IntMux100         : 1;
  __REG32 IntMux101         : 1;
  __REG32 IntMux102         : 1;
  __REG32 IntMux103         : 1;
  __REG32 IntMux104         : 1;
  __REG32 IntMux105         : 1;
  __REG32 IntMux106         : 1;
  __REG32 IntMux107         : 1;
  __REG32 IntMux108         : 1;
  __REG32 IntMux109         : 1;
  __REG32 IntMux110         : 1;
  __REG32 IntMux111         : 1;
  __REG32 IntMux112         : 1;
  __REG32 IntMux113         : 1;
  __REG32 IntMux114         : 1;
  __REG32 IntMux115         : 1;
  __REG32 IntMux116         : 1;
  __REG32 IntMux117         : 1;
  __REG32 IntMux118         : 1;
  __REG32 IntMux119         : 1;
  __REG32 IntMux120         : 1;
  __REG32 IntMux121         : 1;
  __REG32 IntMux122         : 1;
  __REG32 IntMux123         : 1;
  __REG32 IntMux124         : 1;
  __REG32 IntMux125         : 1;
  __REG32 IntMux126         : 1;
  __REG32 IntMux127         : 1;
  __REG32 IntMux128         : 1;
} __dcanintmux78_bits;

/* CAN IF1/2 Command Registers (DCAN IF1/2CMD) */
typedef struct {
  __REG32 MessageNumber       : 8;
  __REG32                     : 6;
  __REG32 DMAactive           : 1;
  __REG32 Busy                : 1;
  __REG32 DataB               : 1;
  __REG32 DataA               : 1;
  __REG32 TxRqst_NewDat       : 1;
  __REG32 ClrIntPnd           : 1;
  __REG32 Control             : 1;
  __REG32 Arb                 : 1;
  __REG32 Mask                : 1;
  __REG32 WR_RD               : 1;
  __REG32                     : 8;
} __dcanifcmd_bits;

/* CAN IF1/IF2/IF2 Mask Registers (DCAN IF1MSK, DCAN IF2MSK, DCAN IF3MSK) */
typedef struct {
  __REG32 Msk0                : 1;
  __REG32 Msk1                : 1;
  __REG32 Msk2                : 1;
  __REG32 Msk3                : 1;
  __REG32 Msk4                : 1;
  __REG32 Msk5                : 1;
  __REG32 Msk6                : 1;
  __REG32 Msk7                : 1;
  __REG32 Msk8                : 1;
  __REG32 Msk9                : 1;
  __REG32 Msk10               : 1;
  __REG32 Msk11               : 1;
  __REG32 Msk12               : 1;
  __REG32 Msk13               : 1;
  __REG32 Msk14               : 1;
  __REG32 Msk15               : 1;
  __REG32 Msk16               : 1;
  __REG32 Msk17               : 1;
  __REG32 Msk18               : 1;
  __REG32 Msk19               : 1;
  __REG32 Msk20               : 1;
  __REG32 Msk21               : 1;
  __REG32 Msk22               : 1;
  __REG32 Msk23               : 1;
  __REG32 Msk24               : 1;
  __REG32 Msk25               : 1;
  __REG32 Msk26               : 1;
  __REG32 Msk27               : 1;
  __REG32 Msk28               : 1;
  __REG32                     : 1;
  __REG32 MDir                : 1;
  __REG32 MXtd                : 1;
} __dcanifmsk_bits;

/* CAN IF1/IF2/IF3 Arbitration Registers (DCAN IF1ARB, DCAN IF2ARB, DCAN IF3ARB) */
typedef struct {
  __REG32 ID0                 : 1;
  __REG32 ID1                 : 1;
  __REG32 ID2                 : 1;
  __REG32 ID3                 : 1;
  __REG32 ID4                 : 1;
  __REG32 ID5                 : 1;
  __REG32 ID6                 : 1;
  __REG32 ID7                 : 1;
  __REG32 ID8                 : 1;
  __REG32 ID9                 : 1;
  __REG32 ID10                : 1;
  __REG32 ID11                : 1;
  __REG32 ID12                : 1;
  __REG32 ID13                : 1;
  __REG32 ID14                : 1;
  __REG32 ID15                : 1;
  __REG32 ID16                : 1;
  __REG32 ID17                : 1;
  __REG32 ID18                : 1;
  __REG32 ID19                : 1;
  __REG32 ID20                : 1;
  __REG32 ID21                : 1;
  __REG32 ID22                : 1;
  __REG32 ID23                : 1;
  __REG32 ID24                : 1;
  __REG32 ID25                : 1;
  __REG32 ID26                : 1;
  __REG32 ID27                : 1;
  __REG32 ID28                : 1;
  __REG32 Dir                 : 1;
  __REG32 Xtd                 : 1;
  __REG32 MsgVal              : 1;
} __dcanifarb_bits;

/* CAN IF1/IF2/IF3 Message Control Registers (DCAN IF1MCTL, DCAN IF2MCTL, DCAN IF3MCTL) */
typedef struct {
  __REG32 DLC                 : 4;
  __REG32                     : 3;
  __REG32 EoB                 : 1;
  __REG32 TxRqst              : 1;
  __REG32 RmtEn               : 1;
  __REG32 RxIE                : 1;
  __REG32 TxIE                : 1;
  __REG32 UMask               : 1;
  __REG32 IntPnd              : 1;
  __REG32 MsgLst              : 1;
  __REG32 NewDat              : 1;
  __REG32                     :16;
} __dcanifmctl_bits;

/* CAN IF1/2/3 Data A Register (DCAN IF1/2/3DATA) */
typedef struct {
  __REG32 Data0               : 8;
  __REG32 Data1               : 8;
  __REG32 Data2               : 8;
  __REG32 Data3               : 8;
} __dcanifdata_bits;

/* CAN IF1/2/3 Data B Register (DCAN IF1/2/3DATB) */
typedef struct {
  __REG32 Data4               : 8;
  __REG32 Data5               : 8;
  __REG32 Data6               : 8;
  __REG32 Data7               : 8;
} __dcanifdatb_bits;

/* CAN IF3 Observation Register (DCAN IF3OBS) */
typedef struct {
  __REG32 Mask                : 1;
  __REG32 Arb                 : 1;
  __REG32 Ctrl                : 1;
  __REG32 DataA               : 1;
  __REG32 DataB               : 1;
  __REG32                     : 3;
  __REG32 IF3_SM              : 1;
  __REG32 IF3_SA              : 1;
  __REG32 IF3_SC              : 1;
  __REG32 IF3_SDA             : 1;
  __REG32 IF3_SDB             : 1;
  __REG32                     : 2;
  __REG32 IF3_Upd             : 1;
  __REG32                     :16;
} __dcanif3obs_bits;

/* CAN IF3 Update Enable Registers (DCAN IF3UPD12) */
typedef struct {
  __REG32 IF3UpdEn1           : 1;
  __REG32 IF3UpdEn2           : 1;
  __REG32 IF3UpdEn3           : 1;
  __REG32 IF3UpdEn4           : 1;
  __REG32 IF3UpdEn5           : 1;
  __REG32 IF3UpdEn6           : 1;
  __REG32 IF3UpdEn7           : 1;
  __REG32 IF3UpdEn8           : 1;
  __REG32 IF3UpdEn9           : 1;
  __REG32 IF3UpdEn10          : 1;
  __REG32 IF3UpdEn11          : 1;
  __REG32 IF3UpdEn12          : 1;
  __REG32 IF3UpdEn13          : 1;
  __REG32 IF3UpdEn14          : 1;
  __REG32 IF3UpdEn15          : 1;
  __REG32 IF3UpdEn16          : 1;
  __REG32 IF3UpdEn17          : 1;
  __REG32 IF3UpdEn18          : 1;
  __REG32 IF3UpdEn19          : 1;
  __REG32 IF3UpdEn20          : 1;
  __REG32 IF3UpdEn21          : 1;
  __REG32 IF3UpdEn22          : 1;
  __REG32 IF3UpdEn23          : 1;
  __REG32 IF3UpdEn24          : 1;
  __REG32 IF3UpdEn25          : 1;
  __REG32 IF3UpdEn26          : 1;
  __REG32 IF3UpdEn27          : 1;
  __REG32 IF3UpdEn28          : 1;
  __REG32 IF3UpdEn29          : 1;
  __REG32 IF3UpdEn30          : 1;
  __REG32 IF3UpdEn31          : 1;
  __REG32 IF3UpdEn32          : 1;
} __dcanif3upd12_bits;

/* CAN IF3 Update Enable Registers (DCAN IF3UPD34) */
typedef struct {
  __REG32 IF3UpdEn33          : 1;
  __REG32 IF3UpdEn34          : 1;
  __REG32 IF3UpdEn35          : 1;
  __REG32 IF3UpdEn36          : 1;
  __REG32 IF3UpdEn37          : 1;
  __REG32 IF3UpdEn38          : 1;
  __REG32 IF3UpdEn39          : 1;
  __REG32 IF3UpdEn40          : 1;
  __REG32 IF3UpdEn41          : 1;
  __REG32 IF3UpdEn42          : 1;
  __REG32 IF3UpdEn43          : 1;
  __REG32 IF3UpdEn44          : 1;
  __REG32 IF3UpdEn45          : 1;
  __REG32 IF3UpdEn46          : 1;
  __REG32 IF3UpdEn47          : 1;
  __REG32 IF3UpdEn48          : 1;
  __REG32 IF3UpdEn49          : 1;
  __REG32 IF3UpdEn50          : 1;
  __REG32 IF3UpdEn51          : 1;
  __REG32 IF3UpdEn52          : 1;
  __REG32 IF3UpdEn53          : 1;
  __REG32 IF3UpdEn54          : 1;
  __REG32 IF3UpdEn55          : 1;
  __REG32 IF3UpdEn56          : 1;
  __REG32 IF3UpdEn57          : 1;
  __REG32 IF3UpdEn58          : 1;
  __REG32 IF3UpdEn59          : 1;
  __REG32 IF3UpdEn60          : 1;
  __REG32 IF3UpdEn61          : 1;
  __REG32 IF3UpdEn62          : 1;
  __REG32 IF3UpdEn63          : 1;
  __REG32 IF3UpdEn64          : 1;
} __dcanif3upd34_bits;

/* CAN IF3 Update Enable Registers (DCAN IF3UPD56) */
typedef struct {
  __REG32 IF3UpdEn65          : 1;
  __REG32 IF3UpdEn66          : 1;
  __REG32 IF3UpdEn67          : 1;
  __REG32 IF3UpdEn68          : 1;
  __REG32 IF3UpdEn69          : 1;
  __REG32 IF3UpdEn70          : 1;
  __REG32 IF3UpdEn71          : 1;
  __REG32 IF3UpdEn72          : 1;
  __REG32 IF3UpdEn73          : 1;
  __REG32 IF3UpdEn74          : 1;
  __REG32 IF3UpdEn75          : 1;
  __REG32 IF3UpdEn76          : 1;
  __REG32 IF3UpdEn77          : 1;
  __REG32 IF3UpdEn78          : 1;
  __REG32 IF3UpdEn79          : 1;
  __REG32 IF3UpdEn80          : 1;
  __REG32 IF3UpdEn81          : 1;
  __REG32 IF3UpdEn82          : 1;
  __REG32 IF3UpdEn83          : 1;
  __REG32 IF3UpdEn84          : 1;
  __REG32 IF3UpdEn85          : 1;
  __REG32 IF3UpdEn86          : 1;
  __REG32 IF3UpdEn87          : 1;
  __REG32 IF3UpdEn88          : 1;
  __REG32 IF3UpdEn89          : 1;
  __REG32 IF3UpdEn90          : 1;
  __REG32 IF3UpdEn91          : 1;
  __REG32 IF3UpdEn92          : 1;
  __REG32 IF3UpdEn93          : 1;
  __REG32 IF3UpdEn94          : 1;
  __REG32 IF3UpdEn95          : 1;
  __REG32 IF3UpdEn96          : 1;
} __dcanif3upd56_bits;

/* CAN IF3 Update Enable Registers (DCAN IF3UPD78) */
typedef struct {
  __REG32 IF3UpdEn97          : 1;
  __REG32 IF3UpdEn98          : 1;
  __REG32 IF3UpdEn99          : 1;
  __REG32 IF3UpdEn100         : 1;
  __REG32 IF3UpdEn101         : 1;
  __REG32 IF3UpdEn102         : 1;
  __REG32 IF3UpdEn103         : 1;
  __REG32 IF3UpdEn104         : 1;
  __REG32 IF3UpdEn105         : 1;
  __REG32 IF3UpdEn106         : 1;
  __REG32 IF3UpdEn107         : 1;
  __REG32 IF3UpdEn108         : 1;
  __REG32 IF3UpdEn109         : 1;
  __REG32 IF3UpdEn110         : 1;
  __REG32 IF3UpdEn111         : 1;
  __REG32 IF3UpdEn112         : 1;
  __REG32 IF3UpdEn113         : 1;
  __REG32 IF3UpdEn114         : 1;
  __REG32 IF3UpdEn115         : 1;
  __REG32 IF3UpdEn116         : 1;
  __REG32 IF3UpdEn117         : 1;
  __REG32 IF3UpdEn118         : 1;
  __REG32 IF3UpdEn119         : 1;
  __REG32 IF3UpdEn120         : 1;
  __REG32 IF3UpdEn121         : 1;
  __REG32 IF3UpdEn122         : 1;
  __REG32 IF3UpdEn123         : 1;
  __REG32 IF3UpdEn124         : 1;
  __REG32 IF3UpdEn125         : 1;
  __REG32 IF3UpdEn126         : 1;
  __REG32 IF3UpdEn127         : 1;
  __REG32 IF3UpdEn128         : 1;
} __dcanif3upd78_bits;

/* CAN TX IO Control Register (DCAN TIOC) */
typedef struct {
  __REG32 In                  : 1;
  __REG32 Out                 : 1;
  __REG32 Dir                 : 1;
  __REG32 Func                : 1;
  __REG32                     :12;
  __REG32 OD                  : 1;
  __REG32 PD                  : 1;
  __REG32 PU                  : 1;
  __REG32                     :13;
} __dcantioc_bits;

/* MCSPIn_REVISION */
typedef struct {
   __REG32 Y_MINOR            : 6;
   __REG32 CUSTOM             : 2;
   __REG32 X_MAJOR            : 3;
   __REG32 R_RTL              : 5;
   __REG32 FUNC               :12;
   __REG32                    : 2;
   __REG32 SCHEME             : 2;
} __mcspi_revision_bits;

/* MCSPIn_SYSCONFIG */
typedef struct {
   __REG32 AUTOIDLE           : 1;
   __REG32 SOFTRESET          : 1;
   __REG32                    : 1;
   __REG32 SIDLEMODE          : 2;
   __REG32                    : 3;
   __REG32 CLOCKACTIVITY      : 2;
   __REG32                    :22;
} __mcspi_sysconfig_bits;

/* MCSPIn_SYSSTATUS */
typedef struct {
   __REG32 RESETDONE          : 1;
   __REG32                    :31;
} __mcspi_sysstatus_bits;

/* MCSPIn_IRQSTATUS */
typedef struct {
   __REG32 TX0_EMPTY          : 1;
   __REG32 TX0_UNDERFLOW      : 1;
   __REG32 RX0_FULL           : 1;
   __REG32 RX0_OVERFLOW       : 1;
   __REG32 TX1_EMPTY          : 1;
   __REG32 TX1_UNDERFLOW      : 1;
   __REG32 RX1_FULL           : 1;
   __REG32                    : 1;
   __REG32 TX2_EMPTY          : 1;
   __REG32 TX2_UNDERFLOW      : 1;
   __REG32 RX2_FULL           : 1;
   __REG32                    : 1;
   __REG32 TX3_EMPTY          : 1;
   __REG32 TX3_UNDERFLOW      : 1;
   __REG32 RX3_FULL           : 1;
   __REG32                    : 2;
   __REG32 EOW                : 1;
   __REG32                    :14;
} __mcspi_irqstatus_bits;

#define MCSPI_IRQ_TX0_EMPTY       (0x1UL<<0)
#define MCSPI_IRQ_TX0_UNDERFLOW   (0x1UL<<1)
#define MCSPI_IRQ_RX0_FULL        (0x1UL<<2)
#define MCSPI_IRQ_RX0_OVERFLOW    (0x1UL<<3)
#define MCSPI_IRQ_TX1_EMPTY       (0x1UL<<4)
#define MCSPI_IRQ_TX1_UNDERFLOW   (0x1UL<<5)
#define MCSPI_IRQ_RX1_FULL        (0x1UL<<6)
#define MCSPI_IRQ_TX2_EMPTY       (0x1UL<<8)
#define MCSPI_IRQ_TX2_UNDERFLOW   (0x1UL<<9)
#define MCSPI_IRQ_RX2_FULL        (0x1UL<<10)
#define MCSPI_IRQ_TX3_EMPTY       (0x1UL<<12)
#define MCSPI_IRQ_TX3_UNDERFLOW   (0x1UL<<13)
#define MCSPI_IRQ_RX3_FULL        (0x1UL<<14)
#define MCSPI_IRQ_EOW             (0x1UL<<17)

/* MCSPIn_IRQENABLE */
typedef struct {
   __REG32 TX0_EMPTY_ENABLE        : 1;
   __REG32 TX0_UNDERFLOW_ENABLE    : 1;
   __REG32 RX0_FULL_ENABLE         : 1;
   __REG32 RX0_OVERFLOW_ENABLE     : 1;
   __REG32 TX1_EMPTY_ENABLE        : 1;
   __REG32 TX1_UNDERFLOW_ENABLE    : 1;
   __REG32 RX1_FULL_ENABLE         : 1;
   __REG32                         : 1;
   __REG32 TX2_EMPTY_ENABLE        : 1;
   __REG32 TX2_UNDERFLOW_ENABLE    : 1;
   __REG32 RX2_FULL_ENABLE         : 1;
   __REG32                         : 1;
   __REG32 TX3_EMPTY_ENABLE        : 1;
   __REG32 TX3_UNDERFLOW_ENABLE    : 1;
   __REG32 RX3_FULL_ENABLE         : 1;
   __REG32                         : 2;
   __REG32 EOWKE                   : 1;
   __REG32                         :14;
} __mcspi_irqenable_bits;

/* MCSPIn_SYST */
typedef struct {
   __REG32 SPIEN_0            : 1;
   __REG32 SPIEN_1            : 1;
   __REG32 SPIEN_2            : 1;
   __REG32 SPIEN_3            : 1;
   __REG32 SPIDAT_0           : 1;
   __REG32 SPIDAT_1           : 1;
   __REG32 SPICLK             : 1;
   __REG32                    : 1;
   __REG32 SPIDATDIR0         : 1;
   __REG32 SPIDATDIR1         : 1;
   __REG32 SPIENDIR           : 1;
   __REG32 SSB                : 1;
   __REG32                    :20;
} __mcspi_syst_bits;

/* MCSPIn_MODULCTRL */
typedef struct {
   __REG32 SINGLE             : 1;
   __REG32 PIN34              : 1;
   __REG32 MS                 : 1;
   __REG32 SYSTEM_TEST        : 1;
   __REG32 INITDLY            : 3;
   __REG32 MOA                : 1;
   __REG32 FDAA               : 1;
   __REG32                    :23;
} __mcspi_modulctrl_bits;

/* MCSPIn_CHxCONF */
typedef struct {
   __REG32 PHA                : 1;
   __REG32 POL                : 1;
   __REG32 CLKD               : 4; //6
   __REG32 EPOL               : 1;
   __REG32 WL                 : 5; //12
   __REG32 TRM                : 2;
   __REG32 DMAW               : 1;
   __REG32 DMAR               : 1;
   __REG32 DPE0               : 1;
   __REG32 DPE1               : 1;
   __REG32 IS                 : 1;
   __REG32 TURBO              : 1; //20
   __REG32 FORCE              : 1;
   __REG32 SPIENSLV           : 2;
   __REG32 SBE                : 1;
   __REG32 SBPOL              : 1;
   __REG32 TCS                : 2;
   __REG32 FFEW               : 1;
   __REG32 FFER               : 1;
   __REG32 CLKG               : 1;
   __REG32                    : 2;
} __mcspi_chxconf_bits;

/* MCSPIn_CHxSTAT */
typedef struct {
   __REG32 RXS                : 1;
   __REG32 TXS                : 1;
   __REG32 EOT                : 1;
   __REG32 TXFFE              : 1;
   __REG32 TXFFF              : 1;
   __REG32 RXFFE              : 1;
   __REG32 RXFFF              : 1;
   __REG32                    :25;
} __mcspi_chxstat_bits;

/* MCSPIn_CHxCTRL */
typedef struct {
   __REG32 EN                 : 1;
   __REG32                    : 7;
   __REG32 EXTCLK             : 8;
   __REG32                    :16;
} __mcspi_chxctrl_bits;

/* MCSPIn_XFERLEVEL */
typedef struct {
   __REG32 AEL                : 8;
   __REG32 AFL                : 8;
   __REG32 WCNT               :16;
} __mcspi_xferlevel_bits;

/* GPIOn_REVISION */
typedef struct {
   __REG32 MINOR              : 6;
   __REG32 CUSTOM             : 2;
   __REG32 MAJOR              : 3;
   __REG32 RTL                : 5;
   __REG32 FUNC               :12;
   __REG32                    : 2;
   __REG32 SCHEME             : 2;
} __gpio_revision_bits;

/* GPIOn_SYSCONFIG */
typedef struct {
   __REG32 AUTOIDLE           : 1;
   __REG32 SOFTRESET          : 1;
   __REG32                    : 1;
   __REG32 IDLEMODE           : 2;
   __REG32                    :27;
} __gpio_sysconfig_bits;

/* GPIO_EOI */
typedef struct {
   __REG32 LINE_NUMBER        : 1;
   __REG32                    :31;
} __gpio_eoi_bits;

/* GPIOn_IRQSTATUS */
typedef struct {
   __REG32 INTLINE0           : 1;
   __REG32 INTLINE1           : 1;
   __REG32 INTLINE2           : 1;
   __REG32 INTLINE3           : 1;
   __REG32 INTLINE4           : 1;
   __REG32 INTLINE5           : 1;
   __REG32 INTLINE6           : 1;
   __REG32 INTLINE7           : 1;
   __REG32 INTLINE8           : 1;
   __REG32 INTLINE9           : 1;
   __REG32 INTLINE10          : 1;
   __REG32 INTLINE11          : 1;
   __REG32 INTLINE12          : 1;
   __REG32 INTLINE13          : 1;
   __REG32 INTLINE14          : 1;
   __REG32 INTLINE15          : 1;
   __REG32 INTLINE16          : 1;
   __REG32 INTLINE17          : 1;
   __REG32 INTLINE18          : 1;
   __REG32 INTLINE19          : 1;
   __REG32 INTLINE20          : 1;
   __REG32 INTLINE21          : 1;
   __REG32 INTLINE22          : 1;
   __REG32 INTLINE23          : 1;
   __REG32 INTLINE24          : 1;
   __REG32 INTLINE25          : 1;
   __REG32 INTLINE26          : 1;
   __REG32 INTLINE27          : 1;
   __REG32 INTLINE28          : 1;
   __REG32 INTLINE29          : 1;
   __REG32 INTLINE30          : 1;
   __REG32 INTLINE31          : 1;
} __gpio_irqstatus_bits;

#define GPIO_IRQ_INTLINE0     (0x1UL<<0)
#define GPIO_IRQ_INTLINE1     (0x1UL<<1)
#define GPIO_IRQ_INTLINE2     (0x1UL<<2)
#define GPIO_IRQ_INTLINE3     (0x1UL<<3)
#define GPIO_IRQ_INTLINE4     (0x1UL<<4)
#define GPIO_IRQ_INTLINE5     (0x1UL<<5)
#define GPIO_IRQ_INTLINE6     (0x1UL<<6)
#define GPIO_IRQ_INTLINE7     (0x1UL<<7)
#define GPIO_IRQ_INTLINE8     (0x1UL<<8)
#define GPIO_IRQ_INTLINE9     (0x1UL<<9)
#define GPIO_IRQ_INTLINE10    (0x1UL<<10)
#define GPIO_IRQ_INTLINE11    (0x1UL<<11)
#define GPIO_IRQ_INTLINE12    (0x1UL<<12)
#define GPIO_IRQ_INTLINE13    (0x1UL<<13)
#define GPIO_IRQ_INTLINE14    (0x1UL<<14)
#define GPIO_IRQ_INTLINE15    (0x1UL<<15)
#define GPIO_IRQ_INTLINE16    (0x1UL<<16)
#define GPIO_IRQ_INTLINE17    (0x1UL<<17)
#define GPIO_IRQ_INTLINE18    (0x1UL<<18)
#define GPIO_IRQ_INTLINE19    (0x1UL<<19)
#define GPIO_IRQ_INTLINE20    (0x1UL<<20)
#define GPIO_IRQ_INTLINE21    (0x1UL<<21)
#define GPIO_IRQ_INTLINE22    (0x1UL<<22)
#define GPIO_IRQ_INTLINE23    (0x1UL<<23)
#define GPIO_IRQ_INTLINE24    (0x1UL<<24)
#define GPIO_IRQ_INTLINE25    (0x1UL<<25)
#define GPIO_IRQ_INTLINE26    (0x1UL<<26)
#define GPIO_IRQ_INTLINE27    (0x1UL<<27)
#define GPIO_IRQ_INTLINE28    (0x1UL<<28)
#define GPIO_IRQ_INTLINE29    (0x1UL<<29)
#define GPIO_IRQ_INTLINE30    (0x1UL<<30)
#define GPIO_IRQ_INTLINE31    (0x1UL<<31)

/* GPIOn_SYSSTATUS */
typedef struct {
   __REG32 RESETDONE          : 1;
   __REG32                    :31;
} __gpio_sysstatus_bits;

/* GPIOn_CTRL */
typedef struct {
   __REG32 DISABLEMODULE      : 1;
   __REG32 GATINGRATIO        : 2;
   __REG32                    :29;
} __gpio_ctrl_bits;

/* GPIOn_OE */
typedef struct {
   __REG32 OUTPUTEN0          : 1;
   __REG32 OUTPUTEN1          : 1;
   __REG32 OUTPUTEN2          : 1;
   __REG32 OUTPUTEN3          : 1;
   __REG32 OUTPUTEN4          : 1;
   __REG32 OUTPUTEN5          : 1;
   __REG32 OUTPUTEN6          : 1;
   __REG32 OUTPUTEN7          : 1;
   __REG32 OUTPUTEN8          : 1;
   __REG32 OUTPUTEN9          : 1;
   __REG32 OUTPUTEN10         : 1;
   __REG32 OUTPUTEN11         : 1;
   __REG32 OUTPUTEN12         : 1;
   __REG32 OUTPUTEN13         : 1;
   __REG32 OUTPUTEN14         : 1;
   __REG32 OUTPUTEN15         : 1;
   __REG32 OUTPUTEN16         : 1;
   __REG32 OUTPUTEN17         : 1;
   __REG32 OUTPUTEN18         : 1;
   __REG32 OUTPUTEN19         : 1;
   __REG32 OUTPUTEN20         : 1;
   __REG32 OUTPUTEN21         : 1;
   __REG32 OUTPUTEN22         : 1;
   __REG32 OUTPUTEN23         : 1;
   __REG32 OUTPUTEN24         : 1;
   __REG32 OUTPUTEN25         : 1;
   __REG32 OUTPUTEN26         : 1;
   __REG32 OUTPUTEN27         : 1;
   __REG32 OUTPUTEN28         : 1;
   __REG32 OUTPUTEN29         : 1;
   __REG32 OUTPUTEN30         : 1;
   __REG32 OUTPUTEN31         : 1;
} __gpio_oe_bits;

/* GPIOn_DATAIN */
typedef struct {
   __REG32 DATAIN0            : 1;
   __REG32 DATAIN1            : 1;
   __REG32 DATAIN2            : 1;
   __REG32 DATAIN3            : 1;
   __REG32 DATAIN4            : 1;
   __REG32 DATAIN5            : 1;
   __REG32 DATAIN6            : 1;
   __REG32 DATAIN7            : 1;
   __REG32 DATAIN8            : 1;
   __REG32 DATAIN9            : 1;
   __REG32 DATAIN10           : 1;
   __REG32 DATAIN11           : 1;
   __REG32 DATAIN12           : 1;
   __REG32 DATAIN13           : 1;
   __REG32 DATAIN14           : 1;
   __REG32 DATAIN15           : 1;
   __REG32 DATAIN16           : 1;
   __REG32 DATAIN17           : 1;
   __REG32 DATAIN18           : 1;
   __REG32 DATAIN19           : 1;
   __REG32 DATAIN20           : 1;
   __REG32 DATAIN21           : 1;
   __REG32 DATAIN22           : 1;
   __REG32 DATAIN23           : 1;
   __REG32 DATAIN24           : 1;
   __REG32 DATAIN25           : 1;
   __REG32 DATAIN26           : 1;
   __REG32 DATAIN27           : 1;
   __REG32 DATAIN28           : 1;
   __REG32 DATAIN29           : 1;
   __REG32 DATAIN30           : 1;
   __REG32 DATAIN31           : 1;
} __gpio_datain_bits;

/* GPIOn_DATAOUT */
typedef struct {
   __REG32 DATAOUT0           : 1;
   __REG32 DATAOUT1           : 1;
   __REG32 DATAOUT2           : 1;
   __REG32 DATAOUT3           : 1;
   __REG32 DATAOUT4           : 1;
   __REG32 DATAOUT5           : 1;
   __REG32 DATAOUT6           : 1;
   __REG32 DATAOUT7           : 1;
   __REG32 DATAOUT8           : 1;
   __REG32 DATAOUT9           : 1;
   __REG32 DATAOUT10          : 1;
   __REG32 DATAOUT11          : 1;
   __REG32 DATAOUT12          : 1;
   __REG32 DATAOUT13          : 1;
   __REG32 DATAOUT14          : 1;
   __REG32 DATAOUT15          : 1;
   __REG32 DATAOUT16          : 1;
   __REG32 DATAOUT17          : 1;
   __REG32 DATAOUT18          : 1;
   __REG32 DATAOUT19          : 1;
   __REG32 DATAOUT20          : 1;
   __REG32 DATAOUT21          : 1;
   __REG32 DATAOUT22          : 1;
   __REG32 DATAOUT23          : 1;
   __REG32 DATAOUT24          : 1;
   __REG32 DATAOUT25          : 1;
   __REG32 DATAOUT26          : 1;
   __REG32 DATAOUT27          : 1;
   __REG32 DATAOUT28          : 1;
   __REG32 DATAOUT29          : 1;
   __REG32 DATAOUT30          : 1;
   __REG32 DATAOUT31          : 1;
} __gpio_dataout_bits;

#define GPIO_DATA0            (0x1UL<<0)
#define GPIO_DATA1            (0x1UL<<1)
#define GPIO_DATA2            (0x1UL<<2)
#define GPIO_DATA3            (0x1UL<<3)
#define GPIO_DATA4            (0x1UL<<4)
#define GPIO_DATA5            (0x1UL<<5)
#define GPIO_DATA6            (0x1UL<<6)
#define GPIO_DATA7            (0x1UL<<7)
#define GPIO_DATA8            (0x1UL<<8)
#define GPIO_DATA9            (0x1UL<<9)
#define GPIO_DATA10           (0x1UL<<10)
#define GPIO_DATA11           (0x1UL<<11)
#define GPIO_DATA12           (0x1UL<<12)
#define GPIO_DATA13           (0x1UL<<13)
#define GPIO_DATA14           (0x1UL<<14)
#define GPIO_DATA15           (0x1UL<<15)
#define GPIO_DATA16           (0x1UL<<16)
#define GPIO_DATA17           (0x1UL<<17)
#define GPIO_DATA18           (0x1UL<<18)
#define GPIO_DATA19           (0x1UL<<19)
#define GPIO_DATA20           (0x1UL<<20)
#define GPIO_DATA21           (0x1UL<<21)
#define GPIO_DATA22           (0x1UL<<22)
#define GPIO_DATA23           (0x1UL<<23)
#define GPIO_DATA24           (0x1UL<<24)
#define GPIO_DATA25           (0x1UL<<25)
#define GPIO_DATA26           (0x1UL<<26)
#define GPIO_DATA27           (0x1UL<<27)
#define GPIO_DATA28           (0x1UL<<28)
#define GPIO_DATA29           (0x1UL<<29)
#define GPIO_DATA30           (0x1UL<<30)
#define GPIO_DATA31           (0x1UL<<31)

/* GPIO1_LEVELDETECT0 */
typedef struct {
   __REG32 LOWLEVEL0          : 1;
   __REG32 LOWLEVEL1          : 1;
   __REG32 LOWLEVEL2          : 1;
   __REG32 LOWLEVEL3          : 1;
   __REG32 LOWLEVEL4          : 1;
   __REG32 LOWLEVEL5          : 1;
   __REG32 LOWLEVEL6          : 1;
   __REG32 LOWLEVEL7          : 1;
   __REG32 LOWLEVEL8          : 1;
   __REG32 LOWLEVEL9          : 1;
   __REG32 LOWLEVEL10         : 1;
   __REG32 LOWLEVEL11         : 1;
   __REG32 LOWLEVEL12         : 1;
   __REG32 LOWLEVEL13         : 1;
   __REG32 LOWLEVEL14         : 1;
   __REG32 LOWLEVEL15         : 1;
   __REG32 LOWLEVEL16         : 1;
   __REG32 LOWLEVEL17         : 1;
   __REG32 LOWLEVEL18         : 1;
   __REG32 LOWLEVEL19         : 1;
   __REG32 LOWLEVEL20         : 1;
   __REG32 LOWLEVEL21         : 1;
   __REG32 LOWLEVEL22         : 1;
   __REG32 LOWLEVEL23         : 1;
   __REG32 LOWLEVEL24         : 1;
   __REG32 LOWLEVEL25         : 1;
   __REG32 LOWLEVEL26         : 1;
   __REG32 LOWLEVEL27         : 1;
   __REG32 LOWLEVEL28         : 1;
   __REG32 LOWLEVEL29         : 1;
   __REG32 LOWLEVEL30         : 1;
   __REG32 LOWLEVEL31         : 1;
} __gpio_leveldetect0_bits;

/* GPIO1_LEVELDETECT1 */
typedef struct {
   __REG32 HIGHLEVEL0         : 1;
   __REG32 HIGHLEVEL1         : 1;
   __REG32 HIGHLEVEL2         : 1;
   __REG32 HIGHLEVEL3         : 1;
   __REG32 HIGHLEVEL4         : 1;
   __REG32 HIGHLEVEL5         : 1;
   __REG32 HIGHLEVEL6         : 1;
   __REG32 HIGHLEVEL7         : 1;
   __REG32 HIGHLEVEL8         : 1;
   __REG32 HIGHLEVEL9         : 1;
   __REG32 HIGHLEVEL10        : 1;
   __REG32 HIGHLEVEL11        : 1;
   __REG32 HIGHLEVEL12        : 1;
   __REG32 HIGHLEVEL13        : 1;
   __REG32 HIGHLEVEL14        : 1;
   __REG32 HIGHLEVEL15        : 1;
   __REG32 HIGHLEVEL16        : 1;
   __REG32 HIGHLEVEL17        : 1;
   __REG32 HIGHLEVEL18        : 1;
   __REG32 HIGHLEVEL19        : 1;
   __REG32 HIGHLEVEL20        : 1;
   __REG32 HIGHLEVEL21        : 1;
   __REG32 HIGHLEVEL22        : 1;
   __REG32 HIGHLEVEL23        : 1;
   __REG32 HIGHLEVEL24        : 1;
   __REG32 HIGHLEVEL25        : 1;
   __REG32 HIGHLEVEL26        : 1;
   __REG32 HIGHLEVEL27        : 1;
   __REG32 HIGHLEVEL28        : 1;
   __REG32 HIGHLEVEL29        : 1;
   __REG32 HIGHLEVEL30        : 1;
   __REG32 HIGHLEVEL31        : 1;
} __gpio_leveldetect1_bits;

/* GPIOn_RISINGDETECT */
typedef struct {
   __REG32 RISINGEDGE0        : 1;
   __REG32 RISINGEDGE1        : 1;
   __REG32 RISINGEDGE2        : 1;
   __REG32 RISINGEDGE3        : 1;
   __REG32 RISINGEDGE4        : 1;
   __REG32 RISINGEDGE5        : 1;
   __REG32 RISINGEDGE6        : 1;
   __REG32 RISINGEDGE7        : 1;
   __REG32 RISINGEDGE8        : 1;
   __REG32 RISINGEDGE9        : 1;
   __REG32 RISINGEDGE10       : 1;
   __REG32 RISINGEDGE11       : 1;
   __REG32 RISINGEDGE12       : 1;
   __REG32 RISINGEDGE13       : 1;
   __REG32 RISINGEDGE14       : 1;
   __REG32 RISINGEDGE15       : 1;
   __REG32 RISINGEDGE16       : 1;
   __REG32 RISINGEDGE17       : 1;
   __REG32 RISINGEDGE18       : 1;
   __REG32 RISINGEDGE19       : 1;
   __REG32 RISINGEDGE20       : 1;
   __REG32 RISINGEDGE21       : 1;
   __REG32 RISINGEDGE22       : 1;
   __REG32 RISINGEDGE23       : 1;
   __REG32 RISINGEDGE24       : 1;
   __REG32 RISINGEDGE25       : 1;
   __REG32 RISINGEDGE26       : 1;
   __REG32 RISINGEDGE27       : 1;
   __REG32 RISINGEDGE28       : 1;
   __REG32 RISINGEDGE29       : 1;
   __REG32 RISINGEDGE30       : 1;
   __REG32 RISINGEDGE31       : 1;
} __gpio_risingdetect_bits;

/* GPIOn_FALLINGDETECT */
typedef struct {
   __REG32 FALLINGEDGE0       : 1;
   __REG32 FALLINGEDGE1       : 1;
   __REG32 FALLINGEDGE2       : 1;
   __REG32 FALLINGEDGE3       : 1;
   __REG32 FALLINGEDGE4       : 1;
   __REG32 FALLINGEDGE5       : 1;
   __REG32 FALLINGEDGE6       : 1;
   __REG32 FALLINGEDGE7       : 1;
   __REG32 FALLINGEDGE8       : 1;
   __REG32 FALLINGEDGE9       : 1;
   __REG32 FALLINGEDGE10      : 1;
   __REG32 FALLINGEDGE11      : 1;
   __REG32 FALLINGEDGE12      : 1;
   __REG32 FALLINGEDGE13      : 1;
   __REG32 FALLINGEDGE14      : 1;
   __REG32 FALLINGEDGE15      : 1;
   __REG32 FALLINGEDGE16      : 1;
   __REG32 FALLINGEDGE17      : 1;
   __REG32 FALLINGEDGE18      : 1;
   __REG32 FALLINGEDGE19      : 1;
   __REG32 FALLINGEDGE20      : 1;
   __REG32 FALLINGEDGE21      : 1;
   __REG32 FALLINGEDGE22      : 1;
   __REG32 FALLINGEDGE23      : 1;
   __REG32 FALLINGEDGE24      : 1;
   __REG32 FALLINGEDGE25      : 1;
   __REG32 FALLINGEDGE26      : 1;
   __REG32 FALLINGEDGE27      : 1;
   __REG32 FALLINGEDGE28      : 1;
   __REG32 FALLINGEDGE29      : 1;
   __REG32 FALLINGEDGE30      : 1;
   __REG32 FALLINGEDGE31      : 1;
} __gpio_fallingdetect_bits;

/* GPIOn_DEBOUNCENABLE */
typedef struct {
   __REG32 DEBOUNCEEN0        : 1;
   __REG32 DEBOUNCEEN1        : 1;
   __REG32 DEBOUNCEEN2        : 1;
   __REG32 DEBOUNCEEN3        : 1;
   __REG32 DEBOUNCEEN4        : 1;
   __REG32 DEBOUNCEEN5        : 1;
   __REG32 DEBOUNCEEN6        : 1;
   __REG32 DEBOUNCEEN7        : 1;
   __REG32 DEBOUNCEEN8        : 1;
   __REG32 DEBOUNCEEN9        : 1;
   __REG32 DEBOUNCEEN10       : 1;
   __REG32 DEBOUNCEEN11       : 1;
   __REG32 DEBOUNCEEN12       : 1;
   __REG32 DEBOUNCEEN13       : 1;
   __REG32 DEBOUNCEEN14       : 1;
   __REG32 DEBOUNCEEN15       : 1;
   __REG32 DEBOUNCEEN16       : 1;
   __REG32 DEBOUNCEEN17       : 1;
   __REG32 DEBOUNCEEN18       : 1;
   __REG32 DEBOUNCEEN19       : 1;
   __REG32 DEBOUNCEEN20       : 1;
   __REG32 DEBOUNCEEN21       : 1;
   __REG32 DEBOUNCEEN22       : 1;
   __REG32 DEBOUNCEEN23       : 1;
   __REG32 DEBOUNCEEN24       : 1;
   __REG32 DEBOUNCEEN25       : 1;
   __REG32 DEBOUNCEEN26       : 1;
   __REG32 DEBOUNCEEN27       : 1;
   __REG32 DEBOUNCEEN28       : 1;
   __REG32 DEBOUNCEEN29       : 1;
   __REG32 DEBOUNCEEN30       : 1;
   __REG32 DEBOUNCEEN31       : 1;
} __gpio_debouncenable_bits;

/* GPIOn_DEBOUNCINGTIME */
typedef struct {
   __REG32 DEBOUNCEVAL        : 8;
   __REG32                    :24;
} __gpio_debouncingtime_bits;

/* DEBUGSS_DRM_xxxx_SCR */
typedef struct {
   __REG32 SensCtrl                 : 1;
   __REG32                          : 2;
   __REG32 Suspend_Default_Override : 1;
   __REG32 Suspend_Sel              : 4;
   __REG32                          :24;
} __debugss_drm_scr_bits;

//#endif    /* __IAR_SYSTEMS_ICC__ */

/* Common declarations  ****************************************************/
/***************************************************************************
 **
 ** PRUSS INTC Registers
 **
 ***************************************************************************/
//__attribute__((noinit,location(0x4A320000))) union {
//    volatile const unsigned long LONG;
//    volatile const  __pruss_intc_revid_bits BIT;
//} PRUSS_INTC_REVID;

__IO_REG32_BIT(PRUSS_INTC_REVID,      0x4A320000,__READ       ,__pruss_intc_revid_bits);
__IO_REG32_BIT(PRUSS_INTC_CR,         0x4A320004,__READ_WRITE ,__pruss_intc_cr_bits);
__IO_REG32_BIT(PRUSS_INTC_GER,        0x4A320010,__READ_WRITE ,__pruss_intc_ger_bits);
__IO_REG32_BIT(PRUSS_INTC_GNLR,       0x4A32001C,__READ_WRITE ,__pruss_intc_gnlr_bits);
__IO_REG32(    PRUSS_INTC_SISR,       0x4A320020,__WRITE      );
__IO_REG32(    PRUSS_INTC_SICR,       0x4A320024,__WRITE      );
__IO_REG32(    PRUSS_INTC_EISR,       0x4A320028,__WRITE      );
__IO_REG32(    PRUSS_INTC_EICR,       0x4A32002C,__WRITE      );
__IO_REG32(    PRUSS_INTC_HIEISR,     0x4A320034,__WRITE      );
__IO_REG32(    PRUSS_INTC_HIDISR,     0x4A320038,__WRITE      );
__IO_REG32_BIT(PRUSS_INTC_GPIR,       0x4A320080,__READ       ,__pruss_intc_gpir_bits);
__IO_REG32_BIT(PRUSS_INTC_SRSR0,      0x4A320200,__READ_WRITE ,__pruss_intc_srsr0_bits);
__IO_REG32_BIT(PRUSS_INTC_SRSR1,      0x4A320204,__READ_WRITE ,__pruss_intc_srsr1_bits);
__IO_REG32_BIT(PRUSS_INTC_SECR0,      0x4A320280,__READ_WRITE ,__pruss_intc_secr0_bits);
__IO_REG32_BIT(PRUSS_INTC_SECR1,      0x4A320284,__READ_WRITE ,__pruss_intc_secr1_bits);
__IO_REG32_BIT(PRUSS_INTC_ESR0,       0x4A320300,__READ_WRITE ,__pruss_intc_esr0_bits);
__IO_REG32_BIT(PRUSS_INTC_ESR1,       0x4A320304,__READ_WRITE ,__pruss_intc_esr1_bits);
__IO_REG32_BIT(PRUSS_INTC_ECR0,       0x4A320380,__READ_WRITE ,__pruss_intc_ecr0_bits);
__IO_REG32_BIT(PRUSS_INTC_ECR1,       0x4A320384,__READ_WRITE ,__pruss_intc_ecr1_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR0,       0x4A320400,__READ_WRITE ,__pruss_intc_cmr0_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR1,       0x4A320404,__READ_WRITE ,__pruss_intc_cmr1_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR2,       0x4A320408,__READ_WRITE ,__pruss_intc_cmr2_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR3,       0x4A32040C,__READ_WRITE ,__pruss_intc_cmr3_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR4,       0x4A320410,__READ_WRITE ,__pruss_intc_cmr4_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR5,       0x4A320414,__READ_WRITE ,__pruss_intc_cmr5_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR6,       0x4A320418,__READ_WRITE ,__pruss_intc_cmr6_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR7,       0x4A32041C,__READ_WRITE ,__pruss_intc_cmr7_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR8,       0x4A320420,__READ_WRITE ,__pruss_intc_cmr8_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR9,       0x4A320424,__READ_WRITE ,__pruss_intc_cmr9_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR10,      0x4A320428,__READ_WRITE ,__pruss_intc_cmr10_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR11,      0x4A32042C,__READ_WRITE ,__pruss_intc_cmr11_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR12,      0x4A320430,__READ_WRITE ,__pruss_intc_cmr12_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR13,      0x4A320434,__READ_WRITE ,__pruss_intc_cmr13_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR14,      0x4A320438,__READ_WRITE ,__pruss_intc_cmr14_bits);
__IO_REG32_BIT(PRUSS_INTC_CMR15,      0x4A32043C,__READ_WRITE ,__pruss_intc_cmr15_bits);
__IO_REG32_BIT(PRUSS_INTC_HMR0,       0x4A320800,__READ_WRITE ,__pruss_intc_hmr0_bits);
__IO_REG32_BIT(PRUSS_INTC_HMR1,       0x4A320804,__READ_WRITE ,__pruss_intc_hmr1_bits);
__IO_REG32_BIT(PRUSS_INTC_HMR2,       0x4A320808,__READ_WRITE ,__pruss_intc_hmr2_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR0,     0x4A320900,__READ       ,__pruss_intc_hipir0_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR1,     0x4A320904,__READ       ,__pruss_intc_hipir1_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR2,     0x4A320908,__READ       ,__pruss_intc_hipir2_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR3,     0x4A32090C,__READ       ,__pruss_intc_hipir3_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR4,     0x4A320910,__READ       ,__pruss_intc_hipir4_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR5,     0x4A320914,__READ       ,__pruss_intc_hipir5_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR6,     0x4A320918,__READ       ,__pruss_intc_hipir6_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR7,     0x4A32091C,__READ       ,__pruss_intc_hipir7_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR8,     0x4A320920,__READ       ,__pruss_intc_hipir8_bits);
__IO_REG32_BIT(PRUSS_INTC_HIPIR9,     0x4A320924,__READ       ,__pruss_intc_hipir9_bits);
__IO_REG32_BIT(PRUSS_INTC_SIPR0,      0x4A320D00,__READ_WRITE ,__pruss_intc_sipr0_bits);
__IO_REG32_BIT(PRUSS_INTC_SIPR1,      0x4A320D04,__READ_WRITE ,__pruss_intc_sipr1_bits);
__IO_REG32_BIT(PRUSS_INTC_SITR0,      0x4A320D80,__READ_WRITE ,__pruss_intc_sitr0_bits);
__IO_REG32_BIT(PRUSS_INTC_SITR1,      0x4A320D84,__READ_WRITE ,__pruss_intc_sitr1_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR0,     0x4A321100,__READ_WRITE ,__pruss_intc_hinlr0_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR1,     0x4A321104,__READ_WRITE ,__pruss_intc_hinlr1_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR2,     0x4A321108,__READ_WRITE ,__pruss_intc_hinlr2_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR3,     0x4A32110C,__READ_WRITE ,__pruss_intc_hinlr3_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR4,     0x4A321110,__READ_WRITE ,__pruss_intc_hinlr4_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR5,     0x4A321114,__READ_WRITE ,__pruss_intc_hinlr5_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR6,     0x4A321118,__READ_WRITE ,__pruss_intc_hinlr6_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR7,     0x4A32111C,__READ_WRITE ,__pruss_intc_hinlr7_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR8,     0x4A321120,__READ_WRITE ,__pruss_intc_hinlr8_bits);
__IO_REG32_BIT(PRUSS_INTC_HINLR9,     0x4A321124,__READ_WRITE ,__pruss_intc_hinlr9_bits);
__IO_REG32_BIT(PRUSS_INTC_HIER,       0x4A321500,__READ_WRITE ,__pruss_intc_hier_bits);

/***************************************************************************
 **
 ** PRUSS PRU0 CTRL Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(PRU0_CTRL,             0x4A322000,__READ_WRITE ,__pru_ctrl_bits);
__IO_REG32_BIT(PRU0_STATUS,           0x4A322004,__READ       ,__pru_status_bits);
__IO_REG32_BIT(PRU0_WAKEUP_EN,        0x4A322008,__READ_WRITE ,__pru_wakeup_en_bits);
__IO_REG32(    PRU0_CYCLE,            0x4A32200C,__READ_WRITE );
__IO_REG32(    PRU0_STALL,            0x4A322010,__READ_WRITE );
__IO_REG32_BIT(PRU0_CTBIR0,           0x4A322020,__READ_WRITE ,__pru_ctbir0_bits);
__IO_REG32_BIT(PRU0_CTBIR1,           0x4A322024,__READ_WRITE ,__pru_ctbir1_bits);
__IO_REG32_BIT(PRU0_CTPPR0,           0x4A322028,__READ_WRITE ,__pru_ctppr0_bits);
__IO_REG32_BIT(PRU0_CTPPR1,           0x4A32202C,__READ_WRITE ,__pru_ctppr1_bits);

/***************************************************************************
 **
 ** PRUSS PRU0 DEBUG Registers
 **
 ***************************************************************************/
__IO_REG32(    PRU0_GPREG0,           0x4A322400,__READ_WRITE );
__IO_REG32(    PRU0_GPREG1,           0x4A322404,__READ_WRITE );
__IO_REG32(    PRU0_GPREG2,           0x4A322408,__READ_WRITE );
__IO_REG32(    PRU0_GPREG3,           0x4A32240C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG4,           0x4A322410,__READ_WRITE );
__IO_REG32(    PRU0_GPREG5,           0x4A322414,__READ_WRITE );
__IO_REG32(    PRU0_GPREG6,           0x4A322418,__READ_WRITE );
__IO_REG32(    PRU0_GPREG7,           0x4A32241C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG8,           0x4A322420,__READ_WRITE );
__IO_REG32(    PRU0_GPREG9,           0x4A322424,__READ_WRITE );
__IO_REG32(    PRU0_GPREG10,          0x4A322428,__READ_WRITE );
__IO_REG32(    PRU0_GPREG11,          0x4A32242C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG12,          0x4A322430,__READ_WRITE );
__IO_REG32(    PRU0_GPREG13,          0x4A322434,__READ_WRITE );
__IO_REG32(    PRU0_GPREG14,          0x4A322438,__READ_WRITE );
__IO_REG32(    PRU0_GPREG15,          0x4A32243C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG16,          0x4A322440,__READ_WRITE );
__IO_REG32(    PRU0_GPREG17,          0x4A322444,__READ_WRITE );
__IO_REG32(    PRU0_GPREG18,          0x4A322448,__READ_WRITE );
__IO_REG32(    PRU0_GPREG19,          0x4A32244C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG20,          0x4A322450,__READ_WRITE );
__IO_REG32(    PRU0_GPREG21,          0x4A322454,__READ_WRITE );
__IO_REG32(    PRU0_GPREG22,          0x4A322458,__READ_WRITE );
__IO_REG32(    PRU0_GPREG23,          0x4A32245C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG24,          0x4A322460,__READ_WRITE );
__IO_REG32(    PRU0_GPREG25,          0x4A322464,__READ_WRITE );
__IO_REG32(    PRU0_GPREG26,          0x4A322468,__READ_WRITE );
__IO_REG32(    PRU0_GPREG27,          0x4A32246C,__READ_WRITE );
__IO_REG32(    PRU0_GPREG28,          0x4A322470,__READ_WRITE );
__IO_REG32(    PRU0_GPREG29,          0x4A322474,__READ_WRITE );
__IO_REG32(    PRU0_GPREG30,          0x4A322478,__READ_WRITE );
__IO_REG32(    PRU0_GPREG31,          0x4A32247C,__READ_WRITE );
__IO_REG32(    PRU0_CT_REG0,          0x4A322480,__READ       );
__IO_REG32(    PRU0_CT_REG1,          0x4A322484,__READ       );
__IO_REG32(    PRU0_CT_REG2,          0x4A322488,__READ       );
__IO_REG32(    PRU0_CT_REG3,          0x4A32248C,__READ       );
__IO_REG32(    PRU0_CT_REG4,          0x4A322490,__READ       );
__IO_REG32(    PRU0_CT_REG5,          0x4A322494,__READ       );
__IO_REG32(    PRU0_CT_REG6,          0x4A322498,__READ       );
__IO_REG32(    PRU0_CT_REG7,          0x4A32249C,__READ       );
__IO_REG32(    PRU0_CT_REG8,          0x4A3224A0,__READ       );
__IO_REG32(    PRU0_CT_REG9,          0x4A3224A4,__READ       );
__IO_REG32(    PRU0_CT_REG10,         0x4A3224A8,__READ       );
__IO_REG32(    PRU0_CT_REG11,         0x4A3224AC,__READ       );
__IO_REG32(    PRU0_CT_REG12,         0x4A3224B0,__READ       );
__IO_REG32(    PRU0_CT_REG13,         0x4A3224B4,__READ       );
__IO_REG32(    PRU0_CT_REG14,         0x4A3224B8,__READ       );
__IO_REG32(    PRU0_CT_REG15,         0x4A3224BC,__READ       );
__IO_REG32(    PRU0_CT_REG16,         0x4A3224C0,__READ       );
__IO_REG32(    PRU0_CT_REG17,         0x4A3224C4,__READ       );
__IO_REG32(    PRU0_CT_REG18,         0x4A3224C8,__READ       );
__IO_REG32(    PRU0_CT_REG19,         0x4A3224CC,__READ       );
__IO_REG32(    PRU0_CT_REG20,         0x4A3224D0,__READ       );
__IO_REG32(    PRU0_CT_REG21,         0x4A3224D4,__READ       );
__IO_REG32(    PRU0_CT_REG22,         0x4A3224D8,__READ       );
__IO_REG32(    PRU0_CT_REG23,         0x4A3224DC,__READ       );
__IO_REG32(    PRU0_CT_REG24,         0x4A3224E0,__READ       );
__IO_REG32(    PRU0_CT_REG25,         0x4A3224E4,__READ       );
__IO_REG32(    PRU0_CT_REG26,         0x4A3224E8,__READ       );
__IO_REG32(    PRU0_CT_REG27,         0x4A3224EC,__READ       );
__IO_REG32(    PRU0_CT_REG28,         0x4A3224F0,__READ       );
__IO_REG32(    PRU0_CT_REG29,         0x4A3224F4,__READ       );
__IO_REG32(    PRU0_CT_REG30,         0x4A3224F8,__READ       );
__IO_REG32(    PRU0_CT_REG31,         0x4A3224FC,__READ       );

/***************************************************************************
 **
 ** PRUSS PRU1 CTRL Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(PRU1_CTRL,             0x4A324000,__READ_WRITE ,__pru_ctrl_bits);
__IO_REG32_BIT(PRU1_STATUS,           0x4A324004,__READ       ,__pru_status_bits);
__IO_REG32_BIT(PRU1_WAKEUP_EN,        0x4A324008,__READ_WRITE ,__pru_wakeup_en_bits);
__IO_REG32(    PRU1_CYCLE,            0x4A32400C,__READ_WRITE );
__IO_REG32(    PRU1_STALL,            0x4A324010,__READ_WRITE );
__IO_REG32_BIT(PRU1_CTBIR0,           0x4A324020,__READ_WRITE ,__pru_ctbir0_bits);
__IO_REG32_BIT(PRU1_CTBIR1,           0x4A324024,__READ_WRITE ,__pru_ctbir1_bits);
__IO_REG32_BIT(PRU1_CTPPR0,           0x4A324028,__READ_WRITE ,__pru_ctppr0_bits);
__IO_REG32_BIT(PRU1_CTPPR1,           0x4A32402C,__READ_WRITE ,__pru_ctppr1_bits);

/***************************************************************************
 **
 ** PRUSS PRU1 DEBUG Registers
 **
 ***************************************************************************/
__IO_REG32(    PRU1_GPREG0,           0x4A324400,__READ_WRITE );
__IO_REG32(    PRU1_GPREG1,           0x4A324404,__READ_WRITE );
__IO_REG32(    PRU1_GPREG2,           0x4A324408,__READ_WRITE );
__IO_REG32(    PRU1_GPREG3,           0x4A32440C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG4,           0x4A324410,__READ_WRITE );
__IO_REG32(    PRU1_GPREG5,           0x4A324414,__READ_WRITE );
__IO_REG32(    PRU1_GPREG6,           0x4A324418,__READ_WRITE );
__IO_REG32(    PRU1_GPREG7,           0x4A32441C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG8,           0x4A324420,__READ_WRITE );
__IO_REG32(    PRU1_GPREG9,           0x4A324424,__READ_WRITE );
__IO_REG32(    PRU1_GPREG10,          0x4A324428,__READ_WRITE );
__IO_REG32(    PRU1_GPREG11,          0x4A32442C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG12,          0x4A324430,__READ_WRITE );
__IO_REG32(    PRU1_GPREG13,          0x4A324434,__READ_WRITE );
__IO_REG32(    PRU1_GPREG14,          0x4A324438,__READ_WRITE );
__IO_REG32(    PRU1_GPREG15,          0x4A32443C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG16,          0x4A324440,__READ_WRITE );
__IO_REG32(    PRU1_GPREG17,          0x4A324444,__READ_WRITE );
__IO_REG32(    PRU1_GPREG18,          0x4A324448,__READ_WRITE );
__IO_REG32(    PRU1_GPREG19,          0x4A32444C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG20,          0x4A324450,__READ_WRITE );
__IO_REG32(    PRU1_GPREG21,          0x4A324454,__READ_WRITE );
__IO_REG32(    PRU1_GPREG22,          0x4A324458,__READ_WRITE );
__IO_REG32(    PRU1_GPREG23,          0x4A32445C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG24,          0x4A324460,__READ_WRITE );
__IO_REG32(    PRU1_GPREG25,          0x4A324464,__READ_WRITE );
__IO_REG32(    PRU1_GPREG26,          0x4A324468,__READ_WRITE );
__IO_REG32(    PRU1_GPREG27,          0x4A32446C,__READ_WRITE );
__IO_REG32(    PRU1_GPREG28,          0x4A324470,__READ_WRITE );
__IO_REG32(    PRU1_GPREG29,          0x4A324474,__READ_WRITE );
__IO_REG32(    PRU1_GPREG30,          0x4A324478,__READ_WRITE );
__IO_REG32(    PRU1_GPREG31,          0x4A32447C,__READ_WRITE );
__IO_REG32(    PRU1_CT_REG0,          0x4A324480,__READ       );
__IO_REG32(    PRU1_CT_REG1,          0x4A324484,__READ       );
__IO_REG32(    PRU1_CT_REG2,          0x4A324488,__READ       );
__IO_REG32(    PRU1_CT_REG3,          0x4A32448C,__READ       );
__IO_REG32(    PRU1_CT_REG4,          0x4A324490,__READ       );
__IO_REG32(    PRU1_CT_REG5,          0x4A324494,__READ       );
__IO_REG32(    PRU1_CT_REG6,          0x4A324498,__READ       );
__IO_REG32(    PRU1_CT_REG7,          0x4A32449C,__READ       );
__IO_REG32(    PRU1_CT_REG8,          0x4A3244A0,__READ       );
__IO_REG32(    PRU1_CT_REG9,          0x4A3244A4,__READ       );
__IO_REG32(    PRU1_CT_REG10,         0x4A3244A8,__READ       );
__IO_REG32(    PRU1_CT_REG11,         0x4A3244AC,__READ       );
__IO_REG32(    PRU1_CT_REG12,         0x4A3244B0,__READ       );
__IO_REG32(    PRU1_CT_REG13,         0x4A3244B4,__READ       );
__IO_REG32(    PRU1_CT_REG14,         0x4A3244B8,__READ       );
__IO_REG32(    PRU1_CT_REG15,         0x4A3244BC,__READ       );
__IO_REG32(    PRU1_CT_REG16,         0x4A3244C0,__READ       );
__IO_REG32(    PRU1_CT_REG17,         0x4A3244C4,__READ       );
__IO_REG32(    PRU1_CT_REG18,         0x4A3244C8,__READ       );
__IO_REG32(    PRU1_CT_REG19,         0x4A3244CC,__READ       );
__IO_REG32(    PRU1_CT_REG20,         0x4A3244D0,__READ       );
__IO_REG32(    PRU1_CT_REG21,         0x4A3244D4,__READ       );
__IO_REG32(    PRU1_CT_REG22,         0x4A3244D8,__READ       );
__IO_REG32(    PRU1_CT_REG23,         0x4A3244DC,__READ       );
__IO_REG32(    PRU1_CT_REG24,         0x4A3244E0,__READ       );
__IO_REG32(    PRU1_CT_REG25,         0x4A3244E4,__READ       );
__IO_REG32(    PRU1_CT_REG26,         0x4A3244E8,__READ       );
__IO_REG32(    PRU1_CT_REG27,         0x4A3244EC,__READ       );
__IO_REG32(    PRU1_CT_REG28,         0x4A3244F0,__READ       );
__IO_REG32(    PRU1_CT_REG29,         0x4A3244F4,__READ       );
__IO_REG32(    PRU1_CT_REG30,         0x4A3244F8,__READ       );
__IO_REG32(    PRU1_CT_REG31,         0x4A3244FC,__READ       );

/***************************************************************************
 **
 ** PRUSS UART0 Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(PRUSS_UART0_RBR,         0x4A328000,__READ_WRITE ,__pruss_uart_rbr_bits);
#define  PRUSS_UART0_THR        PRUSS_UART0_RBR
#define  PRUSS_UART0_THR_bit    PRUSS_UART0_RBR_bit
__IO_REG32_BIT(PRUSS_UART0_IER,         0x4A328004,__READ_WRITE ,__pruss_uart_ier_bits);
__IO_REG32_BIT(PRUSS_UART0_IIR,         0x4A328008,__READ_WRITE ,__pruss_uart_iir_bits);
#define  PRUSS_UART0_FCR        PRUSS_UART0_IIR
#define  PRUSS_UART0_FCR_bit    PRUSS_UART0_IIR_bit.__fcr_bits
__IO_REG32_BIT(PRUSS_UART0_LCR,         0x4A32800C,__READ_WRITE ,__pruss_uart_lcr_bits);
__IO_REG32_BIT(PRUSS_UART0_MCR,         0x4A328010,__READ_WRITE ,__pruss_uart_mcr_bits);
__IO_REG32_BIT(PRUSS_UART0_LSR,         0x4A328014,__READ       ,__pruss_uart_lsr_bits);
__IO_REG32_BIT(PRUSS_UART0_MSR,         0x4A328018,__READ       ,__pruss_uart_msr_bits);
__IO_REG32_BIT(PRUSS_UART0_SCR,         0x4A32801C,__READ_WRITE ,__pruss_uart_scr_bits);
__IO_REG32_BIT(PRUSS_UART0_DLL,         0x4A328020,__READ_WRITE ,__pruss_uart_dll_bits);
__IO_REG32_BIT(PRUSS_UART0_DLH,         0x4A328024,__READ_WRITE ,__pruss_uart_dlh_bits);
__IO_REG32(    PRUSS_UART0_REVID1,      0x4A328028,__READ       );
__IO_REG32_BIT(PRUSS_UART0_REVID2,      0x4A32802C,__READ       ,__pruss_uart_revid2_bits);
__IO_REG32_BIT(PRUSS_UART0_PWREMU_MGMT, 0x4A328030,__READ_WRITE ,__pruss_uart_pwremu_mgmt_bits);
__IO_REG32_BIT(PRUSS_UART0_MDR,         0x4A328034,__READ_WRITE ,__pruss_uart_mdr_bits);

/***************************************************************************
 **
 **  PRUSS eCAP Registers
 **
 ***************************************************************************/
__IO_REG32(    PRUSS_ECAP0_TSCTR,     0x4A330000,__READ_WRITE );
__IO_REG32(    PRUSS_ECAP0_CTRPHS,    0x4A330004,__READ_WRITE );
__IO_REG32(    PRUSS_ECAP0_CAP1,      0x4A330008,__READ_WRITE );
__IO_REG32(    PRUSS_ECAP0_CAP2,      0x4A33000C,__READ_WRITE );
__IO_REG32(    PRUSS_ECAP0_CAP3,      0x4A330010,__READ_WRITE );
__IO_REG32(    PRUSS_ECAP0_CAP4,      0x4A330014,__READ_WRITE );
__IO_REG16_BIT(PRUSS_ECAP0_ECCTL1,    0x4A330028,__READ_WRITE ,__ecap_ecctl1_bits);
__IO_REG16_BIT(PRUSS_ECAP0_ECCTL2,    0x4A33002A,__READ_WRITE ,__ecap_ecctl2_bits);
__IO_REG16_BIT(PRUSS_ECAP0_ECEINT,    0x4A33002C,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG16_BIT(PRUSS_ECAP0_ECFLG,     0x4A33002E,__READ       ,__ecap_ecflg_bits);
__IO_REG16_BIT(PRUSS_ECAP0_ECCLR,     0x4A330030,__READ_WRITE ,__ecap_ecflg_bits);
__IO_REG16_BIT(PRUSS_ECAP0_ECFRC,     0x4A330032,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG32(    PRUSS_ECAP0_REVID,     0x4A33005C,__READ       );

/***************************************************************************
 **
 ** PRUSS CFG Registers
 **
 ***************************************************************************/
__IO_REG32(    PRUSS_CFG_REVID,       0x4A326000,__READ_WRITE );
__IO_REG32_BIT(PRUSS_CFG_SYSCFG,      0x4A326004,__READ_WRITE ,__pruss_cfg_syscfg_bits);
__IO_REG32_BIT(PRUSS_CFG_GPCFG0,      0x4A326008,__READ_WRITE ,__pruss_cfg_gpcfg0_bits);
__IO_REG32_BIT(PRUSS_CFG_GPCFG1,      0x4A32600C,__READ_WRITE ,__pruss_cfg_gpcfg1_bits);
__IO_REG32_BIT(PRUSS_CFG_CGR,         0x4A326010,__READ_WRITE ,__pruss_cfg_cgr_bits);
__IO_REG32_BIT(PRUSS_CFG_ISRP,        0x4A326014,__READ_WRITE ,__pruss_cfg_isrp_bits);
__IO_REG32_BIT(PRUSS_CFG_ISP,         0x4A326018,__READ_WRITE ,__pruss_cfg_isp_bits);
__IO_REG32_BIT(PRUSS_CFG_IESP,        0x4A32601C,__READ_WRITE ,__pruss_cfg_iesp_bits);
__IO_REG32_BIT(PRUSS_CFG_IECP,        0x4A326020,__READ_WRITE ,__pruss_cfg_iecp_bits);
__IO_REG32_BIT(PRUSS_CFG_SCRP,        0x4A326024,__READ_WRITE ,__pruss_cfg_scrp_bits);
__IO_REG32_BIT(PRUSS_CFG_PMAO,        0x4A326028,__READ_WRITE ,__pruss_cfg_pmao_bits);
__IO_REG32_BIT(PRUSS_CFG_MII_RT,      0x4A32602C,__READ_WRITE ,__pruss_cfg_mii_rt_bits);
__IO_REG32_BIT(PRUSS_CFG_IEPCLK,      0x4A326030,__READ_WRITE ,__pruss_cfg_iepclk_bits);
__IO_REG32_BIT(PRUSS_CFG_SPP,         0x4A326034,__READ_WRITE ,__pruss_cfg_spp_bits);
__IO_REG32_BIT(PRUSS_CFG_PIN_MX,      0x4A326040,__READ_WRITE ,__pruss_cfg_pin_mx_bits);

/***************************************************************************
 **
 ** MPU INTC
 **
 ***************************************************************************/
__IO_REG32_BIT(INTC_REVISION,         0x48200000,__READ       ,__intc_revision_bits);
__IO_REG32_BIT(INTC_SYSCONFIG,        0x48200010,__READ_WRITE ,__intc_sysconfig_bits);
__IO_REG32_BIT(INTC_SYSSTATUS,        0x48200014,__READ       ,__intc_sysstatus_bits);
__IO_REG32_BIT(INTC_SIR_IRQ,          0x48200040,__READ       ,__intc_sir_irq_bits);
__IO_REG32_BIT(INTC_SIR_FIQ,          0x48200044,__READ       ,__intc_sir_fiq_bits);
__IO_REG32(    INTC_CONTROL,          0x48200048,__WRITE      );
__IO_REG32_BIT(INTC_PROTECTION,       0x4820004C,__READ_WRITE ,__intc_protection_bits);
__IO_REG32_BIT(INTC_IDLE,             0x48200050,__READ_WRITE ,__intc_idle_bits);
__IO_REG32_BIT(INTC_IRQ_PRIORITY,     0x48200060,__READ       ,__intc_irq_priority_bits);
__IO_REG32_BIT(INTC_FIQ_PRIORITY,     0x48200064,__READ       ,__intc_fiq_priority_bits);
__IO_REG32_BIT(INTC_THRESHOLD,        0x48200068,__READ_WRITE ,__intc_threshold_bits);
__IO_REG32_BIT(INTC_ITR0,             0x48200080,__READ       ,__intc_itr0_bits);
__IO_REG32_BIT(INTC_MIR0,             0x48200084,__READ_WRITE ,__intc_mir0_bits);
__IO_REG32(    INTC_MIR_CLEAR0,       0x48200088,__WRITE      );
__IO_REG32(    INTC_MIR_SET0,         0x4820008C,__WRITE      );
__IO_REG32_BIT(INTC_ISR_SET0,         0x48200090,__READ_WRITE ,__intc_isr_set0_bits);
__IO_REG32(    INTC_ISR_CLEAR0,       0x48200094,__WRITE      );
__IO_REG32_BIT(INTC_PENDING_IRQ0,     0x48200098,__READ       ,__intc_pending_irq0_bits);
__IO_REG32_BIT(INTC_PENDING_FIQ0,     0x4820009C,__READ       ,__intc_pending_fiq0_bits);
__IO_REG32_BIT(INTC_ITR1,             0x482000A0,__READ       ,__intc_itr1_bits);
__IO_REG32_BIT(INTC_MIR1,             0x482000A4,__READ_WRITE ,__intc_mir1_bits);
__IO_REG32(    INTC_MIR_CLEAR1,       0x482000A8,__WRITE      );
__IO_REG32(    INTC_MIR_SET1,         0x482000AC,__WRITE      );
__IO_REG32_BIT(INTC_ISR_SET1,         0x482000B0,__READ_WRITE ,__intc_isr_set1_bits);
__IO_REG32(    INTC_ISR_CLEAR1,       0x482000B4,__WRITE      );
__IO_REG32_BIT(INTC_PENDING_IRQ1,     0x482000B8,__READ       ,__intc_pending_irq1_bits);
__IO_REG32_BIT(INTC_PENDING_FIQ1,     0x482000BC,__READ       ,__intc_pending_fiq1_bits);
__IO_REG32_BIT(INTC_ITR2,             0x482000C0,__READ       ,__intc_itr2_bits);
__IO_REG32_BIT(INTC_MIR2,             0x482000C4,__READ_WRITE ,__intc_mir2_bits);
__IO_REG32(    INTC_MIR_CLEAR2,       0x482000C8,__WRITE      );
__IO_REG32(    INTC_MIR_SET2,         0x482000CC,__WRITE      );
__IO_REG32_BIT(INTC_ISR_SET2,         0x482000D0,__READ_WRITE ,__intc_isr_set2_bits);
__IO_REG32(    INTC_ISR_CLEAR2,       0x482000D4,__WRITE      );
__IO_REG32_BIT(INTC_PENDING_IRQ2,     0x482000D8,__READ       ,__intc_pending_irq2_bits);
__IO_REG32_BIT(INTC_PENDING_FIQ2,     0x482000DC,__READ       ,__intc_pending_fiq2_bits);
__IO_REG32_BIT(INTC_ITR3,             0x482000E0,__READ       ,__intc_itr3_bits);
__IO_REG32_BIT(INTC_MIR3,             0x482000E4,__READ_WRITE ,__intc_mir3_bits);
__IO_REG32(    INTC_MIR_CLEAR3,       0x482000E8,__WRITE      );
__IO_REG32(    INTC_MIR_SET3,         0x482000EC,__WRITE      );
__IO_REG32_BIT(INTC_ISR_SET3,         0x482000F0,__READ_WRITE ,__intc_isr_set3_bits);
__IO_REG32(    INTC_ISR_CLEAR3,       0x482000F4,__WRITE      );
__IO_REG32_BIT(INTC_PENDING_IRQ3,     0x482000F8,__READ       ,__intc_pending_irq3_bits);
__IO_REG32_BIT(INTC_PENDING_FIQ3,     0x482000FC,__READ       ,__intc_pending_fiq3_bits);
__IO_REG32_BIT(INTC_ILR0,             0x48200100,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR1,             0x48200104,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR2,             0x48200108,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR3,             0x4820010C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR4,             0x48200110,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR5,             0x48200114,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR6,             0x48200118,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR7,             0x4820011C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR8,             0x48200120,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR9,             0x48200124,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR10,            0x48200128,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR11,            0x4820012C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR12,            0x48200130,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR13,            0x48200134,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR14,            0x48200138,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR15,            0x4820013C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR16,            0x48200140,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR17,            0x48200144,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR18,            0x48200148,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR19,            0x4820014C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR20,            0x48200150,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR21,            0x48200154,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR22,            0x48200158,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR23,            0x4820015C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR24,            0x48200160,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR25,            0x48200164,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR26,            0x48200168,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR27,            0x4820016C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR28,            0x48200170,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR29,            0x48200174,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR30,            0x48200178,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR31,            0x4820017C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR32,            0x48200180,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR33,            0x48200184,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR34,            0x48200188,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR35,            0x4820018C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR36,            0x48200190,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR37,            0x48200194,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR38,            0x48200198,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR39,            0x4820019C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR40,            0x482001A0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR41,            0x482001A4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR42,            0x482001A8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR43,            0x482001AC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR44,            0x482001B0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR45,            0x482001B4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR46,            0x482001B8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR47,            0x482001BC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR48,            0x482001C0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR49,            0x482001C4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR50,            0x482001C8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR51,            0x482001CC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR52,            0x482001D0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR53,            0x482001D4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR54,            0x482001D8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR55,            0x482001DC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR56,            0x482001E0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR57,            0x482001E4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR58,            0x482001E8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR59,            0x482001EC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR60,            0x482001F0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR61,            0x482001F4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR62,            0x482001F8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR63,            0x482001FC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR64,            0x48200200,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR65,            0x48200204,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR66,            0x48200208,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR67,            0x4820020C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR68,            0x48200210,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR69,            0x48200214,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR70,            0x48200218,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR71,            0x4820021C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR72,            0x48200220,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR73,            0x48200224,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR74,            0x48200228,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR75,            0x4820022C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR76,            0x48200230,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR77,            0x48200234,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR78,            0x48200238,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR79,            0x4820023C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR80,            0x48200240,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR81,            0x48200244,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR82,            0x48200248,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR83,            0x4820024C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR84,            0x48200250,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR85,            0x48200254,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR86,            0x48200258,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR87,            0x4820025C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR88,            0x48200260,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR89,            0x48200264,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR90,            0x48200268,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR91,            0x4820026C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR92,            0x48200270,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR93,            0x48200274,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR94,            0x48200278,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR95,            0x4820027C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR96,            0x48200280,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR97,            0x48200284,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR98,            0x48200288,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR99,            0x4820028C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR100,           0x48200290,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR101,           0x48200294,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR102,           0x48200298,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR103,           0x4820029C,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR104,           0x482002A0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR105,           0x482002A4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR106,           0x482002A8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR107,           0x482002AC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR108,           0x482002B0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR109,           0x482002B4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR110,           0x482002B8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR111,           0x482002BC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR112,           0x482002C0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR113,           0x482002C4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR114,           0x482002C8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR115,           0x482002CC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR116,           0x482002D0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR117,           0x482002D4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR118,           0x482002D8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR119,           0x482002DC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR120,           0x482002E0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR121,           0x482002E4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR122,           0x482002E8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR123,           0x482002EC,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR124,           0x482002F0,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR125,           0x482002F4,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR126,           0x482002F8,__READ_WRITE ,__intc_ilr_bits);
__IO_REG32_BIT(INTC_ILR127,           0x482002FC,__READ_WRITE ,__intc_ilr_bits);

/***************************************************************************
 **
 ** GPMC
 **
 ***************************************************************************/
__IO_REG32_BIT(GPMC_REVISION,         0x50000000,__READ       ,__gpmc_revision_bits);
__IO_REG32_BIT(GPMC_SYSCONFIG,        0x50000010,__READ_WRITE ,__gpmc_sysconfig_bits);
__IO_REG32_BIT(GPMC_SYSSTATUS,        0x50000014,__READ       ,__gpmc_sysstatus_bits);
__IO_REG32_BIT(GPMC_IRQSTATUS,        0x50000018,__READ_WRITE ,__gpmc_irqstatus_bits);
__IO_REG32_BIT(GPMC_IRQENABLE,        0x5000001C,__READ_WRITE ,__gpmc_irqenable_bits);
__IO_REG32_BIT(GPMC_TIMEOUT_CONTROL,  0x50000040,__READ_WRITE ,__gpmc_timeout_control_bits);
__IO_REG32_BIT(GPMC_ERR_ADDRESS,      0x50000044,__READ_WRITE ,__gpmc_err_address_bits);
__IO_REG32_BIT(GPMC_ERR_TYPE,         0x50000048,__READ_WRITE ,__gpmc_err_type_bits);
__IO_REG32_BIT(GPMC_CONFIG,           0x50000050,__READ_WRITE ,__gpmc_config_bits);
__IO_REG32_BIT(GPMC_STATUS,           0x50000054,__READ_WRITE ,__gpmc_status_bits);
__IO_REG32_BIT(GPMC_CONFIG1_0,        0x50000060,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_0,        0x50000064,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_0,        0x50000068,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_0,        0x5000006C,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_0,        0x50000070,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_0,        0x50000074,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_0,        0x50000078,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_0,   0x5000007C,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_0,   0x50000080,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_0,      0x50000084,__READ_WRITE );
__IO_REG32_BIT(GPMC_CONFIG1_1,        0x50000090,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_1,        0x50000094,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_1,        0x50000098,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_1,        0x5000009C,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_1,        0x500000A0,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_1,        0x500000A4,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_1,        0x500000A8,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_1,   0x500000AC,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_1,   0x500000B0,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_1,      0x500000B4,__READ_WRITE );
__IO_REG32_BIT(GPMC_CONFIG1_2,        0x500000C0,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_2,        0x500000C4,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_2,        0x500000C8,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_2,        0x500000CC,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_2,        0x500000D0,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_2,        0x500000D4,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_2,        0x500000D8,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_2,   0x500000DC,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_2,   0x500000E0,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_2,      0x500000E4,__READ_WRITE );
__IO_REG32_BIT(GPMC_CONFIG1_3,        0x500000F0,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_3,        0x500000F4,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_3,        0x500000F8,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_3,        0x500000FC,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_3,        0x50000100,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_3,        0x50000104,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_3,        0x50000108,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_3,   0x5000010C,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_3,   0x50000110,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_3,      0x50000114,__READ_WRITE );
__IO_REG32_BIT(GPMC_CONFIG1_4,        0x50000120,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_4,        0x50000124,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_4,        0x50000128,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_4,        0x5000012C,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_4,        0x50000130,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_4,        0x50000134,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_4,        0x50000138,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_4,   0x5000013C,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_4,   0x50000140,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_4,      0x50000144,__READ_WRITE );
__IO_REG32_BIT(GPMC_CONFIG1_5,        0x50000150,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_5,        0x50000154,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_5,        0x50000158,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_5,        0x5000015C,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_5,        0x50000160,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_5,        0x50000164,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_5,        0x50000168,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_5,   0x5000016C,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_5,   0x50000170,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_5,      0x50000174,__READ_WRITE );
__IO_REG32_BIT(GPMC_CONFIG1_6,        0x50000180,__READ_WRITE ,__gpmc_config1_bits);
__IO_REG32_BIT(GPMC_CONFIG2_6,        0x50000184,__READ_WRITE ,__gpmc_config2_bits);
__IO_REG32_BIT(GPMC_CONFIG3_6,        0x50000188,__READ_WRITE ,__gpmc_config3_bits);
__IO_REG32_BIT(GPMC_CONFIG4_6,        0x5000018C,__READ_WRITE ,__gpmc_config4_bits);
__IO_REG32_BIT(GPMC_CONFIG5_6,        0x50000190,__READ_WRITE ,__gpmc_config5_bits);
__IO_REG32_BIT(GPMC_CONFIG6_6,        0x50000194,__READ_WRITE ,__gpmc_config6_bits);
__IO_REG32_BIT(GPMC_CONFIG7_6,        0x50000198,__READ_WRITE ,__gpmc_config7_bits);
__IO_REG32(    GPMC_NAND_COMMAND_6,   0x5000019C,__WRITE      );
__IO_REG32(    GPMC_NAND_ADDRESS_6,   0x500001A0,__WRITE      );
__IO_REG32(    GPMC_NAND_DATA_6,      0x500001A4,__READ_WRITE );
__IO_REG32_BIT(GPMC_PREFETCH_CONFIG1, 0x500001E0,__READ_WRITE ,__gpmc_prefetch_config1_bits);
__IO_REG32_BIT(GPMC_PREFETCH_CONFIG2, 0x500001E4,__READ_WRITE ,__gpmc_prefetch_config2_bits);
__IO_REG32_BIT(GPMC_PREFETCH_CONTROL, 0x500001EC,__READ_WRITE ,__gpmc_prefetch_control_bits);
__IO_REG32_BIT(GPMC_PREFETCH_STATUS,  0x500001F0,__READ       ,__gpmc_prefetch_status_bits);
__IO_REG32_BIT(GPMC_ECC_CONFIG,       0x500001F4,__READ_WRITE ,__gpmc_ecc_config_bits);
__IO_REG32_BIT(GPMC_ECC_CONTROL,      0x500001F8,__READ_WRITE ,__gpmc_ecc_control_bits);
__IO_REG32_BIT(GPMC_ECC_SIZE_CONFIG,  0x500001FC,__READ_WRITE ,__gpmc_ecc_size_config_bits);
__IO_REG32_BIT(GPMC_ECC1_RESULT,      0x50000200,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC2_RESULT,      0x50000204,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC3_RESULT,      0x50000208,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC4_RESULT,      0x5000020C,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC5_RESULT,      0x50000210,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC6_RESULT,      0x50000214,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC7_RESULT,      0x50000218,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC8_RESULT,      0x5000021C,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32_BIT(GPMC_ECC9_RESULT,      0x50000220,__READ       ,__gpmc_ecc_result_bits);
__IO_REG32(    GPMC_BCH_RESULT0_0,    0x50000240,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_0,    0x50000244,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_0,    0x50000248,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_0,    0x5000024C,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT0_1,    0x50000250,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_1,    0x50000254,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_1,    0x50000258,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_1,    0x5000025C,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT0_2,    0x50000260,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_2,    0x50000264,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_2,    0x50000268,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_2,    0x5000026C,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT0_3,    0x50000270,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_3,    0x50000274,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_3,    0x50000278,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_3,    0x5000027C,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT0_4,    0x50000280,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_4,    0x50000284,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_4,    0x50000288,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_4,    0x5000028C,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT0_5,    0x50000290,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_5,    0x50000294,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_5,    0x50000298,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_5,    0x5000029C,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT0_6,    0x500002A0,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT1_6,    0x500002A4,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT2_6,    0x500002A8,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT3_6,    0x500002AC,__READ_WRITE );
__IO_REG32_BIT(GPMC_BCH_SWDATA,       0x500002D0,__READ_WRITE ,__gpmc_bch_swdata_bits);
__IO_REG32(    GPMC_BCH_RESULT4_0,    0x50000300,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_0,    0x50000304,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_0,    0x50000308,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT4_1,    0x50000310,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_1,    0x50000314,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_1,    0x50000318,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT4_2,    0x50000320,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_2,    0x50000324,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_2,    0x50000328,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT4_3,    0x50000330,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_3,    0x50000334,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_3,    0x50000338,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT4_4,    0x50000340,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_4,    0x50000344,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_4,    0x50000348,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT4_5,    0x50000350,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_5,    0x50000354,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_5,    0x50000358,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT4_6,    0x50000360,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT5_6,    0x50000364,__READ_WRITE );
__IO_REG32(    GPMC_BCH_RESULT6_6,    0x50000368,__READ_WRITE );

/***************************************************************************
 **
 ** EMIF4D Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(EMIF_MOD_ID_REV,             0x4C000000,__READ       ,__emif_mod_id_rev_bits);
__IO_REG32_BIT(EMIF_STATUS,                 0x4C000004,__READ       ,__emif_status_bits);
__IO_REG32_BIT(EMIF_SDRAM_CONFIG,           0x4C000008,__READ_WRITE ,__emif_sdram_config_bits);
__IO_REG32_BIT(EMIF_SDRAM_CONFIG_2,         0x4C00000C,__READ_WRITE ,__emif_sdram_config_2_bits);
__IO_REG32_BIT(EMIF_SDRAM_REF_CTRL,         0x4C000010,__READ_WRITE ,__emif_sdram_ref_ctrl_bits);
__IO_REG32_BIT(EMIF_SDRAM_REF_CTRL_SHDW,    0x4C000014,__READ_WRITE ,__emif_sdram_ref_ctrl_shdw_bits);
__IO_REG32_BIT(EMIF_SDRAM_TIM_1,            0x4C000018,__READ_WRITE ,__emif_sdram_tim_1_bits);
__IO_REG32_BIT(EMIF_SDRAM_TIM_1_SHDW,       0x4C00001C,__READ_WRITE ,__emif_sdram_tim_1_shdw_bits);
__IO_REG32_BIT(EMIF_SDRAM_TIM_2,            0x4C000020,__READ_WRITE ,__emif_sdram_tim_2_bits);
__IO_REG32_BIT(EMIF_SDRAM_TIM_2_SHDW,       0x4C000024,__READ_WRITE ,__emif_sdram_tim_2_shdw_bits);
__IO_REG32_BIT(EMIF_SDRAM_TIM_3,            0x4C000028,__READ_WRITE ,__emif_sdram_tim_3_bits);
__IO_REG32_BIT(EMIF_SDRAM_TIM_3_SHDW,       0x4C00002C,__READ_WRITE ,__emif_sdram_tim_3_shdw_bits);
__IO_REG32_BIT(EMIF_PWR_MGMT_CTRL,          0x4C000038,__READ_WRITE ,__emif_pwr_mgmt_ctrl_bits);
__IO_REG32_BIT(EMIF_PWR_MGMT_CTRL_SHDW,     0x4C00003C,__READ_WRITE ,__emif_pwr_mgmt_ctrl_shdw_bits);
__IO_REG32_BIT(EMIF_IC,                             0x4C000054,__READ_WRITE ,__emif_ic_bits);
__IO_REG32_BIT(EMIF_ICV1,                           0x4C000058,__READ       ,__emif_icv1_bits);
__IO_REG32_BIT(EMIF_ICV2,                           0x4C00005C,__READ       ,__emif_icv2_bits);
__IO_REG32(    EMIF_PERF_CNT_1,             0x4C000080,__READ       );
__IO_REG32(    EMIF_PERF_CNT_2,             0x4C000084,__READ       );
__IO_REG32_BIT(EMIF_PERF_CNT_CFG,           0x4C000088,__READ_WRITE ,__emif_perf_cnt_cfg_bits);
__IO_REG32_BIT(EMIF_PERF_CNT_SEL,           0x4C00008C,__READ_WRITE ,__emif_perf_cnt_sel_bits);
__IO_REG32(    EMIF_PERF_CNT_TIM,           0x4C000090,__READ       );
__IO_REG32_BIT(EMIF_READ_IDLE_CTRL,         0x4C000098,__READ_WRITE ,__emif_read_idle_ctrl_bits);
__IO_REG32_BIT(EMIF_READ_IDLE_CTRL_SHDW,    0x4C00009C,__READ_WRITE ,__emif_read_idle_ctrl_shdw_bits);
__IO_REG32_BIT(EMIF_IRQSTATUS_RAW_SYS,      0x4C0000A4,__READ_WRITE ,__emif_irqstatus_raw_sys_bits);
__IO_REG32_BIT(EMIF_IRQSTATUS_SYS,          0x4C0000AC,__READ_WRITE ,__emif_irqstatus_sys_bits);
__IO_REG32_BIT(EMIF_IRQENABLE_SET_SYS,      0x4C0000B4,__READ_WRITE ,__emif_irqenable_set_sys_bits);
__IO_REG32_BIT(EMIF_IRQENABLE_CLR_SYS,      0x4C0000BC,__READ_WRITE ,__emif_irqenable_clr_sys_bits);
__IO_REG32_BIT(EMIF_ZQ_CONFIG,              0x4C0000C8,__READ_WRITE ,__emif_zq_config_bits);
__IO_REG32_BIT(EMIF_RWLRW,                      0x4C0000D4,__READ               ,__emif_rwlrw_bits);
__IO_REG32_BIT(EMIF_RWLRC,                    0x4C0000D8,__READ_WRITE ,__emif_rwlrc_bits);
__IO_REG32_BIT(EMIF_RWLC,                       0x4C0000DC,__READ_WRITE ,__emif_rwlc_bits);
__IO_REG32_BIT(EMIF_DDR_PHY_CTRL_1,         0x4C0000E4,__READ_WRITE ,__emif_ddr_phy_ctrl_1_bits);
__IO_REG32_BIT(EMIF_DDR_PHY_CTRL_1_SHDW,    0x4C0000E8,__READ_WRITE ,__emif_ddr_phy_ctrl_1_bits);
__IO_REG32_BIT(EMIF_DDR_PHY_CTRL_2,         0x4C0000EC,__READ_WRITE ,__emif_ddr_phy_ctrl_1_bits);
__IO_REG32_BIT(EMIF_PTCOSM,                             0x4C000100,__READ_WRITE ,__emif_ptcosm_bits);
__IO_REG32_BIT(EMIF_CIDTCOS1M,                      0x4C000104,__READ_WRITE ,__emif_cidtcos1m_bits);
__IO_REG32_BIT(EMIF_CIDTCOS2M,                      0x4C000108,__READ_WRITE ,__emif_cidtcos2m_bits);
__IO_REG32_BIT(EMIF_RWET,                                   0x4C000120,__READ_WRITE ,__emif_rwet_bits);

/***************************************************************************
 **
 ** DDR2/3/mDDR PHY Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(CMD0_REG_PHY_CTRL_SLAVE_RATIO_0,     0x44E1201C,__READ_WRITE ,__cmd_reg_phy_ctrl_slave_ratio_0_bits);
__IO_REG32_BIT(CMD0_REG_PHY_DLL_LOCK_DIFF_0,        0x44E12028,__READ_WRITE ,__cmd_reg_phy_dll_lock_diff_0_bits);
__IO_REG32_BIT(CMD0_REG_PHY_INVERT_CLKOUT_0,        0x44E1202C,__READ_WRITE ,__cmd_reg_phy_invert_clkout_0_bits);
__IO_REG32_BIT(CMD1_REG_PHY_CTRL_SLAVE_RATIO_0,     0x44E12050,__READ_WRITE ,__cmd_reg_phy_ctrl_slave_ratio_0_bits);
__IO_REG32_BIT(CMD1_REG_PHY_DLL_LOCK_DIFF_0,        0x44E1205C,__READ_WRITE ,__cmd_reg_phy_dll_lock_diff_0_bits);
__IO_REG32_BIT(CMD1_REG_PHY_INVERT_CLKOUT_0,        0x44E12060,__READ_WRITE ,__cmd_reg_phy_invert_clkout_0_bits);
__IO_REG32_BIT(CMD2_REG_PHY_CTRL_SLAVE_RATIO_0,     0x44E12084,__READ_WRITE ,__cmd_reg_phy_ctrl_slave_ratio_0_bits);
__IO_REG32_BIT(CMD2_REG_PHY_DLL_LOCK_DIFF_0,        0x44E12090,__READ_WRITE ,__cmd_reg_phy_dll_lock_diff_0_bits);
__IO_REG32_BIT(CMD2_REG_PHY_INVERT_CLKOUT_0,        0x44E12094,__READ_WRITE ,__cmd_reg_phy_invert_clkout_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0,  0x44E120C8,__READ_WRITE ,__data_reg_phy_rd_dqs_slave_ratio_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0,  0x44E120DC,__READ_WRITE ,__data_reg_phy_wr_dqs_slave_ratio_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_WRLVL_INIT_RATIO_0,    0x44E120F0,__READ_WRITE ,__data_reg_phy_wrlvl_init_ratio_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_WRLVL_INIT_MODE_0,     0x44E120F8,__READ_WRITE ,__data_reg_phy_wrlvl_init_mode_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_GATELVL_INIT_RATIO_0,  0x44E120FC,__READ_WRITE ,__data_reg_phy_gatelvl_init_ratio_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_GATELVL_INIT_MODE_0,   0x44E12104,__READ_WRITE ,__data_reg_phy_gatelvl_init_mode_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0, 0x44E12108,__READ_WRITE ,__data_reg_phy_fifo_we_slave_ratio_0_bits);
__IO_REG32(    DATA0_REG_PHY_DQ_OFFSET_0,           0x44E1211C,__READ_WRITE );
__IO_REG32_BIT(DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0, 0x44E12120,__READ_WRITE ,__data_reg_phy_wr_data_slave_ratio_0_bits);
__IO_REG32_BIT(DATA0_REG_PHY_USE_RANK0_DELAYS,      0x44E12134,__READ_WRITE ,__data_reg_phy_use_rank0_delays_bits);
__IO_REG32_BIT(DATA0_REG_PHY_DLL_LOCK_DIFF_0,       0x44E12138,__READ_WRITE ,__data_reg_phy_dll_lock_diff_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0,  0x44E1216C,__READ_WRITE ,__data_reg_phy_rd_dqs_slave_ratio_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0,  0x44E12180,__READ_WRITE ,__data_reg_phy_wr_dqs_slave_ratio_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_WRLVL_INIT_RATIO_0,    0x44E12194,__READ_WRITE ,__data_reg_phy_wrlvl_init_ratio_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_WRLVL_INIT_MODE_0,     0x44E1219C,__READ_WRITE ,__data_reg_phy_wrlvl_init_mode_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_GATELVL_INIT_RATIO_0,  0x44E121A0,__READ_WRITE ,__data_reg_phy_gatelvl_init_ratio_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_GATELVL_INIT_MODE_0,   0x44E121A8,__READ_WRITE ,__data_reg_phy_gatelvl_init_mode_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0, 0x44E121AC,__READ_WRITE ,__data_reg_phy_fifo_we_slave_ratio_0_bits);
__IO_REG32(    DATA1_REG_PHY_DQ_OFFSET_0,           0x44E121C0,__READ_WRITE );
__IO_REG32_BIT(DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0, 0x44E121C4,__READ_WRITE ,__data_reg_phy_wr_data_slave_ratio_0_bits);
__IO_REG32_BIT(DATA1_REG_PHY_USE_RANK0_DELAYS,      0x44E121D8,__READ_WRITE ,__data_reg_phy_use_rank0_delays_bits);
__IO_REG32_BIT(DATA1_REG_PHY_DLL_LOCK_DIFF_0,       0x44E121DC,__READ_WRITE ,__data_reg_phy_dll_lock_diff_0_bits);

/***************************************************************************
 **
 ** ELM
 **
 ***************************************************************************/
__IO_REG32(    ELM_REVISION,              0x48080000,__READ       );
__IO_REG32_BIT(ELM_SYSCONFIG,             0x48080010,__READ_WRITE ,__elm_sysconfig_bits);
__IO_REG32_BIT(ELM_SYSSTATUS,             0x48080014,__READ       ,__elm_sysstatus_bits);
__IO_REG32_BIT(ELM_IRQSTATUS,             0x48080018,__READ_WRITE ,__elm_irqstatus_bits);
__IO_REG32_BIT(ELM_IRQENABLE,             0x4808001C,__READ_WRITE ,__elm_irqenable_bits);
__IO_REG32_BIT(ELM_LOCATION_CONFIG,       0x48080020,__READ_WRITE ,__elm_location_config_bits);
__IO_REG32_BIT(ELM_PAGE_CTRL,             0x48080080,__READ_WRITE ,__elm_page_ctrl_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_0, 0x48080400,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_0, 0x48080404,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_0, 0x48080408,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_0, 0x4808040C,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_0, 0x48080410,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_0, 0x48080414,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_0, 0x48080418,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_1, 0x48080440,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_1, 0x48080444,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_1, 0x48080448,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_1, 0x4808044C,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_1, 0x48080450,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_1, 0x48080454,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_1, 0x48080458,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_2, 0x48080480,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_2, 0x48080484,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_2, 0x48080488,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_2, 0x4808048C,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_2, 0x48080490,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_2, 0x48080494,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_2, 0x48080498,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_3, 0x480804C0,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_3, 0x480804C4,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_3, 0x480804C8,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_3, 0x480804CC,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_3, 0x480804D0,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_3, 0x480804D4,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_3, 0x480804D8,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_4, 0x48080500,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_4, 0x48080504,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_4, 0x48080508,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_4, 0x4808050C,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_4, 0x48080510,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_4, 0x48080514,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_4, 0x48080518,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_5, 0x48080540,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_5, 0x48080544,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_5, 0x48080548,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_5, 0x4808054C,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_5, 0x48080550,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_5, 0x48080554,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_5, 0x48080558,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_6, 0x48080580,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_6, 0x48080584,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_6, 0x48080588,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_6, 0x4808058C,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_6, 0x48080590,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_6, 0x48080594,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_6, 0x48080598,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_0_7, 0x480805C0,__READ_WRITE ,__elm_syndrome_fragment_0_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_1_7, 0x480805C4,__READ_WRITE ,__elm_syndrome_fragment_1_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_2_7, 0x480805C8,__READ_WRITE ,__elm_syndrome_fragment_2_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_3_7, 0x480805CC,__READ_WRITE ,__elm_syndrome_fragment_3_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_4_7, 0x480805D0,__READ_WRITE ,__elm_syndrome_fragment_4_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_5_7, 0x480805D4,__READ_WRITE ,__elm_syndrome_fragment_5_bits);
__IO_REG32_BIT(ELM_SYNDROME_FRAGMENT_6_7, 0x480805D8,__READ_WRITE ,__elm_syndrome_fragment_6_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_0,     0x48080800,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_0,    0x48080880,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_0,    0x48080884,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_0,    0x48080888,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_0,    0x4808088C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_0,    0x48080890,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_0,    0x48080894,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_0,    0x48080898,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_0,    0x4808089C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_0,    0x480808A0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_0,    0x480808A4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_0,   0x480808A8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_0,   0x480808AC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_0,   0x480808B0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_0,   0x480808B4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_0,   0x480808B8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_0,   0x480808BC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_1,     0x48080900,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_1,    0x48080980,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_1,    0x48080984,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_1,    0x48080988,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_1,    0x4808098C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_1,    0x48080990,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_1,    0x48080994,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_1,    0x48080998,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_1,    0x4808099C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_1,    0x480809A0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_1,    0x480809A4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_1,   0x480809A8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_1,   0x480809AC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_1,   0x480809B0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_1,   0x480809B4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_1,   0x480809B8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_1,   0x480809BC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_2,     0x48080A00,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_2,    0x48080A80,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_2,    0x48080A84,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_2,    0x48080A88,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_2,    0x48080A8C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_2,    0x48080A90,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_2,    0x48080A94,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_2,    0x48080A98,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_2,    0x48080A9C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_2,    0x48080AA0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_2,    0x48080AA4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_2,   0x48080AA8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_2,   0x48080AAC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_2,   0x48080AB0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_2,   0x48080AB4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_2,   0x48080AB8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_2,   0x48080ABC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_3,     0x48080B00,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_3,    0x48080B80,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_3,    0x48080B84,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_3,    0x48080B88,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_3,    0x48080B8C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_3,    0x48080B90,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_3,    0x48080B94,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_3,    0x48080B98,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_3,    0x48080B9C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_3,    0x48080BA0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_3,    0x48080BA4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_3,   0x48080BA8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_3,   0x48080BAC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_3,   0x48080BB0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_3,   0x48080BB4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_3,   0x48080BB8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_3,   0x48080BBC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_4,     0x48080C00,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_4,    0x48080C80,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_4,    0x48080C84,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_4,    0x48080C88,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_4,    0x48080C8C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_4,    0x48080C90,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_4,    0x48080C94,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_4,    0x48080C98,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_4,    0x48080C9C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_4,    0x48080CA0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_4,    0x48080CA4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_4,   0x48080CA8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_4,   0x48080CAC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_4,   0x48080CB0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_4,   0x48080CB4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_4,   0x48080CB8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_4,   0x48080CBC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_5,     0x48081000,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_5,    0x48080D80,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_5,    0x48080D84,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_5,    0x48080D88,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_5,    0x48080D8C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_5,    0x48080D90,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_5,    0x48080D94,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_5,    0x48080D98,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_5,    0x48080D9C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_5,    0x48080DA0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_5,    0x48080DA4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_5,   0x48080DA8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_5,   0x48080DAC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_5,   0x48080DB0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_5,   0x48080DB4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_5,   0x48080DB8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_5,   0x48080DBC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_6,     0x48081200,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_6,    0x48080E80,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_6,    0x48080E84,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_6,    0x48080E88,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_6,    0x48080E8C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_6,    0x48080E90,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_6,    0x48080E94,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_6,    0x48080E98,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_6,    0x48080E9C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_6,    0x48080EA0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_6,    0x48080EA4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_6,   0x48080EA8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_6,   0x48080EAC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_6,   0x48080EB0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_6,   0x48080EB4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_6,   0x48080EB8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_6,   0x48080EBC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_LOCATION_STATUS_7,     0x48081400,__READ       ,__elm_location_status_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_0_7,    0x48080F80,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_1_7,    0x48080F84,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_2_7,    0x48080F88,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_3_7,    0x48080F8C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_4_7,    0x48080F90,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_5_7,    0x48080F94,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_6_7,    0x48080F98,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_7_7,    0x48080F9C,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_8_7,    0x48080FA0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_9_7,    0x48080FA4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_10_7,   0x48080FA8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_11_7,   0x48080FAC,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_12_7,   0x48080FB0,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_13_7,   0x48080FB4,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_14_7,   0x48080FB8,__READ       ,__elm_error_location_bits);
__IO_REG32_BIT(ELM_ERROR_LOCATION_15_7,   0x48080FBC,__READ       ,__elm_error_location_bits);

/***************************************************************************
 **
 ** CM_PER REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_PER_L4LS_CLKSTCTRL,     0x44E00000,__READ_WRITE ,__cm_per_l4ls_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_L3S_CLKSTCTRL,      0x44E00004,__READ_WRITE ,__cm_per_l3s_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_L3_CLKSTCTRL,       0x44E0000C,__READ_WRITE ,__cm_per_l3_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_CPGMAC0_CLKCTRL,    0x44E00014,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_LCDC_CLKCTRL,       0x44E00018,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_USB0_CLKCTRL,       0x44E0001C,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_TPTC0_CLKCTRL,      0x44E00024,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_EMIF_CLKCTRL,       0x44E00028,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_OCMCRAM_CLKCTRL,    0x44E0002C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_GPMC_CLKCTRL,       0x44E00030,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_MCASP0_CLKCTRL,     0x44E00034,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_UART5_CLKCTRL,      0x44E00038,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_MMC0_CLKCTRL,       0x44E0003C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_ELM_CLKCTRL,        0x44E00040,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_I2C2_CLKCTRL,       0x44E00044,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_I2C1_CLKCTRL,       0x44E00048,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_SPI0_CLKCTRL,       0x44E0004C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_SPI1_CLKCTRL,       0x44E00050,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_L4LS_CLKCTRL,       0x44E00060,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_L4FW_CLKCTRL,       0x44E00064,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_MCASP1_CLKCTRL,     0x44E00068,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_UART1_CLKCTRL,      0x44E0006C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_UART2_CLKCTRL,      0x44E00070,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_UART3_CLKCTRL,      0x44E00074,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_UART4_CLKCTRL,      0x44E00078,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_TIMER7_CLKCTRL,     0x44E0007C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_TIMER2_CLKCTRL,     0x44E00080,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_TIMER3_CLKCTRL,     0x44E00084,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_TIMER4_CLKCTRL,     0x44E00088,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_GPIO1_CLKCTRL,      0x44E000AC,__READ_WRITE ,__cm_per_gpio1_clkctrl_bits);
__IO_REG32_BIT(CM_PER_GPIO2_CLKCTRL,      0x44E000B0,__READ_WRITE ,__cm_per_gpio2_clkctrl_bits);
__IO_REG32_BIT(CM_PER_GPIO3_CLKCTRL,      0x44E000B4,__READ_WRITE ,__cm_per_gpio3_clkctrl_bits);
__IO_REG32_BIT(CM_PER_TPCC_CLKCTRL,       0x44E000BC,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_DCAN0_CLKCTRL,      0x44E000C0,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_DCAN1_CLKCTRL,      0x44E000C4,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_EPWMSS1_CLKCTRL,    0x44E000CC,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_EPWMSS0_CLKCTRL,    0x44E000D4,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_EPWMSS2_CLKCTRL,    0x44E000D8,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_L3_INSTR_CLKCTRL,   0x44E000DC,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_L3_CLKCTRL,         0x44E000E0,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_IEEE5000_CLKCTRL,   0x44E000E4,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_PRU_ICSS_CLKCTRL,   0x44E000E8,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_TIMER5_CLKCTRL,     0x44E000EC,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_TIMER6_CLKCTRL,     0x44E000F0,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_MMC1_CLKCTRL,       0x44E000F4,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_MMC2_CLKCTRL,       0x44E000F8,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_TPTC1_CLKCTRL,      0x44E000FC,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_TPTC2_CLKCTRL,      0x44E00100,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_SPINLOCK_CLKCTRL,   0x44E0010C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_MAILBOX0_CLKCTRL,   0x44E00110,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_L4HS_CLKSTCTRL,     0x44E0011C,__READ_WRITE ,__cm_per_l4hs_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_L4HS_CLKCTRL,       0x44E00120,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_OCPWP_L3_CLKSTCTRL, 0x44E0012C,__READ_WRITE ,__cm_per_ocpwp_l3_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_OCPWP_CLKCTRL,      0x44E00130,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_PER_PRU_ICSS_CLKSTCTRL, 0x44E00140,__READ_WRITE ,__cm_per_pru_icss_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_CPSW_CLKSTCTRL,     0x44E00144,__READ_WRITE ,__cm_per_cpsw_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_LCDC_CLKSTCTRL,     0x44E00148,__READ_WRITE ,__cm_per_lcdc_clkstctrl_bits);
__IO_REG32_BIT(CM_PER_CLKDIV32K_CLKCTRL,  0x44E0014C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_PER_CLK_24MHZ_CLKSTCTRL,0x44E00150,__READ_WRITE ,__cm_per_clk_24mhz_clkstctrl_bits);

/***************************************************************************
 **
 ** CM_WKUP REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_WKUP_CLKSTCTRL,             0x44E00400,__READ_WRITE ,__cm_wkup_clkstctrl_bits);
__IO_REG32_BIT(CM_WKUP_CONTROL_CLKCTRL,       0x44E00404,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_GPIO0_CLKCTRL,         0x44E00408,__READ_WRITE ,__cm_wkup_gpio0_clkctrl_bits);
__IO_REG32_BIT(CM_WKUP_L4WKUP_CLKCTRL,        0x44E0040C,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_TIMER0_CLKCTRL,        0x44E00410,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_DEBUGSS_CLKCTRL,       0x44E00414,__READ_WRITE ,__cm_wkup_debugss_clkctrl_bits);
__IO_REG32_BIT(CM_L3_AON_CLKSTCTRL,           0x44E00418,__READ_WRITE ,__cm_l3_aon_clkstctrl_bits);
__IO_REG32_BIT(CM_AUTOIDLE_DPLL_MPU,          0x44E0041C,__READ_WRITE ,__cm_autoidle_dpll_bits);
__IO_REG32_BIT(CM_IDLEST_DPLL_MPU,            0x44E00420,__READ_WRITE ,__cm_idlest_dpll_bits);
__IO_REG32_BIT(CM_SSC_DELTAMSTEP_DPLL_MPU,    0x44E00424,__READ_WRITE ,__cm_ssc_deltamstep_dpll_bits);
__IO_REG32_BIT(CM_SSC_MODFREQDIV_DPLL_MPU,    0x44E00428,__READ_WRITE ,__cm_ssc_modfreqdiv_dpll_bits);
__IO_REG32_BIT(CM_CLKSEL_DPLL_MPU,            0x44E0042C,__READ_WRITE ,__cm_clksel_dpll_bits);
__IO_REG32_BIT(CM_AUTOIDLE_DPLL_DDR,          0x44E00430,__READ_WRITE ,__cm_autoidle_dpll_bits);
__IO_REG32_BIT(CM_IDLEST_DPLL_DDR,            0x44E00434,__READ_WRITE ,__cm_idlest_dpll_bits);
__IO_REG32_BIT(CM_SSC_DELTAMSTEP_DPLL_DDR,    0x44E00438,__READ_WRITE ,__cm_ssc_deltamstep_dpll_bits);
__IO_REG32_BIT(CM_SSC_MODFREQDIV_DPLL_DDR,    0x44E0043C,__READ_WRITE ,__cm_ssc_modfreqdiv_dpll_bits);
__IO_REG32_BIT(CM_CLKSEL_DPLL_DDR,            0x44E00440,__READ_WRITE ,__cm_clksel_dpll_bits);
__IO_REG32_BIT(CM_AUTOIDLE_DPLL_DISP,         0x44E00444,__READ_WRITE ,__cm_autoidle_dpll_bits);
__IO_REG32_BIT(CM_IDLEST_DPLL_DISP,           0x44E00448,__READ_WRITE ,__cm_idlest_dpll_bits);
__IO_REG32_BIT(CM_SSC_DELTAMSTEP_DPLL_DISP,   0x44E0044C,__READ_WRITE ,__cm_ssc_deltamstep_dpll_bits);
__IO_REG32_BIT(CM_SSC_MODFREQDIV_DPLL_DISP,   0x44E00450,__READ_WRITE ,__cm_ssc_modfreqdiv_dpll_bits);
__IO_REG32_BIT(CM_CLKSEL_DPLL_DISP,           0x44E00454,__READ_WRITE ,__cm_clksel_dpll_bits);
__IO_REG32_BIT(CM_AUTOIDLE_DPLL_CORE,         0x44E00458,__READ_WRITE ,__cm_autoidle_dpll_bits);
__IO_REG32_BIT(CM_IDLEST_DPLL_CORE,           0x44E0045C,__READ_WRITE ,__cm_idlest_dpll_bits);
__IO_REG32_BIT(CM_SSC_DELTAMSTEP_DPLL_CORE,   0x44E00460,__READ_WRITE ,__cm_ssc_deltamstep_dpll_bits);
__IO_REG32_BIT(CM_SSC_MODFREQDIV_DPLL_CORE,   0x44E00464,__READ_WRITE ,__cm_ssc_modfreqdiv_dpll_bits);
__IO_REG32_BIT(CM_CLKSEL_DPLL_CORE,           0x44E00468,__READ_WRITE ,__cm_clksel_dpll_bits);
__IO_REG32_BIT(CM_AUTOIDLE_DPLL_PER,          0x44E0046C,__READ_WRITE ,__cm_autoidle_dpll_bits);
__IO_REG32_BIT(CM_IDLEST_DPLL_PER,            0x44E00470,__READ_WRITE ,__cm_idlest_dpll_bits);
__IO_REG32_BIT(CM_SSC_DELTAMSTEP_DPLL_PER,    0x44E00474,__READ_WRITE ,__cm_ssc_deltamstep_dpll_bits);
__IO_REG32_BIT(CM_SSC_MODFREQDIV_DPLL_PER,    0x44E00478,__READ_WRITE ,__cm_ssc_modfreqdiv_dpll_bits);
__IO_REG32_BIT(CM_CLKDCOLDO_DPLL_PER,         0x44E0047C,__READ_WRITE ,__cm_clkdcoldo_dpll_per_bits);
__IO_REG32_BIT(CM_DIV_M4_DPLL_CORE,           0x44E00480,__READ_WRITE ,__cm_div_m4_dpll_core_bits);
__IO_REG32_BIT(CM_DIV_M5_DPLL_CORE,           0x44E00484,__READ_WRITE ,__cm_div_m5_dpll_core_bits);
__IO_REG32_BIT(CM_CLKMODE_DPLL_MPU,           0x44E00488,__READ_WRITE ,__cm_clkmode_dpll_bits);
__IO_REG32_BIT(CM_CLKMODE_DPLL_PER,           0x44E0048C,__READ_WRITE ,__cm_clkmode_dpll_per_bits);
__IO_REG32_BIT(CM_CLKMODE_DPLL_CORE,          0x44E00490,__READ_WRITE ,__cm_clkmode_dpll_bits);
__IO_REG32_BIT(CM_CLKMODE_DPLL_DDR,           0x44E00494,__READ_WRITE ,__cm_clkmode_dpll_bits);
__IO_REG32_BIT(CM_CLKMODE_DPLL_DISP,          0x44E00498,__READ_WRITE ,__cm_clkmode_dpll_bits);
__IO_REG32_BIT(CM_CLKSEL_DPLL_PERIPH,         0x44E0049C,__READ_WRITE ,__cm_clksel_dpll_periph_bits);
__IO_REG32_BIT(CM_DIV_M2_DPLL_DDR,            0x44E004A0,__READ_WRITE ,__cm_div_m2_dpll_bits);
__IO_REG32_BIT(CM_DIV_M2_DPLL_DISP,           0x44E004A4,__READ_WRITE ,__cm_div_m2_dpll_bits);
__IO_REG32_BIT(CM_DIV_M2_DPLL_MPU,            0x44E004A8,__READ_WRITE ,__cm_div_m2_dpll_bits);
__IO_REG32_BIT(CM_DIV_M2_DPLL_PER,            0x44E004AC,__READ_WRITE ,__cm_div_m2_dpll_per_bits);
__IO_REG32_BIT(CM_WKUP_WKUP_M3_CLKCTRL,       0x44E004B0,__READ_WRITE ,__cm_wkup_wkup_m3_clkctrl_bits);
__IO_REG32_BIT(CM_WKUP_UART0_CLKCTRL,         0x44E004B4,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_I2C0_CLKCTRL,          0x44E004B8,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_ADC_TSC_CLKCTRL,       0x44E004BC,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_SMARTREFLEX0_CLKCTRL,  0x44E004C0,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_TIMER1_CLKCTRL,        0x44E004C4,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_WKUP_SMARTREFLEX1_CLKCTRL,  0x44E004C8,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_L4_WKUP_AON_CLKSTCTRL,      0x44E004CC,__READ_WRITE ,__cm_l4_wkup_aon_clkstctrl_bits);
__IO_REG32_BIT(CM_WKUP_WDT1_CLKCTRL,          0x44E004D4,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_DIV_M6_DPLL_CORE,           0x44E004D8,__READ_WRITE ,__cm_div_m6_dpll_core_bits);

/***************************************************************************
 **
 ** CM_DPLL REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CLKSEL_TIMER7_CLK,       0x44E00504,__READ_WRITE ,__clksel_timer_clk_bits);
__IO_REG32_BIT(CLKSEL_TIMER2_CLK,       0x44E00508,__READ_WRITE ,__clksel_timer_clk_bits);
__IO_REG32_BIT(CLKSEL_TIMER3_CLK,       0x44E0050C,__READ_WRITE ,__clksel_timer_clk_bits);
__IO_REG32_BIT(CLKSEL_TIMER4_CLK,       0x44E00510,__READ_WRITE ,__clksel_timer_clk_bits);
__IO_REG32_BIT(CM_MAC_CLKSEL,           0x44E00514,__READ_WRITE ,__cm_mac_clksel_bits);
__IO_REG32_BIT(CLKSEL_TIMER5_CLK,       0x44E00518,__READ_WRITE ,__clksel_timer_clk_bits);
__IO_REG32_BIT(CLKSEL_TIMER6_CLK,       0x44E0051C,__READ_WRITE ,__clksel_timer_clk_bits);
__IO_REG32_BIT(CM_CPTS_RFT_CLKSEL,      0x44E00524,__READ_WRITE ,__cm_cpts_rft_clksel_bits);
__IO_REG32_BIT(CLKSEL_TIMER1MS_CLK,     0x44E00528,__READ_WRITE ,__clksel_timer1ms_clk_bits);
__IO_REG32_BIT(CLKSEL_GFX_FCLK,         0x44E0052C,__READ_WRITE ,__clksel_gfx_fclk_bits);
__IO_REG32_BIT(CLKSEL_PRU_ICSS_OCP_CLK, 0x44E00530,__READ_WRITE ,__clksel_pru_icss_ocp_clk_bits);
__IO_REG32_BIT(CLKSEL_LCDC_PIXEL_CLK,   0x44E00534,__READ_WRITE ,__clksel_lcdc_pixel_clk_bits);
__IO_REG32_BIT(CLKSEL_WDT1_CLK,         0x44E00538,__READ_WRITE ,__clksel_wdt1_clk_bits);
__IO_REG32_BIT(CLKSEL_GPIO0_DBCLK,      0x44E0053C,__READ_WRITE ,__clksel_gpio0_dbclk_bits);

/***************************************************************************
 **
 ** CM_MPU REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_MPU_CLKSTCTRL,        0x44E00600,__READ_WRITE ,__cm_mpu_clkstctrl_bits);
__IO_REG32_BIT(CM_MPU_MPU_CLKCTRL,      0x44E00604,__READ_WRITE ,__cm_clkctrl0_bits);

/***************************************************************************
 **
 ** CM_DEVICE REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_CLKOUT_CTRL,          0x44E00700,__READ_WRITE ,__cm_clkout_ctrl_bits);

/***************************************************************************
 **
 ** CM_RTC REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_RTC_RTC_CLKCTRL,      0x44E00800,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_RTC_CLKSTCTRL,        0x44E00804,__READ_WRITE ,__cm_rtc_clkstctrl_bits);

/***************************************************************************
 **
 ** CM_GFX REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_GFX_L3_CLKSTCTRL,       0x44E00900,__READ_WRITE ,__cm_gfx_l3_clkstctrl_bits);
__IO_REG32_BIT(CM_GFX_GFX_CLKCTRL,        0x44E00904,__READ_WRITE ,__cm_clkctrl0_bits);
__IO_REG32_BIT(CM_GFX_L4LS_GFX_CLKSTCTRL, 0x44E0090C,__READ_WRITE ,__cm_gfx_l4ls_gfx_clkstctrl_bits);
__IO_REG32_BIT(CM_GFX_MMUCFG_CLKCTRL,     0x44E00910,__READ_WRITE ,__cm_clkctrl1_bits);
__IO_REG32_BIT(CM_GFX_MMUDATA_CLKCTRL,    0x44E00914,__READ_WRITE ,__cm_clkctrl1_bits);

/***************************************************************************
 **
 ** CM_CEFUSE REGISTERS
 **
 ***************************************************************************/
__IO_REG32_BIT(CM_CEFUSE_CLKSTCTRL,       0x44E00A00,__READ_WRITE ,__cm_cefuse_clkstctrl_bits);
__IO_REG32_BIT(CM_CEFUSE_CEFUSE_CLKCTRL,  0x44E00A20,__READ_WRITE ,__cm_clkctrl1_bits);

/***************************************************************************
 **
 ** PRM_IRQ
 **
 ***************************************************************************/
__IO_REG32_BIT(REVISION_PRM,              0x44E00B00,__READ       ,__revision_prm_bits);
__IO_REG32_BIT(PRM_IRQSTATUS_MPU,         0x44E00B04,__READ_WRITE ,__prm_irqstatus_bits);
__IO_REG32_BIT(PRM_IRQENABLE_MPU,         0x44E00B08,__READ_WRITE ,__prm_irqenable_bits);
__IO_REG32_BIT(PRM_IRQSTATUS_M3,          0x44E00B0C,__READ_WRITE ,__prm_irqstatus_bits);
__IO_REG32_BIT(PRM_IRQENABLE_M3,          0x44E00B10,__READ_WRITE ,__prm_irqenable_bits);

/***************************************************************************
 **
 ** PRM_PER
 **
 ***************************************************************************/
__IO_REG32_BIT(RM_PER_RSTCTRL,            0x44E00C00,__READ_WRITE ,__rm_per_rstctrl_bits);
__IO_REG32_BIT(PM_PER_PWRSTST,            0x44E00C08,__READ       ,__pm_per_pwrstst_bits);
__IO_REG32_BIT(PM_PER_PWRSTCTRL,          0x44E00C0C,__READ_WRITE ,__pm_per_pwrstctrl_bits);

/***************************************************************************
 **
 ** PRM_WKUP
 **
 ***************************************************************************/
__IO_REG32_BIT(RM_WKUP_RSTCTRL,           0x44E00D00,__READ_WRITE ,__rm_wkup_rstctrl_bits);
__IO_REG32_BIT(PM_WKUP_PWRSTCTRL,         0x44E00D04,__READ_WRITE ,__pm_wkup_pwrstctrl_bits);
__IO_REG32_BIT(PM_WKUP_PWRSTST,           0x44E00D08,__READ       ,__pm_wkup_pwrstst_bits);
__IO_REG32_BIT(RM_WKUP_RSTS,              0x44E00D0C,__READ_WRITE ,__rm_wkup_rsts_bits);

/***************************************************************************
 **
 ** PRM_MPU
 **
 ***************************************************************************/
__IO_REG32_BIT(PM_MPU_PWRSTCTRL,          0x44E00E00,__READ_WRITE ,__pm_mpu_pwrstctrl_bits);
__IO_REG32_BIT(PM_MPU_PWRSTST,            0x44E00E04,__READ       ,__pm_mpu_pwrstst_bits);
__IO_REG32_BIT(RM_MPU_RSTST,              0x44E00E08,__READ_WRITE ,__rm_mpu_rstst_bits);

/***************************************************************************
 **
 ** PRM_DEVICE
 **
 ***************************************************************************/
__IO_REG32_BIT(PRM_RSTCTRL,               0x44E00F00,__READ_WRITE ,__prm_rstctrl_bits);
__IO_REG32_BIT(PRM_RSTTIME,               0x44E00F04,__READ_WRITE ,__prm_rsttime_bits);
__IO_REG32_BIT(PRM_RSTST,                 0x44E00F08,__READ_WRITE ,__prm_rstst_bits);
__IO_REG32_BIT(PRM_SRAM_COUNT,            0x44E00F0C,__READ_WRITE ,__prm_sram_count_bits);
__IO_REG32_BIT(PRM_LDO_SRAM_CORE_SETUP,   0x44E00F10,__READ_WRITE ,__prm_ldo_sram_setup_bits);
__IO_REG32_BIT(PRM_LDO_SRAM_CORE_CTRL,    0x44E00F14,__READ_WRITE ,__prm_ldo_sram_ctrl_bits);
__IO_REG32_BIT(PRM_LDO_SRAM_MPU_SETUP,    0x44E00F18,__READ_WRITE ,__prm_ldo_sram_setup_bits);
__IO_REG32_BIT(PRM_LDO_SRAM_MPU_CTRL,     0x44E00F1C,__READ_WRITE ,__prm_ldo_sram_ctrl_bits);

/***************************************************************************
 **
 ** PRM_RTC
 **
 ***************************************************************************/
__IO_REG32_BIT(PM_RTC_PWRSTCTRL,          0x44E01000,__READ_WRITE ,__pm_rtc_pwrstctrl_bits);
__IO_REG32_BIT(PM_RTC_PWRSTST,            0x44E01004,__READ       ,__pm_rtc_pwrstst_bits);

/***************************************************************************
 **
 ** PRM_GFX
 **
 ***************************************************************************/
__IO_REG32_BIT(PM_GFX_PWRSTCTRL,          0x44E01100,__READ_WRITE ,__pm_gfx_pwrstctrl_bits);
__IO_REG32_BIT(RM_GFX_RSTCTRL,            0x44E01104,__READ_WRITE ,__rm_gfx_rstctrl_bits);
__IO_REG32_BIT(PM_GFX_PWRSTST,            0x44E01110,__READ       ,__pm_gfx_pwrstst_bits);
__IO_REG32_BIT(RM_GFX_RSTST,              0x44E01114,__READ_WRITE ,__rm_gfx_rstst_bits);

/***************************************************************************
 **
 ** PRM_CEFUSE
 **
 ***************************************************************************/
__IO_REG32_BIT(PM_CEFUSE_PWRSTCTRL,       0x44E01200,__READ_WRITE ,__pm_cefuse_pwrstctrl_bits);
__IO_REG32_BIT(PM_CEFUSE_PWRSTST,         0x44E01204,__READ_WRITE ,__pm_cefuse_pwrstst_bits);

/***************************************************************************
 **
 ** CONTROL_MODULE
 **
 ***************************************************************************/
__IO_REG32_BIT(CONTROL_REVISION,          0x44E10000,__READ       ,__control_revision_bits);
__IO_REG32(    CONTROL_HWINFO,            0x44E10004,__READ       );
__IO_REG32_BIT(CONTROL_SYSCONFIG,         0x44E10010,__READ_WRITE ,__control_sysconfig_bits);
__IO_REG32_BIT(CONTROL_STATUS,            0x44E10040,__READ_WRITE ,__control_status_bits);
__IO_REG32_BIT(CORTEX_VBBLDO_CTRL,        0x44E1041C,__READ_WRITE ,__cortex_vbbldo_ctrl_bits);
__IO_REG32_BIT(CORE_SLDO_CTRL,            0x44E10428,__READ_WRITE ,__sldo_ctrl_bits);
__IO_REG32_BIT(MPU_SLDO_CTRL,             0x44E1042C,__READ_WRITE ,__sldo_ctrl_bits);
__IO_REG32_BIT(CLK32KDIVRATIO_CTRL,       0x44E10444,__READ_WRITE ,__clk32kdivratio_ctrl_bits);
__IO_REG32_BIT(BANDGAP_CTRL,              0x44E10448,__READ_WRITE ,__bandgap_ctrl_bits);
__IO_REG32_BIT(BANDGAP_TRIM,              0x44E1044C,__READ_WRITE ,__bandgap_trim_bits);
__IO_REG32_BIT(PLL_CLKINPULOW_CTRL,       0x44E10458,__READ_WRITE ,__pll_clkinpulow_ctrl_bits);
__IO_REG32_BIT(MOSC_CTRL,                 0x44E10468,__READ_WRITE ,__mosc_ctrl_bits);
__IO_REG32_BIT(RCOSC_CTRL,                0x44E1046C,__READ_WRITE ,__rcosc_ctrl_bits);
__IO_REG32_BIT(DEEPSLEEP_CTRL,            0x44E10470,__READ_WRITE ,__deepsleep_ctrl_bits);
__IO_REG32_BIT(DEVICE_ID,                 0x44E10600,__READ       ,__device_id_bits);
__IO_REG32_BIT(DEV_FEATURE,               0x44E10604,__READ       ,__dev_feature_bits);
__IO_REG32_BIT(INIT_PRIORITY_0,           0x44E10608,__READ_WRITE ,__init_priority_0_bits);
__IO_REG32_BIT(INIT_PRIORITY_1,           0x44E1060C,__READ_WRITE ,__init_priority_1_bits);
__IO_REG32_BIT(MMU_CFG,                   0x44E10610,__READ_WRITE ,__mmu_cfg_bits);
__IO_REG32_BIT(TPTC_CFG,                  0x44E10614,__READ_WRITE ,__tptc_cfg_bits);
__IO_REG32_BIT(USB_CTRL0,                 0x44E10620,__READ_WRITE ,__usb_ctrl_bits);
__IO_REG32_BIT(USB_STS0,                  0x44E10624,__READ       ,__usb_sts_bits);
__IO_REG32_BIT(USB_CTRL1,                 0x44E10628,__READ_WRITE ,__usb_ctrl_bits);
__IO_REG32_BIT(USB_STS1,                  0x44E1062C,__READ       ,__usb_sts_bits);
__IO_REG32_BIT(MAC_ID0_LO,                0x44E10630,__READ       ,__mac_id_lo_bits);
__IO_REG32_BIT(MAC_ID0_HI,                0x44E10634,__READ       ,__mac_id_hi_bits);
__IO_REG32_BIT(MAC_ID1_LO,                0x44E10638,__READ       ,__mac_id_lo_bits);
__IO_REG32_BIT(MAC_ID1_HI,                0x44E1063C,__READ       ,__mac_id_hi_bits);
__IO_REG32_BIT(DCAN_RAMINIT,              0x44E10644,__READ_WRITE ,__dcan_raminit_bits);
__IO_REG32_BIT(USB_WKUP_CTRL,             0x44E10648,__READ_WRITE ,__usb_wkup_ctrl_bits);
__IO_REG32_BIT(GMII_SEL,                  0x44E10650,__READ_WRITE ,__gmii_sel_bits);
__IO_REG32_BIT(PWMSS_CTRL,                0x44E10664,__READ_WRITE ,__pwmss_ctrl_bits);
__IO_REG32_BIT(MREQPRIO_0,                0x44E10670,__READ_WRITE ,__mreqprio_0_bits);
__IO_REG32_BIT(MREQPRIO_1,                0x44E10674,__READ_WRITE ,__mreqprio_1_bits);
__IO_REG32_BIT(HW_EVENT_SEL_GRP1,         0x44E10690,__READ_WRITE ,__hw_event_sel_grp1_bits);
__IO_REG32_BIT(HW_EVENT_SEL_GRP2,         0x44E10694,__READ_WRITE ,__hw_event_sel_grp2_bits);
__IO_REG32_BIT(HW_EVENT_SEL_GRP3,         0x44E10698,__READ_WRITE ,__hw_event_sel_grp3_bits);
__IO_REG32_BIT(HW_EVENT_SEL_GRP4,         0x44E1069C,__READ_WRITE ,__hw_event_sel_grp4_bits);
__IO_REG32_BIT(SMRT_CTRL,                 0x44E106A0,__READ_WRITE ,__smrt_ctrl_bits);
__IO_REG32_BIT(MPUSS_HW_DEBUG_SEL,        0x44E106A4,__READ_WRITE ,__mpuss_hw_debug_sel_bits);
__IO_REG32(    MPUSS_HW_DBG_INFO,         0x44E106A8,__READ_WRITE );
__IO_REG32_BIT(VDD_MPU_OPP_050,           0x44E10770,__READ       ,__vdd_opp_bits);
__IO_REG32_BIT(VDD_MPU_OPP_100,           0x44E10774,__READ       ,__vdd_opp_bits);
__IO_REG32_BIT(VDD_MPU_OPP_120,           0x44E10778,__READ       ,__vdd_opp_bits);
__IO_REG32_BIT(VDD_MPU_OPP_TURBO,         0x44E1077C,__READ       ,__vdd_opp_bits);
__IO_REG32_BIT(VDD_CORE_OPP_050,          0x44E107B8,__READ       ,__vdd_opp_bits);
__IO_REG32_BIT(VDD_CORE_OPP_100,          0x44E107BC,__READ       ,__vdd_opp_bits);
__IO_REG32_BIT(BB_SCALE,                  0x44E107D0,__READ       ,__bb_scale_bits);
__IO_REG32_BIT(USB_VID_PID,               0x44E107F4,__READ       ,__usb_vid_pid_bits);
__IO_REG32_BIT(CONF_GPMC_AD0,             0x44E10800,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD1,             0x44E10804,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD2,             0x44E10808,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD3,             0x44E1080C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD4,             0x44E10810,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD5,             0x44E10814,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD6,             0x44E10818,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD7,             0x44E1081C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD8,             0x44E10820,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD9,             0x44E10824,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD10,            0x44E10828,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD11,            0x44E1082C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD12,            0x44E10830,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD13,            0x44E10834,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD14,            0x44E10838,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_AD15,            0x44E1083C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A0,              0x44E10840,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A1,              0x44E10844,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A2,              0x44E10848,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A3,              0x44E1084C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A4,              0x44E10850,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A5,              0x44E10854,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A6,              0x44E10858,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A7,              0x44E1085C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A8,              0x44E10860,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A9,              0x44E10864,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A10,             0x44E10868,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_A11,             0x44E1086C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_WAIT0,           0x44E10870,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_WPN,             0x44E10874,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_BE1N,            0x44E10878,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_CSN0,            0x44E1087C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_CSN1,            0x44E10880,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_CSN2,            0x44E10884,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_CSN3,            0x44E10888,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_CLK,             0x44E1088C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_ADVN_ALE,        0x44E10890,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_OEN_REN,         0x44E10894,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_WEN,             0x44E10898,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_GPMC_BE0N_CLE,        0x44E1089C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA0,            0x44E108A0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA1,            0x44E108A4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA2,            0x44E108A8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA3,            0x44E108AC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA4,            0x44E108B0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA5,            0x44E108B4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA6,            0x44E108B8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA7,            0x44E108BC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA8,            0x44E108C0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA9,            0x44E108C4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA10,           0x44E108C8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA11,           0x44E108CC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA12,           0x44E108D0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA13,           0x44E108D4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA14,           0x44E108D8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_DATA15,           0x44E108DC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_VSYNC,            0x44E108E0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_HSYNC,            0x44E108E4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_PCLK,             0x44E108E8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_LCD_AC_BIAS_EN,       0x44E108EC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MMC0_DAT3,            0x44E108F0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MMC0_DAT2,            0x44E108F4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MMC0_DAT1,            0x44E108F8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MMC0_DAT0,            0x44E108FC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MMC0_CLK,             0x44E10900,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MMC0_CMD,             0x44E10904,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_COL,             0x44E10908,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_CRS,             0x44E1090C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXERR,           0x44E10910,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_TXEN,            0x44E10914,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXDV,            0x44E10918,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_TXD3,            0x44E1091C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_TXD2,            0x44E10920,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_TXD1,            0x44E10924,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_TXD0,            0x44E10928,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_TXCLK,           0x44E1092C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXCLK,           0x44E10930,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXD3,            0x44E10934,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXD2,            0x44E10938,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXD1,            0x44E1093C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MII1_RXD0,            0x44E10940,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_RMII1_REFCLK,         0x44E10944,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MDIO_DATA,            0x44E10948,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MDIO_CLK,             0x44E1094C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_SPI0_SCLK,            0x44E10950,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_SPI0_D0,              0x44E10954,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_SPI0_D1,              0x44E10958,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_SPI0_CS0,             0x44E1095C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_SPI0_CS1,             0x44E10960,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_ECAP0_IN_PWM0_OUT,    0x44E10964,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART0_CTSN,           0x44E10968,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART0_RTSN,           0x44E1096C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART0_RXD,            0x44E10970,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART0_TXD,            0x44E10974,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART1_CTSN,           0x44E10978,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART1_RTSN,           0x44E1097C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART1_RXD,            0x44E10980,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_UART1_TXD,            0x44E10984,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_I2C0_SDA,             0x44E10988,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_I2C0_SCL,             0x44E1098C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_ACLKX,         0x44E10990,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_FSX,           0x44E10994,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_AXR0,          0x44E10998,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_AHCLKR,        0x44E1099C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_ACLKR,         0x44E109A0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_FSR,           0x44E109A4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_AXR1,          0x44E109A8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_MCASP0_AHCLKX,        0x44E109AC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_XDMA_EVENT_INTR0,     0x44E109B0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_XDMA_EVENT_INTR1,     0x44E109B4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_NRESETIN_OUT,         0x44E109B8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_PORZ,                 0x44E109BC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_NNMI,                 0x44E109C0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_OSC0_IN,              0x44E109C4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_OSC0_OUT,             0x44E109C8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_OSC0_VSS,             0x44E109CC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_TMS,                  0x44E109D0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_TDI,                  0x44E109D4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_TDO,                  0x44E109D8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_TCK,                  0x44E109DC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_NTRST,                0x44E109E0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_EMU0,                 0x44E109E4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_EMU1,                 0x44E109E8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_OSC1_IN,              0x44E109EC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_OSC1_OUT,             0x44E109F0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_OSC1_VSS,             0x44E109F4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_RTC_PORZ,             0x44E109F8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_PMIC_POWER_EN,        0x44E109FC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_EXT_WAKEUP,           0x44E10A00,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_ENZ_KALDO_1P8V,       0x44E10A04,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB0_DM,              0x44E10A08,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB0_DP,              0x44E10A0C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB0_CE,              0x44E10A10,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB0_ID,              0x44E10A14,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB0_VBUS,            0x44E10A18,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB0_DRVVBUS,         0x44E10A1C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB1_DM,              0x44E10A20,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB1_DP,              0x44E10A24,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB1_CE,              0x44E10A28,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB1_ID,              0x44E10A2C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB1_VBUS,            0x44E10A30,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_USB1_DRVVBUS,         0x44E10A34,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_RESETN,           0x44E10A38,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_CSN0,             0x44E10A3C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_CKE,              0x44E10A40,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_NCK,              0x44E10A48,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_CASN,             0x44E10A4C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_RASN,             0x44E10A50,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_WEN,              0x44E10A54,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_BA0,              0x44E10A58,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_BA1,              0x44E10A5C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_BA2,              0x44E10A60,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A0,               0x44E10A64,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A1,               0x44E10A68,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A2,               0x44E10A6C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A3,               0x44E10A70,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A4,               0x44E10A74,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A5,               0x44E10A78,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A6,               0x44E10A7C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A7,               0x44E10A80,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A8,               0x44E10A84,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A9,               0x44E10A88,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A10,              0x44E10A8C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A11,              0x44E10A90,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A12,              0x44E10A94,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A13,              0x44E10A98,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A14,              0x44E10A9C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_A15,              0x44E10AA0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_ODT,              0x44E10AA4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D0,               0x44E10AA8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D1,               0x44E10AAC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D2,               0x44E10AB0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D3,               0x44E10AB4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D4,               0x44E10AB8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D5,               0x44E10ABC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D6,               0x44E10AC0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D7,               0x44E10AC4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D8,               0x44E10AC8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D9,               0x44E10ACC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D10,              0x44E10AD0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D11,              0x44E10AD4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D12,              0x44E10AD8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D13,              0x44E10ADC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D14,              0x44E10AE0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_D15,              0x44E10AE4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_DQM0,             0x44E10AE8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_DQM1,             0x44E10AEC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_DQS0,             0x44E10AF0,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_DQSN0,            0x44E10AF4,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_DQS1,             0x44E10AF8,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_DQSN1,            0x44E10AFC,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_VREF,             0x44E10B00,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_VTP,              0x44E10B04,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_STRBEN0,          0x44E10B08,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_DDR_STRBEN1,          0x44E10B0C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN7,                 0x44E10B10,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN6,                 0x44E10B14,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN5,                 0x44E10B18,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN4,                 0x44E10B1C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN3,                 0x44E10B20,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN2,                 0x44E10B24,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN1,                 0x44E10B28,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AIN0,                 0x44E10B2C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_VREFP,                0x44E10B30,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_VREFN,                0x44E10B34,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AVDD,                 0x44E10B38,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_AVSS,                 0x44E10B3C,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_IFORCE,               0x44E10B40,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_VSENSE,               0x44E10B44,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CONF_TESTOUT,              0x44E10B48,__READ_WRITE ,__conf_module_pin_bits);
__IO_REG32_BIT(CQDETECT_STATUS,           0x44E10E00,__READ       ,__cqdetect_status_bits);
__IO_REG32_BIT(DDR_IO_CTRL,               0x44E10E04,__READ_WRITE ,__ddr_io_ctrl_bits);
__IO_REG32_BIT(VTP_CTRL,                  0x44E10E0C,__READ_WRITE ,__vtp_ctrl_bits);
__IO_REG32_BIT(VREF_CTRL,                 0x44E10E14,__READ_WRITE ,__vref_ctrl_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_0_3,          0x44E10F90,__READ_WRITE ,__tpcc_evt_mux_0_3_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_4_7,          0x44E10F94,__READ_WRITE ,__tpcc_evt_mux_4_7_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_8_11,         0x44E10F98,__READ_WRITE ,__tpcc_evt_mux_8_11_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_12_15,        0x44E10F9C,__READ_WRITE ,__tpcc_evt_mux_12_15_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_16_19,        0x44E10FA0,__READ_WRITE ,__tpcc_evt_mux_16_19_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_20_23,        0x44E10FA4,__READ_WRITE ,__tpcc_evt_mux_20_23_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_24_27,        0x44E10FA8,__READ_WRITE ,__tpcc_evt_mux_24_27_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_28_31,        0x44E10FAC,__READ_WRITE ,__tpcc_evt_mux_28_31_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_32_35,        0x44E10FB0,__READ_WRITE ,__tpcc_evt_mux_32_35_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_36_39,        0x44E10FB4,__READ_WRITE ,__tpcc_evt_mux_36_39_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_40_43,        0x44E10FB8,__READ_WRITE ,__tpcc_evt_mux_40_43_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_44_47,        0x44E10FBC,__READ_WRITE ,__tpcc_evt_mux_44_47_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_48_51,        0x44E10FC0,__READ_WRITE ,__tpcc_evt_mux_48_51_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_52_55,        0x44E10FC4,__READ_WRITE ,__tpcc_evt_mux_52_55_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_56_59,        0x44E10FC8,__READ_WRITE ,__tpcc_evt_mux_56_59_bits);
__IO_REG32_BIT(TPCC_EVT_MUX_60_63,        0x44E10FCC,__READ_WRITE ,__tpcc_evt_mux_60_63_bits);
__IO_REG32_BIT(TIMER_EVT_CAPT,            0x44E10FD0,__READ_WRITE ,__timer_evt_capt_bits);
__IO_REG32_BIT(ECAP_EVT_CAPT,             0x44E10FD4,__READ_WRITE ,__ecap_evt_capt_bits);
__IO_REG32_BIT(ADC_EVT_CAPT,              0x44E10FD8,__READ_WRITE ,__adc_evt_capt_bits);
__IO_REG32_BIT(RESET_ISO,                 0x44E11000,__READ_WRITE ,__reset_iso_bits);
__IO_REG32_BIT(DDR_CKE_CTRL,              0x44E1131C,__READ_WRITE ,__ddr_cke_ctrl_bits);
__IO_REG32_BIT(M3_TXEV_EOI,               0x44E11324,__READ_WRITE ,__m3_txev_eoi_bits);
__IO_REG32_BIT(DDR_CMD0_IOCTRL,           0x44E11404,__READ_WRITE ,__ddr_cmd_ioctrl_bits);
__IO_REG32_BIT(DDR_CMD1_IOCTRL,           0x44E11408,__READ_WRITE ,__ddr_cmd_ioctrl_bits);
__IO_REG32_BIT(DDR_CMD2_IOCTRL,           0x44E1140C,__READ_WRITE ,__ddr_cmd_ioctrl_bits);
__IO_REG32_BIT(DDR_DATA0_IOCTRL,          0x44E11440,__READ_WRITE ,__ddr_data_ioctrl_bits);
__IO_REG32_BIT(DDR_DATA1_IOCTRL,          0x44E11444,__READ_WRITE ,__ddr_data_ioctrl_bits);

/***************************************************************************
 **
 ** EDMA3CC
 **
 ***************************************************************************/
__IO_REG32(    EDMACC_PID,            0x49000000,__READ       );
__IO_REG32_BIT(EDMACC_CCCFG,          0x49000004,__READ       ,__edmacc_cccfg_bits);
__IO_REG32_BIT(EDMACC_DCHMAP0,        0x49000100,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP1,        0x49000104,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP2,        0x49000108,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP3,        0x4900010C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP4,        0x49000110,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP5,        0x49000114,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP6,        0x49000118,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP7,        0x4900011C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP8,        0x49000120,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP9,        0x49000124,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP10,       0x49000128,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP11,       0x4900012C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP12,       0x49000130,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP13,       0x49000134,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP14,       0x49000138,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP15,       0x4900013C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP16,       0x49000140,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP17,       0x49000144,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP18,       0x49000148,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP19,       0x4900014C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP20,       0x49000150,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP21,       0x49000154,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP22,       0x49000158,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP23,       0x4900015C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP24,       0x49000160,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP25,       0x49000164,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP26,       0x49000168,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP27,       0x4900016C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP28,       0x49000170,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP29,       0x49000174,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP30,       0x49000178,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP31,       0x4900017C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP32,       0x49000180,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP33,       0x49000184,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP34,       0x49000188,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP35,       0x4900018C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP36,       0x49000190,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP37,       0x49000194,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP38,       0x49000198,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP39,       0x4900019C,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP40,       0x490001A0,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP41,       0x490001A4,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP42,       0x490001A8,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP43,       0x490001AC,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP44,       0x490001B0,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP45,       0x490001B4,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP46,       0x490001B8,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP47,       0x490001BC,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP48,       0x490001C0,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP49,       0x490001C4,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP50,       0x490001C8,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP51,       0x490001CC,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP52,       0x490001D0,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP53,       0x490001D4,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP54,       0x490001D8,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP55,       0x490001DC,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP56,       0x490001E0,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP57,       0x490001E4,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP58,       0x490001E8,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP59,       0x490001EC,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP60,       0x490001F0,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP61,       0x490001F4,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP62,       0x490001F8,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_DCHMAP63,       0x490001FC,__READ_WRITE ,__edmacc_dchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP0,        0x49000200,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP1,        0x49000204,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP2,        0x49000208,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP3,        0x4900020C,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP4,        0x49000210,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP5,        0x49000214,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP6,        0x49000218,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_QCHMAP7,        0x4900021C,__READ_WRITE ,__edmacc_qchmap_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM0,       0x49000240,__READ_WRITE ,__edmacc_dmaqnum0_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM1,       0x49000244,__READ_WRITE ,__edmacc_dmaqnum1_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM2,       0x49000248,__READ_WRITE ,__edmacc_dmaqnum2_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM3,       0x4900024C,__READ_WRITE ,__edmacc_dmaqnum3_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM4,       0x49000250,__READ_WRITE ,__edmacc_dmaqnum4_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM5,       0x49000254,__READ_WRITE ,__edmacc_dmaqnum5_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM6,       0x49000258,__READ_WRITE ,__edmacc_dmaqnum6_bits);
__IO_REG32_BIT(EDMACC_DMAQNUM7,       0x4900025C,__READ_WRITE ,__edmacc_dmaqnum7_bits);
__IO_REG32_BIT(EDMACC_QDMAQNUM,       0x49000260,__READ_WRITE ,__edmacc_qdmaqnum_bits);
__IO_REG32_BIT(EDMACC_QUEPRI,         0x49000284,__READ_WRITE ,__edmacc_quepri_bits);
__IO_REG32_BIT(EDMACC_EMR,            0x49000300,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_EMRH,           0x49000304,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_EMCR,           0x49000308,__WRITE      );
__IO_REG32(    EDMACC_EMCRH,          0x4900030C,__WRITE      );
__IO_REG32_BIT(EDMACC_QEMR,           0x49000310,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_QEMCR,          0x49000314,__WRITE      );
__IO_REG32_BIT(EDMACC_CCERR,          0x49000318,__READ_WRITE ,__edmacc_ccerr_bits);
__IO_REG32(    EDMACC_CCERRCLR,       0x4900031C,__WRITE      );
__IO_REG32(    EDMACC_EEVAL,          0x49000320,__WRITE      );
__IO_REG32_BIT(EDMACC_DRAE0,          0x49000340,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH0,         0x49000344,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE1,          0x49000348,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH1,         0x4900034C,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE2,          0x49000350,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH2,         0x49000354,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE3,          0x49000358,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH3,         0x4900035C,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE4,          0x49000360,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH4,         0x49000364,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE5,          0x49000368,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH5,         0x4900036C,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE6,          0x49000370,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH6,         0x49000374,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_DRAE7,          0x49000378,__READ_WRITE ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_DRAEH7,         0x4900037C,__READ_WRITE ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_QRAE0,          0x49000380,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE1,          0x49000384,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE2,          0x49000388,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE3,          0x4900038C,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE4,          0x49000390,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE5,          0x49000394,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE6,          0x49000398,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QRAE7,          0x4900039C,__READ_WRITE ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_Q0E0,           0x49000400,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E1,           0x49000404,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E2,           0x49000408,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E3,           0x4900040C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E4,           0x49000410,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E5,           0x49000414,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E6,           0x49000418,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E7,           0x4900041C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E8,           0x49000420,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E9,           0x49000424,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E10,          0x49000428,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E11,          0x4900042C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E12,          0x49000430,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E13,          0x49000434,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E14,          0x49000438,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q0E15,          0x4900043C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E0,           0x49000440,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E1,           0x49000444,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E2,           0x49000448,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E3,           0x4900044C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E4,           0x49000450,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E5,           0x49000454,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E6,           0x49000458,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E7,           0x4900045C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E8,           0x49000460,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E9,           0x49000464,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E10,          0x49000468,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E11,          0x4900046C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E12,          0x49000470,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E13,          0x49000474,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E14,          0x49000478,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q1E15,          0x4900047C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E0,           0x49000480,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E1,           0x49000484,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E2,           0x49000488,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E3,           0x4900048C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E4,           0x49000490,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E5,           0x49000494,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E6,           0x49000498,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E7,           0x4900049C,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E8,           0x490004A0,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E9,           0x490004A4,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E10,          0x490004A8,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E11,          0x490004AC,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E12,          0x490004B0,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E13,          0x490004B4,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E14,          0x490004B8,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q2E15,          0x490004BC,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E0,           0x490004C0,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E1,           0x490004C4,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E2,           0x490004C8,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E3,           0x490004CC,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E4,           0x490004D0,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E5,           0x490004D4,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E6,           0x490004D8,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E7,           0x490004DC,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E8,           0x490004E0,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E9,           0x490004E4,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E10,          0x490004E8,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E11,          0x490004EC,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E12,          0x490004F0,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E13,          0x490004F4,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E14,          0x490004F8,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_Q3E15,          0x490004FC,__READ       ,__edmacc_qe_bits);
__IO_REG32_BIT(EDMACC_QSTAT0,         0x49000600,__READ       ,__edmacc_qstat_bits);
__IO_REG32_BIT(EDMACC_QSTAT1,         0x49000604,__READ       ,__edmacc_qstat_bits);
__IO_REG32_BIT(EDMACC_QSTAT2,         0x49000608,__READ       ,__edmacc_qstat_bits);
__IO_REG32_BIT(EDMACC_QSTAT3,         0x4900060C,__READ       ,__edmacc_qstat_bits);
__IO_REG32_BIT(EDMACC_QWMTHRA,        0x49000620,__READ_WRITE ,__edmacc_qwmthra_bits);
__IO_REG32_BIT(EDMACC_CCSTAT,         0x49000640,__READ       ,__edmacc_ccstat_bits);
__IO_REG32(    EDMACC_MPFAR,          0x49000800,__READ       );
__IO_REG32_BIT(EDMACC_MPFSR,          0x49000804,__READ       ,__edmacc_mpfsr_bits);
__IO_REG32(    EDMACC_MPFCR,          0x49000808,__WRITE      );
__IO_REG32_BIT(EDMACC_MPPAG,          0x4900080C,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA0,          0x49000810,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA1,          0x49000814,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA2,          0x49000818,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA3,          0x4900081C,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA4,          0x49000820,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA5,          0x49000824,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA6,          0x49000828,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_MPPA7,          0x4900082C,__READ_WRITE ,__edmacc_mppa_bits);
__IO_REG32_BIT(EDMACC_ER,             0x49001000,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_ERH,            0x49001004,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_ECR,            0x49001008,__WRITE      );
__IO_REG32(    EDMACC_ECRH,           0x4900100C,__WRITE      );
__IO_REG32(    EDMACC_ESR,            0x49001010,__WRITE      );
__IO_REG32(    EDMACC_ESRH,           0x49001014,__WRITE      );
__IO_REG32_BIT(EDMACC_CER,            0x49001018,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_CERH,           0x4900101C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_EER,            0x49001020,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_EERH,           0x49001024,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_EECR,           0x49001028,__WRITE      );
__IO_REG32(    EDMACC_EECRH,          0x4900102C,__WRITE      );
__IO_REG32(    EDMACC_EESR,           0x49001030,__WRITE      );
__IO_REG32(    EDMACC_EESRH,          0x49001034,__WRITE      );
__IO_REG32_BIT(EDMACC_SER,            0x49001038,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SERH,           0x4900103C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SECR,           0x49001040,__WRITE      );
__IO_REG32(    EDMACC_SECRH,          0x49001044,__WRITE      );
__IO_REG32_BIT(EDMACC_IER,            0x49001050,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_IERH,           0x49001054,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_IECR,           0x49001058,__WRITE      );
__IO_REG32(    EDMACC_IECRH,          0x4900105C,__WRITE      );
__IO_REG32(    EDMACC_IESR,           0x49001060,__WRITE      );
__IO_REG32(    EDMACC_IESRH,          0x49001064,__WRITE      );
__IO_REG32_BIT(EDMACC_IPR,            0x49001068,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_IPRH,           0x4900106C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_ICR,            0x49001070,__WRITE      );
__IO_REG32(    EDMACC_ICRH,           0x49001074,__WRITE      );
__IO_REG32(    EDMACC_IEVAL,          0x49001078,__WRITE      );
__IO_REG32_BIT(EDMACC_QER,            0x49001080,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_QEER,           0x49001084,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_QEECR,          0x49001088,__WRITE      );
__IO_REG32(    EDMACC_QEESR,          0x4900108C,__WRITE      );
__IO_REG32_BIT(EDMACC_QSER,           0x49001090,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_QSECR,          0x49001094,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0ER,          0x49002000,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC0ERH,         0x49002004,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC0ECR,         0x49002008,__WRITE      );
__IO_REG32(    EDMACC_SC0ECRH,        0x4900200C,__WRITE      );
__IO_REG32(    EDMACC_SC0ESR,         0x49002010,__WRITE      );
__IO_REG32(    EDMACC_SC0ESRH,        0x49002014,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0CER,         0x49002018,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC0CERH,        0x4900201C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC0EER,         0x49002020,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC0EERH,        0x49002024,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC0EECR,        0x49002028,__WRITE      );
__IO_REG32(    EDMACC_SC0EECRH,       0x4900202C,__WRITE      );
__IO_REG32(    EDMACC_SC0EESR,        0x49002030,__WRITE      );
__IO_REG32(    EDMACC_SC0EESRH,       0x49002034,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0SER,         0x49002038,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC0SERH,        0x4900203C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC0SECR,        0x49002040,__WRITE      );
__IO_REG32(    EDMACC_SC0SECRH,       0x49002044,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0IER,         0x49002050,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC0IERH,        0x49002054,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC0IECR,        0x49002058,__WRITE      );
__IO_REG32(    EDMACC_SC0IECRH,       0x4900205C,__WRITE      );
__IO_REG32(    EDMACC_SC0IESR,        0x49002060,__WRITE      );
__IO_REG32(    EDMACC_SC0IESRH,       0x49002064,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0IPR,         0x49002068,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC0IPRH,        0x4900206C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC0ICR,         0x49002070,__WRITE      );
__IO_REG32(    EDMACC_SC0ICRH,        0x49002074,__WRITE      );
__IO_REG32(    EDMACC_SC0IEVAL,       0x49002078,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0QER,         0x49002080,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC0QEER,        0x49002084,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC0QEECR,       0x49002088,__WRITE      );
__IO_REG32(    EDMACC_SC0QEESR,       0x4900208C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC0QSER,        0x49002090,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC0QSECR,       0x49002094,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1ER,          0x49002200,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC1ERH,         0x49002204,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC1ECR,         0x49002208,__WRITE      );
__IO_REG32(    EDMACC_SC1ECRH,        0x4900220C,__WRITE      );
__IO_REG32(    EDMACC_SC1ESR,         0x49002210,__WRITE      );
__IO_REG32(    EDMACC_SC1ESRH,        0x49002214,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1CER,         0x49002218,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC1CERH,        0x4900221C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC1EER,         0x49002220,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC1EERH,        0x49002224,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC1EECR,        0x49002228,__WRITE      );
__IO_REG32(    EDMACC_SC1EECRH,       0x4900222C,__WRITE      );
__IO_REG32(    EDMACC_SC1EESR,        0x49002230,__WRITE      );
__IO_REG32(    EDMACC_SC1EESRH,       0x49002234,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1SER,         0x49002238,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC1SERH,        0x4900223C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC1SECR,        0x49002240,__WRITE      );
__IO_REG32(    EDMACC_SC1SECRH,       0x49002244,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1IER,         0x49002250,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC1IERH,        0x49002254,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC1IECR,        0x49002258,__WRITE      );
__IO_REG32(    EDMACC_SC1IECRH,       0x4900225C,__WRITE      );
__IO_REG32(    EDMACC_SC1IESR,        0x49002260,__WRITE      );
__IO_REG32(    EDMACC_SC1IESRH,       0x49002264,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1IPR,         0x49002268,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC1IPRH,        0x4900226C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC1ICR,         0x49002270,__WRITE      );
__IO_REG32(    EDMACC_SC1ICRH,        0x49002274,__WRITE      );
__IO_REG32(    EDMACC_SC1IEVAL,       0x49002278,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1QER,         0x49002280,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC1QEER,        0x49002284,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC1QEECR,       0x49002288,__WRITE      );
__IO_REG32(    EDMACC_SC1QEESR,       0x4900228C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC1QSER,        0x49002290,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC1QSECR,       0x49002294,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2ER,          0x49002400,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC2ERH,         0x49002404,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC2ECR,         0x49002408,__WRITE      );
__IO_REG32(    EDMACC_SC2ECRH,        0x4900240C,__WRITE      );
__IO_REG32(    EDMACC_SC2ESR,         0x49002410,__WRITE      );
__IO_REG32(    EDMACC_SC2ESRH,        0x49002414,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2CER,         0x49002418,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC2CERH,        0x4900241C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC2EER,         0x49002420,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC2EERH,        0x49002424,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC2EECR,        0x49002428,__WRITE      );
__IO_REG32(    EDMACC_SC2EECRH,       0x4900242C,__WRITE      );
__IO_REG32(    EDMACC_SC2EESR,        0x49002430,__WRITE      );
__IO_REG32(    EDMACC_SC2EESRH,       0x49002434,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2SER,         0x49002438,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC2SERH,        0x4900243C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC2SECR,        0x49002440,__WRITE      );
__IO_REG32(    EDMACC_SC2SECRH,       0x49002444,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2IER,         0x49002450,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC2IERH,        0x49002454,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC2IECR,        0x49002458,__WRITE      );
__IO_REG32(    EDMACC_SC2IECRH,       0x4900245C,__WRITE      );
__IO_REG32(    EDMACC_SC2IESR,        0x49002460,__WRITE      );
__IO_REG32(    EDMACC_SC2IESRH,       0x49002464,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2IPR,         0x49002468,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC2IPRH,        0x4900246C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC2ICR,         0x49002470,__WRITE      );
__IO_REG32(    EDMACC_SC2ICRH,        0x49002474,__WRITE      );
__IO_REG32(    EDMACC_SC2IEVAL,       0x49002478,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2QER,         0x49002480,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC2QEER,        0x49002484,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC2QEECR,       0x49002488,__WRITE      );
__IO_REG32(    EDMACC_SC2QEESR,       0x4900248C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC2QSER,        0x49002490,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC2QSECR,       0x49002494,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3ER,          0x49002600,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC3ERH,         0x49002604,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC3ECR,         0x49002608,__WRITE      );
__IO_REG32(    EDMACC_SC3ECRH,        0x4900260C,__WRITE      );
__IO_REG32(    EDMACC_SC3ESR,         0x49002610,__WRITE      );
__IO_REG32(    EDMACC_SC3ESRH,        0x49002614,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3CER,         0x49002618,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC3CERH,        0x4900261C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC3EER,         0x49002620,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC3EERH,        0x49002624,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC3EECR,        0x49002628,__WRITE      );
__IO_REG32(    EDMACC_SC3EECRH,       0x4900262C,__WRITE      );
__IO_REG32(    EDMACC_SC3EESR,        0x49002630,__WRITE      );
__IO_REG32(    EDMACC_SC3EESRH,       0x49002634,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3SER,         0x49002638,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC3SERH,        0x4900263C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC3SECR,        0x49002640,__WRITE      );
__IO_REG32(    EDMACC_SC3SECRH,       0x49002644,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3IER,         0x49002650,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC3IERH,        0x49002654,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC3IECR,        0x49002658,__WRITE      );
__IO_REG32(    EDMACC_SC3IECRH,       0x4900265C,__WRITE      );
__IO_REG32(    EDMACC_SC3IESR,        0x49002660,__WRITE      );
__IO_REG32(    EDMACC_SC3IESRH,       0x49002664,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3IPR,         0x49002668,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC3IPRH,        0x4900266C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC3ICR,         0x49002670,__WRITE      );
__IO_REG32(    EDMACC_SC3ICRH,        0x49002674,__WRITE      );
__IO_REG32(    EDMACC_SC3IEVAL,       0x49002678,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3QER,         0x49002680,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC3QEER,        0x49002684,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC3QEECR,       0x49002688,__WRITE      );
__IO_REG32(    EDMACC_SC3QEESR,       0x4900268C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC3QSER,        0x49002690,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC3QSECR,       0x49002694,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4ER,          0x49002800,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC4ERH,         0x49002804,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC4ECR,         0x49002808,__WRITE      );
__IO_REG32(    EDMACC_SC4ECRH,        0x4900280C,__WRITE      );
__IO_REG32(    EDMACC_SC4ESR,         0x49002810,__WRITE      );
__IO_REG32(    EDMACC_SC4ESRH,        0x49002814,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4CER,         0x49002818,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC4CERH,        0x4900281C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC4EER,         0x49002820,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC4EERH,        0x49002824,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC4EECR,        0x49002828,__WRITE      );
__IO_REG32(    EDMACC_SC4EECRH,       0x4900282C,__WRITE      );
__IO_REG32(    EDMACC_SC4EESR,        0x49002830,__WRITE      );
__IO_REG32(    EDMACC_SC4EESRH,       0x49002834,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4SER,         0x49002838,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC4SERH,        0x4900283C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC4SECR,        0x49002840,__WRITE      );
__IO_REG32(    EDMACC_SC4SECRH,       0x49002844,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4IER,         0x49002850,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC4IERH,        0x49002854,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC4IECR,        0x49002858,__WRITE      );
__IO_REG32(    EDMACC_SC4IECRH,       0x4900285C,__WRITE      );
__IO_REG32(    EDMACC_SC4IESR,        0x49002860,__WRITE      );
__IO_REG32(    EDMACC_SC4IESRH,       0x49002864,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4IPR,         0x49002868,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC4IPRH,        0x4900286C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC4ICR,         0x49002870,__WRITE      );
__IO_REG32(    EDMACC_SC4ICRH,        0x49002874,__WRITE      );
__IO_REG32(    EDMACC_SC4IEVAL,       0x49002878,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4QER,         0x49002880,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC4QEER,        0x49002884,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC4QEECR,       0x49002888,__WRITE      );
__IO_REG32(    EDMACC_SC4QEESR,       0x4900288C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC4QSER,        0x49002890,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC4QSECR,       0x49002894,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5ER,          0x49002A00,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC5ERH,         0x49002A04,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC5ECR,         0x49002A08,__WRITE      );
__IO_REG32(    EDMACC_SC5ECRH,        0x49002A0C,__WRITE      );
__IO_REG32(    EDMACC_SC5ESR,         0x49002A10,__WRITE      );
__IO_REG32(    EDMACC_SC5ESRH,        0x49002A14,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5CER,         0x49002A18,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC5CERH,        0x49002A1C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC5EER,         0x49002A20,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC5EERH,        0x49002A24,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC5EECR,        0x49002A28,__WRITE      );
__IO_REG32(    EDMACC_SC5EECRH,       0x49002A2C,__WRITE      );
__IO_REG32(    EDMACC_SC5EESR,        0x49002A30,__WRITE      );
__IO_REG32(    EDMACC_SC5EESRH,       0x49002A34,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5SER,         0x49002A38,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC5SERH,        0x49002A3C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC5SECR,        0x49002A40,__WRITE      );
__IO_REG32(    EDMACC_SC5SECRH,       0x49002A44,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5IER,         0x49002A50,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC5IERH,        0x49002A54,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC5IECR,        0x49002A58,__WRITE      );
__IO_REG32(    EDMACC_SC5IECRH,       0x49002A5C,__WRITE      );
__IO_REG32(    EDMACC_SC5IESR,        0x49002A60,__WRITE      );
__IO_REG32(    EDMACC_SC5IESRH,       0x49002A64,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5IPR,         0x49002A68,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC5IPRH,        0x49002A6C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC5ICR,         0x49002A70,__WRITE      );
__IO_REG32(    EDMACC_SC5ICRH,        0x49002A74,__WRITE      );
__IO_REG32(    EDMACC_SC5IEVAL,       0x49002A78,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5QER,         0x49002A80,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC5QEER,        0x49002A84,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC5QEECR,       0x49002A88,__WRITE      );
__IO_REG32(    EDMACC_SC5QEESR,       0x49002A8C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC5QSER,        0x49002A90,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC5QSECR,       0x49002A94,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6ER,          0x49002C00,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC6ERH,         0x49002C04,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC6ECR,         0x49002C08,__WRITE      );
__IO_REG32(    EDMACC_SC6ECRH,        0x49002C0C,__WRITE      );
__IO_REG32(    EDMACC_SC6ESR,         0x49002C10,__WRITE      );
__IO_REG32(    EDMACC_SC6ESRH,        0x49002C14,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6CER,         0x49002C18,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC6CERH,        0x49002C1C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC6EER,         0x49002C20,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC6EERH,        0x49002C24,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC6EECR,        0x49002C28,__WRITE      );
__IO_REG32(    EDMACC_SC6EECRH,       0x49002C2C,__WRITE      );
__IO_REG32(    EDMACC_SC6EESR,        0x49002C30,__WRITE      );
__IO_REG32(    EDMACC_SC6EESRH,       0x49002C34,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6SER,         0x49002C38,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC6SERH,        0x49002C3C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC6SECR,        0x49002C40,__WRITE      );
__IO_REG32(    EDMACC_SC6SECRH,       0x49002C44,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6IER,         0x49002C50,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC6IERH,        0x49002C54,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC6IECR,        0x49002C58,__WRITE      );
__IO_REG32(    EDMACC_SC6IECRH,       0x49002C5C,__WRITE      );
__IO_REG32(    EDMACC_SC6IESR,        0x49002C60,__WRITE      );
__IO_REG32(    EDMACC_SC6IESRH,       0x49002C64,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6IPR,         0x49002C68,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC6IPRH,        0x49002C6C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC6ICR,         0x49002C70,__WRITE      );
__IO_REG32(    EDMACC_SC6ICRH,        0x49002C74,__WRITE      );
__IO_REG32(    EDMACC_SC6IEVAL,       0x49002C78,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6QER,         0x49002C80,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC6QEER,        0x49002C84,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC6QEECR,       0x49002C88,__WRITE      );
__IO_REG32(    EDMACC_SC6QEESR,       0x49002C8C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC6QSER,        0x49002C90,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC6QSECR,       0x49002C94,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7ER,          0x49002E00,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC7ERH,         0x49002E04,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC7ECR,         0x49002E08,__WRITE      );
__IO_REG32(    EDMACC_SC7ECRH,        0x49002E0C,__WRITE      );
__IO_REG32(    EDMACC_SC7ESR,         0x49002E10,__WRITE      );
__IO_REG32(    EDMACC_SC7ESRH,        0x49002E14,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7CER,         0x49002E18,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC7CERH,        0x49002E1C,__READ       ,__edmacc_emrh_bits);
__IO_REG32_BIT(EDMACC_SC7EER,         0x49002E20,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC7EERH,        0x49002E24,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC7EECR,        0x49002E28,__WRITE      );
__IO_REG32(    EDMACC_SC7EECRH,       0x49002E2C,__WRITE      );
__IO_REG32(    EDMACC_SC7EESR,        0x49002E30,__WRITE      );
__IO_REG32(    EDMACC_SC7EESRH,       0x49002E34,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7SER,         0x49002E38,__READ       ,__edmacc_emr_bits);
__IO_REG32_BIT(EDMACC_SC7SERH,        0x49002E3C,__READ       ,__edmacc_emrh_bits);
__IO_REG32(    EDMACC_SC7SECR,        0x49002E40,__WRITE      );
__IO_REG32(    EDMACC_SC7SECRH,       0x49002E44,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7IER,         0x49002E50,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC7IERH,        0x49002E54,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC7IECR,        0x49002E58,__WRITE      );
__IO_REG32(    EDMACC_SC7IECRH,       0x49002E5C,__WRITE      );
__IO_REG32(    EDMACC_SC7IESR,        0x49002E60,__WRITE      );
__IO_REG32(    EDMACC_SC7IESRH,       0x49002E64,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7IPR,         0x49002E68,__READ       ,__edmacc_ier_bits);
__IO_REG32_BIT(EDMACC_SC7IPRH,        0x49002E6C,__READ       ,__edmacc_ierh_bits);
__IO_REG32(    EDMACC_SC7ICR,         0x49002E70,__WRITE      );
__IO_REG32(    EDMACC_SC7ICRH,        0x49002E74,__WRITE      );
__IO_REG32(    EDMACC_SC7IEVAL,       0x49002E78,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7QER,         0x49002E80,__READ       ,__edmacc_qemr_bits);
__IO_REG32_BIT(EDMACC_SC7QEER,        0x49002E84,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC7QEECR,       0x49002E88,__WRITE      );
__IO_REG32(    EDMACC_SC7QEESR,       0x49002E8C,__WRITE      );
__IO_REG32_BIT(EDMACC_SC7QSER,        0x49002E90,__READ       ,__edmacc_qemr_bits);
__IO_REG32(    EDMACC_SC7QSECR,       0x49002E94,__WRITE      );
__IO_REG32(    EDMACC_PaRAM_BA,       0x49004000,__READ_WRITE );

/***************************************************************************
 **
 ** EDMA3TC0
 **
 ***************************************************************************/
__IO_REG32(    EDMATC0_PID,           0x49800000,__READ       );
__IO_REG32_BIT(EDMATC0_TCCFG,         0x49800004,__READ       ,__edmatc_tccfg_bits);
__IO_REG32_BIT(EDMATC0_TCSTAT,        0x49800100,__READ       ,__edmatc_tcstat_bits);
__IO_REG32_BIT(EDMATC0_ERRSTAT,       0x49800120,__READ       ,__edmatc_errstat_bits);
__IO_REG32_BIT(EDMATC0_ERREN,         0x49800124,__READ_WRITE ,__edmatc_errstat_bits);
__IO_REG32(    EDMATC0_ERRCLR,        0x49800128,__WRITE      );
__IO_REG32_BIT(EDMATC0_ERRDET,        0x4980012C,__READ       ,__edmatc_errdet_bits);
__IO_REG32(    EDMATC0_ERRCMD,        0x49800130,__WRITE      );
__IO_REG32_BIT(EDMATC0_RDRATE,        0x49800140,__READ_WRITE ,__edmatc_rdrate_bits);
__IO_REG32_BIT(EDMATC0_SAOPT,         0x49800240,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32(    EDMATC0_SASRC,         0x49800244,__READ       );
__IO_REG32_BIT(EDMATC0_SACNT,         0x49800248,__READ       ,__edmatc_sacnt_bits);
__IO_REG32_BIT(EDMATC0_SABIDX,        0x49800250,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC0_SAMPPRXY,      0x49800254,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC0_SACNTRLD,      0x49800258,__READ       ,__edmatc_sacntrld_bits);
__IO_REG32(    EDMATC0_SASRCBREF,     0x4980025C,__READ       );
__IO_REG32_BIT(EDMATC0_DFCNTRLD,      0x49800280,__READ       ,__edmatc_sacntrld_bits);
__IO_REG32(    EDMATC0_DFDSTBREF,     0x49800288,__READ       );
__IO_REG32_BIT(EDMATC0_DFOPT0,        0x49800300,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC0_DFCNT0,        0x49800308,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC0_DFDST0,        0x4980030C,__READ       );
__IO_REG32_BIT(EDMATC0_DFBIDX0,       0x49800310,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC0_DFMPPRXY0,     0x49800314,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC0_DFOPT1,        0x49800340,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC0_DFCNT1,        0x49800348,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC0_DFDST1,        0x4980034C,__READ       );
__IO_REG32_BIT(EDMATC0_DFBIDX1,       0x49800350,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC0_DFMPPRXY1,     0x49800354,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC0_DFOPT2,        0x49800380,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC0_DFCNT2,        0x49800388,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC0_DFDST2,        0x4980038C,__READ       );
__IO_REG32_BIT(EDMATC0_DFBIDX2,       0x49800390,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC0_DFMPPRXY2,     0x49800394,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC0_DFOPT3,        0x498003C0,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC0_DFCNT3,        0x498003C8,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC0_DFDST3,        0x498003CC,__READ       );
__IO_REG32_BIT(EDMATC0_DFBIDX3,       0x498003D0,__READ_WRITE ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC0_DFMPPRXY3,     0x498003D4,__READ_WRITE ,__edmatc_sampprxy_bits);

/***************************************************************************
 **
 ** EDMA3TC1
 **
 ***************************************************************************/
__IO_REG32(    EDMATC1_PID,           0x49900000,__READ       );
__IO_REG32_BIT(EDMATC1_TCCFG,         0x49900004,__READ       ,__edmatc_tccfg_bits);
__IO_REG32_BIT(EDMATC1_TCSTAT,        0x49900100,__READ       ,__edmatc_tcstat_bits);
__IO_REG32_BIT(EDMATC1_ERRSTAT,       0x49900120,__READ       ,__edmatc_errstat_bits);
__IO_REG32_BIT(EDMATC1_ERREN,         0x49900124,__READ_WRITE ,__edmatc_errstat_bits);
__IO_REG32(    EDMATC1_ERRCLR,        0x49900128,__WRITE      );
__IO_REG32_BIT(EDMATC1_ERRDET,        0x4990012C,__READ       ,__edmatc_errdet_bits);
__IO_REG32(    EDMATC1_ERRCMD,        0x49900130,__WRITE      );
__IO_REG32_BIT(EDMATC1_RDRATE,        0x49900140,__READ_WRITE ,__edmatc_rdrate_bits);
__IO_REG32_BIT(EDMATC1_SAOPT,         0x49900240,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32(    EDMATC1_SASRC,         0x49900244,__READ       );
__IO_REG32_BIT(EDMATC1_SACNT,         0x49900248,__READ       ,__edmatc_sacnt_bits);
__IO_REG32_BIT(EDMATC1_SABIDX,        0x49900250,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC1_SAMPPRXY,      0x49900254,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC1_SACNTRLD,      0x49900258,__READ       ,__edmatc_sacntrld_bits);
__IO_REG32(    EDMATC1_SASRCBREF,     0x4990025C,__READ       );
__IO_REG32_BIT(EDMATC1_DFCNTRLD,      0x49900280,__READ       ,__edmatc_sacntrld_bits);
__IO_REG32(    EDMATC1_DFDSTBREF,     0x49900288,__READ       );
__IO_REG32_BIT(EDMATC1_DFOPT0,        0x49900300,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC1_DFCNT0,        0x49900308,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC1_DFDST0,        0x4990030C,__READ       );
__IO_REG32_BIT(EDMATC1_DFBIDX0,       0x49900310,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC1_DFMPPRXY0,     0x49900314,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC1_DFOPT1,        0x49900340,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC1_DFCNT1,        0x49900348,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC1_DFDST1,        0x4990034C,__READ       );
__IO_REG32_BIT(EDMATC1_DFBIDX1,       0x49900350,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC1_DFMPPRXY1,     0x49900354,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC1_DFOPT2,        0x49900380,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC1_DFCNT2,        0x49900388,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC1_DFDST2,        0x4990038C,__READ       );
__IO_REG32_BIT(EDMATC1_DFBIDX2,       0x49900390,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC1_DFMPPRXY2,     0x49900394,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC1_DFOPT3,        0x499003C0,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC1_DFCNT3,        0x499003C8,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC1_DFDST3,        0x499003CC,__READ       );
__IO_REG32_BIT(EDMATC1_DFBIDX3,       0x499003D0,__READ_WRITE ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC1_DFMPPRXY3,     0x499003D4,__READ_WRITE ,__edmatc_sampprxy_bits);

/***************************************************************************
 **
 ** EDMA3TC2
 **
 ***************************************************************************/
__IO_REG32(    EDMATC2_PID,           0x49A00000,__READ       );
__IO_REG32_BIT(EDMATC2_TCCFG,         0x49A00004,__READ       ,__edmatc_tccfg_bits);
__IO_REG32_BIT(EDMATC2_TCSTAT,        0x49A00100,__READ       ,__edmatc_tcstat_bits);
__IO_REG32_BIT(EDMATC2_ERRSTAT,       0x49A00120,__READ       ,__edmatc_errstat_bits);
__IO_REG32_BIT(EDMATC2_ERREN,         0x49A00124,__READ_WRITE ,__edmatc_errstat_bits);
__IO_REG32(    EDMATC2_ERRCLR,        0x49A00128,__WRITE      );
__IO_REG32_BIT(EDMATC2_ERRDET,        0x49A0012C,__READ       ,__edmatc_errdet_bits);
__IO_REG32(    EDMATC2_ERRCMD,        0x49A00130,__WRITE      );
__IO_REG32_BIT(EDMATC2_RDRATE,        0x49A00140,__READ_WRITE ,__edmatc_rdrate_bits);
__IO_REG32_BIT(EDMATC2_SAOPT,         0x49A00240,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32(    EDMATC2_SASRC,         0x49A00244,__READ       );
__IO_REG32_BIT(EDMATC2_SACNT,         0x49A00248,__READ       ,__edmatc_sacnt_bits);
__IO_REG32_BIT(EDMATC2_SABIDX,        0x49A00250,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC2_SAMPPRXY,      0x49A00254,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC2_SACNTRLD,      0x49A00258,__READ       ,__edmatc_sacntrld_bits);
__IO_REG32(    EDMATC2_SASRCBREF,     0x49A0025C,__READ       );
__IO_REG32_BIT(EDMATC2_DFCNTRLD,      0x49A00280,__READ       ,__edmatc_sacntrld_bits);
__IO_REG32(    EDMATC2_DFDSTBREF,     0x49A00288,__READ       );
__IO_REG32_BIT(EDMATC2_DFOPT0,        0x49A00300,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC2_DFCNT0,        0x49A00308,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC2_DFDST0,        0x49A0030C,__READ       );
__IO_REG32_BIT(EDMATC2_DFBIDX0,       0x49A00310,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC2_DFMPPRXY0,     0x49A00314,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC2_DFOPT1,        0x49A00340,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC2_DFCNT1,        0x49A00348,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC2_DFDST1,        0x49A0034C,__READ       );
__IO_REG32_BIT(EDMATC2_DFBIDX1,       0x49A00350,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC2_DFMPPRXY1,     0x49A00354,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC2_DFOPT2,        0x49A00380,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC2_DFCNT2,        0x49A00388,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC2_DFDST2,        0x49A0038C,__READ       );
__IO_REG32_BIT(EDMATC2_DFBIDX2,       0x49A00390,__READ       ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC2_DFMPPRXY2,     0x49A00394,__READ       ,__edmatc_sampprxy_bits);
__IO_REG32_BIT(EDMATC2_DFOPT3,        0x49A003C0,__READ_WRITE ,__edmatc_saopt_bits);
__IO_REG32_BIT(EDMATC2_DFCNT3,        0x49A003C8,__READ       ,__edmatc_sacnt_bits);
__IO_REG32(    EDMATC2_DFDST3,        0x49A003CC,__READ       );
__IO_REG32_BIT(EDMATC2_DFBIDX3,       0x49A003D0,__READ_WRITE ,__edmatc_sabidx_bits);
__IO_REG32_BIT(EDMATC2_DFMPPRXY3,     0x49A003D4,__READ_WRITE ,__edmatc_sampprxy_bits);

/***************************************************************************
 **
 ** TSC_ADC_SS
 **
 ***************************************************************************/
__IO_REG32_BIT(TSC_ADC_SS_REVISION,             0x44E0D000,__READ       ,__tsc_adc_ss_revision_bits);
__IO_REG32_BIT(TSC_ADC_SS_SYSCONFIG,            0x44E0D010,__READ_WRITE ,__tsc_adc_ss_sysconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_IRQ_EOI,              0x44E0D020,__READ_WRITE ,__tsc_adc_ss_irq_eoi_bits);
__IO_REG32_BIT(TSC_ADC_SS_IRQSTATUS_RAW,        0x44E0D024,__READ_WRITE ,__tsc_adc_ss_irq_bits);
__IO_REG32_BIT(TSC_ADC_SS_IRQSTATUS,            0x44E0D028,__READ_WRITE ,__tsc_adc_ss_irq_bits);
__IO_REG32_BIT(TSC_ADC_SS_IRQENABLE_SET,        0x44E0D02C,__READ_WRITE ,__tsc_adc_ss_irq_bits);
__IO_REG32_BIT(TSC_ADC_SS_IRQENABLE_CLR,        0x44E0D030,__READ_WRITE ,__tsc_adc_ss_irq_bits);
__IO_REG32_BIT(TSC_ADC_SS_IRQWAKEUP,            0x44E0D034,__READ_WRITE ,__tsc_adc_ss_irqwakeup_bits);
__IO_REG32_BIT(TSC_ADC_SS_DMAENABLE_SET,        0x44E0D038,__READ_WRITE ,__tsc_adc_ss_dmaenable_bits);
__IO_REG32_BIT(TSC_ADC_SS_DMAENABLE_CLR,        0x44E0D03C,__READ_WRITE ,__tsc_adc_ss_dmaenable_bits);
__IO_REG32_BIT(TSC_ADC_SS_CTRL,                 0x44E0D040,__READ_WRITE ,__tsc_adc_ss_ctrl_bits);
__IO_REG32_BIT(TSC_ADC_SS_ADCSTAT,              0x44E0D044,__READ       ,__tsc_adc_ss_adcstat_bits);
__IO_REG32_BIT(TSC_ADC_SS_ADCRANGE,             0x44E0D048,__READ_WRITE ,__tsc_adc_ss_adcrange_bits);
__IO_REG32_BIT(TSC_ADC_SS_ADC_CLKDIV,           0x44E0D04C,__READ_WRITE ,__tsc_adc_ss_adc_clkdiv_bits);
__IO_REG32_BIT(TSC_ADC_SS_ADC_MISC,             0x44E0D050,__READ_WRITE ,__tsc_adc_ss_adc_misc_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPENABLE,           0x44E0D054,__READ_WRITE ,__tsc_adc_ss_stepenable_bits);
__IO_REG32_BIT(TSC_ADC_SS_IDLECONFIG,           0x44E0D058,__READ_WRITE ,__tsc_adc_ss_idleconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_TS_CHARGE_STEPCONFIG, 0x44E0D05C,__READ_WRITE ,__tsc_adc_ss_idleconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_TS_CHARGE_DELAY,      0x44E0D060,__READ_WRITE ,__tsc_adc_ss_ts_charge_delay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG1,          0x44E0D064,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY1,           0x44E0D068,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG2,          0x44E0D06C,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY2,           0x44E0D070,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG3,          0x44E0D074,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY3,           0x44E0D078,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG4,          0x44E0D07C,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY4,           0x44E0D080,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG5,          0x44E0D084,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY5,           0x44E0D088,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG6,          0x44E0D08C,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY6,           0x44E0D090,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG7,          0x44E0D094,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY7,           0x44E0D098,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG8,          0x44E0D09C,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY8,           0x44E0D0A0,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG9,          0x44E0D0A4,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY9,           0x44E0D0A8,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG10,         0x44E0D0AC,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY10,          0x44E0D0B0,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG11,         0x44E0D0B4,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY11,          0x44E0D0B8,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG12,         0x44E0D0BC,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY12,          0x44E0D0C0,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG13,         0x44E0D0C4,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY13,          0x44E0D0C8,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG14,         0x44E0D0CC,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY14,          0x44E0D0D0,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG15,         0x44E0D0D4,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY15,          0x44E0D0D8,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPCONFIG16,         0x44E0D0DC,__READ_WRITE ,__tsc_adc_ss_stepconfig_bits);
__IO_REG32_BIT(TSC_ADC_SS_STEPDELAY16,          0x44E0D0E0,__READ_WRITE ,__tsc_adc_ss_stepdelay_bits);
__IO_REG32_BIT(TSC_ADC_SS_FIFO0COUNT,           0x44E0D0E4,__READ_WRITE ,__tsc_adc_ss_fifocount_bits);
__IO_REG32_BIT(TSC_ADC_SS_FIFO0THRESHOLD,       0x44E0D0E8,__READ_WRITE ,__tsc_adc_ss_fifothreshold_bits);
__IO_REG32_BIT(TSC_ADC_SS_DMA0REQ,              0x44E0D0EC,__READ_WRITE ,__tsc_adc_ss_dmareq_bits);
__IO_REG32_BIT(TSC_ADC_SS_FIFO1COUNT,           0x44E0D0F0,__READ_WRITE ,__tsc_adc_ss_fifocount_bits);
__IO_REG32_BIT(TSC_ADC_SS_FIFO1THRESHOLD,       0x44E0D0F4,__READ_WRITE ,__tsc_adc_ss_fifothreshold_bits);
__IO_REG32_BIT(TSC_ADC_SS_DMA1REQ,              0x44E0D0F8,__READ_WRITE ,__tsc_adc_ss_dmareq_bits);
__IO_REG32_BIT(TSC_ADC_SS_FIFO0DATA,            0x44E0D100,__READ_WRITE ,__tsc_adc_ss_fifodata_bits);
__IO_REG32_BIT(TSC_ADC_SS_FIFO1DATA,            0x44E0D200,__READ_WRITE ,__tsc_adc_ss_fifodata_bits);

/***************************************************************************
 **
 ** LCDC
 **
 ***************************************************************************/
__IO_REG32_BIT(LCDC_PID,              0x4830E000,__READ       ,__lcdc_pid_bits);
__IO_REG32_BIT(LCDC_CTRL,             0x4830E004,__READ_WRITE ,__lcdc_ctrl_bits);
__IO_REG32_BIT(LIDD_CTRL,             0x4830E00C,__READ_WRITE ,__lidd_ctrl_bits);
__IO_REG32_BIT(LIDD_CS0_CONF,         0x4830E010,__READ_WRITE ,__lcdc_cs_conf_bits);
__IO_REG32_BIT(LIDD_CS0_ADDR,         0x4830E014,__READ_WRITE ,__lcdc_cs_addr_bits);
__IO_REG32_BIT(LIDD_CS0_DATA,         0x4830E018,__READ_WRITE ,__lcdc_cs_data_bits);
__IO_REG32_BIT(LIDD_CS1_CONF,         0x4830E01C,__READ_WRITE ,__lcdc_cs_conf_bits);
__IO_REG32_BIT(LIDD_CS1_ADDR,         0x4830E020,__READ_WRITE ,__lcdc_cs_addr_bits);
__IO_REG32_BIT(LIDD_CS1_DATA,         0x4830E024,__READ_WRITE ,__lcdc_cs_data_bits);
__IO_REG32_BIT(LCDC_RASTER_CTRL,      0x4830E028,__READ_WRITE ,__lcdc_raster_ctrl_bits);
__IO_REG32_BIT(LCDC_RASTER_TIMING_0,  0x4830E02C,__READ_WRITE ,__lcdc_raster_timing_0_bits);
__IO_REG32_BIT(LCDC_RASTER_TIMING_1,  0x4830E030,__READ_WRITE ,__lcdc_raster_timing_1_bits);
__IO_REG32_BIT(LCDC_RASTER_TIMING_2,  0x4830E034,__READ_WRITE ,__lcdc_raster_timing_2_bits);
__IO_REG32_BIT(LCDC_RASTER_SUBPANEL,  0x4830E038,__READ_WRITE ,__lcdc_raster_subpanel_bits);
__IO_REG32_BIT(LCDC_RASTER_SUBPANEL2, 0x4830E03C,__READ_WRITE ,__lcdc_raster_subpanel2_bits);
__IO_REG32_BIT(LCDC_DMA_CTRL,         0x4830E040,__READ_WRITE ,__lcdc_dma_ctrl_bits);
__IO_REG32(    LCDC_DMA_FB0_BASE,     0x4830E044,__READ_WRITE );
__IO_REG32(    LCDC_DMA_FB0_CEILING,  0x4830E048,__READ_WRITE );
__IO_REG32(    LCDC_DMA_FB1_BASE,     0x4830E04C,__READ_WRITE );
__IO_REG32(    LCDC_DMA_FB1_CEILING,  0x4830E050,__READ_WRITE );
__IO_REG32_BIT(LCDC_SYSCONFIG,        0x4830E054,__READ_WRITE ,__lcdc_sysconfig_bits);
__IO_REG32_BIT(LCDC_IRQSTATUS_RAW,    0x4830E058,__READ_WRITE ,__lcdc_irqstatus_raw_bits);
__IO_REG32_BIT(LCDC_IRQSTATUS,        0x4830E05C,__READ_WRITE ,__lcdc_irqstatus_bits);
__IO_REG32_BIT(LCDC_IRQENABLE_SET,    0x4830E060,__READ_WRITE ,__lcdc_irqenable_set_bits);
__IO_REG32_BIT(LCDC_IRQENABLE_CLEAR,  0x4830E064,__READ_WRITE ,__lcdc_irqstatus_bits);
__IO_REG32(    LCDC_IRQEOI_VECTOR,    0x4830E068,__READ_WRITE );
__IO_REG32_BIT(LCDC_CLKC_ENABLE,      0x4830E06C,__READ_WRITE ,__lcdc_clkc_enable_bits);
__IO_REG32_BIT(LCDC_CLKC_RESET,       0x4830E070,__READ_WRITE ,__lcdc_clkc_reset_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_ALE
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_ALE_IDVER,        0x4A100D00,__READ       ,__cpsw_ale_idver_bits);
__IO_REG32_BIT(CPSW_ALE_CONTROL,      0x4A100D08,__READ_WRITE ,__cpsw_ale_control_bits);
__IO_REG32_BIT(CPSW_ALE_PRESCALE,     0x4A100D10,__READ_WRITE ,__cpsw_ale_prescale_bits);
__IO_REG32_BIT(CPSW_ALE_UNKNOWN_VLAN, 0x4A100D18,__READ_WRITE ,__cpsw_ale_unknown_vlan_bits);
__IO_REG32_BIT(CPSW_ALE_TBLCTL,       0x4A100D20,__READ_WRITE ,__cpsw_ale_tblctl_bits);
__IO_REG32(    CPSW_ALE_TBLW2,        0x4A100D34,__READ_WRITE );
__IO_REG32(    CPSW_ALE_TBLW1,        0x4A100D38,__READ_WRITE );
__IO_REG32(    CPSW_ALE_TBLW0,        0x4A100D3C,__READ_WRITE );
__IO_REG32_BIT(CPSW_ALE_PORTCTL0,     0x4A100D40,__READ_WRITE ,__cpsw_ale_portctl_bits);
__IO_REG32_BIT(CPSW_ALE_PORTCTL1,     0x4A100D44,__READ_WRITE ,__cpsw_ale_portctl_bits);
__IO_REG32_BIT(CPSW_ALE_PORTCTL2,     0x4A100D48,__READ_WRITE ,__cpsw_ale_portctl_bits);
__IO_REG32_BIT(CPSW_ALE_PORTCTL3,     0x4A100D4C,__READ_WRITE ,__cpsw_ale_portctl_bits);
__IO_REG32_BIT(CPSW_ALE_PORTCTL4,     0x4A100D50,__READ_WRITE ,__cpsw_ale_portctl_bits);
__IO_REG32_BIT(CPSW_ALE_PORTCTL5,     0x4A100D54,__READ_WRITE ,__cpsw_ale_portctl_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_CPDMA
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_CPDMA_TX_IDVER,           0x4A100800,__READ       ,__cpsw_cpdma_tx_idver_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_CONTROL,         0x4A100804,__READ_WRITE ,__cpsw_cpdam_tx_control_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_TEARDOWN,        0x4A100808,__READ_WRITE ,__cpsw_cpdam_tx_teardown_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX_IDVER,           0x4A100810,__READ       ,__cpsw_cpdam_rx_idver_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX_CONTROL,         0x4A100814,__READ_WRITE ,__cpsw_cpdam_rx_control_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX_TEARDOWN,        0x4A100818,__READ_WRITE ,__cpsw_cpdam_rx_teardown_bits);
__IO_REG32_BIT(CPSW_CPDMA_CPDMA_SOFT_RESET,   0x4A10081C,__READ_WRITE ,__cpsw_cpdma_soft_reset_bits);
__IO_REG32_BIT(CPSW_CPDMA_DMACONTROL,         0x4A100820,__READ_WRITE ,__cpsw_cpdma_dmacontrol_bits);
__IO_REG32_BIT(CPSW_CPDMA_DMASTATUS,          0x4A100824,__READ       ,__cpsw_cpdam_dmastatus_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX_BUFFER_OFFSET,   0x4A100828,__READ_WRITE ,__cpsw_cpdam_rx_buffer_offset_bits);
__IO_REG32_BIT(CPSW_CPDMA_EMCONTROL,          0x4A10082C,__READ_WRITE ,__cpsw_cpdam_emcontrol_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI0_RATE,       0x4A100830,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI1_RATE,       0x4A100834,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI2_RATE,       0x4A100838,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI3_RATE,       0x4A10083C,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI4_RATE,       0x4A100840,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI5_RATE,       0x4A100844,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI6_RATE,       0x4A100848,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_PRI7_RATE,       0x4A10084C,__READ_WRITE ,__cpsw_cpdam_tx_pri_rate_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_INTSTAT_RAW,     0x4A100880,__READ       ,__cpsw_cpdam_tx_intstat_bits);
__IO_REG32_BIT(CPSW_CPDMA_TX_INTSTAT_MASKED,  0x4A100884,__READ       ,__cpsw_cpdam_tx_intstat_bits);
__IO_REG32(    CPSW_CPDMA_TX_INTMASK_SET,     0x4A100888,__WRITE      );
__IO_REG32(    CPSW_CPDMA_TX_INTMASK_CLEAR,   0x4A10088C,__WRITE      );
__IO_REG32(    CPSW_CPDMA_IN_VECTOR,          0x4A100890,__READ       );
__IO_REG32_BIT(CPSW_CPDMA_EOI_VECTOR,         0x4A100894,__READ_WRITE ,__cpsw_cpdma_eoi_vector_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX_INTSTAT_RAW,     0x4A1008A0,__READ       ,__cpsw_cpdam_rx_intstat_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX_INTSTAT_MASKED,  0x4A1008A4,__READ       ,__cpsw_cpdam_rx_intstat_bits);
__IO_REG32(    CPSW_CPDMA_RX_INTMASK_SET,     0x4A1008A8,__WRITE      );
__IO_REG32(    CPSW_CPDMA_RX_INTMASK_CLEAR,   0x4A1008AC,__WRITE      );
__IO_REG32_BIT(CPSW_CPDMA_DMA_INTSTAT_RAW,    0x4A1008B0,__READ       ,__cpsw_cpdam_dma_intstat_bits);
__IO_REG32_BIT(CPSW_CPDMA_DMA_INTSTAT_MASKED, 0x4A1008B4,__READ       ,__cpsw_cpdam_dma_intstat_bits);
__IO_REG32(    CPSW_CPDMA_DMA_INTMASK_SET,    0x4A1008B8,__WRITE      );
__IO_REG32(    CPSW_CPDMA_DMA_INTMASK_CLEAR,  0x4A1008BC,__WRITE      );
__IO_REG32_BIT(CPSW_CPDMA_RX0_PENDTHRESH,     0x4A1008C0,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX1_PENDTHRESH,     0x4A1008C4,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX2_PENDTHRESH,     0x4A1008C8,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX3_PENDTHRESH,     0x4A1008CC,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX4_PENDTHRESH,     0x4A1008D0,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX5_PENDTHRESH,     0x4A1008D4,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX6_PENDTHRESH,     0x4A1008D8,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX7_PENDTHRESH,     0x4A1008DC,__READ_WRITE ,__cpsw_cpdam_rx_pendthresh_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX0_FREEBUFFER,     0x4A1008E0,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX1_FREEBUFFER,     0x4A1008E4,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX2_FREEBUFFER,     0x4A1008E8,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX3_FREEBUFFER,     0x4A1008EC,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX4_FREEBUFFER,     0x4A1008F0,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX5_FREEBUFFER,     0x4A1008F4,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX6_FREEBUFFER,     0x4A1008F8,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32_BIT(CPSW_CPDMA_RX7_FREEBUFFER,     0x4A1008FC,__READ_WRITE ,__cpsw_cpdam_rx_freebuffer_bits);
__IO_REG32(    CPSW_CPDMA_TX0_HDP,            0x4A100A00,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX1_HDP,            0x4A100A04,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX2_HDP,            0x4A100A08,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX3_HDP,            0x4A100A0C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX4_HDP,            0x4A100A10,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX5_HDP,            0x4A100A14,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX6_HDP,            0x4A100A18,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX7_HDP,            0x4A100A1C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX0_HDP,            0x4A100A20,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX1_HDP,            0x4A100A24,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX2_HDP,            0x4A100A28,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX3_HDP,            0x4A100A2C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX4_HDP,            0x4A100A30,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX5_HDP,            0x4A100A34,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX6_HDP,            0x4A100A38,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX7_HDP,            0x4A100A3C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX0_CP,             0x4A100A40,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX1_CP,             0x4A100A44,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX2_CP,             0x4A100A48,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX3_CP,             0x4A100A4C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX4_CP,             0x4A100A50,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX5_CP,             0x4A100A54,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX6_CP,             0x4A100A58,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_TX7_CP,             0x4A100A5C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX0_CP,             0x4A100A60,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX1_CP,             0x4A100A64,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX2_CP,             0x4A100A68,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX3_CP,             0x4A100A6C,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX4_CP,             0x4A100A70,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX5_CP,             0x4A100A74,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX6_CP,             0x4A100A78,__READ_WRITE );
__IO_REG32(    CPSW_CPDMA_RX7_CP,             0x4A100A7C,__READ_WRITE );

/***************************************************************************
 **
 ** Ethernet CPSW_CPTS
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_CPTS_IDVER,           0x4A100C00,__READ       ,__cpsw_cpts_idver_bits);
__IO_REG32_BIT(CPSW_CPTS_CONTROL,         0x4A100C04,__READ_WRITE ,__cpsw_cpts_control_bits);
__IO_REG32(    CPSW_CPTS_TS_PUSH,         0x4A100C0C,__WRITE      );
__IO_REG32(    CPSW_CPTS_TS_LOAD_VAL,     0x4A100C10,__READ_WRITE );
__IO_REG32(    CPSW_CPTS_TS_LOAD_EN,      0x4A100C14,__WRITE      );
__IO_REG32_BIT(CPSW_CPTS_INTSTAT_RAW,     0x4A100C20,__READ_WRITE ,__cpsw_cpts_intstat_raw_bits);
__IO_REG32_BIT(CPSW_CPTS_INTSTAT_MASKED,  0x4A100C24,__READ       ,__cpsw_cpts_intstat_masked_bits);
__IO_REG32_BIT(CPSW_CPTS_INT_ENABLE,      0x4A100C28,__READ_WRITE ,__cpsw_cpts_int_enable_bits);
__IO_REG32(    CPSW_CPTS_EVENT_POP,       0x4A100C30,__WRITE      );
__IO_REG32_BIT(CPSW_CPTS_EVENT_LOW,       0x4A100C34,__READ       ,__cpsw_cpts_event_low_bits);
__IO_REG32_BIT(CPSW_CPTS_EVENT_HIGH,      0x4A100C38,__READ       ,__cpsw_cpts_event_high_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_PORT
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_P0_CONTROL,           0x4A100100,__READ_WRITE ,__cpsw_p0_control_bits);
__IO_REG32_BIT(CPSW_P0_MAX_BLKS,          0x4A100108,__READ_WRITE ,__cpsw_p0_max_blks_bits);
__IO_REG32_BIT(CPSW_P0_BLK_CNT,           0x4A10010C,__READ       ,__cpsw_p0_blk_cnt_bits);
__IO_REG32_BIT(CPSW_P0_TX_IN_CTL,         0x4A100110,__READ_WRITE ,__cpsw_p0_tx_in_ctl_bits);
__IO_REG32_BIT(CPSW_P0_PORT_VLAN,         0x4A100114,__READ_WRITE ,__cpsw_p_port_vlan_bits);
__IO_REG32_BIT(CPSW_P0_TX_PRI_MAP,        0x4A100118,__READ_WRITE ,__cpsw_p_tx_pri_map_bits);
__IO_REG32_BIT(CPSW_P0_CPDMA_TX_PRI_MAP,  0x4A10011C,__READ_WRITE ,__cpsw_p0_cpdma_tx_pri_map_bits);
__IO_REG32_BIT(CPSW_P0_CPDMA_RX_CH_MAP,   0x4A100120,__READ_WRITE ,__cpsw_p0_cpdma_rx_ch_map_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP0,  0x4A100130,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map0_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP1,  0x4A100134,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map1_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP2,  0x4A100138,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map2_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP3,  0x4A10013C,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map3_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP4,  0x4A100140,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map4_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP5,  0x4A100144,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map5_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP6,  0x4A100148,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map6_bits);
__IO_REG32_BIT(CPSW_P0_RX_DSCP_PRI_MAP7,  0x4A10014C,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map7_bits);
__IO_REG32_BIT(CPSW_P1_CONTROL,           0x4A100200,__READ_WRITE ,__cpsw_p1_control_bits);
__IO_REG32_BIT(CPSW_P1_MAX_BLKS,          0x4A100208,__READ_WRITE ,__cpsw_p1_max_blks_bits);
__IO_REG32_BIT(CPSW_P1_BLK_CNT,           0x4A10020C,__READ       ,__cpsw_p1_blk_cnt_bits);
__IO_REG32_BIT(CPSW_P1_TX_IN_CTL,         0x4A100210,__READ_WRITE ,__cpsw_p1_tx_in_ctl_bits);
__IO_REG32_BIT(CPSW_P1_PORT_VLAN,         0x4A100214,__READ_WRITE ,__cpsw_p_port_vlan_bits);
__IO_REG32_BIT(CPSW_P1_TX_PRI_MAP,        0x4A100218,__READ_WRITE ,__cpsw_p_tx_pri_map_bits);
__IO_REG32_BIT(CPSW_P1_TS_SEQ_MTYPE,      0x4A10021C,__READ_WRITE ,__cpsw_p1_ts_seq_mtype_bits);
__IO_REG32_BIT(CPSW_P1_SA_LO,             0x4A100220,__READ_WRITE ,__cpsw_p_sa_lo_bits);
__IO_REG32_BIT(CPSW_P1_SA_HI,             0x4A100224,__READ_WRITE ,__cpsw_p_sa_hi_bits);
__IO_REG32_BIT(CPSW_P1_SEND_PERCENT,      0x4A100228,__READ_WRITE ,__cpsw_p_send_percent_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP0,  0x4A100230,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map0_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP1,  0x4A100234,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map1_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP2,  0x4A100238,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map2_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP3,  0x4A10023C,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map3_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP4,  0x4A100240,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map4_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP5,  0x4A100244,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map5_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP6,  0x4A100248,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map6_bits);
__IO_REG32_BIT(CPSW_P1_RX_DSCP_PRI_MAP7,  0x4A10024C,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map7_bits);
__IO_REG32_BIT(CPSW_P2_CONTROL,           0x4A100300,__READ_WRITE ,__cpsw_p2_control_bits);
__IO_REG32_BIT(CPSW_P2_MAX_BLKS,          0x4A100308,__READ_WRITE ,__cpsw_p2_max_blks_bits);
__IO_REG32_BIT(CPSW_P2_BLK_CNT,           0x4A10030C,__READ       ,__cpsw_p2_blk_cnt_bits);
__IO_REG32_BIT(CPSW_P2_TX_IN_CTL,         0x4A100310,__READ_WRITE ,__cpsw_p2_tx_in_ctl_bits);
__IO_REG32_BIT(CPSW_P2_PORT_VLAN,         0x4A100314,__READ_WRITE ,__cpsw_p_port_vlan_bits);
__IO_REG32_BIT(CPSW_P2_TX_PRI_MAP,        0x4A100318,__READ_WRITE ,__cpsw_p_tx_pri_map_bits);
__IO_REG32_BIT(CPSW_P2_TS_SEQ_MTYPE,      0x4A10031C,__READ_WRITE ,__cpsw_p2_ts_seq_mtype_bits);
__IO_REG32_BIT(CPSW_P2_SA_LO,             0x4A100320,__READ_WRITE ,__cpsw_p_sa_lo_bits);
__IO_REG32_BIT(CPSW_P2_SA_HI,             0x4A100324,__READ_WRITE ,__cpsw_p_sa_hi_bits);
__IO_REG32_BIT(CPSW_P2_SEND_PERCENT,      0x4A100328,__READ_WRITE ,__cpsw_p_send_percent_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP0,  0x4A100330,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map0_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP1,  0x4A100334,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map1_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP2,  0x4A100338,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map2_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP3,  0x4A10033C,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map3_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP4,  0x4A100340,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map4_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP5,  0x4A100344,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map5_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP6,  0x4A100348,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map6_bits);
__IO_REG32_BIT(CPSW_P2_RX_DSCP_PRI_MAP7,  0x4A10034C,__READ_WRITE ,__cpsw_p_rx_dscp_pri_map7_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_SL1
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_SL1_IDVER,        0x4A100D80,__READ       ,__cpsw_sl_idver_bits);
__IO_REG32_BIT(CPSW_SL1_MACCONTROL,   0x4A100D84,__READ_WRITE ,__cpsw_sl_maccontrol_bits);
__IO_REG32_BIT(CPSW_SL1_MACSTATUS,    0x4A100D88,__READ       ,__cpsw_sl_macstatus_bits);
__IO_REG32_BIT(CPSW_SL1_SOFT_RESET,   0x4A100D8C,__READ_WRITE ,__cpsw_sl_soft_reset_bits);
__IO_REG32_BIT(CPSW_SL1_RX_MAXLEN,    0x4A100D90,__READ_WRITE ,__cpsw_sl_rx_maxlen_bits);
__IO_REG32_BIT(CPSW_SL1_BOFFTEST,     0x4A100D94,__READ_WRITE ,__cpsw_sl_bofftest_bits);
__IO_REG32_BIT(CPSW_SL1_RX_PAUSE,     0x4A100D98,__READ       ,__cpsw_sl_rx_pause_bits);
__IO_REG32_BIT(CPSW_SL1_TX_PAUSE,     0x4A100D9C,__READ       ,__cpsw_sl_tx_pause_bits);
__IO_REG32_BIT(CPSW_SL1_EMCONTROL,    0x4A100DA0,__READ_WRITE ,__cpsw_sl_emcontrol_bits);
__IO_REG32_BIT(CPSW_SL1_RX_PRI_MAP,   0x4A100DA4,__READ_WRITE ,__cpsw_sl_rx_pri_map_bits);
__IO_REG32_BIT(CPSW_SL1_TX_GAP,       0x4A100DA8,__READ_WRITE ,__cpsw_sl_tx_gap_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_SL2
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_SL2_IDVER,        0x4A100DC0,__READ       ,__cpsw_sl_idver_bits);
__IO_REG32_BIT(CPSW_SL2_MACCONTROL,   0x4A100DC4,__READ_WRITE ,__cpsw_sl_maccontrol_bits);
__IO_REG32_BIT(CPSW_SL2_MACSTATUS,    0x4A100DC8,__READ       ,__cpsw_sl_macstatus_bits);
__IO_REG32_BIT(CPSW_SL2_SOFT_RESET,   0x4A100DCC,__READ_WRITE ,__cpsw_sl_soft_reset_bits);
__IO_REG32_BIT(CPSW_SL2_RX_MAXLEN,    0x4A100DD0,__READ_WRITE ,__cpsw_sl_rx_maxlen_bits);
__IO_REG32_BIT(CPSW_SL2_BOFFTEST,     0x4A100DD4,__READ_WRITE ,__cpsw_sl_bofftest_bits);
__IO_REG32_BIT(CPSW_SL2_RX_PAUSE,     0x4A100DD8,__READ       ,__cpsw_sl_rx_pause_bits);
__IO_REG32_BIT(CPSW_SL2_TX_PAUSE,     0x4A100DDC,__READ       ,__cpsw_sl_tx_pause_bits);
__IO_REG32_BIT(CPSW_SL2_EMCONTROL,    0x4A100DE0,__READ_WRITE ,__cpsw_sl_emcontrol_bits);
__IO_REG32_BIT(CPSW_SL2_RX_PRI_MAP,   0x4A100DE4,__READ_WRITE ,__cpsw_sl_rx_pri_map_bits);
__IO_REG32_BIT(CPSW_SL2_TX_GAP,       0x4A100DE8,__READ_WRITE ,__cpsw_sl_tx_gap_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_SS
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_SS_IDVER,         0x4A100000,__READ       ,__cpsw_ss_idver_bits);
__IO_REG32_BIT(CPSW_SS_CONTROL,       0x4A100004,__READ_WRITE ,__cpsw_ss_control_bits);
__IO_REG32_BIT(CPSW_SS_SOFT_RESET,    0x4A100008,__READ_WRITE ,__cpsw_ss_soft_reset_bits);
__IO_REG32_BIT(CPSW_SS_STAT_PORT_EN,  0x4A10000C,__READ_WRITE ,__cpsw_ss_stat_port_en_bits);
__IO_REG32_BIT(CPSW_SS_PTYPE,         0x4A100010,__READ_WRITE ,__cpsw_ss_ptype_bits);
__IO_REG32_BIT(CPSW_SS_SOFT_IDLE,     0x4A100014,__READ_WRITE ,__cpsw_ss_soft_idle_bits);
__IO_REG32_BIT(CPSW_SS_THRU_RATE,     0x4A100018,__READ_WRITE ,__cpsw_ss_thru_rate_bits);
__IO_REG32_BIT(CPSW_SS_GAP_THRESH,    0x4A10001C,__READ_WRITE ,__cpsw_ss_gap_thresh_bits);
__IO_REG32_BIT(CPSW_SS_TX_START_WDS,  0x4A100020,__READ_WRITE ,__cpsw_ss_tx_start_wds_bits);
__IO_REG32_BIT(CPSW_SS_FLOW_CONTROL,  0x4A100024,__READ_WRITE ,__cpsw_ss_flow_control_bits);
__IO_REG32_BIT(CPSW_SS_VLAN_LTYPE,    0x4A100028,__READ_WRITE ,__cpsw_ss_vlan_ltype_bits);
__IO_REG32_BIT(CPSW_SS_TS_LTYPE,      0x4A10002C,__READ_WRITE ,__cpsw_ss_ts_ltype_bits);
__IO_REG32_BIT(CPSW_SS_DLR_LTYPE,     0x4A100030,__READ_WRITE ,__cpsw_ss_dlr_ltype_bits);

/***************************************************************************
 **
 ** Ethernet CPSW_WR
 **
 ***************************************************************************/
__IO_REG32_BIT(CPSW_WR_IDVER,             0x4A101200,__READ       ,__cpsw_wr_idver_bits);
__IO_REG32_BIT(CPSW_WR_SOFT_RESET,        0x4A101204,__READ_WRITE ,__cpsw_wr_soft_reset_bits);
__IO_REG32_BIT(CPSW_WR_CONTROL,           0x4A101208,__READ_WRITE ,__cpsw_wr_control_bits);
__IO_REG32_BIT(CPSW_WR_INT_CONTROL,       0x4A10120C,__READ_WRITE ,__cpsw_wr_int_control_bits);
__IO_REG32_BIT(CPSW_WR_C0_RX_THRESH_EN,   0x4A101210,__READ_WRITE ,__cpsw_wr_c0_rx_thresh_en_bits);
__IO_REG32_BIT(CPSW_WR_C0_RX_EN,          0x4A101214,__READ_WRITE ,__cpsw_wr_c0_rx_en_bits);
__IO_REG32_BIT(CPSW_WR_C0_TX_EN,          0x4A101218,__READ_WRITE ,__cpsw_wr_c0_tx_en_bits);
__IO_REG32_BIT(CPSW_WR_C0_MISC_EN,        0x4A10121C,__READ_WRITE ,__cpsw_wr_c0_misc_en_bits);
__IO_REG32_BIT(CPSW_WR_C1_RX_THRESH_EN,   0x4A101220,__READ_WRITE ,__cpsw_wr_c1_rx_thresh_en_bits);
__IO_REG32_BIT(CPSW_WR_C1_RX_EN,          0x4A101224,__READ_WRITE ,__cpsw_wr_c1_rx_en_bits);
__IO_REG32_BIT(CPSW_WR_C1_TX_EN,          0x4A101228,__READ_WRITE ,__cpsw_wr_c1_tx_en_bits);
__IO_REG32_BIT(CPSW_WR_C1_MISC_EN,        0x4A10122C,__READ_WRITE ,__cpsw_wr_c1_misc_en_bits);
__IO_REG32_BIT(CPSW_WR_C2_RX_THRESH_EN,   0x4A101230,__READ_WRITE ,__cpsw_wr_c2_rx_thresh_en_bits);
__IO_REG32_BIT(CPSW_WR_C2_RX_EN,          0x4A101234,__READ_WRITE ,__cpsw_wr_c2_rx_en_bits);
__IO_REG32_BIT(CPSW_WR_C2_TX_EN,          0x4A101238,__READ_WRITE ,__cpsw_wr_c2_tx_en_bits);
__IO_REG32_BIT(CPSW_WR_C2_MISC_EN,        0x4A10123C,__READ_WRITE ,__cpsw_wr_c2_misc_en_bits);
__IO_REG32_BIT(CPSW_WR_C0_RX_THRESH_STAT, 0x4A101240,__READ       ,__cpsw_wr_c0_rx_thresh_stat_bits);
__IO_REG32_BIT(CPSW_WR_C0_RX_STAT,        0x4A101244,__READ       ,__cpsw_wr_c0_rx_stat_bits);
__IO_REG32_BIT(CPSW_WR_C0_TX_STAT,        0x4A101248,__READ       ,__cpsw_wr_c0_tx_stat_bits);
__IO_REG32_BIT(CPSW_WR_C0_MISC_STAT,      0x4A10124C,__READ       ,__cpsw_wr_c0_misc_stat_bits);
__IO_REG32_BIT(CPSW_WR_C1_RX_THRESH_STAT, 0x4A101250,__READ       ,__cpsw_wr_c1_rx_thresh_stat_bits);
__IO_REG32_BIT(CPSW_WR_C1_RX_STAT,        0x4A101254,__READ       ,__cpsw_wr_c1_rx_stat_bits);
__IO_REG32_BIT(CPSW_WR_C1_TX_STAT,        0x4A101258,__READ       ,__cpsw_wr_c1_tx_stat_bits);
__IO_REG32_BIT(CPSW_WR_C1_MISC_STAT,      0x4A10125C,__READ       ,__cpsw_wr_c1_misc_stat_bits);
__IO_REG32_BIT(CPSW_WR_C2_RX_THRESH_STAT, 0x4A101260,__READ       ,__cpsw_wr_c2_rx_thresh_stat_bits);
__IO_REG32_BIT(CPSW_WR_C2_RX_STAT,        0x4A101264,__READ       ,__cpsw_wr_c2_rx_stat_bits);
__IO_REG32_BIT(CPSW_WR_C2_TX_STAT,        0x4A101268,__READ       ,__cpsw_wr_c2_tx_stat_bits);
__IO_REG32_BIT(CPSW_WR_C2_MISC_STAT,      0x4A10126C,__READ       ,__cpsw_wr_c2_misc_stat_bits);
__IO_REG32_BIT(CPSW_WR_C0_RX_IMAX,        0x4A101270,__READ_WRITE ,__cpsw_wr_c0_rx_imax_bits);
__IO_REG32_BIT(CPSW_WR_C0_TX_IMAX,        0x4A101274,__READ_WRITE ,__cpsw_wr_c0_tx_imax_bits);
__IO_REG32_BIT(CPSW_WR_C1_RX_IMAX,        0x4A101278,__READ_WRITE ,__cpsw_wr_c1_rx_imax_bits);
__IO_REG32_BIT(CPSW_WR_C1_TX_IMAX,        0x4A10127C,__READ_WRITE ,__cpsw_wr_c1_tx_imax_bits);
__IO_REG32_BIT(CPSW_WR_C2_RX_IMAX,        0x4A101280,__READ_WRITE ,__cpsw_wr_c2_rx_imax_bits);
__IO_REG32_BIT(CPSW_WR_C2_TX_IMAX,        0x4A101284,__READ_WRITE ,__cpsw_wr_c2_tx_imax_bits);
__IO_REG32_BIT(CPSW_WR_RGMII_CTL,         0x4A101288,__READ       ,__cpsw_wr_rgmii_ctl_bits);

/***************************************************************************
 **
 ** Ethernet MDIO
 **
 ***************************************************************************/
__IO_REG32_BIT(MDIO_VER,              0x4A101000,__READ       ,__mdio_ver_bits);
__IO_REG32_BIT(MDIO_CONTROL,          0x4A101004,__READ_WRITE ,__mdio_control_bits);
__IO_REG32_BIT(MDIO_ALIVE,            0x4A101008,__READ_WRITE ,__mdio_alive_bits);
__IO_REG32_BIT(MDIO_LINK,             0x4A10100C,__READ       ,__mdio_link_bits);
__IO_REG32_BIT(MDIO_LINKINTRAW,       0x4A101010,__READ_WRITE ,__mdio_linkintraw_bits);
__IO_REG32_BIT(MDIO_LINKINTMASKED,    0x4A101014,__READ_WRITE ,__mdio_linkintmasked_bits);
__IO_REG32_BIT(MDIO_USERINTRAW,       0x4A101020,__READ_WRITE ,__mdio_userintraw_bits);
__IO_REG32_BIT(MDIO_USERINTMASKED,    0x4A101024,__READ_WRITE ,__mdio_userintmasked_bits);
__IO_REG32_BIT(MDIO_USERINTMASKSET,   0x4A101028,__READ_WRITE ,__mdio_userintmaskset_bits);
__IO_REG32_BIT(MDIO_USERINTMASKCLR,   0x4A10102C,__READ_WRITE ,__mdio_userintmaskclr_bits);
__IO_REG32_BIT(MDIO_USERACCESS0,      0x4A101080,__READ_WRITE ,__mdio_useraccess_bits);
__IO_REG32_BIT(MDIO_USERPHYSEL0,      0x4A101084,__READ_WRITE ,__mdio_userphysel_bits);
__IO_REG32_BIT(MDIO_USERACCESS1,      0x4A101088,__READ_WRITE ,__mdio_useraccess_bits);
__IO_REG32_BIT(MDIO_USERPHYSEL1,      0x4A10108C,__READ_WRITE ,__mdio_userphysel_bits);

/***************************************************************************
 **
 ** PWMSS Configuration Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(PWMSS_IDVER,           0x48300000,__READ       ,__pwmss_idver_bits);
__IO_REG32_BIT(PWMSS_SYSCONFIG,       0x48300004,__READ_WRITE ,__pwmss_sysconfig_bits);
__IO_REG32_BIT(PWMSS_CLKCONFIG,       0x48300008,__READ_WRITE ,__pwmss_clkconfig_bits);
__IO_REG32_BIT(PWMSS_CLKSTATUS,       0x4830000C,__READ       ,__pwmss_clkstatus_bits);

/***************************************************************************
 **
 ** ePWM0
 **
 ***************************************************************************/
__IO_REG16_BIT(EPWM0_TBCTL,           0x48300200,__READ_WRITE ,__ehrpwm_tbctl_bits);
__IO_REG16_BIT(EPWM0_TBSTS,           0x48300202,__READ_WRITE ,__ehrpwm_tbsts_bits);
__IO_REG16_BIT(EPWM0_TBPHSHR,         0x48300204,__READ_WRITE ,__ehrpwm_tbphshr_bits);
__IO_REG16(    EPWM0_TBPHS,           0x48300206,__READ_WRITE );
__IO_REG16(    EPWM0_TBCNT,           0x48300208,__READ_WRITE );
__IO_REG16(    EPWM0_TBPRD,           0x4830020A,__READ_WRITE );
__IO_REG16_BIT(EPWM0_CMPCTL,          0x4830020E,__READ_WRITE ,__ehrpwm_cmpctl_bits);
__IO_REG16_BIT(EPWM0_CMPAHR,          0x48300210,__READ_WRITE ,__ehrpwm_cmpahr_bits);
__IO_REG16(    EPWM0_CMPA,            0x48300212,__READ_WRITE );
__IO_REG16(    EPWM0_CMPB,            0x48300214,__READ_WRITE );
__IO_REG16_BIT(EPWM0_AQCTLA,          0x48300216,__READ_WRITE ,__ehrpwm_aqctla_bits);
__IO_REG16_BIT(EPWM0_AQCTLB,          0x48300218,__READ_WRITE ,__ehrpwm_aqctla_bits);
__IO_REG16_BIT(EPWM0_AQSFRC,          0x4830021A,__READ_WRITE ,__ehrpwm_aqsfrc_bits);
__IO_REG16_BIT(EPWM0_AQCSFRC,         0x4830021C,__READ_WRITE ,__ehrpwm_aqcsfrc_bits);
__IO_REG16_BIT(EPWM0_DBCTL,           0x4830021E,__READ_WRITE ,__ehrpwm_dbctl_bits);
__IO_REG16_BIT(EPWM0_DBRED,           0x48300220,__READ_WRITE ,__ehrpwm_dbred_bits);
__IO_REG16_BIT(EPWM0_DBFED,           0x48300222,__READ_WRITE ,__ehrpwm_dbred_bits);
__IO_REG16_BIT(EPWM0_TZSEL,           0x48300224,__READ_WRITE ,__ehrpwm_tzsel_bits);
__IO_REG16_BIT(EPWM0_TZCTL,           0x48300228,__READ_WRITE ,__ehrpwm_tzctl_bits);
__IO_REG16_BIT(EPWM0_TZEINT,          0x4830022A,__READ_WRITE ,__ehrpwm_tzeint_bits);
__IO_REG16_BIT(EPWM0_TZFLG,           0x4830022C,__READ       ,__ehrpwm_tzflg_bits);
__IO_REG16_BIT(EPWM0_TZCLR,           0x4830022E,__READ_WRITE ,__ehrpwm_tzflg_bits);
__IO_REG16_BIT(EPWM0_TZFRC,           0x48300230,__READ_WRITE ,__ehrpwm_tzeint_bits);
__IO_REG16_BIT(EPWM0_ETSEL,           0x48300232,__READ_WRITE ,__ehrpwm_etsel_bits);
__IO_REG16_BIT(EPWM0_ETPS,            0x48300234,__READ_WRITE ,__ehrpwm_etps_bits);
__IO_REG16_BIT(EPWM0_ETFLG,           0x48300236,__READ       ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM0_ETCLR,           0x48300238,__READ_WRITE ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM0_ETFRC,           0x4830023A,__READ_WRITE ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM0_PCCTL,           0x4830023C,__READ_WRITE ,__ehrpwm_pcctl_bits);
__IO_REG16_BIT(EPWM0_HRCTL,           0x48300240,__READ_WRITE ,__ehrpwm_hrctl_bits);

/***************************************************************************
 **
 ** ePWM1
 **
 ***************************************************************************/
__IO_REG16_BIT(EPWM1_TBCTL,           0x48302200,__READ_WRITE ,__ehrpwm_tbctl_bits);
__IO_REG16_BIT(EPWM1_TBSTS,           0x48302202,__READ_WRITE ,__ehrpwm_tbsts_bits);
__IO_REG16_BIT(EPWM1_TBPHSHR,         0x48302204,__READ_WRITE ,__ehrpwm_tbphshr_bits);
__IO_REG16(    EPWM1_TBPHS,           0x48302206,__READ_WRITE );
__IO_REG16(    EPWM1_TBCNT,           0x48302208,__READ_WRITE );
__IO_REG16(    EPWM1_TBPRD,           0x4830220A,__READ_WRITE );
__IO_REG16_BIT(EPWM1_CMPCTL,          0x4830220E,__READ_WRITE ,__ehrpwm_cmpctl_bits);
__IO_REG16_BIT(EPWM1_CMPAHR,          0x48302210,__READ_WRITE ,__ehrpwm_cmpahr_bits);
__IO_REG16(    EPWM1_CMPA,            0x48302212,__READ_WRITE );
__IO_REG16(    EPWM1_CMPB,            0x48302214,__READ_WRITE );
__IO_REG16_BIT(EPWM1_AQCTLA,          0x48302216,__READ_WRITE ,__ehrpwm_aqctla_bits);
__IO_REG16_BIT(EPWM1_AQCTLB,          0x48302218,__READ_WRITE ,__ehrpwm_aqctla_bits);
__IO_REG16_BIT(EPWM1_AQSFRC,          0x4830221A,__READ_WRITE ,__ehrpwm_aqsfrc_bits);
__IO_REG16_BIT(EPWM1_AQCSFRC,         0x4830221C,__READ_WRITE ,__ehrpwm_aqcsfrc_bits);
__IO_REG16_BIT(EPWM1_DBCTL,           0x4830221E,__READ_WRITE ,__ehrpwm_dbctl_bits);
__IO_REG16_BIT(EPWM1_DBRED,           0x48302220,__READ_WRITE ,__ehrpwm_dbred_bits);
__IO_REG16_BIT(EPWM1_DBFED,           0x48302222,__READ_WRITE ,__ehrpwm_dbred_bits);
__IO_REG16_BIT(EPWM1_TZSEL,           0x48302224,__READ_WRITE ,__ehrpwm_tzsel_bits);
__IO_REG16_BIT(EPWM1_TZCTL,           0x48302228,__READ_WRITE ,__ehrpwm_tzctl_bits);
__IO_REG16_BIT(EPWM1_TZEINT,          0x4830222A,__READ_WRITE ,__ehrpwm_tzeint_bits);
__IO_REG16_BIT(EPWM1_TZFLG,           0x4830222C,__READ       ,__ehrpwm_tzflg_bits);
__IO_REG16_BIT(EPWM1_TZCLR,           0x4830222E,__READ_WRITE ,__ehrpwm_tzflg_bits);
__IO_REG16_BIT(EPWM1_TZFRC,           0x48302230,__READ_WRITE ,__ehrpwm_tzeint_bits);
__IO_REG16_BIT(EPWM1_ETSEL,           0x48302232,__READ_WRITE ,__ehrpwm_etsel_bits);
__IO_REG16_BIT(EPWM1_ETPS,            0x48302234,__READ_WRITE ,__ehrpwm_etps_bits);
__IO_REG16_BIT(EPWM1_ETFLG,           0x48302236,__READ       ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM1_ETCLR,           0x48302238,__READ_WRITE ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM1_ETFRC,           0x4830223A,__READ_WRITE ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM1_PCCTL,           0x4830223C,__READ_WRITE ,__ehrpwm_pcctl_bits);
__IO_REG16_BIT(EPWM1_HRCTL,           0x48302240,__READ_WRITE ,__ehrpwm_hrctl_bits);

/***************************************************************************
 **
 ** ePWM2
 **
 ***************************************************************************/
__IO_REG16_BIT(EPWM2_TBCTL,           0x48304200,__READ_WRITE ,__ehrpwm_tbctl_bits);
__IO_REG16_BIT(EPWM2_TBSTS,           0x48304202,__READ_WRITE ,__ehrpwm_tbsts_bits);
__IO_REG16_BIT(EPWM2_TBPHSHR,         0x48304204,__READ_WRITE ,__ehrpwm_tbphshr_bits);
__IO_REG16(    EPWM2_TBPHS,           0x48304206,__READ_WRITE );
__IO_REG16(    EPWM2_TBCNT,           0x48304208,__READ_WRITE );
__IO_REG16(    EPWM2_TBPRD,           0x4830420A,__READ_WRITE );
__IO_REG16_BIT(EPWM2_CMPCTL,          0x4830420E,__READ_WRITE ,__ehrpwm_cmpctl_bits);
__IO_REG16_BIT(EPWM2_CMPAHR,          0x48304210,__READ_WRITE ,__ehrpwm_cmpahr_bits);
__IO_REG16(    EPWM2_CMPA,            0x48304212,__READ_WRITE );
__IO_REG16(    EPWM2_CMPB,            0x48304214,__READ_WRITE );
__IO_REG16_BIT(EPWM2_AQCTLA,          0x48304216,__READ_WRITE ,__ehrpwm_aqctla_bits);
__IO_REG16_BIT(EPWM2_AQCTLB,          0x48304218,__READ_WRITE ,__ehrpwm_aqctla_bits);
__IO_REG16_BIT(EPWM2_AQSFRC,          0x4830421A,__READ_WRITE ,__ehrpwm_aqsfrc_bits);
__IO_REG16_BIT(EPWM2_AQCSFRC,         0x4830421C,__READ_WRITE ,__ehrpwm_aqcsfrc_bits);
__IO_REG16_BIT(EPWM2_DBCTL,           0x4830421E,__READ_WRITE ,__ehrpwm_dbctl_bits);
__IO_REG16_BIT(EPWM2_DBRED,           0x48304220,__READ_WRITE ,__ehrpwm_dbred_bits);
__IO_REG16_BIT(EPWM2_DBFED,           0x48304222,__READ_WRITE ,__ehrpwm_dbred_bits);
__IO_REG16_BIT(EPWM2_TZSEL,           0x48304224,__READ_WRITE ,__ehrpwm_tzsel_bits);
__IO_REG16_BIT(EPWM2_TZCTL,           0x48304228,__READ_WRITE ,__ehrpwm_tzctl_bits);
__IO_REG16_BIT(EPWM2_TZEINT,          0x4830422A,__READ_WRITE ,__ehrpwm_tzeint_bits);
__IO_REG16_BIT(EPWM2_TZFLG,           0x4830422C,__READ       ,__ehrpwm_tzflg_bits);
__IO_REG16_BIT(EPWM2_TZCLR,           0x4830422E,__READ_WRITE ,__ehrpwm_tzflg_bits);
__IO_REG16_BIT(EPWM2_TZFRC,           0x48304230,__READ_WRITE ,__ehrpwm_tzeint_bits);
__IO_REG16_BIT(EPWM2_ETSEL,           0x48304232,__READ_WRITE ,__ehrpwm_etsel_bits);
__IO_REG16_BIT(EPWM2_ETPS,            0x48304234,__READ_WRITE ,__ehrpwm_etps_bits);
__IO_REG16_BIT(EPWM2_ETFLG,           0x48304236,__READ       ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM2_ETCLR,           0x48304238,__READ_WRITE ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM2_ETFRC,           0x4830423A,__READ_WRITE ,__ehrpwm_etflg_bits);
__IO_REG16_BIT(EPWM2_PCCTL,           0x4830423C,__READ_WRITE ,__ehrpwm_pcctl_bits);
__IO_REG16_BIT(EPWM2_HRCTL,           0x48304240,__READ_WRITE ,__ehrpwm_hrctl_bits);

/***************************************************************************
 **
 ** eCAP0 Registers
 **
 ***************************************************************************/
__IO_REG32(    ECAP0_TSCTR,           0x48300100,__READ_WRITE );
__IO_REG32(    ECAP0_CTRPHS,          0x48300104,__READ_WRITE );
__IO_REG32(    ECAP0_CAP1,            0x48300108,__READ_WRITE );
__IO_REG32(    ECAP0_CAP2,            0x4830010C,__READ_WRITE );
__IO_REG32(    ECAP0_CAP3,            0x48300110,__READ_WRITE );
__IO_REG32(    ECAP0_CAP4,            0x48300114,__READ_WRITE );
__IO_REG16_BIT(ECAP0_ECCTL1,          0x48300128,__READ_WRITE ,__ecap_ecctl1_bits);
__IO_REG16_BIT(ECAP0_ECCTL2,          0x4830012A,__READ_WRITE ,__ecap_ecctl2_bits);
__IO_REG16_BIT(ECAP0_ECEINT,          0x4830012C,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG16_BIT(ECAP0_ECFLG,           0x4830012E,__READ       ,__ecap_ecflg_bits);
__IO_REG16_BIT(ECAP0_ECCLR,           0x48300130,__READ_WRITE ,__ecap_ecflg_bits);
__IO_REG16_BIT(ECAP0_ECFRC,           0x48300132,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG32(    ECAP0_REVID,           0x4830015C,__READ       );

/***************************************************************************
 **
 ** eCAP1 Registers
 **
 ***************************************************************************/
__IO_REG32(    ECAP1_TSCTR,           0x48302100,__READ_WRITE );
__IO_REG32(    ECAP1_CTRPHS,          0x48302104,__READ_WRITE );
__IO_REG32(    ECAP1_CAP1,            0x48302108,__READ_WRITE );
__IO_REG32(    ECAP1_CAP2,            0x4830210C,__READ_WRITE );
__IO_REG32(    ECAP1_CAP3,            0x48302110,__READ_WRITE );
__IO_REG32(    ECAP1_CAP4,            0x48302114,__READ_WRITE );
__IO_REG16_BIT(ECAP1_ECCTL1,          0x48302128,__READ_WRITE ,__ecap_ecctl1_bits);
__IO_REG16_BIT(ECAP1_ECCTL2,          0x4830212A,__READ_WRITE ,__ecap_ecctl2_bits);
__IO_REG16_BIT(ECAP1_ECEINT,          0x4830212C,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG16_BIT(ECAP1_ECFLG,           0x4830212E,__READ       ,__ecap_ecflg_bits);
__IO_REG16_BIT(ECAP1_ECCLR,           0x48302130,__READ_WRITE ,__ecap_ecflg_bits);
__IO_REG16_BIT(ECAP1_ECFRC,           0x48302132,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG32(    ECAP1_REVID,           0x4830215C,__READ       );

/***************************************************************************
 **
 ** eCAP2 Registers
 **
 ***************************************************************************/
__IO_REG32(    ECAP2_TSCTR,           0x48304100,__READ_WRITE );
__IO_REG32(    ECAP2_CTRPHS,          0x48304104,__READ_WRITE );
__IO_REG32(    ECAP2_CAP1,            0x48304108,__READ_WRITE );
__IO_REG32(    ECAP2_CAP2,            0x4830410C,__READ_WRITE );
__IO_REG32(    ECAP2_CAP3,            0x48304110,__READ_WRITE );
__IO_REG32(    ECAP2_CAP4,            0x48304114,__READ_WRITE );
__IO_REG16_BIT(ECAP2_ECCTL1,          0x48304128,__READ_WRITE ,__ecap_ecctl1_bits);
__IO_REG16_BIT(ECAP2_ECCTL2,          0x4830412A,__READ_WRITE ,__ecap_ecctl2_bits);
__IO_REG16_BIT(ECAP2_ECEINT,          0x4830412C,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG16_BIT(ECAP2_ECFLG,           0x4830412E,__READ       ,__ecap_ecflg_bits);
__IO_REG16_BIT(ECAP2_ECCLR,           0x48304130,__READ_WRITE ,__ecap_ecflg_bits);
__IO_REG16_BIT(ECAP2_ECFRC,           0x48304132,__READ_WRITE ,__ecap_eceint_bits);
__IO_REG32(    ECAP2_REVID,           0x4830415C,__READ       );

/***************************************************************************
 **
 ** EQEP0
 **
 ***************************************************************************/
__IO_REG32(    EQEP0_QPOSCNT,         0x48300180,__READ_WRITE );
__IO_REG32(    EQEP0_QPOSINIT,        0x48300184,__READ_WRITE );
__IO_REG32(    EQEP0_QPOSMAX,         0x48300188,__READ_WRITE );
__IO_REG32(    EQEP0_QPOSCMP,         0x4830018C,__READ_WRITE );
__IO_REG32(    EQEP0_QPOSILAT,        0x48300190,__READ       );
__IO_REG32(    EQEP0_QPOSSLAT,        0x48300194,__READ       );
__IO_REG32(    EQEP0_QPOSLAT,         0x48300198,__READ       );
__IO_REG32(    EQEP0_QUTMR,           0x4830019C,__READ_WRITE );
__IO_REG32(    EQEP0_QUPRD,           0x483001A0,__READ_WRITE );
__IO_REG16(    EQEP0_QWDTMR,          0x483001A4,__READ_WRITE );
__IO_REG16(    EQEP0_QWDPRD,          0x483001A6,__READ_WRITE );
__IO_REG16_BIT(EQEP0_QDECCTL,         0x483001A8,__READ_WRITE ,__eqep_qdecctl_bits);
__IO_REG16_BIT(EQEP0_QEPCTL,          0x483001AA,__READ_WRITE ,__eqep_qepctl_bits);
__IO_REG16_BIT(EQEP0_QCAPCTL,         0x483001AC,__READ_WRITE ,__eqep_qcapctl_bits);
__IO_REG16_BIT(EQEP0_QPOSCTL,         0x483001AE,__READ_WRITE ,__eqep_qposctl_bits);
__IO_REG16_BIT(EQEP0_QEINT,           0x483001B0,__READ_WRITE ,__eqep_qeint_bits);
__IO_REG16_BIT(EQEP0_QFLG,            0x483001B2,__READ       ,__eqep_qflg_bits);
__IO_REG16_BIT(EQEP0_QCLR,            0x483001B4,__READ_WRITE ,__eqep_qflg_bits);
__IO_REG16_BIT(EQEP0_QFRC,            0x483001B6,__READ_WRITE ,__eqep_qeint_bits);
__IO_REG16_BIT(EQEP0_QEPSTS,          0x483001B8,__READ_WRITE ,__eqep_qepsts_bits);
__IO_REG16(    EQEP0_QCTMR,           0x483001BA,__READ_WRITE );
__IO_REG16(    EQEP0_QCPRD,           0x483001BC,__READ_WRITE );
__IO_REG16(    EQEP0_QCTMRLAT,        0x483001BE,__READ       );
__IO_REG16(    EQEP0_QCPRDLAT,        0x483001C0,__READ_WRITE );
__IO_REG16(    EQEP0_REVID,           0x483001DC,__READ       );

/***************************************************************************
 **
 ** EQEP1
 **
 ***************************************************************************/
__IO_REG32(    EQEP1_QPOSCNT,         0x48302180,__READ_WRITE );
__IO_REG32(    EQEP1_QPOSINIT,        0x48302184,__READ_WRITE );
__IO_REG32(    EQEP1_QPOSMAX,         0x48302188,__READ_WRITE );
__IO_REG32(    EQEP1_QPOSCMP,         0x4830218C,__READ_WRITE );
__IO_REG32(    EQEP1_QPOSILAT,        0x48302190,__READ       );
__IO_REG32(    EQEP1_QPOSSLAT,        0x48302194,__READ       );
__IO_REG32(    EQEP1_QPOSLAT,         0x48302198,__READ       );
__IO_REG32(    EQEP1_QUTMR,           0x4830219C,__READ_WRITE );
__IO_REG32(    EQEP1_QUPRD,           0x483021A0,__READ_WRITE );
__IO_REG16(    EQEP1_QWDTMR,          0x483021A4,__READ_WRITE );
__IO_REG16(    EQEP1_QWDPRD,          0x483021A6,__READ_WRITE );
__IO_REG16_BIT(EQEP1_QDECCTL,         0x483021A8,__READ_WRITE ,__eqep_qdecctl_bits);
__IO_REG16_BIT(EQEP1_QEPCTL,          0x483021AA,__READ_WRITE ,__eqep_qepctl_bits);
__IO_REG16_BIT(EQEP1_QCAPCTL,         0x483021AC,__READ_WRITE ,__eqep_qcapctl_bits);
__IO_REG16_BIT(EQEP1_QPOSCTL,         0x483021AE,__READ_WRITE ,__eqep_qposctl_bits);
__IO_REG16_BIT(EQEP1_QEINT,           0x483021B0,__READ_WRITE ,__eqep_qeint_bits);
__IO_REG16_BIT(EQEP1_QFLG,            0x483021B2,__READ       ,__eqep_qflg_bits);
__IO_REG16_BIT(EQEP1_QCLR,            0x483021B4,__READ_WRITE ,__eqep_qflg_bits);
__IO_REG16_BIT(EQEP1_QFRC,            0x483021B6,__READ_WRITE ,__eqep_qeint_bits);
__IO_REG16_BIT(EQEP1_QEPSTS,          0x483021B8,__READ_WRITE ,__eqep_qepsts_bits);
__IO_REG16(    EQEP1_QCTMR,           0x483021BA,__READ_WRITE );
__IO_REG16(    EQEP1_QCPRD,           0x483021BC,__READ_WRITE );
__IO_REG16(    EQEP1_QCTMRLAT,        0x483021BE,__READ       );
__IO_REG16(    EQEP1_QCPRDLAT,        0x483021C0,__READ_WRITE );
__IO_REG16(    EQEP1_REVID,           0x483021DC,__READ       );

/***************************************************************************
 **
 ** EQEP2
 **
 ***************************************************************************/
__IO_REG32(    EQEP2_QPOSCNT,         0x48304180,__READ_WRITE );
__IO_REG32(    EQEP2_QPOSINIT,        0x48304184,__READ_WRITE );
__IO_REG32(    EQEP2_QPOSMAX,         0x48304188,__READ_WRITE );
__IO_REG32(    EQEP2_QPOSCMP,         0x4830418C,__READ_WRITE );
__IO_REG32(    EQEP2_QPOSILAT,        0x48304190,__READ       );
__IO_REG32(    EQEP2_QPOSSLAT,        0x48304194,__READ       );
__IO_REG32(    EQEP2_QPOSLAT,         0x48304198,__READ       );
__IO_REG32(    EQEP2_QUTMR,           0x4830419C,__READ_WRITE );
__IO_REG32(    EQEP2_QUPRD,           0x483041A0,__READ_WRITE );
__IO_REG16(    EQEP2_QWDTMR,          0x483041A4,__READ_WRITE );
__IO_REG16(    EQEP2_QWDPRD,          0x483041A6,__READ_WRITE );
__IO_REG16_BIT(EQEP2_QDECCTL,         0x483041A8,__READ_WRITE ,__eqep_qdecctl_bits);
__IO_REG16_BIT(EQEP2_QEPCTL,          0x483041AA,__READ_WRITE ,__eqep_qepctl_bits);
__IO_REG16_BIT(EQEP2_QCAPCTL,         0x483041AC,__READ_WRITE ,__eqep_qcapctl_bits);
__IO_REG16_BIT(EQEP2_QPOSCTL,         0x483041AE,__READ_WRITE ,__eqep_qposctl_bits);
__IO_REG16_BIT(EQEP2_QEINT,           0x483041B0,__READ_WRITE ,__eqep_qeint_bits);
__IO_REG16_BIT(EQEP2_QFLG,            0x483041B2,__READ       ,__eqep_qflg_bits);
__IO_REG16_BIT(EQEP2_QCLR,            0x483041B4,__READ_WRITE ,__eqep_qflg_bits);
__IO_REG16_BIT(EQEP2_QFRC,            0x483041B6,__READ_WRITE ,__eqep_qeint_bits);
__IO_REG16_BIT(EQEP2_QEPSTS,          0x483041B8,__READ_WRITE ,__eqep_qepsts_bits);
__IO_REG16(    EQEP2_QCTMR,           0x483041BA,__READ_WRITE );
__IO_REG16(    EQEP2_QCPRD,           0x483041BC,__READ_WRITE );
__IO_REG16(    EQEP2_QCTMRLAT,        0x483041BE,__READ       );
__IO_REG16(    EQEP2_QCPRDLAT,        0x483041C0,__READ_WRITE );
__IO_REG16(    EQEP2_REVID,           0x483041DC,__READ       );

/***************************************************************************
 **
 ** USBSS Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(USBSS_REVREG,            0x47400000,__READ       ,__usbss_revreg_bits);
__IO_REG32_BIT(USBSS_SYSCONFIG,         0x47400010,__READ_WRITE ,__usbss_sysconfig_bits);
__IO_REG32_BIT(USBSS_EOI,               0x47400020,__READ_WRITE ,__usbss_eoi_bits);
__IO_REG32_BIT(USBSS_IRQSTATRAW,        0x47400024,__READ_WRITE ,__usbss_irq_bits);
__IO_REG32_BIT(USBSS_IRQSTAT,           0x47400028,__READ_WRITE ,__usbss_irq_bits);
__IO_REG32_BIT(USBSS_IRQENABLER,        0x4740002C,__READ_WRITE ,__usbss_irq_bits);
__IO_REG32_BIT(USBSS_IRQCLEARR,         0x47400030,__READ_WRITE ,__usbss_irq_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX00,   0x47400100,__READ_WRITE ,__usbss_irqdmatholdtx00_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX01,   0x47400104,__READ_WRITE ,__usbss_irqdmatholdtx01_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX02,   0x47400108,__READ_WRITE ,__usbss_irqdmatholdtx02_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX03,   0x4740010C,__READ_WRITE ,__usbss_irqdmatholdtx03_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX00,   0x47400110,__READ_WRITE ,__usbss_irqdmatholdrx00_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX01,   0x47400114,__READ_WRITE ,__usbss_irqdmatholdrx01_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX02,   0x47400118,__READ_WRITE ,__usbss_irqdmatholdrx02_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX03,   0x4740011C,__READ_WRITE ,__usbss_irqdmatholdrx03_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX10,   0x47400120,__READ_WRITE ,__usbss_irqdmatholdtx10_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX11,   0x47400124,__READ_WRITE ,__usbss_irqdmatholdtx11_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX12,   0x47400128,__READ_WRITE ,__usbss_irqdmatholdtx12_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDTX13,   0x4740012C,__READ_WRITE ,__usbss_irqdmatholdtx13_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX10,   0x47400130,__READ_WRITE ,__usbss_irqdmatholdrx10_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX11,   0x47400134,__READ_WRITE ,__usbss_irqdmatholdrx11_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX12,   0x47400138,__READ_WRITE ,__usbss_irqdmatholdrx12_bits);
__IO_REG32_BIT(USBSS_IRQDMATHOLDRX13,   0x4740013C,__READ_WRITE ,__usbss_irqdmatholdrx13_bits);
__IO_REG32_BIT(USBSS_IRQDMAENABLE0,     0x47400140,__READ_WRITE ,__usbss_irqdmaenable0_bits);
__IO_REG32_BIT(USBSS_IRQDMAENABLE1,     0x47400144,__READ_WRITE ,__usbss_irqdmaenable1_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX00, 0x47400200,__READ_WRITE ,__usbss_irqframetholdtx00_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX01, 0x47400204,__READ_WRITE ,__usbss_irqframetholdtx01_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX02, 0x47400208,__READ_WRITE ,__usbss_irqframetholdtx02_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX03, 0x4740020C,__READ_WRITE ,__usbss_irqframetholdtx03_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX00, 0x47400210,__READ_WRITE ,__usbss_irqframetholdrx00_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX01, 0x47400214,__READ_WRITE ,__usbss_irqframetholdrx01_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX02, 0x47400218,__READ_WRITE ,__usbss_irqframetholdrx02_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX03, 0x4740021C,__READ_WRITE ,__usbss_irqframetholdrx03_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX10, 0x47400220,__READ_WRITE ,__usbss_irqframetholdtx10_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX11, 0x47400224,__READ_WRITE ,__usbss_irqframetholdtx11_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX12, 0x47400228,__READ_WRITE ,__usbss_irqframetholdtx12_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDTX13, 0x4740022C,__READ_WRITE ,__usbss_irqframetholdtx13_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX10, 0x47400230,__READ_WRITE ,__usbss_irqframetholdrx10_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX11, 0x47400234,__READ_WRITE ,__usbss_irqframetholdrx11_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX12, 0x47400238,__READ_WRITE ,__usbss_irqframetholdrx12_bits);
__IO_REG32_BIT(USBSS_IRQFRAMETHOLDRX13, 0x4740023C,__READ_WRITE ,__usbss_irqframetholdrx13_bits);
__IO_REG32_BIT(USBSS_IRQFRAMEENABLE0,   0x47400240,__READ_WRITE ,__usbss_irqframeenable0_bits);
__IO_REG32_BIT(USBSS_IRQFRAMEENABLE1,   0x47400244,__READ_WRITE ,__usbss_irqframeenable1_bits);

/***************************************************************************
 **
 ** USB0_CTRL Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(USB0REV,                 0x47401000,__READ       ,__usbrev_bits);
__IO_REG32_BIT(USB0CTRL,                0x47401014,__READ_WRITE ,__usbctrl_bits);
__IO_REG32_BIT(USB0STAT,                0x47401018,__READ       ,__usbstat_bits);
__IO_REG32_BIT(USB0IRQMSTAT,            0x47401020,__READ       ,__usbirqmstat_bits);
__IO_REG32_BIT(USB0IRQEOI,              0x47401024,__READ_WRITE ,__usbirqeoi_bits);
__IO_REG32_BIT(USB0IRQSTATRAW0,         0x47401028,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB0IRQSTATRAW1,         0x4740102C,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB0IRQSTAT0,            0x47401030,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB0IRQSTAT1,            0x47401034,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB0IRQENABLESET0,       0x47401038,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB0IRQENABLESET1,       0x4740103C,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB0IRQENABLECLR0,       0x47401040,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB0IRQENABLECLR1,       0x47401044,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB0TXMODE,              0x47401070,__READ_WRITE ,__usbtxmode_bits);
__IO_REG32_BIT(USB0RXMODE,              0x47401074,__READ_WRITE ,__usbrxmode_bits);
__IO_REG32_BIT(USB0GENRNDISEP1,         0x47401080,__READ_WRITE ,__usbgenrndisep1_bits);
__IO_REG32_BIT(USB0GENRNDISEP2,         0x47401084,__READ_WRITE ,__usbgenrndisep2_bits);
__IO_REG32_BIT(USB0GENRNDISEP3,         0x47401088,__READ_WRITE ,__usbgenrndisep3_bits);
__IO_REG32_BIT(USB0GENRNDISEP4,         0x4740108C,__READ_WRITE ,__usbgenrndisep4_bits);
__IO_REG32_BIT(USB0GENRNDISEP5,         0x47401090,__READ_WRITE ,__usbgenrndisep5_bits);
__IO_REG32_BIT(USB0GENRNDISEP6,         0x47401094,__READ_WRITE ,__usbgenrndisep6_bits);
__IO_REG32_BIT(USB0GENRNDISEP7,         0x47401098,__READ_WRITE ,__usbgenrndisep7_bits);
__IO_REG32_BIT(USB0GENRNDISEP8,         0x4740109C,__READ_WRITE ,__usbgenrndisep8_bits);
__IO_REG32_BIT(USB0GENRNDISEP9,         0x474010A0,__READ_WRITE ,__usbgenrndisep9_bits);
__IO_REG32_BIT(USB0GENRNDISEP10,        0x474010A4,__READ_WRITE ,__usbgenrndisep10_bits);
__IO_REG32_BIT(USB0GENRNDISEP11,        0x474010A8,__READ_WRITE ,__usbgenrndisep11_bits);
__IO_REG32_BIT(USB0GENRNDISEP12,        0x474010AC,__READ_WRITE ,__usbgenrndisep12_bits);
__IO_REG32_BIT(USB0GENRNDISEP13,        0x474010B0,__READ_WRITE ,__usbgenrndisep13_bits);
__IO_REG32_BIT(USB0GENRNDISEP14,        0x474010B4,__READ_WRITE ,__usbgenrndisep14_bits);
__IO_REG32_BIT(USB0GENRNDISEP15,        0x474010B8,__READ_WRITE ,__usbgenrndisep15_bits);
__IO_REG32_BIT(USB0AUTOREQ,             0x474010D0,__READ_WRITE ,__usbautoreq_bits);
__IO_REG32(    USB0SRPFIXTIME,          0x474010D4,__READ_WRITE );
__IO_REG32_BIT(USB0TDOWN,               0x474010D8,__READ_WRITE ,__usbtdown_bits);
__IO_REG32_BIT(USB0UTMI,                0x474010E0,__READ_WRITE ,__usbutmi_bits);
__IO_REG32_BIT(USB0UTMILB,              0x474010E4,__READ_WRITE ,__usbutmilb_bits);
__IO_REG32_BIT(USB0MODE,                0x474010E8,__READ_WRITE ,__usbmode_bits);

/***************************************************************************
 **
 ** USB1_CTRL Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(USB1REV,                 0x47401800,__READ       ,__usbrev_bits);
__IO_REG32_BIT(USB1CTRL,                0x47401814,__READ_WRITE ,__usbctrl_bits);
__IO_REG32_BIT(USB1STAT,                0x47401818,__READ       ,__usbstat_bits);
__IO_REG32_BIT(USB1IRQMSTAT,            0x47401820,__READ       ,__usbirqmstat_bits);
__IO_REG32_BIT(USB1IRQEOI,              0x47401824,__READ_WRITE ,__usbirqeoi_bits);
__IO_REG32_BIT(USB1IRQSTATRAW0,         0x47401828,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB1IRQSTATRAW1,         0x4740182C,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB1IRQSTAT0,            0x47401830,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB1IRQSTAT1,            0x47401834,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB1IRQENABLESET0,       0x47401838,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB1IRQENABLESET1,       0x4740183C,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB1IRQENABLECLR0,       0x47401840,__READ_WRITE ,__usbirq0_bits);
__IO_REG32_BIT(USB1IRQENABLECLR1,       0x47401844,__READ_WRITE ,__usbirq1_bits);
__IO_REG32_BIT(USB1TXMODE,              0x47401870,__READ_WRITE ,__usbtxmode_bits);
__IO_REG32_BIT(USB1RXMODE,              0x47401874,__READ_WRITE ,__usbrxmode_bits);
__IO_REG32_BIT(USB1GENRNDISEP1,         0x47401880,__READ_WRITE ,__usbgenrndisep1_bits);
__IO_REG32_BIT(USB1GENRNDISEP2,         0x47401884,__READ_WRITE ,__usbgenrndisep2_bits);
__IO_REG32_BIT(USB1GENRNDISEP3,         0x47401888,__READ_WRITE ,__usbgenrndisep3_bits);
__IO_REG32_BIT(USB1GENRNDISEP4,         0x4740188C,__READ_WRITE ,__usbgenrndisep4_bits);
__IO_REG32_BIT(USB1GENRNDISEP5,         0x47401890,__READ_WRITE ,__usbgenrndisep5_bits);
__IO_REG32_BIT(USB1GENRNDISEP6,         0x47401894,__READ_WRITE ,__usbgenrndisep6_bits);
__IO_REG32_BIT(USB1GENRNDISEP7,         0x47401898,__READ_WRITE ,__usbgenrndisep7_bits);
__IO_REG32_BIT(USB1GENRNDISEP8,         0x4740189C,__READ_WRITE ,__usbgenrndisep8_bits);
__IO_REG32_BIT(USB1GENRNDISEP9,         0x474018A0,__READ_WRITE ,__usbgenrndisep9_bits);
__IO_REG32_BIT(USB1GENRNDISEP10,        0x474018A4,__READ_WRITE ,__usbgenrndisep10_bits);
__IO_REG32_BIT(USB1GENRNDISEP11,        0x474018A8,__READ_WRITE ,__usbgenrndisep11_bits);
__IO_REG32_BIT(USB1GENRNDISEP12,        0x474018AC,__READ_WRITE ,__usbgenrndisep12_bits);
__IO_REG32_BIT(USB1GENRNDISEP13,        0x474018B0,__READ_WRITE ,__usbgenrndisep13_bits);
__IO_REG32_BIT(USB1GENRNDISEP14,        0x474018B4,__READ_WRITE ,__usbgenrndisep14_bits);
__IO_REG32_BIT(USB1GENRNDISEP15,        0x474018B8,__READ_WRITE ,__usbgenrndisep15_bits);
__IO_REG32_BIT(USB1AUTOREQ,             0x474018D0,__READ_WRITE ,__usbautoreq_bits);
__IO_REG32(    USB1SRPFIXTIME,          0x474018D4,__READ_WRITE );
__IO_REG32_BIT(USB1TDOWN,               0x474018D8,__READ_WRITE ,__usbtdown_bits);
__IO_REG32_BIT(USB1UTMI,                0x474018E0,__READ_WRITE ,__usbutmi_bits);
__IO_REG32_BIT(USB1UTMILB,              0x474018E4,__READ_WRITE ,__usbutmilb_bits);
__IO_REG32_BIT(USB1MODE,                0x474018E8,__READ_WRITE ,__usbmode_bits);

/***************************************************************************
 **
 ** USB0_PHY Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(USB0_PHY_TERM_CTRL,              0x47401300,__READ_WRITE ,__usb_phy_term_ctrl_bits);
__IO_REG32_BIT(USB0_PHY_RX_CALIB,               0x47401304,__READ_WRITE ,__usb_phy_rx_calib_bits);
__IO_REG32_BIT(USB0_PHY_DLLHS_2,                0x47401308,__READ_WRITE ,__usb_phy_dllhs_2_bits);
__IO_REG32_BIT(USB0_PHY_RX_TEST_2,              0x4740130C,__READ_WRITE ,__usb_phy_rx_test_2_bits);
__IO_REG32_BIT(USB0_PHY_CHRG_DET,               0x47401314,__READ_WRITE ,__usb_phy_chrg_det_bits);
__IO_REG32_BIT(USB0_PHY_PWR_CNTL,               0x47401318,__READ_WRITE ,__usb_phy_pwr_cntl_bits);
__IO_REG32_BIT(USB0_PHY_UTMI_INTERFACE_CNTL_1,  0x4740131C,__READ_WRITE ,__usb_phy_utmi_interface_cntl_1_bits);
__IO_REG32_BIT(USB0_PHY_UTMI_INTERFACE_CNTL_2,  0x47401320,__READ_WRITE ,__usb_phy_utmi_interface_cntl_2_bits);
__IO_REG32_BIT(USB0_PHY_BIST,                   0x47401324,__READ_WRITE ,__usb_phy_bist_bits);
__IO_REG32(    USB0_PHY_BIST_CRC,               0x47401328,__READ_WRITE );
__IO_REG32_BIT(USB0_PHY_CDR_BIST2,              0x4740132C,__READ_WRITE ,__usb_phy_cdr_bist2_bits);
__IO_REG32_BIT(USB0_PHY_GPIO,                   0x47401330,__READ_WRITE ,__usb_phy_gpio_bits);
__IO_REG32_BIT(USB0_PHY_DLLHS,                  0x47401334,__READ_WRITE ,__usb_phy_dllhs_bits);
__IO_REG32_BIT(USB0_PHY_CM_TRIM,                0x47401338,__READ_WRITE ,__usb_phy_cm_trim_bits);
__IO_REG32_BIT(USB0_PHY_CM_CONFIG,              0x4740133C,__READ_WRITE ,__usb_phy_cm_config_bits);
__IO_REG32_BIT(USB0_PHY_USBOTG,                 0x47401340,__READ_WRITE ,__usb_phy_usbotg_bits);
__IO_REG32_BIT(USB0_PHY_AD_INTERFACE_REG1,      0x47401344,__READ_WRITE ,__usb_phy_ad_interface_reg1_bits);
__IO_REG32_BIT(USB0_PHY_AD_INTERFACE_REG2,      0x47401348,__READ_WRITE ,__usb_phy_ad_interface_reg2_bits);
__IO_REG32_BIT(USB0_PHY_AD_INTERFACE_REG3,      0x4740134C,__READ_WRITE ,__usb_phy_ad_interface_reg3_bits);
__IO_REG32_BIT(USB0_PHY_ANA_CONFIG1,            0x47401350,__READ_WRITE ,__usb_phy_ana_config1_bits);
__IO_REG32_BIT(USB0_PHY_ANA_CONFIG2,            0x47401354,__READ_WRITE ,__usb_phy_ana_config2_bits);

/***************************************************************************
 **
 ** USB1_PHY Registers
 **
 ***************************************************************************/
__IO_REG32_BIT(USB1_PHY_TERM_CTRL,              0x47401B00,__READ_WRITE ,__usb_phy_term_ctrl_bits);
__IO_REG32_BIT(USB1_PHY_RX_CALIB,               0x47401B04,__READ_WRITE ,__usb_phy_rx_calib_bits);
__IO_REG32_BIT(USB1_PHY_DLLHS_2,                0x47401B08,__READ_WRITE ,__usb_phy_dllhs_2_bits);
__IO_REG32_BIT(USB1_PHY_RX_TEST_2,              0x47401B0C,__READ_WRITE ,__usb_phy_rx_test_2_bits);
__IO_REG32_BIT(USB1_PHY_CHRG_DET,               0x47401B14,__READ_WRITE ,__usb_phy_chrg_det_bits);
__IO_REG32_BIT(USB1_PHY_PWR_CNTL,               0x47401B18,__READ_WRITE ,__usb_phy_pwr_cntl_bits);
__IO_REG32_BIT(USB1_PHY_UTMI_INTERFACE_CNTL_1,  0x47401B1C,__READ_WRITE ,__usb_phy_utmi_interface_cntl_1_bits);
__IO_REG32_BIT(USB1_PHY_UTMI_INTERFACE_CNTL_2,  0x47401B20,__READ_WRITE ,__usb_phy_utmi_interface_cntl_2_bits);
__IO_REG32_BIT(USB1_PHY_BIST,                   0x47401B24,__READ_WRITE ,__usb_phy_bist_bits);
__IO_REG32(    USB1_PHY_BIST_CRC,               0x47401B28,__READ_WRITE );
__IO_REG32_BIT(USB1_PHY_CDR_BIST2,              0x47401B2C,__READ_WRITE ,__usb_phy_cdr_bist2_bits);
__IO_REG32_BIT(USB1_PHY_GPIO,                   0x47401B30,__READ_WRITE ,__usb_phy_gpio_bits);
__IO_REG32_BIT(USB1_PHY_DLLHS,                  0x47401B34,__READ_WRITE ,__usb_phy_dllhs_bits);
__IO_REG32_BIT(USB1_PHY_CM_TRIM,                0x47401B38,__READ_WRITE ,__usb_phy_cm_trim_bits);
__IO_REG32_BIT(USB1_PHY_CM_CONFIG,              0x47401B3C,__READ_WRITE ,__usb_phy_cm_config_bits);
__IO_REG32_BIT(USB1_PHY_USBOTG,                 0x47401B40,__READ_WRITE ,__usb_phy_usbotg_bits);
__IO_REG32_BIT(USB1_PHY_AD_INTERFACE_REG1,      0x47401B44,__READ_WRITE ,__usb_phy_ad_interface_reg1_bits);
__IO_REG32_BIT(USB1_PHY_AD_INTERFACE_REG2,      0x47401B48,__READ_WRITE ,__usb_phy_ad_interface_reg2_bits);
__IO_REG32_BIT(USB1_PHY_AD_INTERFACE_REG3,      0x47401B4C,__READ_WRITE ,__usb_phy_ad_interface_reg3_bits);
__IO_REG32_BIT(USB1_PHY_ANA_CONFIG1,            0x47401B50,__READ_WRITE ,__usb_phy_ana_config1_bits);
__IO_REG32_BIT(USB1_PHY_ANA_CONFIG2,            0x47401B54,__READ_WRITE ,__usb_phy_ana_config2_bits);

/***************************************************************************
 **
 ** USB CPPI DMA Controller
 **
 ***************************************************************************/
__IO_REG32_BIT(CPPI_DMA_DMAREVID,     0x47402000,__READ       ,__cppi_dma_dmarevid_bits);
__IO_REG32_BIT(CPPI_DMA_TDFDQ,        0x47402004,__READ_WRITE ,__cppi_dma_tdfdq_bits);
__IO_REG32_BIT(CPPI_DMA_DMAEMU,       0x47402008,__READ_WRITE ,__cppi_dma_dmaemu_bits);
__IO_REG32_BIT(CPPI_DMA_TXGCR0,       0x47402800,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR0,       0x47402808,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA0,     0x4740280C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB0,     0x47402810,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR1,       0x47402820,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR1,       0x47402828,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA1,     0x4740282C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB1,     0x47402830,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR2,       0x47402840,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR2,       0x47402848,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA2,     0x4740284C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB2,     0x47402850,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR3,       0x47402860,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR3,       0x47402868,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA3,     0x4740286C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB3,     0x47402870,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR4,       0x47402880,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR4,       0x47402888,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA4,     0x4740288C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB4,     0x47402890,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR5,       0x474028A0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR5,       0x474028A8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA5,     0x474028AC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB5,     0x474028B0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR6,       0x474028C0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR6,       0x474028C8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA6,     0x474028CC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB6,     0x474028D0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR7,       0x474028E0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR7,       0x474028E8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA7,     0x474028EC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB7,     0x474028F0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR8,       0x47402900,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR8,       0x47402908,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA8,     0x4740290C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB8,     0x47402910,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR9,       0x47402920,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR9,       0x47402928,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA9,     0x4740292C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB9,     0x47402930,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR10,      0x47402940,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR10,      0x47402948,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA10,    0x4740294C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB10,    0x47402950,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR11,      0x47402960,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR11,      0x47402968,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA11,    0x4740296C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB11,    0x47402970,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR12,      0x47402980,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR12,      0x47402988,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA12,    0x4740298C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB12,    0x47402990,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR13,      0x474029A0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR13,      0x474029A8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA13,    0x474029AC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB13,    0x474029B0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR14,      0x474029C0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR14,      0x474029C8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA14,    0x474029CC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB14,    0x474029D0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR15,      0x474029E0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR15,      0x474029E8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA15,    0x474029EC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB15,    0x474029F0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR16,      0x47402A00,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR16,      0x47402A08,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA16,    0x47402A0C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB16,    0x47402A10,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR17,      0x47402A20,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR17,      0x47402A28,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA17,    0x47402A2C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB17,    0x47402A30,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR18,      0x47402A40,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR18,      0x47402A48,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA18,    0x47402A4C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB18,    0x47402A50,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR19,      0x47402A60,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR19,      0x47402A68,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA19,    0x47402A6C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB19,    0x47402A70,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR20,      0x47402A80,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR20,      0x47402A88,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA20,    0x47402A8C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB20,    0x47402A90,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR21,      0x47402AA0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR21,      0x47402AA8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA21,    0x47402AAC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB21,    0x47402AB0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR22,      0x47402AC0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR22,      0x47402AC8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA22,    0x47402ACC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB22,    0x47402AD0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR23,      0x47402AE0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR23,      0x47402AE8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA23,    0x47402AEC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB23,    0x47402AF0,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR24,      0x47402B00,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR24,      0x47402B08,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA24,    0x47402B0C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB24,    0x47402B10,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR25,      0x47402B20,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR25,      0x47402B28,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA25,    0x47402B2C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB25,    0x47402B30,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR26,      0x47402B40,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR26,      0x47402B48,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA26,    0x47402B4C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB26,    0x47402B50,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR27,      0x47402B60,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR27,      0x47402B68,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA27,    0x47402B6C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB27,    0x47402B70,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR28,      0x47402B80,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR28,      0x47402B88,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA28,    0x47402B8C,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB28,    0x47402B90,__WRITE      );
__IO_REG32_BIT(CPPI_DMA_TXGCR29,      0x47402BA0,__READ_WRITE ,__cppi_dma_txgcr_bits);
__IO_REG32_BIT(CPPI_DMA_RXGCR29,      0x47402BA8,__READ_WRITE ,__cppi_dma_rxgcr_bits);
__IO_REG32(    CPPI_DMA_RXHPCRA29,    0x47402BAC,__WRITE      );
__IO_REG32(    CPPI_DMA_RXHPCRB29,    0x47402BB0,__WRITE      );

/***************************************************************************
 **
 ** USB CPPI DMA SCHEDULER
 **
 ***************************************************************************/
__IO_REG32_BIT(CPPI_DMA_SCHED_CTRL,   0x47403000,__READ_WRITE ,__cppi_dma_sched_ctrl_bits);
__IO_REG32(    CPPI_DMA_SCHED_WORD0,  0x47403800,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD1,  0x47403804,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD2,  0x47403808,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD3,  0x4740380C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD4,  0x47403810,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD5,  0x47403814,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD6,  0x47403818,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD7,  0x4740381C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD8,  0x47403820,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD9,  0x47403824,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD10, 0x47403828,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD11, 0x4740382C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD12, 0x47403830,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD13, 0x47403834,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD14, 0x47403838,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD15, 0x4740383C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD16, 0x47403840,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD17, 0x47403844,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD18, 0x47403848,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD19, 0x4740384C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD20, 0x47403850,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD21, 0x47403854,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD22, 0x47403858,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD23, 0x4740385C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD24, 0x47403860,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD25, 0x47403864,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD26, 0x47403868,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD27, 0x4740386C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD28, 0x47403870,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD29, 0x47403874,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD30, 0x47403878,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD31, 0x4740387C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD32, 0x47403880,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD33, 0x47403884,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD34, 0x47403888,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD35, 0x4740388C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD36, 0x47403890,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD37, 0x47403894,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD38, 0x47403898,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD39, 0x4740389C,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD40, 0x474038A0,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD41, 0x474038A4,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD42, 0x474038A8,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD43, 0x474038AC,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD44, 0x474038B0,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD45, 0x474038B4,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD46, 0x474038B8,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD47, 0x474038BC,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD48, 0x474038C0,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD49, 0x474038C4,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD50, 0x474038C8,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD51, 0x474038CC,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD52, 0x474038D0,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD53, 0x474038D4,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD54, 0x474038D8,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD55, 0x474038DC,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD56, 0x474038E0,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD57, 0x474038E4,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD58, 0x474038E8,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD59, 0x474038EC,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD60, 0x474038F0,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD61, 0x474038F4,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD62, 0x474038F8,__WRITE      );
__IO_REG32(    CPPI_DMA_SCHED_WORD63, 0x474038FC,__WRITE      );

/***************************************************************************
 **
 ** USB QUEUE MGR Control
 **
 ***************************************************************************/
__IO_REG32_BIT(USB_QMGR_REVID,        0x47404000,__READ_WRITE ,__usb_qmgr_revid_bits);
__IO_REG32(    USB_QMGR_RST,          0x47404008,__WRITE      );
__IO_REG32_BIT(USB_QMGR_FDBSC0,       0x47404020,__READ       ,__usb_qmgr_fdbsc0_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC1,       0x47404024,__READ       ,__usb_qmgr_fdbsc1_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC2,       0x47404028,__READ       ,__usb_qmgr_fdbsc2_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC3,       0x4740402C,__READ       ,__usb_qmgr_fdbsc3_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC4,       0x47404030,__READ       ,__usb_qmgr_fdbsc4_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC5,       0x47404034,__READ       ,__usb_qmgr_fdbsc5_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC6,       0x47404038,__READ       ,__usb_qmgr_fdbsc6_bits);
__IO_REG32_BIT(USB_QMGR_FDBSC7,       0x4740403C,__READ       ,__usb_qmgr_fdbsc7_bits);
__IO_REG32(    USB_QMGR_LRAM0BASE,    0x47404080,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_LRAM0SIZE,    0x47404084,__READ_WRITE ,__usb_qmgr_lramsize_bits);
__IO_REG32(    USB_QMGR_LRAM1BASE,    0x47404088,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_PEND0,        0x47404090,__READ       ,__usb_qmgr_pend0_bits);
__IO_REG32_BIT(USB_QMGR_PEND1,        0x47404094,__READ       ,__usb_qmgr_pend1_bits);
__IO_REG32_BIT(USB_QMGR_PEND2,        0x47404098,__READ       ,__usb_qmgr_pend2_bits);
__IO_REG32_BIT(USB_QMGR_PEND3,        0x4740409C,__READ       ,__usb_qmgr_pend3_bits);
__IO_REG32_BIT(USB_QMGR_PEND4,        0x474040A0,__READ       ,__usb_qmgr_pend4_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE0,   0x47405000,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL0,    0x47405004,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE1,   0x47405010,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL1,    0x47405014,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE2,   0x47405020,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL2,    0x47405024,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE3,   0x47405030,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL3,    0x47405034,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE4,   0x47405040,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL4,    0x47405044,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE5,   0x47405050,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL5,    0x47405054,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE6,   0x47405060,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL6,    0x47405064,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);
__IO_REG32(    USB_QMGR_QMEMRBASE7,   0x47405070,__READ_WRITE );
__IO_REG32_BIT(USB_QMGR_QMEMCTRL7,    0x47405074,__READ_WRITE ,__usb_qmgr_qmemctrl_bits);

/***************************************************************************
 **
 ** USB QUEUE MGR
 **
 ***************************************************************************/
__IO_REG32_BIT(USB_QMGR_QUEUE_0_A,    0x47406000,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_0_B,    0x47406004,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_0_C,    0x47406008,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_0_D,    0x4740600C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_A,    0x47406010,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_B,    0x47406014,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_C,    0x47406018,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_D,    0x4740601C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_A,    0x47406020,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_B,    0x47406024,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_C,    0x47406028,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_D,    0x4740602C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_A,    0x47406030,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_B,    0x47406034,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_C,    0x47406038,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_D,    0x4740603C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_A,    0x47406040,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_B,    0x47406044,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_C,    0x47406048,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_D,    0x4740604C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_A,    0x47406050,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_B,    0x47406054,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_C,    0x47406058,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_D,    0x4740605C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_A,    0x47406060,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_B,    0x47406064,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_C,    0x47406068,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_D,    0x4740606C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_A,    0x47406070,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_B,    0x47406074,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_C,    0x47406078,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_D,    0x4740607C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_A,    0x47406080,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_B,    0x47406084,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_C,    0x47406088,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_D,    0x4740608C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_A,    0x47406090,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_B,    0x47406094,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_C,    0x47406098,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_D,    0x4740609C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_A,   0x474060A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_B,   0x474060A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_C,   0x474060A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_D,   0x474060AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_A,   0x474060B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_B,   0x474060B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_C,   0x474060B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_D,   0x474060BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_A,   0x474060C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_B,   0x474060C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_C,   0x474060C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_D,   0x474060CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_A,   0x474060D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_B,   0x474060D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_C,   0x474060D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_D,   0x474060DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_A,   0x474060E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_B,   0x474060E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_C,   0x474060E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_D,   0x474060EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_A,   0x474060F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_B,   0x474060F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_C,   0x474060F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_D,   0x474060FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_A,   0x47406100,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_B,   0x47406104,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_C,   0x47406108,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_D,   0x4740610C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_A,   0x47406110,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_B,   0x47406114,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_C,   0x47406118,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_D,   0x4740611C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_A,   0x47406120,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_B,   0x47406124,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_C,   0x47406128,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_D,   0x4740612C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_A,   0x47406130,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_B,   0x47406134,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_C,   0x47406138,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_D,   0x4740613C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_A,   0x47406140,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_B,   0x47406144,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_C,   0x47406148,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_D,   0x4740614C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_A,   0x47406150,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_B,   0x47406154,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_C,   0x47406158,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_D,   0x4740615C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_A,   0x47406160,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_B,   0x47406164,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_C,   0x47406168,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_D,   0x4740616C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_A,   0x47406170,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_B,   0x47406174,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_C,   0x47406178,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_D,   0x4740617C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_A,   0x47406180,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_B,   0x47406184,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_C,   0x47406188,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_D,   0x4740618C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_A,   0x47406190,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_B,   0x47406194,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_C,   0x47406198,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_D,   0x4740619C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_A,   0x474061A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_B,   0x474061A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_C,   0x474061A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_D,   0x474061AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_A,   0x474061B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_B,   0x474061B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_C,   0x474061B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_D,   0x474061BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_A,   0x474061C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_B,   0x474061C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_C,   0x474061C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_D,   0x474061CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_A,   0x474061D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_B,   0x474061D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_C,   0x474061D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_D,   0x474061DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_A,   0x474061E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_B,   0x474061E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_C,   0x474061E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_D,   0x474061EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_A,   0x474061F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_B,   0x474061F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_C,   0x474061F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_D,   0x474061FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_A,   0x47406200,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_B,   0x47406204,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_C,   0x47406208,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_D,   0x4740620C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_A,   0x47406210,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_B,   0x47406214,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_C,   0x47406218,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_D,   0x4740621C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_A,   0x47406220,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_B,   0x47406224,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_C,   0x47406228,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_D,   0x4740622C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_A,   0x47406230,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_B,   0x47406234,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_C,   0x47406238,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_D,   0x4740623C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_A,   0x47406240,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_B,   0x47406244,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_C,   0x47406248,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_D,   0x4740624C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_A,   0x47406250,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_B,   0x47406254,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_C,   0x47406258,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_D,   0x4740625C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_A,   0x47406260,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_B,   0x47406264,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_C,   0x47406268,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_D,   0x4740626C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_A,   0x47406270,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_B,   0x47406274,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_C,   0x47406278,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_D,   0x4740627C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_A,   0x47406280,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_B,   0x47406284,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_C,   0x47406288,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_D,   0x4740628C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_A,   0x47406290,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_B,   0x47406294,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_C,   0x47406298,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_D,   0x4740629C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_A,   0x474062A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_B,   0x474062A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_C,   0x474062A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_D,   0x474062AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_A,   0x474062B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_B,   0x474062B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_C,   0x474062B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_D,   0x474062BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_A,   0x474062C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_B,   0x474062C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_C,   0x474062C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_D,   0x474062CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_A,   0x474062D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_B,   0x474062D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_C,   0x474062D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_D,   0x474062DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_A,   0x474062E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_B,   0x474062E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_C,   0x474062E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_D,   0x474062EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_A,   0x474062F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_B,   0x474062F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_C,   0x474062F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_D,   0x474062FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_A,   0x47406300,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_B,   0x47406304,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_C,   0x47406308,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_D,   0x4740630C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_A,   0x47406310,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_B,   0x47406314,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_C,   0x47406318,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_D,   0x4740631C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_A,   0x47406320,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_B,   0x47406324,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_C,   0x47406328,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_D,   0x4740632C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_A,   0x47406330,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_B,   0x47406334,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_C,   0x47406338,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_D,   0x4740633C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_A,   0x47406340,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_B,   0x47406344,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_C,   0x47406348,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_D,   0x4740634C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_A,   0x47406350,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_B,   0x47406354,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_C,   0x47406358,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_D,   0x4740635C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_A,   0x47406360,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_B,   0x47406364,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_C,   0x47406368,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_D,   0x4740636C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_A,   0x47406370,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_B,   0x47406374,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_C,   0x47406378,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_D,   0x4740637C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_A,   0x47406380,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_B,   0x47406384,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_C,   0x47406388,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_D,   0x4740638C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_A,   0x47406390,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_B,   0x47406394,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_C,   0x47406398,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_D,   0x4740639C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_A,   0x474063A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_B,   0x474063A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_C,   0x474063A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_D,   0x474063AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_A,   0x474063B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_B,   0x474063B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_C,   0x474063B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_D,   0x474063BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_A,   0x474063C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_B,   0x474063C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_C,   0x474063C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_D,   0x474063CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_A,   0x474063D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_B,   0x474063D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_C,   0x474063D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_D,   0x474063DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_A,   0x474063E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_B,   0x474063E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_C,   0x474063E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_D,   0x474063EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_A,   0x474063F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_B,   0x474063F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_C,   0x474063F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_D,   0x474063FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_A,   0x47406400,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_B,   0x47406404,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_C,   0x47406408,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_D,   0x4740640C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_A,   0x47406410,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_B,   0x47406414,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_C,   0x47406418,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_D,   0x4740641C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_A,   0x47406420,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_B,   0x47406424,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_C,   0x47406428,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_D,   0x4740642C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_A,   0x47406430,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_B,   0x47406434,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_C,   0x47406438,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_D,   0x4740643C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_A,   0x47406440,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_B,   0x47406444,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_C,   0x47406448,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_D,   0x4740644C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_A,   0x47406450,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_B,   0x47406454,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_C,   0x47406458,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_D,   0x4740645C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_A,   0x47406460,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_B,   0x47406464,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_C,   0x47406468,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_D,   0x4740646C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_A,   0x47406470,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_B,   0x47406474,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_C,   0x47406478,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_D,   0x4740647C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_A,   0x47406480,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_B,   0x47406484,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_C,   0x47406488,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_D,   0x4740648C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_A,   0x47406490,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_B,   0x47406494,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_C,   0x47406498,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_D,   0x4740649C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_A,   0x474064A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_B,   0x474064A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_C,   0x474064A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_D,   0x474064AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_A,   0x474064B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_B,   0x474064B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_C,   0x474064B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_D,   0x474064BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_A,   0x474064C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_B,   0x474064C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_C,   0x474064C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_D,   0x474064CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_A,   0x474064D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_B,   0x474064D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_C,   0x474064D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_D,   0x474064DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_A,   0x474064E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_B,   0x474064E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_C,   0x474064E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_D,   0x474064EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_A,   0x474064F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_B,   0x474064F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_C,   0x474064F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_D,   0x474064FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_A,   0x47406500,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_B,   0x47406504,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_C,   0x47406508,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_D,   0x4740650C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_A,   0x47406510,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_B,   0x47406514,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_C,   0x47406518,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_D,   0x4740651C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_A,   0x47406520,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_B,   0x47406524,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_C,   0x47406528,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_D,   0x4740652C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_A,   0x47406530,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_B,   0x47406534,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_C,   0x47406538,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_D,   0x4740653C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_A,   0x47406540,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_B,   0x47406544,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_C,   0x47406548,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_D,   0x4740654C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_A,   0x47406550,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_B,   0x47406554,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_C,   0x47406558,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_D,   0x4740655C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_A,   0x47406560,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_B,   0x47406564,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_C,   0x47406568,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_D,   0x4740656C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_A,   0x47406570,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_B,   0x47406574,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_C,   0x47406578,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_D,   0x4740657C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_A,   0x47406580,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_B,   0x47406584,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_C,   0x47406588,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_D,   0x4740658C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_A,   0x47406590,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_B,   0x47406594,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_C,   0x47406598,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_D,   0x4740659C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_A,   0x474065A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_B,   0x474065A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_C,   0x474065A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_D,   0x474065AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_A,   0x474065B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_B,   0x474065B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_C,   0x474065B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_D,   0x474065BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_A,   0x474065C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_B,   0x474065C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_C,   0x474065C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_D,   0x474065CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_A,   0x474065D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_B,   0x474065D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_C,   0x474065D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_D,   0x474065DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_A,   0x474065E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_B,   0x474065E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_C,   0x474065E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_D,   0x474065EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_A,   0x474065F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_B,   0x474065F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_C,   0x474065F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_D,   0x474065FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_A,   0x47406600,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_B,   0x47406604,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_C,   0x47406608,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_D,   0x4740660C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_A,   0x47406610,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_B,   0x47406614,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_C,   0x47406618,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_D,   0x4740661C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_A,   0x47406620,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_B,   0x47406624,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_C,   0x47406628,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_D,   0x4740662C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_A,   0x47406630,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_B,   0x47406634,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_C,   0x47406638,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_D,   0x4740663C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_A,  0x47406640,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_B,  0x47406644,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_C,  0x47406648,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_D,  0x4740664C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_A,  0x47406650,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_B,  0x47406654,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_C,  0x47406658,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_D,  0x4740665C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_A,  0x47406660,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_B,  0x47406664,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_C,  0x47406668,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_D,  0x4740666C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_A,  0x47406670,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_B,  0x47406674,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_C,  0x47406678,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_D,  0x4740667C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_A,  0x47406680,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_B,  0x47406684,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_C,  0x47406688,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_D,  0x4740668C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_A,  0x47406690,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_B,  0x47406694,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_C,  0x47406698,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_D,  0x4740669C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_A,  0x474066A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_B,  0x474066A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_C,  0x474066A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_D,  0x474066AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_A,  0x474066B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_B,  0x474066B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_C,  0x474066B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_D,  0x474066BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_A,  0x474066C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_B,  0x474066C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_C,  0x474066C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_D,  0x474066CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_A,  0x474066D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_B,  0x474066D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_C,  0x474066D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_D,  0x474066DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_A,  0x474066E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_B,  0x474066E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_C,  0x474066E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_D,  0x474066EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_A,  0x474066F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_B,  0x474066F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_C,  0x474066F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_D,  0x474066FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_A,  0x47406700,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_B,  0x47406704,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_C,  0x47406708,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_D,  0x4740670C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_A,  0x47406710,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_B,  0x47406714,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_C,  0x47406718,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_D,  0x4740671C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_A,  0x47406720,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_B,  0x47406724,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_C,  0x47406728,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_D,  0x4740672C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_A,  0x47406730,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_B,  0x47406734,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_C,  0x47406738,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_D,  0x4740673C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_A,  0x47406740,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_B,  0x47406744,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_C,  0x47406748,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_D,  0x4740674C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_A,  0x47406750,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_B,  0x47406754,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_C,  0x47406758,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_D,  0x4740675C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_A,  0x47406760,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_B,  0x47406764,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_C,  0x47406768,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_D,  0x4740676C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_A,  0x47406770,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_B,  0x47406774,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_C,  0x47406778,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_D,  0x4740677C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_A,  0x47406780,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_B,  0x47406784,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_C,  0x47406788,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_D,  0x4740678C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_A,  0x47406790,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_B,  0x47406794,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_C,  0x47406798,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_D,  0x4740679C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_A,  0x474067A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_B,  0x474067A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_C,  0x474067A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_D,  0x474067AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_A,  0x474067B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_B,  0x474067B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_C,  0x474067B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_D,  0x474067BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_A,  0x474067C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_B,  0x474067C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_C,  0x474067C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_D,  0x474067CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_A,  0x474067D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_B,  0x474067D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_C,  0x474067D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_D,  0x474067DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_A,  0x474067E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_B,  0x474067E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_C,  0x474067E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_D,  0x474067EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_A,  0x474067F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_B,  0x474067F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_C,  0x474067F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_D,  0x474067FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_A,  0x47406800,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_B,  0x47406804,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_C,  0x47406808,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_D,  0x4740680C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_A,  0x47406810,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_B,  0x47406814,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_C,  0x47406818,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_D,  0x4740681C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_A,  0x47406820,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_B,  0x47406824,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_C,  0x47406828,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_D,  0x4740682C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_A,  0x47406830,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_B,  0x47406834,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_C,  0x47406838,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_D,  0x4740683C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_A,  0x47406840,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_B,  0x47406844,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_C,  0x47406848,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_D,  0x4740684C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_A,  0x47406850,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_B,  0x47406854,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_C,  0x47406858,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_D,  0x4740685C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_A,  0x47406860,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_B,  0x47406864,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_C,  0x47406868,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_D,  0x4740686C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_A,  0x47406870,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_B,  0x47406874,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_C,  0x47406878,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_D,  0x4740687C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_A,  0x47406880,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_B,  0x47406884,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_C,  0x47406888,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_D,  0x4740688C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_A,  0x47406890,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_B,  0x47406894,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_C,  0x47406898,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_D,  0x4740689C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_A,  0x474068A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_B,  0x474068A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_C,  0x474068A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_D,  0x474068AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_A,  0x474068B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_B,  0x474068B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_C,  0x474068B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_D,  0x474068BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_A,  0x474068C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_B,  0x474068C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_C,  0x474068C8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_D,  0x474068CC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_A,  0x474068D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_B,  0x474068D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_C,  0x474068D8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_D,  0x474068DC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_A,  0x474068E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_B,  0x474068E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_C,  0x474068E8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_D,  0x474068EC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_A,  0x474068F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_B,  0x474068F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_C,  0x474068F8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_D,  0x474068FC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_A,  0x47406900,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_B,  0x47406904,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_C,  0x47406908,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_D,  0x4740690C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_A,  0x47406910,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_B,  0x47406914,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_C,  0x47406918,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_D,  0x4740691C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_A,  0x47406920,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_B,  0x47406924,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_C,  0x47406928,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_D,  0x4740692C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_A,  0x47406930,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_B,  0x47406934,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_C,  0x47406938,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_D,  0x4740693C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_A,  0x47406940,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_B,  0x47406944,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_C,  0x47406948,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_D,  0x4740694C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_A,  0x47406950,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_B,  0x47406954,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_C,  0x47406958,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_D,  0x4740695C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_A,  0x47406960,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_B,  0x47406964,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_C,  0x47406968,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_D,  0x4740696C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_A,  0x47406970,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_B,  0x47406974,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_C,  0x47406978,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_D,  0x4740697C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_A,  0x47406980,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_B,  0x47406984,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_C,  0x47406988,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_D,  0x4740698C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_A,  0x47406990,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_B,  0x47406994,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_C,  0x47406998,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_D,  0x4740699C,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_A,  0x474069A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_B,  0x474069A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_C,  0x474069A8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_D,  0x474069AC,__READ_WRITE ,__usb_qmgr_queue_d_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_A,  0x474069B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_B,  0x474069B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_C,  0x474069B8,__READ_WRITE ,__usb_qmgr_queue_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_D,  0x474069BC,__READ_WRITE ,__usb_qmgr_queue_d_bits);

/***************************************************************************
 **
 ** USB QUEUE STATUS
 **
 ***************************************************************************/
__IO_REG32_BIT(USB_QMGR_QUEUE_0_STATUS_A,    0x47407000,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_0_STATUS_B,    0x47407004,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_0_STATUS_C,    0x47407008,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_STATUS_A,    0x47407010,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_STATUS_B,    0x47407014,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_1_STATUS_C,    0x47407018,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_STATUS_A,    0x47407020,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_STATUS_B,    0x47407024,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_2_STATUS_C,    0x47407028,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_STATUS_A,    0x47407030,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_STATUS_B,    0x47407034,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_3_STATUS_C,    0x47407038,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_STATUS_A,    0x47407040,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_STATUS_B,    0x47407044,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_4_STATUS_C,    0x47407048,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_STATUS_A,    0x47407050,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_STATUS_B,    0x47407054,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_5_STATUS_C,    0x47407058,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_STATUS_A,    0x47407060,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_STATUS_B,    0x47407064,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_6_STATUS_C,    0x47407068,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_STATUS_A,    0x47407070,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_STATUS_B,    0x47407074,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_7_STATUS_C,    0x47407078,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_STATUS_A,    0x47407080,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_STATUS_B,    0x47407084,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_8_STATUS_C,    0x47407088,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_STATUS_A,    0x47407090,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_STATUS_B,    0x47407094,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_9_STATUS_C,    0x47407098,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_STATUS_A,   0x474070A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_STATUS_B,   0x474070A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_10_STATUS_C,   0x474070A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_STATUS_A,   0x474070B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_STATUS_B,   0x474070B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_11_STATUS_C,   0x474070B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_STATUS_A,   0x474070C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_STATUS_B,   0x474070C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_12_STATUS_C,   0x474070C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_STATUS_A,   0x474070D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_STATUS_B,   0x474070D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_13_STATUS_C,   0x474070D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_STATUS_A,   0x474070E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_STATUS_B,   0x474070E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_14_STATUS_C,   0x474070E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_STATUS_A,   0x474070F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_STATUS_B,   0x474070F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_15_STATUS_C,   0x474070F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_STATUS_A,   0x47407100,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_STATUS_B,   0x47407104,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_16_STATUS_C,   0x47407108,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_STATUS_A,   0x47407110,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_STATUS_B,   0x47407114,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_17_STATUS_C,   0x47407118,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_STATUS_A,   0x47407120,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_STATUS_B,   0x47407124,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_18_STATUS_C,   0x47407128,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_STATUS_A,   0x47407130,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_STATUS_B,   0x47407134,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_19_STATUS_C,   0x47407138,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_STATUS_A,   0x47407140,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_STATUS_B,   0x47407144,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_20_STATUS_C,   0x47407148,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_STATUS_A,   0x47407150,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_STATUS_B,   0x47407154,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_21_STATUS_C,   0x47407158,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_STATUS_A,   0x47407160,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_STATUS_B,   0x47407164,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_22_STATUS_C,   0x47407168,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_STATUS_A,   0x47407170,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_STATUS_B,   0x47407174,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_23_STATUS_C,   0x47407178,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_STATUS_A,   0x47407180,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_STATUS_B,   0x47407184,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_24_STATUS_C,   0x47407188,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_STATUS_A,   0x47407190,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_STATUS_B,   0x47407194,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_25_STATUS_C,   0x47407198,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_STATUS_A,   0x474071A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_STATUS_B,   0x474071A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_26_STATUS_C,   0x474071A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_STATUS_A,   0x474071B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_STATUS_B,   0x474071B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_27_STATUS_C,   0x474071B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_STATUS_A,   0x474071C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_STATUS_B,   0x474071C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_28_STATUS_C,   0x474071C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_STATUS_A,   0x474071D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_STATUS_B,   0x474071D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_29_STATUS_C,   0x474071D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_STATUS_A,   0x474071E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_STATUS_B,   0x474071E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_30_STATUS_C,   0x474071E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_STATUS_A,   0x474071F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_STATUS_B,   0x474071F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_31_STATUS_C,   0x474071F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_STATUS_A,   0x47407200,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_STATUS_B,   0x47407204,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_32_STATUS_C,   0x47407208,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_STATUS_A,   0x47407210,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_STATUS_B,   0x47407214,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_33_STATUS_C,   0x47407218,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_STATUS_A,   0x47407220,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_STATUS_B,   0x47407224,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_34_STATUS_C,   0x47407228,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_STATUS_A,   0x47407230,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_STATUS_B,   0x47407234,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_35_STATUS_C,   0x47407238,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_STATUS_A,   0x47407240,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_STATUS_B,   0x47407244,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_36_STATUS_C,   0x47407248,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_STATUS_A,   0x47407250,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_STATUS_B,   0x47407254,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_37_STATUS_C,   0x47407258,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_STATUS_A,   0x47407260,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_STATUS_B,   0x47407264,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_38_STATUS_C,   0x47407268,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_STATUS_A,   0x47407270,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_STATUS_B,   0x47407274,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_39_STATUS_C,   0x47407278,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_STATUS_A,   0x47407280,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_STATUS_B,   0x47407284,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_40_STATUS_C,   0x47407288,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_STATUS_A,   0x47407290,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_STATUS_B,   0x47407294,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_41_STATUS_C,   0x47407298,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_STATUS_A,   0x474072A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_STATUS_B,   0x474072A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_42_STATUS_C,   0x474072A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_STATUS_A,   0x474072B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_STATUS_B,   0x474072B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_43_STATUS_C,   0x474072B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_STATUS_A,   0x474072C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_STATUS_B,   0x474072C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_44_STATUS_C,   0x474072C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_STATUS_A,   0x474072D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_STATUS_B,   0x474072D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_45_STATUS_C,   0x474072D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_STATUS_A,   0x474072E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_STATUS_B,   0x474072E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_46_STATUS_C,   0x474072E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_STATUS_A,   0x474072F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_STATUS_B,   0x474072F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_47_STATUS_C,   0x474072F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_STATUS_A,   0x47407300,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_STATUS_B,   0x47407304,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_48_STATUS_C,   0x47407308,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_STATUS_A,   0x47407310,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_STATUS_B,   0x47407314,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_49_STATUS_C,   0x47407318,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_STATUS_A,   0x47407320,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_STATUS_B,   0x47407324,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_50_STATUS_C,   0x47407328,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_STATUS_A,   0x47407330,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_STATUS_B,   0x47407334,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_51_STATUS_C,   0x47407338,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_STATUS_A,   0x47407340,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_STATUS_B,   0x47407344,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_52_STATUS_C,   0x47407348,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_STATUS_A,   0x47407350,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_STATUS_B,   0x47407354,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_53_STATUS_C,   0x47407358,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_STATUS_A,   0x47407360,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_STATUS_B,   0x47407364,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_54_STATUS_C,   0x47407368,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_STATUS_A,   0x47407370,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_STATUS_B,   0x47407374,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_55_STATUS_C,   0x47407378,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_STATUS_A,   0x47407380,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_STATUS_B,   0x47407384,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_56_STATUS_C,   0x47407388,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_STATUS_A,   0x47407390,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_STATUS_B,   0x47407394,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_57_STATUS_C,   0x47407398,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_STATUS_A,   0x474073A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_STATUS_B,   0x474073A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_58_STATUS_C,   0x474073A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_STATUS_A,   0x474073B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_STATUS_B,   0x474073B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_59_STATUS_C,   0x474073B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_STATUS_A,   0x474073C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_STATUS_B,   0x474073C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_60_STATUS_C,   0x474073C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_STATUS_A,   0x474073D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_STATUS_B,   0x474073D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_61_STATUS_C,   0x474073D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_STATUS_A,   0x474073E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_STATUS_B,   0x474073E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_62_STATUS_C,   0x474073E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_STATUS_A,   0x474073F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_STATUS_B,   0x474073F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_63_STATUS_C,   0x474073F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_STATUS_A,   0x47407400,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_STATUS_B,   0x47407404,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_64_STATUS_C,   0x47407408,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_STATUS_A,   0x47407410,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_STATUS_B,   0x47407414,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_65_STATUS_C,   0x47407418,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_STATUS_A,   0x47407420,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_STATUS_B,   0x47407424,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_66_STATUS_C,   0x47407428,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_STATUS_A,   0x47407430,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_STATUS_B,   0x47407434,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_67_STATUS_C,   0x47407438,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_STATUS_A,   0x47407440,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_STATUS_B,   0x47407444,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_68_STATUS_C,   0x47407448,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_STATUS_A,   0x47407450,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_STATUS_B,   0x47407454,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_69_STATUS_C,   0x47407458,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_STATUS_A,   0x47407460,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_STATUS_B,   0x47407464,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_70_STATUS_C,   0x47407468,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_STATUS_A,   0x47407470,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_STATUS_B,   0x47407474,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_71_STATUS_C,   0x47407478,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_STATUS_A,   0x47407480,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_STATUS_B,   0x47407484,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_72_STATUS_C,   0x47407488,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_STATUS_A,   0x47407490,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_STATUS_B,   0x47407494,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_73_STATUS_C,   0x47407498,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_STATUS_A,   0x474074A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_STATUS_B,   0x474074A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_74_STATUS_C,   0x474074A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_STATUS_A,   0x474074B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_STATUS_B,   0x474074B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_75_STATUS_C,   0x474074B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_STATUS_A,   0x474074C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_STATUS_B,   0x474074C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_76_STATUS_C,   0x474074C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_STATUS_A,   0x474074D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_STATUS_B,   0x474074D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_77_STATUS_C,   0x474074D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_STATUS_A,   0x474074E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_STATUS_B,   0x474074E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_78_STATUS_C,   0x474074E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_STATUS_A,   0x474074F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_STATUS_B,   0x474074F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_79_STATUS_C,   0x474074F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_STATUS_A,   0x47407500,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_STATUS_B,   0x47407504,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_80_STATUS_C,   0x47407508,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_STATUS_A,   0x47407510,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_STATUS_B,   0x47407514,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_81_STATUS_C,   0x47407518,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_STATUS_A,   0x47407520,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_STATUS_B,   0x47407524,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_82_STATUS_C,   0x47407528,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_STATUS_A,   0x47407530,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_STATUS_B,   0x47407534,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_83_STATUS_C,   0x47407538,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_STATUS_A,   0x47407540,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_STATUS_B,   0x47407544,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_84_STATUS_C,   0x47407548,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_STATUS_A,   0x47407550,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_STATUS_B,   0x47407554,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_85_STATUS_C,   0x47407558,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_STATUS_A,   0x47407560,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_STATUS_B,   0x47407564,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_86_STATUS_C,   0x47407568,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_STATUS_A,   0x47407570,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_STATUS_B,   0x47407574,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_87_STATUS_C,   0x47407578,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_STATUS_A,   0x47407580,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_STATUS_B,   0x47407584,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_88_STATUS_C,   0x47407588,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_STATUS_A,   0x47407590,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_STATUS_B,   0x47407594,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_89_STATUS_C,   0x47407598,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_STATUS_A,   0x474075A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_STATUS_B,   0x474075A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_90_STATUS_C,   0x474075A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_STATUS_A,   0x474075B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_STATUS_B,   0x474075B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_91_STATUS_C,   0x474075B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_STATUS_A,   0x474075C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_STATUS_B,   0x474075C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_92_STATUS_C,   0x474075C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_STATUS_A,   0x474075D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_STATUS_B,   0x474075D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_93_STATUS_C,   0x474075D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_STATUS_A,   0x474075E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_STATUS_B,   0x474075E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_94_STATUS_C,   0x474075E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_STATUS_A,   0x474075F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_STATUS_B,   0x474075F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_95_STATUS_C,   0x474075F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_STATUS_A,   0x47407600,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_STATUS_B,   0x47407604,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_96_STATUS_C,   0x47407608,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_STATUS_A,   0x47407610,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_STATUS_B,   0x47407614,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_97_STATUS_C,   0x47407618,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_STATUS_A,   0x47407620,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_STATUS_B,   0x47407624,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_98_STATUS_C,   0x47407628,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_STATUS_A,   0x47407630,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_STATUS_B,   0x47407634,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_99_STATUS_C,   0x47407638,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_STATUS_A,  0x47407640,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_STATUS_B,  0x47407644,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_100_STATUS_C,  0x47407648,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_STATUS_A,  0x47407650,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_STATUS_B,  0x47407654,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_101_STATUS_C,  0x47407658,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_STATUS_A,  0x47407660,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_STATUS_B,  0x47407664,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_102_STATUS_C,  0x47407668,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_STATUS_A,  0x47407670,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_STATUS_B,  0x47407674,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_103_STATUS_C,  0x47407678,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_STATUS_A,  0x47407680,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_STATUS_B,  0x47407684,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_104_STATUS_C,  0x47407688,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_STATUS_A,  0x47407690,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_STATUS_B,  0x47407694,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_105_STATUS_C,  0x47407698,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_STATUS_A,  0x474076A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_STATUS_B,  0x474076A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_106_STATUS_C,  0x474076A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_STATUS_A,  0x474076B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_STATUS_B,  0x474076B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_107_STATUS_C,  0x474076B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_STATUS_A,  0x474076C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_STATUS_B,  0x474076C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_108_STATUS_C,  0x474076C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_STATUS_A,  0x474076D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_STATUS_B,  0x474076D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_109_STATUS_C,  0x474076D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_STATUS_A,  0x474076E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_STATUS_B,  0x474076E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_110_STATUS_C,  0x474076E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_STATUS_A,  0x474076F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_STATUS_B,  0x474076F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_111_STATUS_C,  0x474076F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_STATUS_A,  0x47407700,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_STATUS_B,  0x47407704,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_112_STATUS_C,  0x47407708,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_STATUS_A,  0x47407710,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_STATUS_B,  0x47407714,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_113_STATUS_C,  0x47407718,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_STATUS_A,  0x47407720,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_STATUS_B,  0x47407724,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_114_STATUS_C,  0x47407728,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_STATUS_A,  0x47407730,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_STATUS_B,  0x47407734,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_115_STATUS_C,  0x47407738,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_STATUS_A,  0x47407740,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_STATUS_B,  0x47407744,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_116_STATUS_C,  0x47407748,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_STATUS_A,  0x47407750,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_STATUS_B,  0x47407754,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_117_STATUS_C,  0x47407758,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_STATUS_A,  0x47407760,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_STATUS_B,  0x47407764,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_118_STATUS_C,  0x47407768,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_STATUS_A,  0x47407770,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_STATUS_B,  0x47407774,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_119_STATUS_C,  0x47407778,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_STATUS_A,  0x47407780,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_STATUS_B,  0x47407784,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_120_STATUS_C,  0x47407788,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_STATUS_A,  0x47407790,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_STATUS_B,  0x47407794,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_121_STATUS_C,  0x47407798,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_STATUS_A,  0x474077A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_STATUS_B,  0x474077A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_122_STATUS_C,  0x474077A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_STATUS_A,  0x474077B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_STATUS_B,  0x474077B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_123_STATUS_C,  0x474077B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_STATUS_A,  0x474077C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_STATUS_B,  0x474077C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_124_STATUS_C,  0x474077C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_STATUS_A,  0x474077D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_STATUS_B,  0x474077D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_125_STATUS_C,  0x474077D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_STATUS_A,  0x474077E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_STATUS_B,  0x474077E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_126_STATUS_C,  0x474077E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_STATUS_A,  0x474077F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_STATUS_B,  0x474077F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_127_STATUS_C,  0x474077F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_STATUS_A,  0x47407800,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_STATUS_B,  0x47407804,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_128_STATUS_C,  0x47407808,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_STATUS_A,  0x47407810,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_STATUS_B,  0x47407814,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_129_STATUS_C,  0x47407818,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_STATUS_A,  0x47407820,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_STATUS_B,  0x47407824,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_130_STATUS_C,  0x47407828,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_STATUS_A,  0x47407830,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_STATUS_B,  0x47407834,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_131_STATUS_C,  0x47407838,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_STATUS_A,  0x47407840,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_STATUS_B,  0x47407844,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_132_STATUS_C,  0x47407848,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_STATUS_A,  0x47407850,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_STATUS_B,  0x47407854,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_133_STATUS_C,  0x47407858,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_STATUS_A,  0x47407860,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_STATUS_B,  0x47407864,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_134_STATUS_C,  0x47407868,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_STATUS_A,  0x47407870,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_STATUS_B,  0x47407874,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_135_STATUS_C,  0x47407878,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_STATUS_A,  0x47407880,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_STATUS_B,  0x47407884,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_136_STATUS_C,  0x47407888,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_STATUS_A,  0x47407890,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_STATUS_B,  0x47407894,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_137_STATUS_C,  0x47407898,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_STATUS_A,  0x474078A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_STATUS_B,  0x474078A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_138_STATUS_C,  0x474078A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_STATUS_A,  0x474078B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_STATUS_B,  0x474078B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_139_STATUS_C,  0x474078B8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_STATUS_A,  0x474078C0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_STATUS_B,  0x474078C4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_140_STATUS_C,  0x474078C8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_STATUS_A,  0x474078D0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_STATUS_B,  0x474078D4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_141_STATUS_C,  0x474078D8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_STATUS_A,  0x474078E0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_STATUS_B,  0x474078E4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_142_STATUS_C,  0x474078E8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_STATUS_A,  0x474078F0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_STATUS_B,  0x474078F4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_143_STATUS_C,  0x474078F8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_STATUS_A,  0x47407900,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_STATUS_B,  0x47407904,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_144_STATUS_C,  0x47407908,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_STATUS_A,  0x47407910,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_STATUS_B,  0x47407914,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_145_STATUS_C,  0x47407918,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_STATUS_A,  0x47407920,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_STATUS_B,  0x47407924,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_146_STATUS_C,  0x47407928,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_STATUS_A,  0x47407930,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_STATUS_B,  0x47407934,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_147_STATUS_C,  0x47407938,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_STATUS_A,  0x47407940,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_STATUS_B,  0x47407944,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_148_STATUS_C,  0x47407948,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_STATUS_A,  0x47407950,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_STATUS_B,  0x47407954,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_149_STATUS_C,  0x47407958,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_STATUS_A,  0x47407960,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_STATUS_B,  0x47407964,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_150_STATUS_C,  0x47407968,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_STATUS_A,  0x47407970,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_STATUS_B,  0x47407974,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_151_STATUS_C,  0x47407978,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_STATUS_A,  0x47407980,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_STATUS_B,  0x47407984,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_152_STATUS_C,  0x47407988,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_STATUS_A,  0x47407990,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_STATUS_B,  0x47407994,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_153_STATUS_C,  0x47407998,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_STATUS_A,  0x474079A0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_STATUS_B,  0x474079A4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_154_STATUS_C,  0x474079A8,__READ       ,__usb_qmgr_queue_status_c_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_STATUS_A,  0x474079B0,__READ       ,__usb_qmgr_queue_a_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_STATUS_B,  0x474079B4,__READ       ,__usb_qmgr_queue_b_bits);
__IO_REG32_BIT(USB_QMGR_QUEUE_155_STATUS_C,  0x474079B8,__READ       ,__usb_qmgr_queue_status_c_bits);

/***************************************************************************
 **
 ** MAILBOX
 **
 ***************************************************************************/
__IO_REG32_BIT(MAILBOX_REVISION,        0x480C8000,__READ       ,__mailbox_revision_bits);
__IO_REG32_BIT(MAILBOX_SYSCONFIG,       0x480C8010,__READ_WRITE ,__mailbox_sysconfig_bits);
__IO_REG32(    MAILBOX_MESSAGE_0,       0x480C8040,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_1,       0x480C8044,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_2,       0x480C8048,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_3,       0x480C804C,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_4,       0x480C8050,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_5,       0x480C8054,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_6,       0x480C8058,__READ_WRITE );
__IO_REG32(    MAILBOX_MESSAGE_7,       0x480C805C,__READ_WRITE );
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_0,    0x480C8080,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_1,    0x480C8084,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_2,    0x480C8088,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_3,    0x480C808C,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_4,    0x480C8090,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_5,    0x480C8094,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_6,    0x480C8098,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_FIFOSTATUS_7,    0x480C809C,__READ       ,__mailbox_fifostatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_0,     0x480C80C0,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_1,     0x480C80C4,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_2,     0x480C80C8,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_3,     0x480C80CC,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_4,     0x480C80D0,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_5,     0x480C80D4,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_6,     0x480C80D8,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_MSGSTATUS_7,     0x480C80DC,__READ       ,__mailbox_msgstatus_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_RAW_0, 0x480C8100,__READ_WRITE ,__mailbox_irqstatus_0_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_CLR_0, 0x480C8104,__READ_WRITE ,__mailbox_irqstatus_0_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_SET_0, 0x480C8108,__READ_WRITE ,__mailbox_irqenable_0_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_CLR_0, 0x480C810C,__READ_WRITE ,__mailbox_irqenable_0_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_RAW_1, 0x480C8110,__READ_WRITE ,__mailbox_irqstatus_1_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_CLR_1, 0x480C8114,__READ_WRITE ,__mailbox_irqstatus_1_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_SET_1, 0x480C8118,__READ_WRITE ,__mailbox_irqenable_1_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_CLR_1, 0x480C811C,__READ_WRITE ,__mailbox_irqenable_1_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_RAW_2, 0x480C8120,__READ_WRITE ,__mailbox_irqstatus_2_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_CLR_2, 0x480C8124,__READ_WRITE ,__mailbox_irqstatus_2_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_SET_2, 0x480C8128,__READ_WRITE ,__mailbox_irqenable_2_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_CLR_2, 0x480C812C,__READ_WRITE ,__mailbox_irqenable_2_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_RAW_3, 0x480C8130,__READ_WRITE ,__mailbox_irqstatus_3_bits);
__IO_REG32_BIT(MAILBOX_IRQSTATUS_CLR_3, 0x480C8134,__READ_WRITE ,__mailbox_irqstatus_3_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_SET_3, 0x480C8138,__READ_WRITE ,__mailbox_irqenable_3_bits);
__IO_REG32_BIT(MAILBOX_IRQENABLE_CLR_3, 0x480C813C,__READ_WRITE ,__mailbox_irqenable_3_bits);

/***************************************************************************
 **
 ** SPINLOCK Registers
 **
 ***************************************************************************/
__IO_REG32(    SPINLOCK_REV,          0x480CA000,__READ       );
__IO_REG32_BIT(SPINLOCK_SYSCONFIG,    0x480CA010,__READ_WRITE ,__spinlock_sysconfig_bits);
__IO_REG32_BIT(SPINLOCK_SYSTATUS,     0x480CA014,__READ       ,__spinlock_systatus_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_0,   0x480CA800,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_1,   0x480CA804,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_2,   0x480CA808,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_3,   0x480CA80C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_4,   0x480CA810,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_5,   0x480CA814,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_6,   0x480CA818,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_7,   0x480CA81C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_8,   0x480CA820,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_9,   0x480CA824,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_10,  0x480CA828,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_11,  0x480CA82C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_12,  0x480CA830,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_13,  0x480CA834,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_14,  0x480CA838,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_15,  0x480CA83C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_16,  0x480CA840,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_17,  0x480CA844,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_18,  0x480CA848,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_19,  0x480CA84C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_20,  0x480CA850,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_21,  0x480CA854,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_22,  0x480CA858,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_23,  0x480CA85C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_24,  0x480CA860,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_25,  0x480CA864,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_26,  0x480CA868,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_27,  0x480CA86C,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_28,  0x480CA870,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_29,  0x480CA874,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_30,  0x480CA878,__READ_WRITE ,__spinlock_lock_reg_bits);
__IO_REG32_BIT(SPINLOCK_LOCK_REG_31,  0x480CA87C,__READ_WRITE ,__spinlock_lock_reg_bits);

/***************************************************************************
 **
 ** MMCHS0
 **
 ***************************************************************************/
__IO_REG32_BIT(MMCHS0_SYSCONFIG,            0x48060110,__READ_WRITE  ,__mmchs_sysconfig_bits);
__IO_REG32_BIT(MMCHS0_SYSSTATUS,            0x48060114,__READ        ,__mmchs_sysstatus_bits);
__IO_REG32(    MMCHS0_CSRE,                 0x48060124,__READ_WRITE  );
__IO_REG32_BIT(MMCHS0_SYSTEST,              0x48060128,__READ_WRITE  ,__mmchs_systest_bits);
__IO_REG32_BIT(MMCHS0_CON,                  0x4806012C,__READ_WRITE  ,__mmchs_con_bits);
__IO_REG32_BIT(MMCHS0_PWCNT,                0x48060130,__READ_WRITE  ,__mmchs_pwcnt_bits);
__IO_REG32(    MMCHS0_SDMASA,               0x48060200,__READ        );
__IO_REG32_BIT(MMCHS0_BLK,                  0x48060204,__READ_WRITE  ,__mmchs_blk_bits);
__IO_REG32(    MMCHS0_ARG,                  0x48060208,__READ_WRITE  );
__IO_REG32_BIT(MMCHS0_CMD,                  0x4806020C,__READ_WRITE  ,__mmchs_cmd_bits);
__IO_REG32_BIT(MMCHS0_RSP10,                0x48060210,__READ        ,__mmchs_rsp10_bits);
__IO_REG32_BIT(MMCHS0_RSP32,                0x48060214,__READ        ,__mmchs_rsp32_bits);
__IO_REG32_BIT(MMCHS0_RSP54,                0x48060218,__READ        ,__mmchs_rsp54_bits);
__IO_REG32_BIT(MMCHS0_RSP76,                0x4806021C,__READ        ,__mmchs_rsp76_bits);
__IO_REG32(    MMCHS0_DATA,                 0x48060220,__READ_WRITE  );
__IO_REG32_BIT(MMCHS0_PSTATE,               0x48060224,__READ        ,__mmchs_pstate_bits);
__IO_REG32_BIT(MMCHS0_HCTL,                 0x48060228,__READ_WRITE  ,__mmchs_hctl_bits);
__IO_REG32_BIT(MMCHS0_SYSCTL,               0x4806022C,__READ_WRITE  ,__mmchs_sysctl_bits);
__IO_REG32_BIT(MMCHS0_STAT,                 0x48060230,__READ_WRITE  ,__mmchs_stat_bits);
__IO_REG32_BIT(MMCHS0_IE,                   0x48060234,__READ_WRITE  ,__mmchs_ie_bits);
__IO_REG32_BIT(MMCHS0_ISE,                  0x48060238,__READ_WRITE  ,__mmchs_ise_bits);
__IO_REG32_BIT(MMCHS0_AC12,                 0x4806023C,__READ        ,__mmchs_ac12_bits);
__IO_REG32_BIT(MMCHS0_CAPA,                 0x48060240,__READ_WRITE  ,__mmchs_capa_bits);
__IO_REG32_BIT(MMCHS0_CUR_CAPA,             0x48060248,__READ_WRITE  ,__mmchs_cur_capa_bits);
__IO_REG32(    MMCHS0_FE,                   0x48060250,__WRITE       );
__IO_REG32_BIT(MMCHS0_ADMAES,               0x48060254,__READ_WRITE  ,__mmchs_admaes_bits);
__IO_REG32(    MMCHS0_ADMASAL,              0x48060258,__READ_WRITE  );
__IO_REG32(    MMCHS0_ADMASAH,              0x4806025C,__READ_WRITE  );
__IO_REG32_BIT(MMCHS0_REV,                  0x480602FC,__READ        ,__mmchs_rev_bits);

/***************************************************************************
 **
 ** MMCHS1
 **
 ***************************************************************************/
__IO_REG32_BIT(MMCHS1_SYSCONFIG,            0x481D8110,__READ_WRITE  ,__mmchs_sysconfig_bits);
__IO_REG32_BIT(MMCHS1_SYSSTATUS,            0x481D8114,__READ        ,__mmchs_sysstatus_bits);
__IO_REG32(    MMCHS1_CSRE,                 0x481D8124,__READ_WRITE  );
__IO_REG32_BIT(MMCHS1_SYSTEST,              0x481D8128,__READ_WRITE  ,__mmchs_systest_bits);
__IO_REG32_BIT(MMCHS1_CON,                  0x481D812C,__READ_WRITE  ,__mmchs_con_bits);
__IO_REG32_BIT(MMCHS1_PWCNT,                0x481D8130,__READ_WRITE  ,__mmchs_pwcnt_bits);
__IO_REG32(    MMCHS1_SDMASA,               0x481D8200,__READ        );
__IO_REG32_BIT(MMCHS1_BLK,                  0x481D8204,__READ_WRITE  ,__mmchs_blk_bits);
__IO_REG32(    MMCHS1_ARG,                  0x481D8208,__READ_WRITE  );
__IO_REG32_BIT(MMCHS1_CMD,                  0x481D820C,__READ_WRITE  ,__mmchs_cmd_bits);
__IO_REG32_BIT(MMCHS1_RSP10,                0x481D8210,__READ        ,__mmchs_rsp10_bits);
__IO_REG32_BIT(MMCHS1_RSP32,                0x481D8214,__READ        ,__mmchs_rsp32_bits);
__IO_REG32_BIT(MMCHS1_RSP54,                0x481D8218,__READ        ,__mmchs_rsp54_bits);
__IO_REG32_BIT(MMCHS1_RSP76,                0x481D821C,__READ        ,__mmchs_rsp76_bits);
__IO_REG32(    MMCHS1_DATA,                 0x481D8220,__READ_WRITE  );
__IO_REG32_BIT(MMCHS1_PSTATE,               0x481D8224,__READ        ,__mmchs_pstate_bits);
__IO_REG32_BIT(MMCHS1_HCTL,                 0x481D8228,__READ_WRITE  ,__mmchs_hctl_bits);
__IO_REG32_BIT(MMCHS1_SYSCTL,               0x481D822C,__READ_WRITE  ,__mmchs_sysctl_bits);
__IO_REG32_BIT(MMCHS1_STAT,                 0x481D8230,__READ_WRITE  ,__mmchs_stat_bits);
__IO_REG32_BIT(MMCHS1_IE,                   0x481D8234,__READ_WRITE  ,__mmchs_ie_bits);
__IO_REG32_BIT(MMCHS1_ISE,                  0x481D8238,__READ_WRITE  ,__mmchs_ise_bits);
__IO_REG32_BIT(MMCHS1_AC12,                 0x481D823C,__READ        ,__mmchs_ac12_bits);
__IO_REG32_BIT(MMCHS1_CAPA,                 0x481D8240,__READ_WRITE  ,__mmchs_capa_bits);
__IO_REG32_BIT(MMCHS1_CUR_CAPA,             0x481D8248,__READ_WRITE  ,__mmchs_cur_capa_bits);
__IO_REG32(    MMCHS1_FE,                   0x481D8250,__WRITE       );
__IO_REG32_BIT(MMCHS1_ADMAES,               0x481D8254,__READ_WRITE  ,__mmchs_admaes_bits);
__IO_REG32(    MMCHS1_ADMASAL,              0x481D8258,__READ_WRITE  );
__IO_REG32(    MMCHS1_ADMASAH,              0x481D825C,__READ_WRITE  );
__IO_REG32_BIT(MMCHS1_REV,                  0x481D82FC,__READ        ,__mmchs_rev_bits);

/***************************************************************************
 **
 ** MMCHS2
 **
 ***************************************************************************/
__IO_REG32_BIT(MMCHS2_SYSCONFIG,            0x47810110,__READ_WRITE  ,__mmchs_sysconfig_bits);
__IO_REG32_BIT(MMCHS2_SYSSTATUS,            0x47810114,__READ        ,__mmchs_sysstatus_bits);
__IO_REG32(    MMCHS2_CSRE,                 0x47810124,__READ_WRITE  );
__IO_REG32_BIT(MMCHS2_SYSTEST,              0x47810128,__READ_WRITE  ,__mmchs_systest_bits);
__IO_REG32_BIT(MMCHS2_CON,                  0x4781012C,__READ_WRITE  ,__mmchs_con_bits);
__IO_REG32_BIT(MMCHS2_PWCNT,                0x47810130,__READ_WRITE  ,__mmchs_pwcnt_bits);
__IO_REG32(    MMCHS2_SDMASA,               0x47810200,__READ        );
__IO_REG32_BIT(MMCHS2_BLK,                  0x47810204,__READ_WRITE  ,__mmchs_blk_bits);
__IO_REG32(    MMCHS2_ARG,                  0x47810208,__READ_WRITE  );
__IO_REG32_BIT(MMCHS2_CMD,                  0x4781020C,__READ_WRITE  ,__mmchs_cmd_bits);
__IO_REG32_BIT(MMCHS2_RSP10,                0x47810210,__READ        ,__mmchs_rsp10_bits);
__IO_REG32_BIT(MMCHS2_RSP32,                0x47810214,__READ        ,__mmchs_rsp32_bits);
__IO_REG32_BIT(MMCHS2_RSP54,                0x47810218,__READ        ,__mmchs_rsp54_bits);
__IO_REG32_BIT(MMCHS2_RSP76,                0x4781021C,__READ        ,__mmchs_rsp76_bits);
__IO_REG32(    MMCHS2_DATA,                 0x47810220,__READ_WRITE  );
__IO_REG32_BIT(MMCHS2_PSTATE,               0x47810224,__READ        ,__mmchs_pstate_bits);
__IO_REG32_BIT(MMCHS2_HCTL,                 0x47810228,__READ_WRITE  ,__mmchs_hctl_bits);
__IO_REG32_BIT(MMCHS2_SYSCTL,               0x4781022C,__READ_WRITE  ,__mmchs_sysctl_bits);
__IO_REG32_BIT(MMCHS2_STAT,                 0x47810230,__READ_WRITE  ,__mmchs_stat_bits);
__IO_REG32_BIT(MMCHS2_IE,                   0x47810234,__READ_WRITE  ,__mmchs_ie_bits);
__IO_REG32_BIT(MMCHS2_ISE,                  0x47810238,__READ_WRITE  ,__mmchs_ise_bits);
__IO_REG32_BIT(MMCHS2_AC12,                 0x4781023C,__READ        ,__mmchs_ac12_bits);
__IO_REG32_BIT(MMCHS2_CAPA,                 0x47810240,__READ_WRITE  ,__mmchs_capa_bits);
__IO_REG32_BIT(MMCHS2_CUR_CAPA,             0x47810248,__READ_WRITE  ,__mmchs_cur_capa_bits);
__IO_REG32(    MMCHS2_FE,                   0x47810250,__WRITE       );
__IO_REG32_BIT(MMCHS2_ADMAES,               0x47810254,__READ_WRITE  ,__mmchs_admaes_bits);
__IO_REG32(    MMCHS2_ADMASAL,              0x47810258,__READ_WRITE  );
__IO_REG32(    MMCHS2_ADMASAH,              0x4781025C,__READ_WRITE  );
__IO_REG32_BIT(MMCHS2_REV,                  0x478102FC,__READ        ,__mmchs_rev_bits);

/***************************************************************************
 **
 ** UART0
 **
 ***************************************************************************/
__IO_REG8(     UART0_DLL,             0x44E09000,__READ_WRITE );
#define        UART0_RHR              UART0_DLL
#define        UART0_THR              UART0_DLL
__IO_REG8_BIT( UART0_IER,             0x44E09004,__READ_WRITE ,__uart_ier_bits);
#define        UART0_DLH              UART0_IER
#define        UART0_IrDA_IER         UART0_IER
#define        UART0_IrDA_IER_bit     UART0_IER_bit.IrDA
#define        UART0_CIR_IER          UART0_IER
#define        UART0_CIR_IER_bit      UART0_IER_bit.CIR
__IO_REG8_BIT( UART0_IIR,             0x44E09008,__READ_WRITE ,__uart_iir_bits);
#define        UART0_FCR              UART0_IIR
#define        UART0_IrDA_IIR         UART0_IIR
#define        UART0_IrDA_IIR_bit     UART0_IIR_bit.IrDA
#define        UART0_CIR_IIR          UART0_IIR
#define        UART0_CIR_IIR_bit      UART0_IIR_bit.CIR
#define        UART0_EFR              UART0_IIR
#define        UART0_EFR_bit          UART0_IIR_bit
__IO_REG8_BIT( UART0_LCR,             0x44E0900C,__READ_WRITE ,__uart_lcr_bits);
__IO_REG8_BIT( UART0_MCR,             0x44E09010,__READ_WRITE ,__uart_mcr_bits);
#define        UART0_XON1_ADDR1       UART0_MCR
__IO_REG8_BIT( UART0_LSR,             0x44E09014,__READ_WRITE ,__uart_lsr_bits);
#define        UART0_XON2_ADDR2       UART0_LSR
#define        UART0_LSR_IrDA         UART0_LSR
#define        UART0_LSR_IrDA_bit     UART0_LSR_bit.IrDA
#define        UART0_LSR_CIR          UART0_LSR
#define        UART0_LSR_CIR_bit      UART0_LSR_bit.CIR
__IO_REG8_BIT( UART0_TCR,             0x44E09018,__READ_WRITE ,__uart_tcr_bits);
#define        UART0_XOFF1            UART0_TCR
#define        UART0_MSR              UART0_TCR
#define        UART0_MSR_bit          UART0_TCR_bit
__IO_REG8_BIT( UART0_TLR,             0x44E0901C,__READ_WRITE ,__uart_tlr_bits);
#define        UART0_SPR              UART0_TLR
#define        UART0_XOFF2            UART0_TLR
__IO_REG8_BIT( UART0_MDR1,            0x44E09020,__READ_WRITE ,__uart_mdr1_bits);
__IO_REG8_BIT( UART0_MDR2,            0x44E09024,__READ_WRITE ,__uart_mdr2_bits);
__IO_REG8_BIT( UART0_SFLSR,           0x44E09028,__READ_WRITE ,__uart_sflsr_bits);
#define        UART0_TXFLL            UART0_SFLSR
__IO_REG8_BIT( UART0_TXFLH,           0x44E0902C,__READ_WRITE ,__uart_txflh_bits);
#define        UART0_RESUME           UART0_TXFLH
__IO_REG8(     UART0_SFREGL,          0x44E09030,__READ_WRITE );
#define        UART0_RXFLL            UART0_SFREGL
__IO_REG8_BIT( UART0_SFREGH,          0x44E09034,__READ_WRITE ,__uart_sfregh_bits);
#define        UART0_RXFLH            UART0_SFREGH
#define        UART0_RXFLH_bit        UART0_SFREGH_bit
__IO_REG8_BIT( UART0_UASR,            0x44E09038,__READ_WRITE ,__uart_uasr_bits);
#define        UART0_BLR              UART0_UASR
#define        UART0_BLR_bit          UART0_UASR_bit
__IO_REG8_BIT( UART0_ACREG,           0x44E0903C,__READ_WRITE ,__uart_acreg_bits);
__IO_REG8_BIT( UART0_SCR,             0x44E09040,__READ_WRITE ,__uart_scr_bits);
__IO_REG8_BIT( UART0_SSR,             0x44E09044,__READ_WRITE ,__uart_ssr_bits);
__IO_REG8(     UART0_EBLR,            0x44E09048,__READ_WRITE );
__IO_REG8_BIT( UART0_MVR,             0x44E09050,__READ       ,__uart_mvr_bits);
__IO_REG8_BIT( UART0_SYSC,            0x44E09054,__READ_WRITE ,__uart_sysc_bits);
__IO_REG8_BIT( UART0_SYSS,            0x44E09058,__READ       ,__uart_syss_bits);
__IO_REG8_BIT( UART0_WER,             0x44E0905C,__READ_WRITE ,__uart_wer_bits);
__IO_REG8(     UART0_CFPS,            0x44E09060,__READ_WRITE );

/***************************************************************************
 **
 ** UART1
 **
 ***************************************************************************/
__IO_REG8(     UART1_DLL,             0x48022000,__READ_WRITE );
#define        UART1_RHR              UART1_DLL
#define        UART1_THR              UART1_DLL
__IO_REG8_BIT( UART1_IER,             0x48022004,__READ_WRITE ,__uart_ier_bits);
#define        UART1_DLH              UART1_IER
#define        UART1_IrDA_IER         UART1_IER
#define        UART1_IrDA_IER_bit     UART1_IER_bit.IrDA
#define        UART1_CIR_IER          UART1_IER
#define        UART1_CIR_IER_bit      UART1_IER_bit.CIR
__IO_REG8_BIT( UART1_IIR,             0x48022008,__READ_WRITE ,__uart_iir_bits);
#define        UART1_FCR              UART1_IIR
#define        UART1_IrDA_IIR         UART1_IIR
#define        UART1_IrDA_IIR_bit     UART1_IIR_bit.IrDA
#define        UART1_CIR_IIR          UART1_IIR
#define        UART1_CIR_IIR_bit      UART1_IIR_bit.CIR
#define        UART1_EFR              UART1_IIR
#define        UART1_EFR_bit          UART1_IIR_bit
__IO_REG8_BIT( UART1_LCR,             0x4802200C,__READ_WRITE ,__uart_lcr_bits);
__IO_REG8_BIT( UART1_MCR,             0x48022010,__READ_WRITE ,__uart_mcr_bits);
#define        UART1_XON1_ADDR1       UART1_MCR
__IO_REG8_BIT( UART1_LSR,             0x48022014,__READ_WRITE ,__uart_lsr_bits);
#define        UART1_XON2_ADDR2       UART1_LSR
#define        UART1_LSR_IrDA         UART1_LSR
#define        UART1_LSR_IrDA_bit     UART1_LSR_bit.IrDA
#define        UART1_LSR_CIR          UART1_LSR
#define        UART1_LSR_CIR_bit      UART1_LSR_bit.CIR
__IO_REG8_BIT( UART1_TCR,             0x48022018,__READ_WRITE ,__uart_tcr_bits);
#define        UART1_XOFF1            UART1_TCR
#define        UART1_MSR              UART1_TCR
#define        UART1_MSR_bit          UART1_TCR_bit
__IO_REG8_BIT( UART1_TLR,             0x4802201C,__READ_WRITE ,__uart_tlr_bits);
#define        UART1_SPR              UART1_TLR
#define        UART1_XOFF2            UART1_TLR
__IO_REG8_BIT( UART1_MDR1,            0x48022020,__READ_WRITE ,__uart_mdr1_bits);
__IO_REG8_BIT( UART1_MDR2,            0x48022024,__READ_WRITE ,__uart_mdr2_bits);
__IO_REG8_BIT( UART1_SFLSR,           0x48022028,__READ_WRITE ,__uart_sflsr_bits);
#define        UART1_TXFLL            UART1_SFLSR
__IO_REG8_BIT( UART1_TXFLH,           0x4802202C,__READ_WRITE ,__uart_txflh_bits);
#define        UART1_RESUME           UART1_TXFLH
__IO_REG8(     UART1_SFREGL,          0x48022030,__READ_WRITE );
#define        UART1_RXFLL            UART1_SFREGL
__IO_REG8_BIT( UART1_SFREGH,          0x48022034,__READ_WRITE ,__uart_sfregh_bits);
#define        UART1_RXFLH            UART1_SFREGH
#define        UART1_RXFLH_bit        UART1_SFREGH_bit
__IO_REG8_BIT( UART1_UASR,            0x48022038,__READ_WRITE ,__uart_uasr_bits);
#define        UART1_BLR              UART1_UASR
#define        UART1_BLR_bit          UART1_UASR_bit
__IO_REG8_BIT( UART1_ACREG,           0x4802203C,__READ_WRITE ,__uart_acreg_bits);
__IO_REG8_BIT( UART1_SCR,             0x48022040,__READ_WRITE ,__uart_scr_bits);
__IO_REG8_BIT( UART1_SSR,             0x48022044,__READ_WRITE ,__uart_ssr_bits);
__IO_REG8(     UART1_EBLR,            0x48022048,__READ_WRITE );
__IO_REG8_BIT( UART1_MVR,             0x48022050,__READ       ,__uart_mvr_bits);
__IO_REG8_BIT( UART1_SYSC,            0x48022054,__READ_WRITE ,__uart_sysc_bits);
__IO_REG8_BIT( UART1_SYSS,            0x48022058,__READ       ,__uart_syss_bits);
__IO_REG8_BIT( UART1_WER,             0x4802205C,__READ_WRITE ,__uart_wer_bits);
__IO_REG8(     UART1_CFPS,            0x48022060,__READ_WRITE );

/***************************************************************************
 **
 ** UART2
 **
 ***************************************************************************/
__IO_REG8(     UART2_DLL,             0x48024000,__READ_WRITE );
#define        UART2_RHR              UART2_DLL
#define        UART2_THR              UART2_DLL
__IO_REG8_BIT( UART2_IER,             0x48024004,__READ_WRITE ,__uart_ier_bits);
#define        UART2_DLH              UART2_IER
#define        UART2_IrDA_IER         UART2_IER
#define        UART2_IrDA_IER_bit     UART2_IER_bit.IrDA
#define        UART2_CIR_IER          UART2_IER
#define        UART2_CIR_IER_bit      UART2_IER_bit.CIR
__IO_REG8_BIT( UART2_IIR,             0x48024008,__READ_WRITE ,__uart_iir_bits);
#define        UART2_FCR              UART2_IIR
#define        UART2_IrDA_IIR         UART2_IIR
#define        UART2_IrDA_IIR_bit     UART2_IIR_bit.IrDA
#define        UART2_CIR_IIR          UART2_IIR
#define        UART2_CIR_IIR_bit      UART2_IIR_bit.CIR
#define        UART2_EFR              UART2_IIR
#define        UART2_EFR_bit          UART2_IIR_bit
__IO_REG8_BIT( UART2_LCR,             0x4802400C,__READ_WRITE ,__uart_lcr_bits);
__IO_REG8_BIT( UART2_MCR,             0x48024010,__READ_WRITE ,__uart_mcr_bits);
#define        UART2_XON1_ADDR1       UART2_MCR
__IO_REG8_BIT( UART2_LSR,             0x48024014,__READ_WRITE ,__uart_lsr_bits);
#define        UART2_XON2_ADDR2       UART2_LSR
#define        UART2_LSR_IrDA         UART2_LSR
#define        UART2_LSR_IrDA_bit     UART2_LSR_bit.IrDA
#define        UART2_LSR_CIR          UART2_LSR
#define        UART2_LSR_CIR_bit      UART2_LSR_bit.CIR
__IO_REG8_BIT( UART2_TCR,             0x48024018,__READ_WRITE ,__uart_tcr_bits);
#define        UART2_XOFF1            UART2_TCR
#define        UART2_MSR              UART2_TCR
#define        UART2_MSR_bit          UART2_TCR_bit
__IO_REG8_BIT( UART2_TLR,             0x4802401C,__READ_WRITE ,__uart_tlr_bits);
#define        UART2_SPR              UART2_TLR
#define        UART2_XOFF2            UART2_TLR
__IO_REG8_BIT( UART2_MDR1,            0x48024020,__READ_WRITE ,__uart_mdr1_bits);
__IO_REG8_BIT( UART2_MDR2,            0x48024024,__READ_WRITE ,__uart_mdr2_bits);
__IO_REG8_BIT( UART2_SFLSR,           0x48024028,__READ_WRITE ,__uart_sflsr_bits);
#define        UART2_TXFLL            UART2_SFLSR
__IO_REG8_BIT( UART2_TXFLH,           0x4802402C,__READ_WRITE ,__uart_txflh_bits);
#define        UART2_RESUME           UART2_TXFLH
__IO_REG8(     UART2_SFREGL,          0x48024030,__READ_WRITE );
#define        UART2_RXFLL            UART2_SFREGL
__IO_REG8_BIT( UART2_SFREGH,          0x48024034,__READ_WRITE ,__uart_sfregh_bits);
#define        UART2_RXFLH            UART2_SFREGH
#define        UART2_RXFLH_bit        UART2_SFREGH_bit
__IO_REG8_BIT( UART2_UASR,            0x48024038,__READ_WRITE ,__uart_uasr_bits);
#define        UART2_BLR              UART2_UASR
#define        UART2_BLR_bit          UART2_UASR_bit
__IO_REG8_BIT( UART2_ACREG,           0x4802403C,__READ_WRITE ,__uart_acreg_bits);
__IO_REG8_BIT( UART2_SCR,             0x48024040,__READ_WRITE ,__uart_scr_bits);
__IO_REG8_BIT( UART2_SSR,             0x48024044,__READ_WRITE ,__uart_ssr_bits);
__IO_REG8(     UART2_EBLR,            0x48024048,__READ_WRITE );
__IO_REG8_BIT( UART2_MVR,             0x48024050,__READ       ,__uart_mvr_bits);
__IO_REG8_BIT( UART2_SYSC,            0x48024054,__READ_WRITE ,__uart_sysc_bits);
__IO_REG8_BIT( UART2_SYSS,            0x48024058,__READ       ,__uart_syss_bits);
__IO_REG8_BIT( UART2_WER,             0x4802405C,__READ_WRITE ,__uart_wer_bits);
__IO_REG8(     UART2_CFPS,            0x48024060,__READ_WRITE );

/***************************************************************************
 **
 ** UART3
 **
 ***************************************************************************/
__IO_REG8(     UART3_DLL,             0x481A6000,__READ_WRITE );
#define        UART3_RHR              UART3_DLL
#define        UART3_THR              UART3_DLL
__IO_REG8_BIT( UART3_IER,             0x481A6004,__READ_WRITE ,__uart_ier_bits);
#define        UART3_DLH              UART3_IER
#define        UART3_IrDA_IER         UART3_IER
#define        UART3_IrDA_IER_bit     UART3_IER_bit.IrDA
#define        UART3_CIR_IER          UART3_IER
#define        UART3_CIR_IER_bit      UART3_IER_bit.CIR
__IO_REG8_BIT( UART3_IIR,             0x481A6008,__READ_WRITE ,__uart_iir_bits);
#define        UART3_FCR              UART3_IIR
#define        UART3_IrDA_IIR         UART3_IIR
#define        UART3_IrDA_IIR_bit     UART3_IIR_bit.IrDA
#define        UART3_CIR_IIR          UART3_IIR
#define        UART3_CIR_IIR_bit      UART3_IIR_bit.CIR
#define        UART3_EFR              UART3_IIR
#define        UART3_EFR_bit          UART3_IIR_bit
__IO_REG8_BIT( UART3_LCR,             0x481A600C,__READ_WRITE ,__uart_lcr_bits);
__IO_REG8_BIT( UART3_MCR,             0x481A6010,__READ_WRITE ,__uart_mcr_bits);
#define        UART3_XON1_ADDR1       UART3_MCR
__IO_REG8_BIT( UART3_LSR,             0x481A6014,__READ_WRITE ,__uart_lsr_bits);
#define        UART3_XON2_ADDR2       UART3_LSR
#define        UART3_LSR_IrDA         UART3_LSR
#define        UART3_LSR_IrDA_bit     UART3_LSR_bit.IrDA
#define        UART3_LSR_CIR          UART3_LSR
#define        UART3_LSR_CIR_bit      UART3_LSR_bit.CIR
__IO_REG8_BIT( UART3_TCR,             0x481A6018,__READ_WRITE ,__uart_tcr_bits);
#define        UART3_XOFF1            UART3_TCR
#define        UART3_MSR              UART3_TCR
#define        UART3_MSR_bit          UART3_TCR_bit
__IO_REG8_BIT( UART3_TLR,             0x481A601C,__READ_WRITE ,__uart_tlr_bits);
#define        UART3_SPR              UART3_TLR
#define        UART3_XOFF2            UART3_TLR
__IO_REG8_BIT( UART3_MDR1,            0x481A6020,__READ_WRITE ,__uart_mdr1_bits);
__IO_REG8_BIT( UART3_MDR2,            0x481A6024,__READ_WRITE ,__uart_mdr2_bits);
__IO_REG8_BIT( UART3_SFLSR,           0x481A6028,__READ_WRITE ,__uart_sflsr_bits);
#define        UART3_TXFLL            UART3_SFLSR
__IO_REG8_BIT( UART3_TXFLH,           0x481A602C,__READ_WRITE ,__uart_txflh_bits);
#define        UART3_RESUME           UART3_TXFLH
__IO_REG8(     UART3_SFREGL,          0x481A6030,__READ_WRITE );
#define        UART3_RXFLL            UART3_SFREGL
__IO_REG8_BIT( UART3_SFREGH,          0x481A6034,__READ_WRITE ,__uart_sfregh_bits);
#define        UART3_RXFLH            UART3_SFREGH
#define        UART3_RXFLH_bit        UART3_SFREGH_bit
__IO_REG8_BIT( UART3_UASR,            0x481A6038,__READ_WRITE ,__uart_uasr_bits);
#define        UART3_BLR              UART3_UASR
#define        UART3_BLR_bit          UART3_UASR_bit
__IO_REG8_BIT( UART3_ACREG,           0x481A603C,__READ_WRITE ,__uart_acreg_bits);
__IO_REG8_BIT( UART3_SCR,             0x481A6040,__READ_WRITE ,__uart_scr_bits);
__IO_REG8_BIT( UART3_SSR,             0x481A6044,__READ_WRITE ,__uart_ssr_bits);
__IO_REG8(     UART3_EBLR,            0x481A6048,__READ_WRITE );
__IO_REG8_BIT( UART3_MVR,             0x481A6050,__READ       ,__uart_mvr_bits);
__IO_REG8_BIT( UART3_SYSC,            0x481A6054,__READ_WRITE ,__uart_sysc_bits);
__IO_REG8_BIT( UART3_SYSS,            0x481A6058,__READ       ,__uart_syss_bits);
__IO_REG8_BIT( UART3_WER,             0x481A605C,__READ_WRITE ,__uart_wer_bits);
__IO_REG8(     UART3_CFPS,            0x481A6060,__READ_WRITE );

/***************************************************************************
 **
 ** UART4
 **
 ***************************************************************************/
__IO_REG8(     UART4_DLL,             0x481A8000,__READ_WRITE );
#define        UART4_RHR              UART4_DLL
#define        UART4_THR              UART4_DLL
__IO_REG8_BIT( UART4_IER,             0x481A8004,__READ_WRITE ,__uart_ier_bits);
#define        UART4_DLH              UART4_IER
#define        UART4_IrDA_IER         UART4_IER
#define        UART4_IrDA_IER_bit     UART4_IER_bit.IrDA
#define        UART4_CIR_IER          UART4_IER
#define        UART4_CIR_IER_bit      UART4_IER_bit.CIR
__IO_REG8_BIT( UART4_IIR,             0x481A8008,__READ_WRITE ,__uart_iir_bits);
#define        UART4_FCR              UART4_IIR
#define        UART4_IrDA_IIR         UART4_IIR
#define        UART4_IrDA_IIR_bit     UART4_IIR_bit.IrDA
#define        UART4_CIR_IIR          UART4_IIR
#define        UART4_CIR_IIR_bit      UART4_IIR_bit.CIR
#define        UART4_EFR              UART4_IIR
#define        UART4_EFR_bit          UART4_IIR_bit
__IO_REG8_BIT( UART4_LCR,             0x481A800C,__READ_WRITE ,__uart_lcr_bits);
__IO_REG8_BIT( UART4_MCR,             0x481A8010,__READ_WRITE ,__uart_mcr_bits);
#define        UART4_XON1_ADDR1       UART4_MCR
__IO_REG8_BIT( UART4_LSR,             0x481A8014,__READ_WRITE ,__uart_lsr_bits);
#define        UART4_XON2_ADDR2       UART4_LSR
#define        UART4_LSR_IrDA         UART4_LSR
#define        UART4_LSR_IrDA_bit     UART4_LSR_bit.IrDA
#define        UART4_LSR_CIR          UART4_LSR
#define        UART4_LSR_CIR_bit      UART4_LSR_bit.CIR
__IO_REG8_BIT( UART4_TCR,             0x481A8018,__READ_WRITE ,__uart_tcr_bits);
#define        UART4_XOFF1            UART4_TCR
#define        UART4_MSR              UART4_TCR
#define        UART4_MSR_bit          UART4_TCR_bit
__IO_REG8_BIT( UART4_TLR,             0x481A801C,__READ_WRITE ,__uart_tlr_bits);
#define        UART4_SPR              UART4_TLR
#define        UART4_XOFF2            UART4_TLR
__IO_REG8_BIT( UART4_MDR1,            0x481A8020,__READ_WRITE ,__uart_mdr1_bits);
__IO_REG8_BIT( UART4_MDR2,            0x481A8024,__READ_WRITE ,__uart_mdr2_bits);
__IO_REG8_BIT( UART4_SFLSR,           0x481A8028,__READ_WRITE ,__uart_sflsr_bits);
#define        UART4_TXFLL            UART4_SFLSR
__IO_REG8_BIT( UART4_TXFLH,           0x481A802C,__READ_WRITE ,__uart_txflh_bits);
#define        UART4_RESUME           UART4_TXFLH
__IO_REG8(     UART4_SFREGL,          0x481A8030,__READ_WRITE );
#define        UART4_RXFLL            UART4_SFREGL
__IO_REG8_BIT( UART4_SFREGH,          0x481A8034,__READ_WRITE ,__uart_sfregh_bits);
#define        UART4_RXFLH            UART4_SFREGH
#define        UART4_RXFLH_bit        UART4_SFREGH_bit
__IO_REG8_BIT( UART4_UASR,            0x481A8038,__READ_WRITE ,__uart_uasr_bits);
#define        UART4_BLR              UART4_UASR
#define        UART4_BLR_bit          UART4_UASR_bit
__IO_REG8_BIT( UART4_ACREG,           0x481A803C,__READ_WRITE ,__uart_acreg_bits);
__IO_REG8_BIT( UART4_SCR,             0x481A8040,__READ_WRITE ,__uart_scr_bits);
__IO_REG8_BIT( UART4_SSR,             0x481A8044,__READ_WRITE ,__uart_ssr_bits);
__IO_REG8(     UART4_EBLR,            0x481A8048,__READ_WRITE );
__IO_REG8_BIT( UART4_MVR,             0x481A8050,__READ       ,__uart_mvr_bits);
__IO_REG8_BIT( UART4_SYSC,            0x481A8054,__READ_WRITE ,__uart_sysc_bits);
__IO_REG8_BIT( UART4_SYSS,            0x481A8058,__READ       ,__uart_syss_bits);
__IO_REG8_BIT( UART4_WER,             0x481A805C,__READ_WRITE ,__uart_wer_bits);
__IO_REG8(     UART4_CFPS,            0x481A8060,__READ_WRITE );

/***************************************************************************
 **
 ** UART5
 **
 ***************************************************************************/
__IO_REG8(     UART5_DLL,             0x481AA000,__READ_WRITE );
#define        UART5_RHR              UART5_DLL
#define        UART5_THR              UART5_DLL
__IO_REG8_BIT( UART5_IER,             0x481AA004,__READ_WRITE ,__uart_ier_bits);
#define        UART5_DLH              UART5_IER
#define        UART5_IrDA_IER         UART5_IER
#define        UART5_IrDA_IER_bit     UART5_IER_bit.IrDA
#define        UART5_CIR_IER          UART5_IER
#define        UART5_CIR_IER_bit      UART5_IER_bit.CIR
__IO_REG8_BIT( UART5_IIR,             0x481AA008,__READ_WRITE ,__uart_iir_bits);
#define        UART5_FCR              UART5_IIR
#define        UART5_IrDA_IIR         UART5_IIR
#define        UART5_IrDA_IIR_bit     UART5_IIR_bit.IrDA
#define        UART5_CIR_IIR          UART5_IIR
#define        UART5_CIR_IIR_bit      UART5_IIR_bit.CIR
#define        UART5_EFR              UART5_IIR
#define        UART5_EFR_bit          UART5_IIR_bit
__IO_REG8_BIT( UART5_LCR,             0x481AA00C,__READ_WRITE ,__uart_lcr_bits);
__IO_REG8_BIT( UART5_MCR,             0x481AA010,__READ_WRITE ,__uart_mcr_bits);
#define        UART5_XON1_ADDR1       UART5_MCR
__IO_REG8_BIT( UART5_LSR,             0x481AA014,__READ_WRITE ,__uart_lsr_bits);
#define        UART5_XON2_ADDR2       UART5_LSR
#define        UART5_LSR_IrDA         UART5_LSR
#define        UART5_LSR_IrDA_bit     UART5_LSR_bit.IrDA
#define        UART5_LSR_CIR          UART5_LSR
#define        UART5_LSR_CIR_bit      UART5_LSR_bit.CIR
__IO_REG8_BIT( UART5_TCR,             0x481AA018,__READ_WRITE ,__uart_tcr_bits);
#define        UART5_XOFF1            UART5_TCR
#define        UART5_MSR              UART5_TCR
#define        UART5_MSR_bit          UART5_TCR_bit
__IO_REG8_BIT( UART5_TLR,             0x481AA01C,__READ_WRITE ,__uart_tlr_bits);
#define        UART5_SPR              UART5_TLR
#define        UART5_XOFF2            UART5_TLR
__IO_REG8_BIT( UART5_MDR1,            0x481AA020,__READ_WRITE ,__uart_mdr1_bits);
__IO_REG8_BIT( UART5_MDR2,            0x481AA024,__READ_WRITE ,__uart_mdr2_bits);
__IO_REG8_BIT( UART5_SFLSR,           0x481AA028,__READ_WRITE ,__uart_sflsr_bits);
#define        UART5_TXFLL            UART5_SFLSR
__IO_REG8_BIT( UART5_TXFLH,           0x481AA02C,__READ_WRITE ,__uart_txflh_bits);
#define        UART5_RESUME           UART5_TXFLH
__IO_REG8(     UART5_SFREGL,          0x481AA030,__READ_WRITE );
#define        UART5_RXFLL            UART5_SFREGL
__IO_REG8_BIT( UART5_SFREGH,          0x481AA034,__READ_WRITE ,__uart_sfregh_bits);
#define        UART5_RXFLH            UART5_SFREGH
#define        UART5_RXFLH_bit        UART5_SFREGH_bit
__IO_REG8_BIT( UART5_UASR,            0x481AA038,__READ_WRITE ,__uart_uasr_bits);
#define        UART5_BLR              UART5_UASR
#define        UART5_BLR_bit          UART5_UASR_bit
__IO_REG8_BIT( UART5_ACREG,           0x481AA03C,__READ_WRITE ,__uart_acreg_bits);
__IO_REG8_BIT( UART5_SCR,             0x481AA040,__READ_WRITE ,__uart_scr_bits);
__IO_REG8_BIT( UART5_SSR,             0x481AA044,__READ_WRITE ,__uart_ssr_bits);
__IO_REG8(     UART5_EBLR,            0x481AA048,__READ_WRITE );
__IO_REG8_BIT( UART5_MVR,             0x481AA050,__READ       ,__uart_mvr_bits);
__IO_REG8_BIT( UART5_SYSC,            0x481AA054,__READ_WRITE ,__uart_sysc_bits);
__IO_REG8_BIT( UART5_SYSS,            0x481AA058,__READ       ,__uart_syss_bits);
__IO_REG8_BIT( UART5_WER,             0x481AA05C,__READ_WRITE ,__uart_wer_bits);
__IO_REG8(     UART5_CFPS,            0x481AA060,__READ_WRITE );

/***************************************************************************
 **
 ** DMTIMER0
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER0_TIDR,           0x44E05000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER0_TIOCP_CFG,      0x44E05010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER0_IRQ_EOI,        0x44E05020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER0_IRQSTATUS_RAW,  0x44E05024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER0_IRQSTATUS,      0x44E05028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER0_IRQENABLE_SET,  0x44E0502C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER0_IRQENABLE_CLR,  0x44E05030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER0_IRQWAKEEN,      0x44E05034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER0_TCLR,           0x44E05038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER0_TCRR,           0x44E0503C,__READ_WRITE );
__IO_REG32(    DMTIMER0_TLDR,           0x44E05040,__READ_WRITE );
__IO_REG32(    DMTIMER0_TTGR,           0x44E05044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER0_TWPS,           0x44E05048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER0_TMAR,           0x44E0504C,__READ_WRITE );
__IO_REG32(    DMTIMER0_TCAR1,          0x44E05050,__READ       );
__IO_REG32_BIT(DMTIMER0_TSICR,          0x44E05054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER0_TCAR2,          0x44E05058,__READ       );

/***************************************************************************
 **
 ** DMTIMER1_1ms
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER1_1MS_TIDR,         0x44E31000,__READ       ,__dmt_1ms_tidr_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TIOCP_CFG,    0x44E31010,__READ_WRITE ,__dmt_1ms_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TISTAT,       0x44E31014,__READ       ,__dmt_1ms_tistat_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TISR,         0x44E31018,__READ_WRITE ,__dmt_1ms_tisr_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TIER,         0x44E3101C,__READ_WRITE ,__dmt_1ms_tier_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TWER,         0x44E31020,__READ_WRITE ,__dmt_1ms_twer_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TCLR,         0x44E31024,__READ_WRITE ,__dmt_1ms_tclr_bits);
__IO_REG32(    DMTIMER1_1MS_TCRR,         0x44E31028,__READ_WRITE );
__IO_REG32(    DMTIMER1_1MS_TLDR,         0x44E3102C,__READ_WRITE );
__IO_REG32(    DMTIMER1_1MS_TTGR,         0x44E31030,__READ_WRITE );
__IO_REG32_BIT(DMTIMER1_1MS_TWPS,         0x44E31034,__READ       ,__dmt_1ms_twps_bits);
__IO_REG32(    DMTIMER1_1MS_TMAR,         0x44E31038,__READ_WRITE );
__IO_REG32(    DMTIMER1_1MS_TCAR1,        0x44E3103C,__READ       );
__IO_REG32_BIT(DMTIMER1_1MS_TSICR,        0x44E31040,__READ_WRITE ,__dmt_1ms_tsicr_bits);
__IO_REG32(    DMTIMER1_1MS_TCAR2,        0x44E31044,__READ       );
__IO_REG32(    DMTIMER1_1MS_TPIR,         0x44E31048,__READ_WRITE );
__IO_REG32(    DMTIMER1_1MS_TNIR,         0x44E3104C,__READ_WRITE );
__IO_REG32(    DMTIMER1_1MS_TCVR,         0x44E31050,__READ_WRITE );
__IO_REG32_BIT(DMTIMER1_1MS_TOCR,         0x44E31054,__READ_WRITE ,__dmt_1ms_tocr_bits);
__IO_REG32_BIT(DMTIMER1_1MS_TOWR,         0x44E31058,__READ_WRITE ,__dmt_1ms_towr_bits);

/***************************************************************************
 **
 ** DMTIMER2
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER2_TIDR,           0x48040000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER2_TIOCP_CFG,      0x48040010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER2_IRQ_EOI,        0x48040020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER2_IRQSTATUS_RAW,  0x48040024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER2_IRQSTATUS,      0x48040028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER2_IRQENABLE_SET,  0x4804002C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER2_IRQENABLE_CLR,  0x48040030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER2_IRQWAKEEN,      0x48040034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER2_TCLR,           0x48040038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER2_TCRR,           0x4804003C,__READ_WRITE );
__IO_REG32(    DMTIMER2_TLDR,           0x48040040,__READ_WRITE );
__IO_REG32(    DMTIMER2_TTGR,           0x48040044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER2_TWPS,           0x48040048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER2_TMAR,           0x4804004C,__READ_WRITE );
__IO_REG32(    DMTIMER2_TCAR1,          0x48040050,__READ       );
__IO_REG32_BIT(DMTIMER2_TSICR,          0x48040054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER2_TCAR2,          0x48040058,__READ       );

/***************************************************************************
 **
 ** DMTIMER3
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER3_TIDR,           0x48042000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER3_TIOCP_CFG,      0x48042010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER3_IRQ_EOI,        0x48042020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER3_IRQSTATUS_RAW,  0x48042024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER3_IRQSTATUS,      0x48042028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER3_IRQENABLE_SET,  0x4804202C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER3_IRQENABLE_CLR,  0x48042030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER3_IRQWAKEEN,      0x48042034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER3_TCLR,           0x48042038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER3_TCRR,           0x4804203C,__READ_WRITE );
__IO_REG32(    DMTIMER3_TLDR,           0x48042040,__READ_WRITE );
__IO_REG32(    DMTIMER3_TTGR,           0x48042044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER3_TWPS,           0x48042048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER3_TMAR,           0x4804204C,__READ_WRITE );
__IO_REG32(    DMTIMER3_TCAR1,          0x48042050,__READ       );
__IO_REG32_BIT(DMTIMER3_TSICR,          0x48042054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER3_TCAR2,          0x48042058,__READ       );

/***************************************************************************
 **
 ** DMTIMER4
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER4_TIDR,           0x48044000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER4_TIOCP_CFG,      0x48044010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER4_IRQ_EOI,        0x48044020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER4_IRQSTATUS_RAW,  0x48044024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER4_IRQSTATUS,      0x48044028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER4_IRQENABLE_SET,  0x4804402C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER4_IRQENABLE_CLR,  0x48044030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER4_IRQWAKEEN,      0x48044034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER4_TCLR,           0x48044038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER4_TCRR,           0x4804403C,__READ_WRITE );
__IO_REG32(    DMTIMER4_TLDR,           0x48044040,__READ_WRITE );
__IO_REG32(    DMTIMER4_TTGR,           0x48044044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER4_TWPS,           0x48044048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER4_TMAR,           0x4804404C,__READ_WRITE );
__IO_REG32(    DMTIMER4_TCAR1,          0x48044050,__READ       );
__IO_REG32_BIT(DMTIMER4_TSICR,          0x48044054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER4_TCAR2,          0x48044058,__READ       );

/***************************************************************************
 **
 ** DMTIMER5
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER5_TIDR,           0x48046000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER5_TIOCP_CFG,      0x48046010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER5_IRQ_EOI,        0x48046020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER5_IRQSTATUS_RAW,  0x48046024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER5_IRQSTATUS,      0x48046028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER5_IRQENABLE_SET,  0x4804602C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER5_IRQENABLE_CLR,  0x48046030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER5_IRQWAKEEN,      0x48046034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER5_TCLR,           0x48046038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER5_TCRR,           0x4804603C,__READ_WRITE );
__IO_REG32(    DMTIMER5_TLDR,           0x48046040,__READ_WRITE );
__IO_REG32(    DMTIMER5_TTGR,           0x48046044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER5_TWPS,           0x48046048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER5_TMAR,           0x4804604C,__READ_WRITE );
__IO_REG32(    DMTIMER5_TCAR1,          0x48046050,__READ       );
__IO_REG32_BIT(DMTIMER5_TSICR,          0x48046054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER5_TCAR2,          0x48046058,__READ       );

/***************************************************************************
 **
 ** DMTIMER6
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER6_TIDR,           0x48048000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER6_TIOCP_CFG,      0x48048010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER6_IRQ_EOI,        0x48048020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER6_IRQSTATUS_RAW,  0x48048024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER6_IRQSTATUS,      0x48048028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER6_IRQENABLE_SET,  0x4804802C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER6_IRQENABLE_CLR,  0x48048030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER6_IRQWAKEEN,      0x48048034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER6_TCLR,           0x48048038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER6_TCRR,           0x4804803C,__READ_WRITE );
__IO_REG32(    DMTIMER6_TLDR,           0x48048040,__READ_WRITE );
__IO_REG32(    DMTIMER6_TTGR,           0x48048044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER6_TWPS,           0x48048048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER6_TMAR,           0x4804804C,__READ_WRITE );
__IO_REG32(    DMTIMER6_TCAR1,          0x48048050,__READ       );
__IO_REG32_BIT(DMTIMER6_TSICR,          0x48048054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER6_TCAR2,          0x48048058,__READ       );

/***************************************************************************
 **
 ** DMTIMER7
 **
 ***************************************************************************/
__IO_REG32_BIT(DMTIMER7_TIDR,           0x4804A000,__READ       ,__dmt_tidr_bits);
__IO_REG32_BIT(DMTIMER7_TIOCP_CFG,      0x4804A010,__READ_WRITE ,__dmt_tiocp_cfg_bits);
__IO_REG32_BIT(DMTIMER7_IRQ_EOI,        0x4804A020,__READ_WRITE ,__dmt_irq_eoi_bits);
__IO_REG32_BIT(DMTIMER7_IRQSTATUS_RAW,  0x4804A024,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER7_IRQSTATUS,      0x4804A028,__READ_WRITE ,__dmt_irqstatus_bits);
__IO_REG32_BIT(DMTIMER7_IRQENABLE_SET,  0x4804A02C,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER7_IRQENABLE_CLR,  0x4804A030,__READ_WRITE ,__dmt_irqenable_bits);
__IO_REG32_BIT(DMTIMER7_IRQWAKEEN,      0x4804A034,__READ_WRITE ,__dmt_irqwakeen_bits);
__IO_REG32_BIT(DMTIMER7_TCLR,           0x4804A038,__READ_WRITE ,__dmt_tclr_bits);
__IO_REG32(    DMTIMER7_TCRR,           0x4804A03C,__READ_WRITE );
__IO_REG32(    DMTIMER7_TLDR,           0x4804A040,__READ_WRITE );
__IO_REG32(    DMTIMER7_TTGR,           0x4804A044,__READ_WRITE );
__IO_REG32_BIT(DMTIMER7_TWPS,           0x4804A048,__READ       ,__dmt_twps_bits);
__IO_REG32(    DMTIMER7_TMAR,           0x4804A04C,__READ_WRITE );
__IO_REG32(    DMTIMER7_TCAR1,          0x4804A050,__READ       );
__IO_REG32_BIT(DMTIMER7_TSICR,          0x4804A054,__READ_WRITE ,__dmt_tsicr_bits);
__IO_REG32(    DMTIMER7_TCAR2,          0x4804A058,__READ       );

/***************************************************************************
 **
 ** RTC
 **
 ***************************************************************************/
__IO_REG32_BIT(RTC_SECONDS,           0x44E3E000,__READ_WRITE ,__rtc_second_bits);
__IO_REG32_BIT(RTC_MINUTES,           0x44E3E004,__READ_WRITE ,__rtc_minute_bits);
__IO_REG32_BIT(RTC_HOURS,             0x44E3E008,__READ_WRITE ,__rtc_hour_bits);
__IO_REG32_BIT(RTC_DAYS,              0x44E3E00C,__READ_WRITE ,__rtc_day_bits);
__IO_REG32_BIT(RTC_MONTHS,            0x44E3E010,__READ_WRITE ,__rtc_month_bits);
__IO_REG32_BIT(RTC_YEARS,             0x44E3E014,__READ_WRITE ,__rtc_year_bits);
__IO_REG32_BIT(RTC_WEEK,              0x44E3E018,__READ_WRITE ,__rtc_dotw_bits);
__IO_REG32_BIT(RTC_ALARM_SECONDS,     0x44E3E020,__READ_WRITE ,__rtc_second_bits);
__IO_REG32_BIT(RTC_ALARM_MINUTES,     0x44E3E024,__READ_WRITE ,__rtc_minute_bits);
__IO_REG32_BIT(RTC_ALARM_HOURS,       0x44E3E028,__READ_WRITE ,__rtc_hour_bits);
__IO_REG32_BIT(RTC_ALARM_DAYS,        0x44E3E02C,__READ_WRITE ,__rtc_day_bits);
__IO_REG32_BIT(RTC_ALARM_MONTHS,      0x44E3E030,__READ_WRITE ,__rtc_month_bits);
__IO_REG32_BIT(RTC_ALARM_YEARS,       0x44E3E034,__READ_WRITE ,__rtc_year_bits);
__IO_REG32_BIT(RTC_CTRL,              0x44E3E040,__READ_WRITE ,__rtc_ctrl_bits);
__IO_REG32_BIT(RTC_STATUS,            0x44E3E044,__READ_WRITE ,__rtc_status_bits);
__IO_REG32_BIT(RTC_INTERRUPT,         0x44E3E048,__READ_WRITE ,__rtc_interrupt_bits);
__IO_REG32_BIT(RTC_COMP_LSB,          0x44E3E04C,__READ_WRITE ,__rtc_complsb_bits);
__IO_REG32_BIT(RTC_COMP_MSB,          0x44E3E050,__READ_WRITE ,__rtc_compmsb_bits);
__IO_REG32_BIT(RTC_OSC,               0x44E3E054,__READ_WRITE ,__rtc_osc_bits);
__IO_REG32(    RTC_SCRATCH0,          0x44E3E060,__READ_WRITE );
__IO_REG32(    RTC_SCRATCH1,          0x44E3E064,__READ_WRITE );
__IO_REG32(    RTC_SCRATCH2,          0x44E3E068,__READ_WRITE );
__IO_REG32(    RTC_KICK0,             0x44E3E06C,__READ_WRITE );
__IO_REG32(    RTC_KICK1,             0x44E3E070,__READ_WRITE );
__IO_REG32_BIT(RTC_REVISION,          0x44E3E074,__READ       ,__rtc_revision_bits);
__IO_REG32_BIT(RTC_SYSCONFIG,         0x44E3E078,__READ_WRITE ,__rtc_sysconfig_bits);
__IO_REG32_BIT(RTC_IRQWAKEEN,         0x44E3E07C,__READ_WRITE ,__rtc_irqwakeen_bits);
__IO_REG32_BIT(RTC_ALARM2_SECONDS,    0x44E3E080,__READ_WRITE ,__rtc_second_bits);
__IO_REG32_BIT(RTC_ALARM2_MINUTES,    0x44E3E084,__READ_WRITE ,__rtc_minute_bits);
__IO_REG32_BIT(RTC_ALARM2_HOURS,      0x44E3E088,__READ_WRITE ,__rtc_hour_bits);
__IO_REG32_BIT(RTC_ALARM2_DAYS,       0x44E3E08C,__READ_WRITE ,__rtc_day_bits);
__IO_REG32_BIT(RTC_ALARM2_MONTHS,     0x44E3E090,__READ_WRITE ,__rtc_month_bits);
__IO_REG32_BIT(RTC_ALARM2_YEARS,      0x44E3E094,__READ_WRITE ,__rtc_year_bits);
__IO_REG32_BIT(RTC_PMIC,              0x44E3E098,__READ_WRITE ,__rtc_pmic_bits);
__IO_REG32_BIT(RTC_DEBOUNCE,          0x44E3E09C,__READ_WRITE ,__rtc_debounce_bits);

/***************************************************************************
 **
 ** WDT1
 **
 ***************************************************************************/
__IO_REG32(    WDT1_WIDR,             0x44E35000,__READ        );
__IO_REG32_BIT(WDT1_WDSC,             0x44E35010,__READ_WRITE  ,__wdt_wdsc_bits);
__IO_REG32_BIT(WDT1_WDST,             0x44E35014,__READ        ,__wdt_wdst_bits);
__IO_REG32_BIT(WDT1_WISR,             0x44E35018,__READ_WRITE  ,__wdt_wisr_bits);
__IO_REG32_BIT(WDT1_WIER,             0x44E3501C,__READ_WRITE  ,__wdt_wier_bits);
__IO_REG32_BIT(WDT1_WCLR,             0x44E35024,__READ_WRITE  ,__wdt_wclr_bits);
__IO_REG32(    WDT1_WCRR,             0x44E35028,__READ_WRITE  );
__IO_REG32(    WDT1_WLDR,             0x44E3502C,__READ_WRITE  );
__IO_REG32(    WDT1_WTGR,             0x44E35030,__READ_WRITE  );
__IO_REG32_BIT(WDT1_WWPS,             0x44E35034,__READ        ,__wdt_wwps_bits);
__IO_REG32(    WDT1_WDLY,             0x44E35044,__READ_WRITE  );
__IO_REG32(    WDT1_WSPR,             0x44E35048,__READ_WRITE  );
__IO_REG32_BIT(WDT1_WIRQSTATRAW,      0x44E35054,__READ_WRITE  ,__wdt_wirqstat_bits);
__IO_REG32_BIT(WDT1_WIRQSTAT,         0x44E35058,__READ_WRITE  ,__wdt_wirqstat_bits);
__IO_REG32_BIT(WDT1_WIRQENSET,        0x44E3505C,__READ_WRITE  ,__wdt_wirqen_bits);
__IO_REG32_BIT(WDT1_WIRQENCLR,        0x44E35060,__READ_WRITE  ,__wdt_wirqen_bits);

/***************************************************************************
 **
 ** I2C0
 **
 ***************************************************************************/
__IO_REG32_BIT(I2C0_REVNB_LO,         0x44E0B000,__READ_WRITE ,__i2c_revnb_lo_bits);
__IO_REG32_BIT(I2C0_REVNB_HI,         0x44E0B004,__READ_WRITE ,__i2c_revnb_hi_bits);
__IO_REG32_BIT(I2C0_SYSC,             0x44E0B010,__READ_WRITE ,__i2c_sysc_bits);
__IO_REG32_BIT(I2C0_EOI,              0x44E0B020,__READ_WRITE ,__i2c_eoi_bits);
__IO_REG32_BIT(I2C0_IRQSTATUS_RAW,    0x44E0B024,__READ_WRITE ,__i2c_irqstatus_bits);
__IO_REG32_BIT(I2C0_IRQSTATUS,        0x44E0B028,__READ_WRITE ,__i2c_irqstatus_bits);
__IO_REG32_BIT(I2C0_IRQENABLE_SET,    0x44E0B02C,__READ_WRITE ,__i2c_irqenable_bits);
__IO_REG32_BIT(I2C0_IRQENABLE_CLR,    0x44E0B030,__READ_WRITE ,__i2c_irqenable_bits);
__IO_REG32_BIT(I2C0_WE,               0x44E0B034,__READ_WRITE ,__i2c_we_bits);
__IO_REG32_BIT(I2C0_DMARXENABLE_SET,  0x44E0B038,__READ_WRITE ,__i2c_dmarxenable_set_bits);
__IO_REG32_BIT(I2C0_DMATXENABLE_SET,  0x44E0B03C,__READ_WRITE ,__i2c_dmatxenable_set_bits);
__IO_REG32_BIT(I2C0_DMARXENABLE_CLR,  0x44E0B040,__READ_WRITE ,__i2c_dmarxenable_clr_bits);
__IO_REG32_BIT(I2C0_DMATXENABLE_CLR,  0x44E0B044,__READ_WRITE ,__i2c_dmatxenable_clr_bits);
__IO_REG32_BIT(I2C0_DMARXWAKE_EN,     0x44E0B048,__READ_WRITE ,__i2c_dmawake_en_bits);
__IO_REG32_BIT(I2C0_DMATXWAKE_EN,     0x44E0B04C,__READ_WRITE ,__i2c_dmawake_en_bits);
__IO_REG32_BIT(I2C0_SYSS,             0x44E0B090,__READ       ,__i2c_syss_bits);
__IO_REG32_BIT(I2C0_BUF,              0x44E0B094,__READ_WRITE ,__i2c_buf_bits);
__IO_REG32_BIT(I2C0_CNT,              0x44E0B098,__READ_WRITE ,__i2c_cnt_bits);
__IO_REG32_BIT(I2C0_DATA,             0x44E0B09C,__READ_WRITE ,__i2c_data_bits);
__IO_REG32_BIT(I2C0_CON,              0x44E0B0A4,__READ_WRITE ,__i2c_con_bits);
__IO_REG32_BIT(I2C0_OA,               0x44E0B0A8,__READ_WRITE ,__i2c_oa_bits);
__IO_REG32_BIT(I2C0_SA,               0x44E0B0AC,__READ_WRITE ,__i2c_sa_bits);
__IO_REG32_BIT(I2C0_PSC,              0x44E0B0B0,__READ_WRITE ,__i2c_psc_bits);
__IO_REG32_BIT(I2C0_SCLL,             0x44E0B0B4,__READ_WRITE ,__i2c_scll_bits);
__IO_REG32_BIT(I2C0_SCLH,             0x44E0B0B8,__READ_WRITE ,__i2c_sclh_bits);
__IO_REG32_BIT(I2C0_SYSTEST,          0x44E0B0BC,__READ_WRITE ,__i2c_systest_bits);
__IO_REG32_BIT(I2C0_BUFSTAT,          0x44E0B0C0,__READ       ,__i2c_bufstat_bits);
__IO_REG32_BIT(I2C0_OA1,              0x44E0B0C4,__READ_WRITE ,__i2c_oa1_bits);
__IO_REG32_BIT(I2C0_OA2,              0x44E0B0C8,__READ_WRITE ,__i2c_oa2_bits);
__IO_REG32_BIT(I2C0_OA3,              0x44E0B0CC,__READ_WRITE ,__i2c_oa3_bits);
__IO_REG32_BIT(I2C0_ACTOA,            0x44E0B0D0,__READ       ,__i2c_actoa_bits);
__IO_REG32_BIT(I2C0_SBLOCK,           0x44E0B0D4,__READ_WRITE ,__i2c_sblock_bits);

/***************************************************************************
 **
 ** I2C1
 **
 ***************************************************************************/
__IO_REG32_BIT(I2C1_REVNB_LO,         0x4802A000,__READ_WRITE ,__i2c_revnb_lo_bits);
__IO_REG32_BIT(I2C1_REVNB_HI,         0x4802A004,__READ_WRITE ,__i2c_revnb_hi_bits);
__IO_REG32_BIT(I2C1_SYSC,             0x4802A010,__READ_WRITE ,__i2c_sysc_bits);
__IO_REG32_BIT(I2C1_EOI,              0x4802A020,__READ_WRITE ,__i2c_eoi_bits);
__IO_REG32_BIT(I2C1_IRQSTATUS_RAW,    0x4802A024,__READ_WRITE ,__i2c_irqstatus_bits);
__IO_REG32_BIT(I2C1_IRQSTATUS,        0x4802A028,__READ_WRITE ,__i2c_irqstatus_bits);
__IO_REG32_BIT(I2C1_IRQENABLE_SET,    0x4802A02C,__READ_WRITE ,__i2c_irqenable_bits);
__IO_REG32_BIT(I2C1_IRQENABLE_CLR,    0x4802A030,__READ_WRITE ,__i2c_irqenable_bits);
__IO_REG32_BIT(I2C1_WE,               0x4802A034,__READ_WRITE ,__i2c_we_bits);
__IO_REG32_BIT(I2C1_DMARXENABLE_SET,  0x4802A038,__READ_WRITE ,__i2c_dmarxenable_set_bits);
__IO_REG32_BIT(I2C1_DMATXENABLE_SET,  0x4802A03C,__READ_WRITE ,__i2c_dmatxenable_set_bits);
__IO_REG32_BIT(I2C1_DMARXENABLE_CLR,  0x4802A040,__READ_WRITE ,__i2c_dmarxenable_clr_bits);
__IO_REG32_BIT(I2C1_DMATXENABLE_CLR,  0x4802A044,__READ_WRITE ,__i2c_dmatxenable_clr_bits);
__IO_REG32_BIT(I2C1_DMARXWAKE_EN,     0x4802A048,__READ_WRITE ,__i2c_dmawake_en_bits);
__IO_REG32_BIT(I2C1_DMATXWAKE_EN,     0x4802A04C,__READ_WRITE ,__i2c_dmawake_en_bits);
__IO_REG32_BIT(I2C1_SYSS,             0x4802A090,__READ       ,__i2c_syss_bits);
__IO_REG32_BIT(I2C1_BUF,              0x4802A094,__READ_WRITE ,__i2c_buf_bits);
__IO_REG32_BIT(I2C1_CNT,              0x4802A098,__READ_WRITE ,__i2c_cnt_bits);
__IO_REG32_BIT(I2C1_DATA,             0x4802A09C,__READ_WRITE ,__i2c_data_bits);
__IO_REG32_BIT(I2C1_CON,              0x4802A0A4,__READ_WRITE ,__i2c_con_bits);
__IO_REG32_BIT(I2C1_OA,               0x4802A0A8,__READ_WRITE ,__i2c_oa_bits);
__IO_REG32_BIT(I2C1_SA,               0x4802A0AC,__READ_WRITE ,__i2c_sa_bits);
__IO_REG32_BIT(I2C1_PSC,              0x4802A0B0,__READ_WRITE ,__i2c_psc_bits);
__IO_REG32_BIT(I2C1_SCLL,             0x4802A0B4,__READ_WRITE ,__i2c_scll_bits);
__IO_REG32_BIT(I2C1_SCLH,             0x4802A0B8,__READ_WRITE ,__i2c_sclh_bits);
__IO_REG32_BIT(I2C1_SYSTEST,          0x4802A0BC,__READ_WRITE ,__i2c_systest_bits);
__IO_REG32_BIT(I2C1_BUFSTAT,          0x4802A0C0,__READ       ,__i2c_bufstat_bits);
__IO_REG32_BIT(I2C1_OA1,              0x4802A0C4,__READ_WRITE ,__i2c_oa1_bits);
__IO_REG32_BIT(I2C1_OA2,              0x4802A0C8,__READ_WRITE ,__i2c_oa2_bits);
__IO_REG32_BIT(I2C1_OA3,              0x4802A0CC,__READ_WRITE ,__i2c_oa3_bits);
__IO_REG32_BIT(I2C1_ACTOA,            0x4802A0D0,__READ       ,__i2c_actoa_bits);
__IO_REG32_BIT(I2C1_SBLOCK,           0x4802A0D4,__READ_WRITE ,__i2c_sblock_bits);

/***************************************************************************
 **
 ** I2C2
 **
 ***************************************************************************/
__IO_REG32_BIT(I2C2_REVNB_LO,         0x4819C000,__READ_WRITE ,__i2c_revnb_lo_bits);
__IO_REG32_BIT(I2C2_REVNB_HI,         0x4819C004,__READ_WRITE ,__i2c_revnb_hi_bits);
__IO_REG32_BIT(I2C2_SYSC,             0x4819C010,__READ_WRITE ,__i2c_sysc_bits);
__IO_REG32_BIT(I2C2_EOI,              0x4819C020,__READ_WRITE ,__i2c_eoi_bits);
__IO_REG32_BIT(I2C2_IRQSTATUS_RAW,    0x4819C024,__READ_WRITE ,__i2c_irqstatus_bits);
__IO_REG32_BIT(I2C2_IRQSTATUS,        0x4819C028,__READ_WRITE ,__i2c_irqstatus_bits);
__IO_REG32_BIT(I2C2_IRQENABLE_SET,    0x4819C02C,__READ_WRITE ,__i2c_irqenable_bits);
__IO_REG32_BIT(I2C2_IRQENABLE_CLR,    0x4819C030,__READ_WRITE ,__i2c_irqenable_bits);
__IO_REG32_BIT(I2C2_WE,               0x4819C034,__READ_WRITE ,__i2c_we_bits);
__IO_REG32_BIT(I2C2_DMARXENABLE_SET,  0x4819C038,__READ_WRITE ,__i2c_dmarxenable_set_bits);
__IO_REG32_BIT(I2C2_DMATXENABLE_SET,  0x4819C03C,__READ_WRITE ,__i2c_dmatxenable_set_bits);
__IO_REG32_BIT(I2C2_DMARXENABLE_CLR,  0x4819C040,__READ_WRITE ,__i2c_dmarxenable_clr_bits);
__IO_REG32_BIT(I2C2_DMATXENABLE_CLR,  0x4819C044,__READ_WRITE ,__i2c_dmatxenable_clr_bits);
__IO_REG32_BIT(I2C2_DMARXWAKE_EN,     0x4819C048,__READ_WRITE ,__i2c_dmawake_en_bits);
__IO_REG32_BIT(I2C2_DMATXWAKE_EN,     0x4819C04C,__READ_WRITE ,__i2c_dmawake_en_bits);
__IO_REG32_BIT(I2C2_SYSS,             0x4819C090,__READ       ,__i2c_syss_bits);
__IO_REG32_BIT(I2C2_BUF,              0x4819C094,__READ_WRITE ,__i2c_buf_bits);
__IO_REG32_BIT(I2C2_CNT,              0x4819C098,__READ_WRITE ,__i2c_cnt_bits);
__IO_REG32_BIT(I2C2_DATA,             0x4819C09C,__READ_WRITE ,__i2c_data_bits);
__IO_REG32_BIT(I2C2_CON,              0x4819C0A4,__READ_WRITE ,__i2c_con_bits);
__IO_REG32_BIT(I2C2_OA,               0x4819C0A8,__READ_WRITE ,__i2c_oa_bits);
__IO_REG32_BIT(I2C2_SA,               0x4819C0AC,__READ_WRITE ,__i2c_sa_bits);
__IO_REG32_BIT(I2C2_PSC,              0x4819C0B0,__READ_WRITE ,__i2c_psc_bits);
__IO_REG32_BIT(I2C2_SCLL,             0x4819C0B4,__READ_WRITE ,__i2c_scll_bits);
__IO_REG32_BIT(I2C2_SCLH,             0x4819C0B8,__READ_WRITE ,__i2c_sclh_bits);
__IO_REG32_BIT(I2C2_SYSTEST,          0x4819C0BC,__READ_WRITE ,__i2c_systest_bits);
__IO_REG32_BIT(I2C2_BUFSTAT,          0x4819C0C0,__READ       ,__i2c_bufstat_bits);
__IO_REG32_BIT(I2C2_OA1,              0x4819C0C4,__READ_WRITE ,__i2c_oa1_bits);
__IO_REG32_BIT(I2C2_OA2,              0x4819C0C8,__READ_WRITE ,__i2c_oa2_bits);
__IO_REG32_BIT(I2C2_OA3,              0x4819C0CC,__READ_WRITE ,__i2c_oa3_bits);
__IO_REG32_BIT(I2C2_ACTOA,            0x4819C0D0,__READ       ,__i2c_actoa_bits);
__IO_REG32_BIT(I2C2_SBLOCK,           0x4819C0D4,__READ_WRITE ,__i2c_sblock_bits);

/***************************************************************************
 **
 ** McASP0
 **
 ***************************************************************************/
__IO_REG32(    McASP0_REV,            0x48038000,__READ       );
__IO_REG32_BIT(McASP0_PFUNC,          0x48038010,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP0_PDIR,           0x48038014,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP0_PDOUT,          0x48038018,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP0_PDIN,           0x4803801C,__READ_WRITE ,__mcasp_pfunc_bits);
#define McASP0_PDSET     McASP0_PDIN
#define McASP0_PDSET_bit McASP0_PDIN_bit
__IO_REG32_BIT(McASP0_PDCLR,          0x48038020,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP0_GBLCTL,         0x48038044,__READ_WRITE ,__mcasp_gblctl_bits);
__IO_REG32_BIT(McASP0_AMUTE,          0x48038048,__READ_WRITE ,__mcasp_amute_bits);
__IO_REG32_BIT(McASP0_DLBCTL,         0x4803804C,__READ_WRITE ,__mcasp_dlbctl_bits);
__IO_REG32_BIT(McASP0_DITCTL,         0x48038050,__READ_WRITE ,__mcasp_ditctl_bits);
__IO_REG32_BIT(McASP0_RGBLCTL,        0x48038060,__READ_WRITE ,__mcasp_rgblctl_bits);
__IO_REG32_BIT(McASP0_RMASK,          0x48038064,__READ_WRITE ,__mcasp_rmask_bits);
__IO_REG32_BIT(McASP0_RFMT,           0x48038068,__READ_WRITE ,__mcasp_rfmt_bits);
__IO_REG32_BIT(McASP0_AFSRCTL,        0x4803806C,__READ_WRITE ,__mcasp_afsrctl_bits);
__IO_REG32_BIT(McASP0_ACLKRCTL,       0x48038070,__READ_WRITE ,__mcasp_aclkrctl_bits);
__IO_REG32_BIT(McASP0_AHCLKRCTL,      0x48038074,__READ_WRITE ,__mcasp_ahclkrctl_bits);
__IO_REG32_BIT(McASP0_RTDM,           0x48038078,__READ_WRITE ,__mcasp_rtdm_bits);
__IO_REG32_BIT(McASP0_RINTCTL,        0x4803807C,__READ_WRITE ,__mcasp_rintctl_bits);
__IO_REG32_BIT(McASP0_RSTAT,          0x48038080,__READ_WRITE ,__mcasp_rstat_bits);
__IO_REG32_BIT(McASP0_RSLOT,          0x48038084,__READ       ,__mcasp_rslot_bits);
__IO_REG32_BIT(McASP0_RCLKCHK,        0x48038088,__READ_WRITE ,__mcasp_rclkchk_bits);
__IO_REG32_BIT(McASP0_REVTCTL,        0x4803808C,__READ_WRITE ,__mcasp_revtctl_bits);
__IO_REG32_BIT(McASP0_XGBLCTL,        0x480380A0,__READ_WRITE ,__mcasp_xgblctl_bits);
__IO_REG32_BIT(McASP0_XMASK,          0x480380A4,__READ_WRITE ,__mcasp_xmask_bits);
__IO_REG32_BIT(McASP0_XFMT,           0x480380A8,__READ_WRITE ,__mcasp_xfmt_bits);
__IO_REG32_BIT(McASP0_AFSXCTL,        0x480380AC,__READ_WRITE ,__mcasp_afsxctl_bits);
__IO_REG32_BIT(McASP0_ACLKXCTL,       0x480380B0,__READ_WRITE ,__mcasp_aclkxctl_bits);
__IO_REG32_BIT(McASP0_AHCLKXCTL,      0x480380B4,__READ_WRITE ,__mcasp_ahclkxctl_bits);
__IO_REG32_BIT(McASP0_XTDM,           0x480380B8,__READ_WRITE ,__mcasp_xtdm_bits);
__IO_REG32_BIT(McASP0_XINTCTL,        0x480380BC,__READ_WRITE ,__mcasp_xintctl_bits);
__IO_REG32_BIT(McASP0_XSTAT,          0x480380C0,__READ_WRITE ,__mcasp_xstat_bits);
__IO_REG32_BIT(McASP0_XSLOT,          0x480380C4,__READ       ,__mcasp_xslot_bits);
__IO_REG32_BIT(McASP0_XCLKCHK,        0x480380C8,__READ_WRITE ,__mcasp_xclkchk_bits);
__IO_REG32_BIT(McASP0_XEVTCTL,        0x480380CC,__READ_WRITE ,__mcasp_xevtctl_bits);
__IO_REG32(    McASP0_DITCSRA0,       0x48038100,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRA1,       0x48038104,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRA2,       0x48038108,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRA3,       0x4803810C,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRA4,       0x48038110,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRA5,       0x48038114,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRB0,       0x48038118,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRB1,       0x4803811C,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRB2,       0x48038120,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRB3,       0x48038124,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRB4,       0x48038128,__READ_WRITE );
__IO_REG32(    McASP0_DITCSRB5,       0x4803812C,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRA0,       0x48038130,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRA1,       0x48038134,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRA2,       0x48038138,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRA3,       0x4803813C,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRA4,       0x48038140,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRA5,       0x48038144,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRB0,       0x48038148,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRB1,       0x4803814C,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRB2,       0x48038150,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRB3,       0x48038154,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRB4,       0x48038158,__READ_WRITE );
__IO_REG32(    McASP0_DITUDRB5,       0x4803815C,__READ_WRITE );
__IO_REG32_BIT(McASP0_SRCTL0,         0x48038180,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32_BIT(McASP0_SRCTL1,         0x48038184,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32_BIT(McASP0_SRCTL2,         0x48038188,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32_BIT(McASP0_SRCTL3,         0x4803818C,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32(    McASP0_XBUF0,          0x48038200,__READ_WRITE );
__IO_REG32(    McASP0_XBUF1,          0x48038204,__READ_WRITE );
__IO_REG32(    McASP0_XBUF2,          0x48038208,__READ_WRITE );
__IO_REG32(    McASP0_XBUF3,          0x4803820C,__READ_WRITE );
__IO_REG32(    McASP0_RBUF0,          0x48038280,__READ_WRITE );
__IO_REG32(    McASP0_RBUF1,          0x48038284,__READ_WRITE );
__IO_REG32(    McASP0_RBUF2,          0x48038288,__READ_WRITE );
__IO_REG32(    McASP0_RBUF3,          0x4803828C,__READ_WRITE );
__IO_REG32_BIT(McASP0_WFIFOCTL,       0x48039000,__READ_WRITE ,__mcasp_wfifoctl_bits);
__IO_REG32_BIT(McASP0_WFIFOSTS,       0x48039004,__READ       ,__mcasp_wfifosts_bits);
__IO_REG32_BIT(McASP0_RFIFOCTL,       0x48039008,__READ_WRITE ,__mcasp_rfifoctl_bits);
__IO_REG32_BIT(McASP0_RFIFOSTS,       0x4803900C,__READ       ,__mcasp_rfifosts_bits);
__IO_REG32(    McASP0_RBUF,           0x46000000,__READ_WRITE );
#define McASP0_XBUF      McASP0_RBUF

/***************************************************************************
 **
 ** McASP1
 **
 ***************************************************************************/
__IO_REG32(    McASP1_REV,            0x4803C000,__READ       );
__IO_REG32_BIT(McASP1_PFUNC,          0x4803C010,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP1_PDIR,           0x4803C014,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP1_PDOUT,          0x4803C018,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP1_PDIN,           0x4803C01C,__READ_WRITE ,__mcasp_pfunc_bits);
#define McASP1_PDSET     McASP1_PDIN
#define McASP1_PDSET_bit McASP1_PDIN_bit
__IO_REG32_BIT(McASP1_PDCLR,          0x4803C020,__READ_WRITE ,__mcasp_pfunc_bits);
__IO_REG32_BIT(McASP1_GBLCTL,         0x4803C044,__READ_WRITE ,__mcasp_gblctl_bits);
__IO_REG32_BIT(McASP1_AMUTE,          0x4803C048,__READ_WRITE ,__mcasp_amute_bits);
__IO_REG32_BIT(McASP1_DLBCTL,         0x4803C04C,__READ_WRITE ,__mcasp_dlbctl_bits);
__IO_REG32_BIT(McASP1_DITCTL,         0x4803C050,__READ_WRITE ,__mcasp_ditctl_bits);
__IO_REG32_BIT(McASP1_RGBLCTL,        0x4803C060,__READ_WRITE ,__mcasp_rgblctl_bits);
__IO_REG32_BIT(McASP1_RMASK,          0x4803C064,__READ_WRITE ,__mcasp_rmask_bits);
__IO_REG32_BIT(McASP1_RFMT,           0x4803C068,__READ_WRITE ,__mcasp_rfmt_bits);
__IO_REG32_BIT(McASP1_AFSRCTL,        0x4803C06C,__READ_WRITE ,__mcasp_afsrctl_bits);
__IO_REG32_BIT(McASP1_ACLKRCTL,       0x4803C070,__READ_WRITE ,__mcasp_aclkrctl_bits);
__IO_REG32_BIT(McASP1_AHCLKRCTL,      0x4803C074,__READ_WRITE ,__mcasp_ahclkrctl_bits);
__IO_REG32_BIT(McASP1_RTDM,           0x4803C078,__READ_WRITE ,__mcasp_rtdm_bits);
__IO_REG32_BIT(McASP1_RINTCTL,        0x4803C07C,__READ_WRITE ,__mcasp_rintctl_bits);
__IO_REG32_BIT(McASP1_RSTAT,          0x4803C080,__READ_WRITE ,__mcasp_rstat_bits);
__IO_REG32_BIT(McASP1_RSLOT,          0x4803C084,__READ       ,__mcasp_rslot_bits);
__IO_REG32_BIT(McASP1_RCLKCHK,        0x4803C088,__READ_WRITE ,__mcasp_rclkchk_bits);
__IO_REG32_BIT(McASP1_REVTCTL,        0x4803C08C,__READ_WRITE ,__mcasp_revtctl_bits);
__IO_REG32_BIT(McASP1_XGBLCTL,        0x4803C0A0,__READ_WRITE ,__mcasp_xgblctl_bits);
__IO_REG32_BIT(McASP1_XMASK,          0x4803C0A4,__READ_WRITE ,__mcasp_xmask_bits);
__IO_REG32_BIT(McASP1_XFMT,           0x4803C0A8,__READ_WRITE ,__mcasp_xfmt_bits);
__IO_REG32_BIT(McASP1_AFSXCTL,        0x4803C0AC,__READ_WRITE ,__mcasp_afsxctl_bits);
__IO_REG32_BIT(McASP1_ACLKXCTL,       0x4803C0B0,__READ_WRITE ,__mcasp_aclkxctl_bits);
__IO_REG32_BIT(McASP1_AHCLKXCTL,      0x4803C0B4,__READ_WRITE ,__mcasp_ahclkxctl_bits);
__IO_REG32_BIT(McASP1_XTDM,           0x4803C0B8,__READ_WRITE ,__mcasp_xtdm_bits);
__IO_REG32_BIT(McASP1_XINTCTL,        0x4803C0BC,__READ_WRITE ,__mcasp_xintctl_bits);
__IO_REG32_BIT(McASP1_XSTAT,          0x4803C0C0,__READ_WRITE ,__mcasp_xstat_bits);
__IO_REG32_BIT(McASP1_XSLOT,          0x4803C0C4,__READ       ,__mcasp_xslot_bits);
__IO_REG32_BIT(McASP1_XCLKCHK,        0x4803C0C8,__READ_WRITE ,__mcasp_xclkchk_bits);
__IO_REG32_BIT(McASP1_XEVTCTL,        0x4803C0CC,__READ_WRITE ,__mcasp_xevtctl_bits);
__IO_REG32(    McASP1_DITCSRA0,       0x4803C100,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRA1,       0x4803C104,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRA2,       0x4803C108,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRA3,       0x4803C10C,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRA4,       0x4803C110,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRA5,       0x4803C114,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRB0,       0x4803C118,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRB1,       0x4803C11C,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRB2,       0x4803C120,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRB3,       0x4803C124,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRB4,       0x4803C128,__READ_WRITE );
__IO_REG32(    McASP1_DITCSRB5,       0x4803C12C,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRA0,       0x4803C130,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRA1,       0x4803C134,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRA2,       0x4803C138,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRA3,       0x4803C13C,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRA4,       0x4803C140,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRA5,       0x4803C144,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRB0,       0x4803C148,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRB1,       0x4803C14C,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRB2,       0x4803C150,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRB3,       0x4803C154,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRB4,       0x4803C158,__READ_WRITE );
__IO_REG32(    McASP1_DITUDRB5,       0x4803C15C,__READ_WRITE );
__IO_REG32_BIT(McASP1_SRCTL0,         0x4803C180,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32_BIT(McASP1_SRCTL1,         0x4803C184,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32_BIT(McASP1_SRCTL2,         0x4803C188,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32_BIT(McASP1_SRCTL3,         0x4803C18C,__READ_WRITE ,__mcasp_srctl_bits);
__IO_REG32(    McASP1_XBUF0,          0x4803C200,__READ_WRITE );
__IO_REG32(    McASP1_XBUF1,          0x4803C204,__READ_WRITE );
__IO_REG32(    McASP1_XBUF2,          0x4803C208,__READ_WRITE );
__IO_REG32(    McASP1_XBUF3,          0x4803C20C,__READ_WRITE );
__IO_REG32(    McASP1_RBUF0,          0x4803C280,__READ_WRITE );
__IO_REG32(    McASP1_RBUF1,          0x4803C284,__READ_WRITE );
__IO_REG32(    McASP1_RBUF2,          0x4803C288,__READ_WRITE );
__IO_REG32(    McASP1_RBUF3,          0x4803C28C,__READ_WRITE );
__IO_REG32_BIT(McASP1_WFIFOCTL,       0x4803D000,__READ_WRITE ,__mcasp_wfifoctl_bits);
__IO_REG32_BIT(McASP1_WFIFOSTS,       0x4803D004,__READ       ,__mcasp_wfifosts_bits);
__IO_REG32_BIT(McASP1_RFIFOCTL,       0x4803D008,__READ_WRITE ,__mcasp_rfifoctl_bits);
__IO_REG32_BIT(McASP1_RFIFOSTS,       0x4803D00C,__READ       ,__mcasp_rfifosts_bits);
__IO_REG32(    McASP1_RBUF,           0x46400000,__READ_WRITE );
#define McASP1_XBUF      McASP1_RBUF

/***************************************************************************
 **
 ** DCAN0 (Controller Area Network)
 **
 ***************************************************************************/
__IO_REG32_BIT(DCAN0CTL,          0x481CC000,__READ_WRITE ,__dcanctl_bits);
__IO_REG32_BIT(DCAN0ES,           0x481CC004,__READ_WRITE ,__dcanes_bits);
#define DCAN0PARITYERR_EOI        DCAN0ES
__IO_REG32_BIT(DCAN0ERRC,         0x481CC008,__READ       ,__dcanerrc_bits);
__IO_REG32_BIT(DCAN0BTR,          0x481CC00C,__READ_WRITE ,__dcanbtr_bits);
__IO_REG32_BIT(DCAN0INT,          0x481CC010,__READ       ,__dcanint_bits);
__IO_REG32_BIT(DCAN0TEST,         0x481CC014,__READ_WRITE ,__dcantest_bits);
__IO_REG32_BIT(DCAN0PERR,         0x481CC01C,__READ       ,__dcanperr_bits);
__IO_REG32(    DCAN0ABOTR,        0x481CC080,__READ_WRITE );
__IO_REG32_BIT(DCAN0TXRQX,        0x481CC084,__READ       ,__dcantxrqx_bits);
__IO_REG32_BIT(DCAN0TXRQ12,       0x481CC088,__READ       ,__dcantxrq12_bits);
__IO_REG32_BIT(DCAN0TXRQ34,       0x481CC08C,__READ       ,__dcantxrq34_bits);
__IO_REG32_BIT(DCAN0TXRQ56,       0x481CC090,__READ       ,__dcantxrq56_bits);
__IO_REG32_BIT(DCAN0TXRQ78,       0x481CC094,__READ       ,__dcantxrq78_bits);
__IO_REG32_BIT(DCAN0NWDATX,       0x481CC098,__READ       ,__dcannwdatx_bits);
__IO_REG32_BIT(DCAN0NWDAT12,      0x481CC09C,__READ       ,__dcannwdat12_bits);
__IO_REG32_BIT(DCAN0NWDAT34,      0x481CC0A0,__READ       ,__dcannwdat34_bits);
__IO_REG32_BIT(DCAN0NWDAT56,      0x481CC0A4,__READ       ,__dcannwdat56_bits);
__IO_REG32_BIT(DCAN0NWDAT78,      0x481CC0A8,__READ       ,__dcannwdat78_bits);
__IO_REG32_BIT(DCAN0INTPNDX,      0x481CC0AC,__READ       ,__dcanintpndx_bits);
__IO_REG32_BIT(DCAN0INTPND12,     0x481CC0B0,__READ       ,__dcanintpnd12_bits);
__IO_REG32_BIT(DCAN0INTPND34,     0x481CC0B4,__READ       ,__dcanintpnd34_bits);
__IO_REG32_BIT(DCAN0INTPND56,     0x481CC0B8,__READ       ,__dcanintpnd56_bits);
__IO_REG32_BIT(DCAN0INTPND78,     0x481CC0BC,__READ       ,__dcanintpnd78_bits);
__IO_REG32_BIT(DCAN0MSGVALX,      0x481CC0C0,__READ       ,__dcanmsgvalx_bits);
__IO_REG32_BIT(DCAN0MSGVAL12,     0x481CC0C4,__READ       ,__dcanmsgval12_bits);
__IO_REG32_BIT(DCAN0MSGVAL34,     0x481CC0C8,__READ       ,__dcanmsgval34_bits);
__IO_REG32_BIT(DCAN0MSGVAL56,     0x481CC0CC,__READ       ,__dcanmsgval56_bits);
__IO_REG32_BIT(DCAN0MSGVAL78,     0x481CC0D0,__READ       ,__dcanmsgval78_bits);
__IO_REG32_BIT(DCAN0INTMUX12,     0x481CC0D8,__READ_WRITE ,__dcanintmux12_bits);
__IO_REG32_BIT(DCAN0INTMUX34,     0x481CC0DC,__READ_WRITE ,__dcanintmux34_bits);
__IO_REG32_BIT(DCAN0INTMUX56,     0x481CC0E0,__READ_WRITE ,__dcanintmux56_bits);
__IO_REG32_BIT(DCAN0INTMUX78,     0x481CC0E4,__READ_WRITE ,__dcanintmux78_bits);
__IO_REG32_BIT(DCAN0IF1CMD,       0x481CC100,__READ_WRITE ,__dcanifcmd_bits);
__IO_REG32_BIT(DCAN0IF1MSK,       0x481CC104,__READ_WRITE ,__dcanifmsk_bits);
__IO_REG32_BIT(DCAN0IF1ARB,       0x481CC108,__READ_WRITE ,__dcanifarb_bits);
__IO_REG32_BIT(DCAN0IF1MCTL,      0x481CC10C,__READ_WRITE ,__dcanifmctl_bits);
__IO_REG32_BIT(DCAN0IF1DATA,      0x481CC110,__READ_WRITE ,__dcanifdata_bits);
__IO_REG32_BIT(DCAN0IF1DATB,      0x481CC114,__READ_WRITE ,__dcanifdatb_bits);
__IO_REG32_BIT(DCAN0IF2CMD,       0x481CC120,__READ_WRITE ,__dcanifcmd_bits);
__IO_REG32_BIT(DCAN0IF2MSK,       0x481CC124,__READ_WRITE ,__dcanifmsk_bits);
__IO_REG32_BIT(DCAN0IF2ARB,       0x481CC128,__READ_WRITE ,__dcanifarb_bits);
__IO_REG32_BIT(DCAN0IF2MCTL,      0x481CC12C,__READ_WRITE ,__dcanifmctl_bits);
__IO_REG32_BIT(DCAN0IF2DATA,      0x481CC130,__READ_WRITE ,__dcanifdata_bits);
__IO_REG32_BIT(DCAN0IF2DATB,      0x481CC134,__READ_WRITE ,__dcanifdatb_bits);
__IO_REG32_BIT(DCAN0IF3OBS,       0x481CC140,__READ_WRITE ,__dcanif3obs_bits);
__IO_REG32_BIT(DCAN0IF3MSK,       0x481CC144,__READ       ,__dcanifmsk_bits);
__IO_REG32_BIT(DCAN0IF3ARB,       0x481CC148,__READ       ,__dcanifarb_bits);
__IO_REG32_BIT(DCAN0IF3MCTL,      0x481CC14C,__READ       ,__dcanifmctl_bits);
__IO_REG32_BIT(DCAN0IF3DATA,      0x481CC150,__READ       ,__dcanifdata_bits);
__IO_REG32_BIT(DCAN0IF3DATB,      0x481CC154,__READ       ,__dcanifdatb_bits);
__IO_REG32_BIT(DCAN0IF3UPD12,     0x481CC160,__READ_WRITE ,__dcanif3upd12_bits);
__IO_REG32_BIT(DCAN0IF3UPD34,     0x481CC164,__READ_WRITE ,__dcanif3upd34_bits);
__IO_REG32_BIT(DCAN0IF3UPD56,     0x481CC168,__READ_WRITE ,__dcanif3upd56_bits);
__IO_REG32_BIT(DCAN0IF3UPD78,     0x481CC16C,__READ_WRITE ,__dcanif3upd78_bits);
__IO_REG32_BIT(DCAN0TIOC,         0x481CC1E0,__READ_WRITE ,__dcantioc_bits);
__IO_REG32_BIT(DCAN0RIOC,         0x481CC1E4,__READ_WRITE ,__dcantioc_bits);

/***************************************************************************
 **
 ** DCAN0 (Controller Area Network)
 **
 ***************************************************************************/
__IO_REG32_BIT(DCAN1CTL,          0x481D0000,__READ_WRITE ,__dcanctl_bits);
__IO_REG32_BIT(DCAN1ES,           0x481D0004,__READ_WRITE ,__dcanes_bits);
#define DCAN1PARITYERR_EOI        DCAN1ES
__IO_REG32_BIT(DCAN1ERRC,         0x481D0008,__READ       ,__dcanerrc_bits);
__IO_REG32_BIT(DCAN1BTR,          0x481D000C,__READ_WRITE ,__dcanbtr_bits);
__IO_REG32_BIT(DCAN1INT,          0x481D0010,__READ       ,__dcanint_bits);
__IO_REG32_BIT(DCAN1TEST,         0x481D0014,__READ_WRITE ,__dcantest_bits);
__IO_REG32_BIT(DCAN1PERR,         0x481D001C,__READ       ,__dcanperr_bits);
__IO_REG32(    DCAN1ABOTR,        0x481D0080,__READ_WRITE );
__IO_REG32_BIT(DCAN1TXRQX,        0x481D0084,__READ       ,__dcantxrqx_bits);
__IO_REG32_BIT(DCAN1TXRQ12,       0x481D0088,__READ       ,__dcantxrq12_bits);
__IO_REG32_BIT(DCAN1TXRQ34,       0x481D008C,__READ       ,__dcantxrq34_bits);
__IO_REG32_BIT(DCAN1TXRQ56,       0x481D0090,__READ       ,__dcantxrq56_bits);
__IO_REG32_BIT(DCAN1TXRQ78,       0x481D0094,__READ       ,__dcantxrq78_bits);
__IO_REG32_BIT(DCAN1NWDATX,       0x481D0098,__READ       ,__dcannwdatx_bits);
__IO_REG32_BIT(DCAN1NWDAT12,      0x481D009C,__READ       ,__dcannwdat12_bits);
__IO_REG32_BIT(DCAN1NWDAT34,      0x481D00A0,__READ       ,__dcannwdat34_bits);
__IO_REG32_BIT(DCAN1NWDAT56,      0x481D00A4,__READ       ,__dcannwdat56_bits);
__IO_REG32_BIT(DCAN1NWDAT78,      0x481D00A8,__READ       ,__dcannwdat78_bits);
__IO_REG32_BIT(DCAN1INTPNDX,      0x481D00AC,__READ       ,__dcanintpndx_bits);
__IO_REG32_BIT(DCAN1INTPND12,     0x481D00B0,__READ       ,__dcanintpnd12_bits);
__IO_REG32_BIT(DCAN1INTPND34,     0x481D00B4,__READ       ,__dcanintpnd34_bits);
__IO_REG32_BIT(DCAN1INTPND56,     0x481D00B8,__READ       ,__dcanintpnd56_bits);
__IO_REG32_BIT(DCAN1INTPND78,     0x481D00BC,__READ       ,__dcanintpnd78_bits);
__IO_REG32_BIT(DCAN1MSGVALX,      0x481D00C0,__READ       ,__dcanmsgvalx_bits);
__IO_REG32_BIT(DCAN1MSGVAL12,     0x481D00C4,__READ       ,__dcanmsgval12_bits);
__IO_REG32_BIT(DCAN1MSGVAL34,     0x481D00C8,__READ       ,__dcanmsgval34_bits);
__IO_REG32_BIT(DCAN1MSGVAL56,     0x481D00CC,__READ       ,__dcanmsgval56_bits);
__IO_REG32_BIT(DCAN1MSGVAL78,     0x481D00D0,__READ       ,__dcanmsgval78_bits);
__IO_REG32_BIT(DCAN1INTMUX12,     0x481D00D8,__READ_WRITE ,__dcanintmux12_bits);
__IO_REG32_BIT(DCAN1INTMUX34,     0x481D00DC,__READ_WRITE ,__dcanintmux34_bits);
__IO_REG32_BIT(DCAN1INTMUX56,     0x481D00E0,__READ_WRITE ,__dcanintmux56_bits);
__IO_REG32_BIT(DCAN1INTMUX78,     0x481D00E4,__READ_WRITE ,__dcanintmux78_bits);
__IO_REG32_BIT(DCAN1IF1CMD,       0x481D0100,__READ_WRITE ,__dcanifcmd_bits);
__IO_REG32_BIT(DCAN1IF1MSK,       0x481D0104,__READ_WRITE ,__dcanifmsk_bits);
__IO_REG32_BIT(DCAN1IF1ARB,       0x481D0108,__READ_WRITE ,__dcanifarb_bits);
__IO_REG32_BIT(DCAN1IF1MCTL,      0x481D010C,__READ_WRITE ,__dcanifmctl_bits);
__IO_REG32_BIT(DCAN1IF1DATA,      0x481D0110,__READ_WRITE ,__dcanifdata_bits);
__IO_REG32_BIT(DCAN1IF1DATB,      0x481D0114,__READ_WRITE ,__dcanifdatb_bits);
__IO_REG32_BIT(DCAN1IF2CMD,       0x481D0120,__READ_WRITE ,__dcanifcmd_bits);
__IO_REG32_BIT(DCAN1IF2MSK,       0x481D0124,__READ_WRITE ,__dcanifmsk_bits);
__IO_REG32_BIT(DCAN1IF2ARB,       0x481D0128,__READ_WRITE ,__dcanifarb_bits);
__IO_REG32_BIT(DCAN1IF2MCTL,      0x481D012C,__READ_WRITE ,__dcanifmctl_bits);
__IO_REG32_BIT(DCAN1IF2DATA,      0x481D0130,__READ_WRITE ,__dcanifdata_bits);
__IO_REG32_BIT(DCAN1IF2DATB,      0x481D0134,__READ_WRITE ,__dcanifdatb_bits);
__IO_REG32_BIT(DCAN1IF3OBS,       0x481D0140,__READ_WRITE ,__dcanif3obs_bits);
__IO_REG32_BIT(DCAN1IF3MSK,       0x481D0144,__READ       ,__dcanifmsk_bits);
__IO_REG32_BIT(DCAN1IF3ARB,       0x481D0148,__READ       ,__dcanifarb_bits);
__IO_REG32_BIT(DCAN1IF3MCTL,      0x481D014C,__READ       ,__dcanifmctl_bits);
__IO_REG32_BIT(DCAN1IF3DATA,      0x481D0150,__READ       ,__dcanifdata_bits);
__IO_REG32_BIT(DCAN1IF3DATB,      0x481D0154,__READ       ,__dcanifdatb_bits);
__IO_REG32_BIT(DCAN1IF3UPD12,     0x481D0160,__READ_WRITE ,__dcanif3upd12_bits);
__IO_REG32_BIT(DCAN1IF3UPD34,     0x481D0164,__READ_WRITE ,__dcanif3upd34_bits);
__IO_REG32_BIT(DCAN1IF3UPD56,     0x481D0168,__READ_WRITE ,__dcanif3upd56_bits);
__IO_REG32_BIT(DCAN1IF3UPD78,     0x481D016C,__READ_WRITE ,__dcanif3upd78_bits);
__IO_REG32_BIT(DCAN1TIOC,         0x481D01E0,__READ_WRITE ,__dcantioc_bits);
__IO_REG32_BIT(DCAN1RIOC,         0x481D01E4,__READ_WRITE ,__dcantioc_bits);

/***************************************************************************
 **
 ** McSPI0
 **
 ***************************************************************************/
__IO_REG32_BIT(MCSPI0_REVISION,             0x48030000,__READ        ,__mcspi_revision_bits);
__IO_REG32_BIT(MCSPI0_SYSCONFIG,            0x48030110,__READ_WRITE  ,__mcspi_sysconfig_bits);
__IO_REG32_BIT(MCSPI0_SYSSTATUS,            0x48030114,__READ        ,__mcspi_sysstatus_bits);
__IO_REG32_BIT(MCSPI0_IRQSTATUS,            0x48030118,__READ_WRITE  ,__mcspi_irqstatus_bits);
__IO_REG32_BIT(MCSPI0_IRQENABLE,            0x4803011C,__READ_WRITE  ,__mcspi_irqenable_bits);
__IO_REG32_BIT(MCSPI0_SYST,                 0x48030124,__READ_WRITE  ,__mcspi_syst_bits);
__IO_REG32_BIT(MCSPI0_MODULCTRL,            0x48030128,__READ_WRITE  ,__mcspi_modulctrl_bits);
__IO_REG32_BIT(MCSPI0_CH0CONF,              0x4803012C,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI0_CH0STAT,              0x48030130,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI0_CH0CTRL,              0x48030134,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI0_TX0,                  0x48030138,__READ_WRITE  );
__IO_REG32(    MCSPI0_RX0,                  0x4803013C,__READ        );
__IO_REG32_BIT(MCSPI0_CH1CONF,              0x48030140,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI0_CH1STAT,              0x48030144,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI0_CH1CTRL,              0x48030148,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI0_TX1,                  0x4803014C,__READ_WRITE  );
__IO_REG32(    MCSPI0_RX1,                  0x48030150,__READ        );
__IO_REG32_BIT(MCSPI0_CH2CONF,              0x48030154,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI0_CH2STAT,              0x48030158,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI0_CH2CTRL,              0x4803015C,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI0_TX2,                  0x48030160,__READ_WRITE  );
__IO_REG32(    MCSPI0_RX2,                  0x48030164,__READ        );
__IO_REG32_BIT(MCSPI0_CH3CONF,              0x48030168,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI0_CH3STAT,              0x4803016C,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI0_CH3CTRL,              0x48030170,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI0_TX3,                  0x48030174,__READ_WRITE  );
__IO_REG32(    MCSPI0_RX3,                  0x48030178,__READ        );
__IO_REG32_BIT(MCSPI0_XFERLEVEL,            0x4803017C,__READ_WRITE  ,__mcspi_xferlevel_bits);
__IO_REG32(    MCSPI0_DAFTX,                0x48030180,__READ_WRITE  );
__IO_REG32(    MCSPI0_DAFRX,                0x48030184,__READ        );

/***************************************************************************
 **
 ** McSPI1
 **
 ***************************************************************************/
__IO_REG32_BIT(MCSPI1_REVISION,             0x481A0000,__READ        ,__mcspi_revision_bits);
__IO_REG32_BIT(MCSPI1_SYSCONFIG,            0x481A0110,__READ_WRITE  ,__mcspi_sysconfig_bits);
__IO_REG32_BIT(MCSPI1_SYSSTATUS,            0x481A0114,__READ        ,__mcspi_sysstatus_bits);
__IO_REG32_BIT(MCSPI1_IRQSTATUS,            0x481A0118,__READ_WRITE  ,__mcspi_irqstatus_bits);
__IO_REG32_BIT(MCSPI1_IRQENABLE,            0x481A011C,__READ_WRITE  ,__mcspi_irqenable_bits);
__IO_REG32_BIT(MCSPI1_SYST,                 0x481A0124,__READ_WRITE  ,__mcspi_syst_bits);
__IO_REG32_BIT(MCSPI1_MODULCTRL,            0x481A0128,__READ_WRITE  ,__mcspi_modulctrl_bits);
__IO_REG32_BIT(MCSPI1_CH0CONF,              0x481A012C,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI1_CH0STAT,              0x481A0130,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI1_CH0CTRL,              0x481A0134,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI1_TX0,                  0x481A0138,__READ_WRITE  );
__IO_REG32(    MCSPI1_RX0,                  0x481A013C,__READ        );
__IO_REG32_BIT(MCSPI1_CH1CONF,              0x481A0140,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI1_CH1STAT,              0x481A0144,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI1_CH1CTRL,              0x481A0148,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI1_TX1,                  0x481A014C,__READ_WRITE  );
__IO_REG32(    MCSPI1_RX1,                  0x481A0150,__READ        );
__IO_REG32_BIT(MCSPI1_CH2CONF,              0x481A0154,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI1_CH2STAT,              0x481A0158,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI1_CH2CTRL,              0x481A015C,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI1_TX2,                  0x481A0160,__READ_WRITE  );
__IO_REG32(    MCSPI1_RX2,                  0x481A0164,__READ        );
__IO_REG32_BIT(MCSPI1_CH3CONF,              0x481A0168,__READ_WRITE  ,__mcspi_chxconf_bits);
__IO_REG32_BIT(MCSPI1_CH3STAT,              0x481A016C,__READ        ,__mcspi_chxstat_bits);
__IO_REG32_BIT(MCSPI1_CH3CTRL,              0x481A0170,__READ_WRITE  ,__mcspi_chxctrl_bits);
__IO_REG32(    MCSPI1_TX3,                  0x481A0174,__READ_WRITE  );
__IO_REG32(    MCSPI1_RX3,                  0x481A0178,__READ        );
__IO_REG32_BIT(MCSPI1_XFERLEVEL,            0x481A017C,__READ_WRITE  ,__mcspi_xferlevel_bits);
__IO_REG32(    MCSPI1_DAFTX,                0x481A0180,__READ_WRITE  );
__IO_REG32(    MCSPI1_DAFRX,                0x481A0184,__READ        );

/***************************************************************************
 **
 ** GPIO0
 **
 ***************************************************************************/
__IO_REG32_BIT(GPIO0_REVISION,              0x44E07000,__READ        ,__gpio_revision_bits);
__IO_REG32_BIT(GPIO0_SYSCONFIG,             0x44E07010,__READ_WRITE  ,__gpio_sysconfig_bits);
__IO_REG32_BIT(GPIO0_GPIO_EOI,              0x44E07020,__READ_WRITE  ,__gpio_eoi_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_RAW_0,       0x44E07024,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_RAW_1,       0x44E07028,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_0,           0x44E0702C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_1,           0x44E07030,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_SET_0,       0x44E07034,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_SET_1,       0x44E07038,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_CLR_0,       0x44E0703C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_IRQSTATUS_CLR_1,       0x44E07040,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO0_SYSSTATUS,             0x44E07114,__READ        ,__gpio_sysstatus_bits);
__IO_REG32_BIT(GPIO0_CTRL,                  0x44E07130,__READ_WRITE  ,__gpio_ctrl_bits);
__IO_REG32_BIT(GPIO0_OE,                    0x44E07134,__READ_WRITE  ,__gpio_oe_bits);
__IO_REG32_BIT(GPIO0_DATAIN,                0x44E07138,__READ        ,__gpio_datain_bits);
__IO_REG32_BIT(GPIO0_DATAOUT,               0x44E0713C,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO0_LEVELDETECT0,          0x44E07140,__READ_WRITE  ,__gpio_leveldetect0_bits);
__IO_REG32_BIT(GPIO0_LEVELDETECT1,          0x44E07144,__READ_WRITE  ,__gpio_leveldetect1_bits);
__IO_REG32_BIT(GPIO0_RISINGDETECT,          0x44E07148,__READ_WRITE  ,__gpio_risingdetect_bits);
__IO_REG32_BIT(GPIO0_FALLINGDETECT,         0x44E0714C,__READ_WRITE  ,__gpio_fallingdetect_bits);
__IO_REG32_BIT(GPIO0_DEBOUNCENABLE,         0x44E07150,__READ_WRITE  ,__gpio_debouncenable_bits);
__IO_REG32_BIT(GPIO0_DEBOUNCINGTIME,        0x44E07154,__READ_WRITE  ,__gpio_debouncingtime_bits);
__IO_REG32_BIT(GPIO0_CLEARDATAOUT,          0x44E07190,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO0_SETDATAOUT,            0x44E07194,__READ_WRITE  ,__gpio_dataout_bits);

/***************************************************************************
 **
 ** GPIO1
 **
 ***************************************************************************/
__IO_REG32_BIT(GPIO1_REVISION,              0x4804C000,__READ        ,__gpio_revision_bits);
__IO_REG32_BIT(GPIO1_SYSCONFIG,             0x4804C010,__READ_WRITE  ,__gpio_sysconfig_bits);
__IO_REG32_BIT(GPIO1_GPIO_EOI,              0x4804C020,__READ_WRITE  ,__gpio_eoi_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_RAW_0,       0x4804C024,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_RAW_1,       0x4804C028,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_0,           0x4804C02C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_1,           0x4804C030,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_SET_0,       0x4804C034,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_SET_1,       0x4804C038,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_CLR_0,       0x4804C03C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_IRQSTATUS_CLR_1,       0x4804C040,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO1_SYSSTATUS,             0x4804C114,__READ        ,__gpio_sysstatus_bits);
__IO_REG32_BIT(GPIO1_CTRL,                  0x4804C130,__READ_WRITE  ,__gpio_ctrl_bits);
__IO_REG32_BIT(GPIO1_OE,                    0x4804C134,__READ_WRITE  ,__gpio_oe_bits);
__IO_REG32_BIT(GPIO1_DATAIN,                0x4804C138,__READ        ,__gpio_datain_bits);
__IO_REG32_BIT(GPIO1_DATAOUT,               0x4804C13C,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO1_LEVELDETECT0,          0x4804C140,__READ_WRITE  ,__gpio_leveldetect0_bits);
__IO_REG32_BIT(GPIO1_LEVELDETECT1,          0x4804C144,__READ_WRITE  ,__gpio_leveldetect1_bits);
__IO_REG32_BIT(GPIO1_RISINGDETECT,          0x4804C148,__READ_WRITE  ,__gpio_risingdetect_bits);
__IO_REG32_BIT(GPIO1_FALLINGDETECT,         0x4804C14C,__READ_WRITE  ,__gpio_fallingdetect_bits);
__IO_REG32_BIT(GPIO1_DEBOUNCENABLE,         0x4804C150,__READ_WRITE  ,__gpio_debouncenable_bits);
__IO_REG32_BIT(GPIO1_DEBOUNCINGTIME,        0x4804C154,__READ_WRITE  ,__gpio_debouncingtime_bits);
__IO_REG32_BIT(GPIO1_CLEARDATAOUT,          0x4804C190,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO1_SETDATAOUT,            0x4804C194,__READ_WRITE  ,__gpio_dataout_bits);

/***************************************************************************
 **
 ** GPIO2
 **
 ***************************************************************************/
__IO_REG32_BIT(GPIO2_REVISION,              0x481AC000,__READ        ,__gpio_revision_bits);
__IO_REG32_BIT(GPIO2_SYSCONFIG,             0x481AC010,__READ_WRITE  ,__gpio_sysconfig_bits);
__IO_REG32_BIT(GPIO2_GPIO_EOI,              0x481AC020,__READ_WRITE  ,__gpio_eoi_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_RAW_0,       0x481AC024,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_RAW_1,       0x481AC028,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_0,           0x481AC02C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_1,           0x481AC030,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_SET_0,       0x481AC034,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_SET_1,       0x481AC038,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_CLR_0,       0x481AC03C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_IRQSTATUS_CLR_1,       0x481AC040,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO2_SYSSTATUS,             0x481AC114,__READ        ,__gpio_sysstatus_bits);
__IO_REG32_BIT(GPIO2_CTRL,                  0x481AC130,__READ_WRITE  ,__gpio_ctrl_bits);
__IO_REG32_BIT(GPIO2_OE,                    0x481AC134,__READ_WRITE  ,__gpio_oe_bits);
__IO_REG32_BIT(GPIO2_DATAIN,                0x481AC138,__READ        ,__gpio_datain_bits);
__IO_REG32_BIT(GPIO2_DATAOUT,               0x481AC13C,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO2_LEVELDETECT0,          0x481AC140,__READ_WRITE  ,__gpio_leveldetect0_bits);
__IO_REG32_BIT(GPIO2_LEVELDETECT1,          0x481AC144,__READ_WRITE  ,__gpio_leveldetect1_bits);
__IO_REG32_BIT(GPIO2_RISINGDETECT,          0x481AC148,__READ_WRITE  ,__gpio_risingdetect_bits);
__IO_REG32_BIT(GPIO2_FALLINGDETECT,         0x481AC14C,__READ_WRITE  ,__gpio_fallingdetect_bits);
__IO_REG32_BIT(GPIO2_DEBOUNCENABLE,         0x481AC150,__READ_WRITE  ,__gpio_debouncenable_bits);
__IO_REG32_BIT(GPIO2_DEBOUNCINGTIME,        0x481AC154,__READ_WRITE  ,__gpio_debouncingtime_bits);
__IO_REG32_BIT(GPIO2_CLEARDATAOUT,          0x481AC190,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO2_SETDATAOUT,            0x481AC194,__READ_WRITE  ,__gpio_dataout_bits);

/***************************************************************************
 **
 ** GPIO3
 **
 ***************************************************************************/
__IO_REG32_BIT(GPIO3_REVISION,              0x481AE000,__READ        ,__gpio_revision_bits);
__IO_REG32_BIT(GPIO3_SYSCONFIG,             0x481AE010,__READ_WRITE  ,__gpio_sysconfig_bits);
__IO_REG32_BIT(GPIO3_GPIO_EOI,              0x481AE020,__READ_WRITE  ,__gpio_eoi_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_RAW_0,       0x481AE024,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_RAW_1,       0x481AE028,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_0,           0x481AE02C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_1,           0x481AE030,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_SET_0,       0x481AE034,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_SET_1,       0x481AE038,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_CLR_0,       0x481AE03C,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_IRQSTATUS_CLR_1,       0x481AE040,__READ_WRITE  ,__gpio_irqstatus_bits);
__IO_REG32_BIT(GPIO3_SYSSTATUS,             0x481AE114,__READ        ,__gpio_sysstatus_bits);
__IO_REG32_BIT(GPIO3_CTRL,                  0x481AE130,__READ_WRITE  ,__gpio_ctrl_bits);
__IO_REG32_BIT(GPIO3_OE,                    0x481AE134,__READ_WRITE  ,__gpio_oe_bits);
__IO_REG32_BIT(GPIO3_DATAIN,                0x481AE138,__READ        ,__gpio_datain_bits);
__IO_REG32_BIT(GPIO3_DATAOUT,               0x481AE13C,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO3_LEVELDETECT0,          0x481AE140,__READ_WRITE  ,__gpio_leveldetect0_bits);
__IO_REG32_BIT(GPIO3_LEVELDETECT1,          0x481AE144,__READ_WRITE  ,__gpio_leveldetect1_bits);
__IO_REG32_BIT(GPIO3_RISINGDETECT,          0x481AE148,__READ_WRITE  ,__gpio_risingdetect_bits);
__IO_REG32_BIT(GPIO3_FALLINGDETECT,         0x481AE14C,__READ_WRITE  ,__gpio_fallingdetect_bits);
__IO_REG32_BIT(GPIO3_DEBOUNCENABLE,         0x481AE150,__READ_WRITE  ,__gpio_debouncenable_bits);
__IO_REG32_BIT(GPIO3_DEBOUNCINGTIME,        0x481AE154,__READ_WRITE  ,__gpio_debouncingtime_bits);
__IO_REG32_BIT(GPIO3_CLEARDATAOUT,          0x481AE190,__READ_WRITE  ,__gpio_dataout_bits);
__IO_REG32_BIT(GPIO3_SETDATAOUT,            0x481AE194,__READ_WRITE  ,__gpio_dataout_bits);

/***************************************************************************
 **
 ** DebugSS_DRM
 **
 ***************************************************************************/
__IO_REG32_BIT(DEBUGSS_DRM_WDT_SCR,         0x4B160200,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER0_SCR,    0x4B160204,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER1_SCR,    0x4B160208,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER2_SCR,    0x4B16020C,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER3_SCR,    0x4B160210,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER4_SCR,    0x4B160214,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER5_SCR,    0x4B160218,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER6_SCR,    0x4B16021C,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_EMAC_SCR,        0x4B160220,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_USB2_SCR,        0x4B160224,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_I2C0_SCR,        0x4B160228,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_I2C1_SCR,        0x4B16022C,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_I2C2_SCR,        0x4B160230,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_EHRPWM0_SCR,     0x4B160234,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_EHRPWM1_SCR,     0x4B160238,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_EHRPWM2_SCR,     0x4B16023C,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_CAN0_SCR,        0x4B160240,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_CAN1_SCR,        0x4B160244,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_PRU_ICSS_SCR,    0x4B160248,__READ_WRITE   ,__debugss_drm_scr_bits);
__IO_REG32_BIT(DEBUGSS_DRM_DMTIMER7_SCR,    0x4B160260,__READ_WRITE   ,__debugss_drm_scr_bits);

/***************************************************************************
 **
 **  Interrupt vector table
 **
 ***************************************************************************/
#define RESETV  0x00  /* Reset                           */
#define UNDEFV  0x04  /* Undefined instruction           */
#define SWIV    0x08  /* Software interrupt              */
#define PABORTV 0x0c  /* Prefetch abort                  */
#define DABORTV 0x10  /* Data abort                      */
#define IRQV    0x18  /* Normal interrupt                */
#define FIQV    0x1c  /* Fast interrupt                  */

/***************************************************************************
 **
 ** Interrupt Mapping to the MPU Subsystem
 **
 ***************************************************************************/
#define INT_EMUINT                  0
#define INT_COMMTX                  1
#define INT_COMMRX                  2
#define INT_BENCH                   3
#define INT_ELM_IRQ                 4
#define INT_NMI                     7
#define INT_L3DEBUG                 9
#define INT_L3APPINT                10
#define INT_PRCMINT                 11
#define INT_EDMACOMPINT             12
#define INT_EDMAMPERR               13
#define INT_EDMAERRINT              14
#define INT_ADC_TSC_GENINT          16
#define INT_USBSSINT                17
#define INT_USBINT0                 18
#define INT_USBINT1                 19
#define INT_PRUSS1_EVTOUT0          20
#define INT_PRUSS1_EVTOUT1          21
#define INT_PRUSS1_EVTOUT2          22
#define INT_PRUSS1_EVTOUT3          23
#define INT_PRUSS1_EVTOUT4          24
#define INT_PRUSS1_EVTOUT5          25
#define INT_PRUSS1_EVTOUT6          26
#define INT_PRUSS1_EVTOUT7          27
#define INT_MMCSD1INT               28
#define INT_MMCSD2INT               29
#define INT_I2C2INT                 30
#define INT_ECAP0INT                31
#define INT_GPIOINT2A               32
#define INT_GPIOINT2B               33
#define INT_USBWAKEUP               34
#define INT_LCDCINT                 36
#define INT_GFXINT                  37
#define INT_EPWM2INT                39
#define INT_3PGSWRXTHR0             40
#define INT_3PGSWRXINT0             41
#define INT_3PGSWTXINT0             42
#define INT_3PGSWMISC0              43
#define INT_UART3INT                44
#define INT_UART4INT                45
#define INT_UART5INT                46
#define INT_ECAP1INT                47
#define INT_DCAN0_INT0              52
#define INT_DCAN0_INT1              53
#define INT_DCAN0_PARITY            54
#define INT_DCAN1_INT0              55
#define INT_DCAN1_INT1              56
#define INT_DCAN1_PARITY            57
#define INT_EPWM0_TZINT             58
#define INT_EPWM1_TZINT             59
#define INT_EPWM2_TZINT             60
#define INT_ECAP2INT                61
#define INT_GPIOINT3A               62
#define INT_GPIOINT3B               63
#define INT_MMCSD0INT               64
#define INT_MCSPI0INT               65
#define INT_TINT0                   66
#define INT_TINT1_1MS               67
#define INT_TINT2                   68
#define INT_TINT3                   69
#define INT_I2C0INT                 70
#define INT_I2C1INT                 71
#define INT_UART0INT                72
#define INT_UART1INT                73
#define INT_UART2INT                74
#define INT_RTCINT                  75
#define INT_RTCALARMINT             76
#define INT_MBINT0                  77
#define INT_M3_TXEV                 78
#define INT_EQEP0INT                79
#define INT_MCATXINT0               80
#define INT_MCARXINT0               81
#define INT_MCATXINT1               82
#define INT_MCARXINT1               83
#define INT_EPWM0INT                86
#define INT_EPWM1INT                87
#define INT_EQEP1INT                88
#define INT_EQEP2INT                89
#define INT_DMA_INTR_PIN2           90
#define INT_WDT1INT                 91
#define INT_TINT4                   92
#define INT_TINT5                   93
#define INT_TINT6                   94
#define INT_TINT7                   95
#define INT_GPIOINT0A               96
#define INT_GPIOINT0B               97
#define INT_GPIOINT1A               98
#define INT_GPIOINT1B               99
#define INT_GPMCINT                 100
#define INT_DDRERR0                 101
#define INT_TCERRINT0               112
#define INT_TCERRINT1               113
#define INT_TCERRINT2               114
#define INT_ADC_TSC_PENINT          115
#define INT_SMRFLX_MPU              120
#define INT_SMRFLX_CORE             121
#define INT_DMA_INTR_PIN0           123
#define INT_DMA_INTR_PIN1           124
#define INT_MCSPI1INT               125

#endif /* __IOAM3358_H */
