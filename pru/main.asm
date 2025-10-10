;******************************************************************************
;* PRU C/C++ Codegen                                              Unix v2.3.3 *
;* Date/Time created: Fri Oct 10 16:19:48 2025                                *
;******************************************************************************
	.compiler_opts --abi=eabi --diag_wrap=off --endian=little --hll_source=on --object_format=elf --silicon_version=3 --symdebug:dwarf --symdebug:dwarf_version=3 

$C$DW$CU	.dwtag  DW_TAG_compile_unit
	.dwattr $C$DW$CU, DW_AT_name("main.c")
	.dwattr $C$DW$CU, DW_AT_producer("TI PRU C/C++ Codegen Unix v2.3.3 Copyright (c) 2012-2018 Texas Instruments Incorporated")
	.dwattr $C$DW$CU, DW_AT_TI_version(0x01)
	.dwattr $C$DW$CU, DW_AT_comp_dir("/Volumes/Dev/pseudo-jit/pru")
	.global	__PRU_CREG_PRU_CFG
	.weak	||CT_CFG||
||CT_CFG||:	.usect	".creg.PRU_CFG.noload.near",68,1
$C$DW$1	.dwtag  DW_TAG_variable
	.dwattr $C$DW$1, DW_AT_name("CT_CFG")
	.dwattr $C$DW$1, DW_AT_TI_symbol_name("CT_CFG")
	.dwattr $C$DW$1, DW_AT_location[DW_OP_addr ||CT_CFG||]
	.dwattr $C$DW$1, DW_AT_type(*$C$DW$T$64)
	.dwattr $C$DW$1, DW_AT_external
	.dwattr $C$DW$1, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$1, DW_AT_decl_line(0xf2)
	.dwattr $C$DW$1, DW_AT_decl_column(0x17)

	.global	||smem||
	.data
	.align	1
	.elfsym	||smem||,SYM_SIZE(4)
||smem||:
	.bits		0x10000,32
			; smem @ 0

$C$DW$2	.dwtag  DW_TAG_variable
	.dwattr $C$DW$2, DW_AT_name("smem")
	.dwattr $C$DW$2, DW_AT_TI_symbol_name("smem")
	.dwattr $C$DW$2, DW_AT_location[DW_OP_addr ||smem||]
	.dwattr $C$DW$2, DW_AT_type(*$C$DW$T$68)
	.dwattr $C$DW$2, DW_AT_external
	.dwattr $C$DW$2, DW_AT_decl_file("main.c")
	.dwattr $C$DW$2, DW_AT_decl_line(0x14)
	.dwattr $C$DW$2, DW_AT_decl_column(0x14)

;	/Applications/ti/ti-cgt-pru_2.3.3/bin/acpiapru -@/var/tmp/TI8bsCxaoxv 
	.sect	".text:main"
	.clink
	.global	||main||

$C$DW$3	.dwtag  DW_TAG_subprogram
	.dwattr $C$DW$3, DW_AT_name("main")
	.dwattr $C$DW$3, DW_AT_low_pc(||main||)
	.dwattr $C$DW$3, DW_AT_high_pc(0x00)
	.dwattr $C$DW$3, DW_AT_TI_symbol_name("main")
	.dwattr $C$DW$3, DW_AT_external
	.dwattr $C$DW$3, DW_AT_TI_begin_file("main.c")
	.dwattr $C$DW$3, DW_AT_TI_begin_line(0x23)
	.dwattr $C$DW$3, DW_AT_TI_begin_column(0x06)
	.dwattr $C$DW$3, DW_AT_decl_file("main.c")
	.dwattr $C$DW$3, DW_AT_decl_line(0x23)
	.dwattr $C$DW$3, DW_AT_decl_column(0x06)
	.dwattr $C$DW$3, DW_AT_TI_max_frame_size(0x04)
	.dwpsn	file "main.c",line 35,column 17,is_stmt,address ||main||,isa 0

	.dwfde $C$DW$CIE, ||main||
;----------------------------------------------------------------------
;  35 | void main(void) {                                                      
;----------------------------------------------------------------------

;***************************************************************
;* FNAME: main                          FR SIZE:   4           *
;*                                                             *
;* FUNCTION ENVIRONMENT                                        *
;*                                                             *
;* FUNCTION PROPERTIES                                         *
;*                            4 Auto,  0 SOE     *
;***************************************************************

||main||:
;* --------------------------------------------------------------------------*
	.dwcfi	cfa_offset, 0
        SUB       r2, r2, 0x04          ; [ALU_PRU] 
	.dwcfi	cfa_offset, 4
$C$DW$4	.dwtag  DW_TAG_variable
	.dwattr $C$DW$4, DW_AT_name("smem")
	.dwattr $C$DW$4, DW_AT_TI_symbol_name("smem")
	.dwattr $C$DW$4, DW_AT_type(*$C$DW$T$68)
	.dwattr $C$DW$4, DW_AT_location[DW_OP_breg8 0]

        NOP                             ; [ALU_PRU] 
$C$DW$5	.dwtag  DW_TAG_label
	.dwattr $C$DW$5, DW_AT_name("state0")
	.dwattr $C$DW$5, DW_AT_low_pc(||$C$L6||)
	.dwattr $C$DW$5, DW_AT_TI_symbol_name("state0")

        NOP                             ; [ALU_PRU] 
$C$DW$6	.dwtag  DW_TAG_label
	.dwattr $C$DW$6, DW_AT_name("state1")
	.dwattr $C$DW$6, DW_AT_low_pc(||$C$L9||)
	.dwattr $C$DW$6, DW_AT_TI_symbol_name("state1")

        NOP                             ; [ALU_PRU] 
$C$DW$7	.dwtag  DW_TAG_label
	.dwattr $C$DW$7, DW_AT_name("state2")
	.dwattr $C$DW$7, DW_AT_low_pc(||$C$L10||)
	.dwattr $C$DW$7, DW_AT_TI_symbol_name("state2")

        NOP                             ; [ALU_PRU] 
$C$DW$8	.dwtag  DW_TAG_label
	.dwattr $C$DW$8, DW_AT_name("state3")
	.dwattr $C$DW$8, DW_AT_low_pc(||$C$L14||)
	.dwattr $C$DW$8, DW_AT_TI_symbol_name("state3")

        NOP                             ; [ALU_PRU] 
$C$DW$9	.dwtag  DW_TAG_label
	.dwattr $C$DW$9, DW_AT_name("state4")
	.dwattr $C$DW$9, DW_AT_low_pc(||$C$L19||)
	.dwattr $C$DW$9, DW_AT_TI_symbol_name("state4")

        NOP                             ; [ALU_PRU] 
$C$DW$10	.dwtag  DW_TAG_label
	.dwattr $C$DW$10, DW_AT_name("state5")
	.dwattr $C$DW$10, DW_AT_low_pc(||$C$L23||)
	.dwattr $C$DW$10, DW_AT_TI_symbol_name("state5")

        NOP                             ; [ALU_PRU] 
$C$DW$11	.dwtag  DW_TAG_label
	.dwattr $C$DW$11, DW_AT_name("state6")
	.dwattr $C$DW$11, DW_AT_low_pc(||$C$L24||)
	.dwattr $C$DW$11, DW_AT_TI_symbol_name("state6")

        NOP                             ; [ALU_PRU] 
$C$DW$12	.dwtag  DW_TAG_label
	.dwattr $C$DW$12, DW_AT_name("state7")
	.dwattr $C$DW$12, DW_AT_low_pc(||$C$L28||)
	.dwattr $C$DW$12, DW_AT_TI_symbol_name("state7")

        NOP                             ; [ALU_PRU] 
;* r14_0 assigned to e_counter
$C$DW$13	.dwtag  DW_TAG_variable
	.dwattr $C$DW$13, DW_AT_name("e_counter")
	.dwattr $C$DW$13, DW_AT_TI_symbol_name("e_counter")
	.dwattr $C$DW$13, DW_AT_type(*$C$DW$T$67)
	.dwattr $C$DW$13, DW_AT_location[DW_OP_regx 0x38]

        NOP                             ; [ALU_PRU] 
;* r0_0  assigned to _in
$C$DW$14	.dwtag  DW_TAG_variable
	.dwattr $C$DW$14, DW_AT_name("_in")
	.dwattr $C$DW$14, DW_AT_TI_symbol_name("_in")
	.dwattr $C$DW$14, DW_AT_type(*$C$DW$T$32)
	.dwattr $C$DW$14, DW_AT_location[DW_OP_reg0]

        NOP                             ; [ALU_PRU] 
	.dwpsn	file "main.c",line 36,column 32,is_stmt,isa 0
;----------------------------------------------------------------------
;  36 | register uint8_t e_counter = 0;                                        
;  37 | register uint32_t _in;                                                 
;----------------------------------------------------------------------
        LDI       r14.b0, 0x00          ; [ALU_PRU] |36| e_counter
	.dwpsn	file "main.c",line 40,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  40 | CT_CFG.GPCFG0 = 0x0000;                                                
;----------------------------------------------------------------------
        LDI       r0, 0x0000            ; [ALU_PRU] |40| 
        SBCO      &r0, __PRU_CREG_PRU_CFG, $CSBREL(||CT_CFG||+8), 4 ; [ALU_PRU] |40| CT_CFG
	.dwpsn	file "main.c",line 42,column 29,is_stmt,isa 0
;----------------------------------------------------------------------
;  42 | volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;     
;----------------------------------------------------------------------
        LDI32     r1, 0x00010000        ; [ALU_PRU] |42| 
        SBBO      &r1, r2, 0, 4         ; [ALU_PRU] |42| smem
	.dwpsn	file "main.c",line 43,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  43 | smem[0] = 0;                                                           
;----------------------------------------------------------------------
        SBBO      &r0, r1, 0, 4         ; [ALU_PRU] |43| 
	.dwpsn	file "main.c",line 44,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  44 | smem[1] = 0;                                                           
;----------------------------------------------------------------------
        LBBO      &r1, r2, 0, 4         ; [ALU_PRU] |44| smem
        SBBO      &r0, r1, 4, 4         ; [ALU_PRU] |44| 
	.dwpsn	file "main.c",line 45,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  45 | smem[2] = 0;                                                           
;  47 | // Count edges until main tells us to stop                             
;----------------------------------------------------------------------
        LBBO      &r1, r2, 0, 4         ; [ALU_PRU] |45| smem
        SBBO      &r0, r1, 8, 4         ; [ALU_PRU] |45| 
	.dwpsn	file "main.c",line 48,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  48 | while (smem[0] == 0) {                                                 
;----------------------------------------------------------------------
        JMP       ||$C$L3||             ; [ALU_PRU] |48| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L1||
;*
;*   Loop source line                : 49
;*   Loop closing brace source line  : 49
;*   Known Minimum Trip Count        : 1
;*   Known Maximum Trip Count        : 4294967295
;*   Known Max Trip Count Factor     : 1
;* --------------------------------------------------------------------------*
||$C$L1||:    
	.dwpsn	file "main.c",line 49,column 9,is_stmt,isa 0
;----------------------------------------------------------------------
;  49 | wait_high();                                                           
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |49| _in
        QBBC      ||$C$L1||, r0, 0x04   ; [ALU_PRU] |49| _in
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L2||
;*
;*   Loop source line                : 50
;*   Loop closing brace source line  : 50
;*   Known Minimum Trip Count        : 1
;*   Known Maximum Trip Count        : 4294967295
;*   Known Max Trip Count Factor     : 1
;* --------------------------------------------------------------------------*
||$C$L2||:    
	.dwpsn	file "main.c",line 50,column 9,is_stmt,isa 0
;----------------------------------------------------------------------
;  50 | wait_low();                                                            
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |50| _in
        QBBS      ||$C$L2||, r0, 0x04   ; [ALU_PRU] |50| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 51,column 9,is_stmt,isa 0
;----------------------------------------------------------------------
;  51 | smem[2]++;                                                             
;----------------------------------------------------------------------
        LBBO      &r0, r1, 8, 4         ; [ALU_PRU] |51| 
        ADD       r0, r0, 0x01          ; [ALU_PRU] |51| 
        SBBO      &r0, r1, 8, 4         ; [ALU_PRU] |51| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L3||
;* --------------------------------------------------------------------------*
||$C$L3||:    
	.dwpsn	file "main.c",line 48,column 12,is_stmt,isa 0
        LBBO      &r1, r2, 0, 4         ; [ALU_PRU] |48| smem
        LBBO      &r0, r1, 0, 4         ; [ALU_PRU] |48| 
        QBEQ      ||$C$L1||, r0, 0x00   ; [ALU_PRU] |48| 
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 54,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  54 | if (smem[0] > 10) smem[0] = 10; else if (smem[0] < 7) smem[0] = 7;     
;----------------------------------------------------------------------
        LBBO      &r0, r1, 0, 4         ; [ALU_PRU] |54| 
        QBGE      ||$C$L4||, r0, 0x0a   ; [ALU_PRU] |54| 
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 54,column 23,is_stmt,isa 0
        LDI       r0, 0x000a            ; [ALU_PRU] |54| 
        SBBO      &r0, r1, 0, 4         ; [ALU_PRU] |54| 
        JMP       ||$C$L5||             ; [ALU_PRU] |54| 
;* --------------------------------------------------------------------------*
||$C$L4||:    
	.dwpsn	file "main.c",line 54,column 42,is_stmt,isa 0
        LBBO      &r0, r1, 0, 4         ; [ALU_PRU] |54| 
        QBLE      ||$C$L5||, r0, 0x07   ; [ALU_PRU] |54| 
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 54,column 59,is_stmt,isa 0
        LDI       r0, 0x0007            ; [ALU_PRU] |54| 
        SBBO      &r0, r1, 0, 4         ; [ALU_PRU] |54| 
;* --------------------------------------------------------------------------*
||$C$L5||:    
	.dwpsn	file "main.c",line 55,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  55 | smem[1] = smem[0];                                                     
;----------------------------------------------------------------------
        LBBO      &r0, r2, 0, 4         ; [ALU_PRU] |55| smem
        LBBO      &r1, r0, 0, 4         ; [ALU_PRU] |55| 
        SBBO      &r1, r0, 4, 4         ; [ALU_PRU] |55| 
	.dwpsn	file "main.c",line 57,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  57 | __R30 = VMAn;                                                          
;  59 | state0:                                                                
;----------------------------------------------------------------------
        LDI       r30, 0x0001           ; [ALU_PRU] |57| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L6||
;* --------------------------------------------------------------------------*
||$C$L6||:    
	.dwpsn	file "main.c",line 60,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  60 | wait_low();                                                            
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |60| _in
        QBBS      ||$C$L6||, r0, 0x04   ; [ALU_PRU] |60| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 61,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  61 | do_e_bus();                                                            
;  62 | // should be asserted by GPMC here                                     
;  63 | // M68K_RW_n <= 1'b1;                                                  
;  66 | state1:                                                                
;----------------------------------------------------------------------
        QBNE      ||$C$L7||, r14.b0, 0x09 ; [ALU_PRU] |61| e_counter
;* --------------------------------------------------------------------------*
        CLR       r30, r30, 0x00000007  ; [ALU_PRU] |61| 
        LDI       r14.b0, 0x00          ; [ALU_PRU] |61| e_counter
        JMP       ||$C$L9||             ; [ALU_PRU] |61| 
;* --------------------------------------------------------------------------*
||$C$L7||:    
        QBNE      ||$C$L8||, r14.b0, 0x05 ; [ALU_PRU] |61| e_counter
;* --------------------------------------------------------------------------*
        SET       r30, r30, 0x00000007  ; [ALU_PRU] |61| 
;* --------------------------------------------------------------------------*
||$C$L8||:    
        ADD       r14.b0, r14.b0, 0x01  ; [ALU_PRU] |61| e_counter,e_counter
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L9||
;* --------------------------------------------------------------------------*
||$C$L9||:    
	.dwpsn	file "main.c",line 67,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  67 | wait_high();                                                           
;  68 | // ***PROBLEM, AS_n gets read here about 35+ns *after* GPMC asserts    
;  69 | // the + here is whatever our PRU latency & jitter is (??)             
;  70 | // we should delay AS_n as much as possible and assert OE and WE       
;  71 | // early and echo the logical AND of these                             
;  72 | // e.g.,                                                               
;  73 | //   _   _   _   _   _   _                                             
;  74 | // _| |_| |_| |_| |_| |_|   CLK                                        
;  75 | // ______           ______                                             
;  76 | //       \\________//       "Real" ASn (35ns pD)                       
;  77 | // _____           _______                                             
;  78 | //      \_________/         OEn/WEn (35ns early)                       
;  79 | // ______           ______                                             
;  80 | //       \_________/        "Fake" ASn (35ns pD)                       
;  82 | // if (!(_in & ASn)) __R30 &= ~EWAITn;                                 
;  84 | state2:                                                                
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |67| _in
        QBBC      ||$C$L9||, r0, 0x04   ; [ALU_PRU] |67| _in
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L10||
;*
;*   Loop source line                : 67
;*   Loop closing brace source line  : 67
;*   Known Minimum Trip Count        : 1
;*   Known Maximum Trip Count        : 4294967295
;*   Known Max Trip Count Factor     : 1
;* --------------------------------------------------------------------------*
||$C$L10||:    
	.dwpsn	file "main.c",line 85,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  85 | wait_low();                                                            
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |85| _in
        QBBS      ||$C$L10||, r0, 0x04  ; [ALU_PRU] |85| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 86,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  86 | do_e_bus();                                                            
;  87 | // Note: the 68020 asserts DS at the same time as AS                   
;  88 | // regardless of R/W and this is never corrected on                    
;  89 | // accelerator hardware; however, if we want a delay                   
;  90 | // in DS that's in the GreenPAK on falling edge of CLK                 
;  92 | // should be asserted by GPMC here                                     
;  93 | // M68K_RW based on r/w                                                
;  94 | // M68K_UDS_n read;                                                    
;  95 | // M68K_LDS_n read;                                                    
;----------------------------------------------------------------------
        QBNE      ||$C$L11||, r14.b0, 0x09 ; [ALU_PRU] |86| e_counter
;* --------------------------------------------------------------------------*
        CLR       r30, r30, 0x00000007  ; [ALU_PRU] |86| 
        LDI       r14.b0, 0x00          ; [ALU_PRU] |86| e_counter
        JMP       ||$C$L13||            ; [ALU_PRU] |86| 
;* --------------------------------------------------------------------------*
||$C$L11||:    
        QBNE      ||$C$L12||, r14.b0, 0x05 ; [ALU_PRU] |86| e_counter
;* --------------------------------------------------------------------------*
        SET       r30, r30, 0x00000007  ; [ALU_PRU] |86| 
;* --------------------------------------------------------------------------*
||$C$L12||:    
        ADD       r14.b0, r14.b0, 0x01  ; [ALU_PRU] |86| e_counter,e_counter
;* --------------------------------------------------------------------------*
||$C$L13||:    
	.dwpsn	file "main.c",line 97,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
;  97 | if (_in & ASn) goto state1;                                            
;  99 | state3:                                                                
;----------------------------------------------------------------------
        QBBC      ||$C$L14||, r0, 0x0d  ; [ALU_PRU] |97| _in
;* --------------------------------------------------------------------------*
        JMP       ||$C$L9||             ; [ALU_PRU] |97| 
;* --------------------------------------------------------------------------*
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L14||
;* --------------------------------------------------------------------------*
||$C$L14||:    
	.dwpsn	file "main.c",line 100,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 100 | wait_high();                                                           
; 102 | // We proceed when either VMA or DTACK are asserted                    
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |100| _in
        QBBC      ||$C$L14||, r0, 0x04  ; [ALU_PRU] |100| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 103,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 103 | if (!(_in & DTACKn) || (!(__R30 & VMAn) && (e_counter == 8))) {        
; 104 |     goto state4;                                                       
;----------------------------------------------------------------------
        QBBC      ||$C$L19||, r0, 0x0c  ; [ALU_PRU] |103| _in
;* --------------------------------------------------------------------------*
        QBBS      ||$C$L15||, r30, 0x00 ; [ALU_PRU] |103| 
;* --------------------------------------------------------------------------*
        QBNE      ||$C$L15||, r14.b0, 0x08 ; [ALU_PRU] |103| e_counter
;* --------------------------------------------------------------------------*
        JMP       ||$C$L19||            ; [ALU_PRU] |103| 
;* --------------------------------------------------------------------------*
;* --------------------------------------------------------------------------*
||$C$L15||:    
	.dwpsn	file "main.c",line 105,column 12,is_stmt,isa 0
;----------------------------------------------------------------------
; 105 | } else if(!(_in & VPAn) && (e_counter == 2)) {                         
;----------------------------------------------------------------------
        QBBS      ||$C$L16||, r0, 0x10  ; [ALU_PRU] |105| _in
;* --------------------------------------------------------------------------*
        QBNE      ||$C$L16||, r14.b0, 0x02 ; [ALU_PRU] |105| e_counter
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 106,column 9,is_stmt,isa 0
;----------------------------------------------------------------------
; 106 | __R30 &= ~VMAn;                                                        
; 108 | // Add wait states                                                     
;----------------------------------------------------------------------
        CLR       r30, r30, 0x00000000  ; [ALU_PRU] |106| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L16||
;*
;*   Loop source line                : 109
;*   Loop closing brace source line  : 109
;*   Known Minimum Trip Count        : 1
;*   Known Maximum Trip Count        : 4294967295
;*   Known Max Trip Count Factor     : 1
;* --------------------------------------------------------------------------*
||$C$L16||:    
	.dwpsn	file "main.c",line 109,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 109 | wait_low();                                                            
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |109| _in
        QBBS      ||$C$L16||, r0, 0x04  ; [ALU_PRU] |109| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 110,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 110 | do_e_bus();                                                            
;----------------------------------------------------------------------
        QBNE      ||$C$L17||, r14.b0, 0x09 ; [ALU_PRU] |110| e_counter
;* --------------------------------------------------------------------------*
        CLR       r30, r30, 0x00000007  ; [ALU_PRU] |110| 
        LDI       r14.b0, 0x00          ; [ALU_PRU] |110| e_counter
        JMP       ||$C$L14||            ; [ALU_PRU] |110| 
;* --------------------------------------------------------------------------*
||$C$L17||:    
        QBNE      ||$C$L18||, r14.b0, 0x05 ; [ALU_PRU] |110| e_counter
;* --------------------------------------------------------------------------*
        SET       r30, r30, 0x00000007  ; [ALU_PRU] |110| 
;* --------------------------------------------------------------------------*
||$C$L18||:    
        ADD       r14.b0, r14.b0, 0x01  ; [ALU_PRU] |110| e_counter,e_counter
	.dwpsn	file "main.c",line 111,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 111 | goto state3;                                                           
; 113 | state4:                                                                
;----------------------------------------------------------------------
        JMP       ||$C$L14||            ; [ALU_PRU] |111| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L19||
;* --------------------------------------------------------------------------*
||$C$L19||:    
	.dwpsn	file "main.c",line 114,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 114 | wait_low();                                                            
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |114| _in
        QBBS      ||$C$L19||, r0, 0x04  ; [ALU_PRU] |114| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 115,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 115 | do_e_bus();                                                            
; 116 | // free the GPMC from waiting to read/write                            
;----------------------------------------------------------------------
        QBNE      ||$C$L20||, r14.b0, 0x09 ; [ALU_PRU] |115| e_counter
;* --------------------------------------------------------------------------*
        CLR       r30, r30, 0x00000007  ; [ALU_PRU] |115| 
        LDI       r14.b0, 0x00          ; [ALU_PRU] |115| e_counter
        JMP       ||$C$L22||            ; [ALU_PRU] |115| 
;* --------------------------------------------------------------------------*
||$C$L20||:    
        QBNE      ||$C$L21||, r14.b0, 0x05 ; [ALU_PRU] |115| e_counter
;* --------------------------------------------------------------------------*
        SET       r30, r30, 0x00000007  ; [ALU_PRU] |115| 
;* --------------------------------------------------------------------------*
||$C$L21||:    
        ADD       r14.b0, r14.b0, 0x01  ; [ALU_PRU] |115| e_counter,e_counter
;* --------------------------------------------------------------------------*
||$C$L22||:    
	.dwpsn	file "main.c",line 117,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 117 | __R30 |= EWAITn;                                                       
; 119 | state5:                                                                
;----------------------------------------------------------------------
        SET       r30, r30, 0x00000006  ; [ALU_PRU] |117| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L23||
;* --------------------------------------------------------------------------*
||$C$L23||:    
	.dwpsn	file "main.c",line 120,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 120 | wait_high();                                                           
; 121 | // GPMC should have read by now                                        
; 123 | state6:                                                                
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |120| _in
        QBBC      ||$C$L23||, r0, 0x04  ; [ALU_PRU] |120| _in
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L24||
;*
;*   Loop source line                : 120
;*   Loop closing brace source line  : 120
;*   Known Minimum Trip Count        : 1
;*   Known Maximum Trip Count        : 4294967295
;*   Known Max Trip Count Factor     : 1
;* --------------------------------------------------------------------------*
||$C$L24||:    
	.dwpsn	file "main.c",line 124,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 124 | wait_low();                                                            
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |124| _in
        QBBS      ||$C$L24||, r0, 0x04  ; [ALU_PRU] |124| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 125,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 125 | do_e_bus();                                                            
;----------------------------------------------------------------------
        QBNE      ||$C$L25||, r14.b0, 0x09 ; [ALU_PRU] |125| e_counter
;* --------------------------------------------------------------------------*
        CLR       r30, r30, 0x00000007  ; [ALU_PRU] |125| 
        LDI       r14.b0, 0x00          ; [ALU_PRU] |125| e_counter
        JMP       ||$C$L27||            ; [ALU_PRU] |125| 
;* --------------------------------------------------------------------------*
||$C$L25||:    
        QBNE      ||$C$L26||, r14.b0, 0x05 ; [ALU_PRU] |125| e_counter
;* --------------------------------------------------------------------------*
        SET       r30, r30, 0x00000007  ; [ALU_PRU] |125| 
;* --------------------------------------------------------------------------*
||$C$L26||:    
        ADD       r14.b0, r14.b0, 0x01  ; [ALU_PRU] |125| e_counter,e_counter
;* --------------------------------------------------------------------------*
||$C$L27||:    
	.dwpsn	file "main.c",line 126,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 126 | __R30 |= VMAn;                                                         
;----------------------------------------------------------------------
        SET       r30, r30, 0x00000000  ; [ALU_PRU] |126| 
	.dwpsn	file "main.c",line 127,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 127 | __R30 &= ~EWAITn;                                                      
; 129 | state7:                                                                
;----------------------------------------------------------------------
        CLR       r30, r30, 0x00000006  ; [ALU_PRU] |127| 
;* --------------------------------------------------------------------------*
;*   BEGIN LOOP ||$C$L28||
;* --------------------------------------------------------------------------*
||$C$L28||:    
	.dwpsn	file "main.c",line 130,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 130 | wait_high();                                                           
; 131 | // should be asserted by GPMC here                                     
; 132 | // M68K_AS_n <= 1'b1;                                                  
; 133 | // M68K_UDS_n <= 1'b1;                                                 
; 134 | // M68K_LDS_n <= 1'b1;                                                 
;----------------------------------------------------------------------
        MOV       r0, r31               ; [ALU_PRU] |130| _in
        QBBC      ||$C$L28||, r0, 0x04  ; [ALU_PRU] |130| _in
;* --------------------------------------------------------------------------*
	.dwpsn	file "main.c",line 136,column 5,is_stmt,isa 0
;----------------------------------------------------------------------
; 136 | goto state0;                                                           
;----------------------------------------------------------------------
        JMP       ||$C$L6||             ; [ALU_PRU] |136| 
;* --------------------------------------------------------------------------*
	.dwattr $C$DW$3, DW_AT_TI_end_file("main.c")
	.dwattr $C$DW$3, DW_AT_TI_end_line(0x89)
	.dwattr $C$DW$3, DW_AT_TI_end_column(0x01)
	.dwendentry
	.dwendtag $C$DW$3


;******************************************************************************
;* TYPE INFORMATION                                                           *
;******************************************************************************

$C$DW$T$19	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$19, DW_AT_byte_size(0x04)
$C$DW$15	.dwtag  DW_TAG_member
	.dwattr $C$DW$15, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$15, DW_AT_name("REVID")
	.dwattr $C$DW$15, DW_AT_TI_symbol_name("REVID")
	.dwattr $C$DW$15, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$15, DW_AT_bit_size(0x20)
	.dwattr $C$DW$15, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$15, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$15, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$15, DW_AT_decl_line(0x2d)
	.dwattr $C$DW$15, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$19, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$19, DW_AT_decl_line(0x2c)
	.dwattr $C$DW$T$19, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$19

$C$DW$T$37	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$37, DW_AT_type(*$C$DW$T$19)


$C$DW$T$20	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$20, DW_AT_byte_size(0x04)
$C$DW$16	.dwtag  DW_TAG_member
	.dwattr $C$DW$16, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$16, DW_AT_name("IDLE_MODE")
	.dwattr $C$DW$16, DW_AT_TI_symbol_name("IDLE_MODE")
	.dwattr $C$DW$16, DW_AT_bit_offset(0x1e)
	.dwattr $C$DW$16, DW_AT_bit_size(0x02)
	.dwattr $C$DW$16, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$16, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$16, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$16, DW_AT_decl_line(0x37)
	.dwattr $C$DW$16, DW_AT_decl_column(0x0d)

$C$DW$17	.dwtag  DW_TAG_member
	.dwattr $C$DW$17, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$17, DW_AT_name("STANDBY_MODE")
	.dwattr $C$DW$17, DW_AT_TI_symbol_name("STANDBY_MODE")
	.dwattr $C$DW$17, DW_AT_bit_offset(0x1c)
	.dwattr $C$DW$17, DW_AT_bit_size(0x02)
	.dwattr $C$DW$17, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$17, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$17, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$17, DW_AT_decl_line(0x38)
	.dwattr $C$DW$17, DW_AT_decl_column(0x0d)

$C$DW$18	.dwtag  DW_TAG_member
	.dwattr $C$DW$18, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$18, DW_AT_name("STANDBY_INIT")
	.dwattr $C$DW$18, DW_AT_TI_symbol_name("STANDBY_INIT")
	.dwattr $C$DW$18, DW_AT_bit_offset(0x1b)
	.dwattr $C$DW$18, DW_AT_bit_size(0x01)
	.dwattr $C$DW$18, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$18, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$18, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$18, DW_AT_decl_line(0x39)
	.dwattr $C$DW$18, DW_AT_decl_column(0x0d)

$C$DW$19	.dwtag  DW_TAG_member
	.dwattr $C$DW$19, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$19, DW_AT_name("SUB_MWAIT")
	.dwattr $C$DW$19, DW_AT_TI_symbol_name("SUB_MWAIT")
	.dwattr $C$DW$19, DW_AT_bit_offset(0x1a)
	.dwattr $C$DW$19, DW_AT_bit_size(0x01)
	.dwattr $C$DW$19, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$19, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$19, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$19, DW_AT_decl_line(0x3a)
	.dwattr $C$DW$19, DW_AT_decl_column(0x0d)

$C$DW$20	.dwtag  DW_TAG_member
	.dwattr $C$DW$20, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$20, DW_AT_name("rsvd6")
	.dwattr $C$DW$20, DW_AT_TI_symbol_name("rsvd6")
	.dwattr $C$DW$20, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$20, DW_AT_bit_size(0x1a)
	.dwattr $C$DW$20, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$20, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$20, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$20, DW_AT_decl_line(0x3b)
	.dwattr $C$DW$20, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$20, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$20, DW_AT_decl_line(0x36)
	.dwattr $C$DW$T$20, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$20

$C$DW$T$39	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$39, DW_AT_type(*$C$DW$T$20)


$C$DW$T$21	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$21, DW_AT_byte_size(0x04)
$C$DW$21	.dwtag  DW_TAG_member
	.dwattr $C$DW$21, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$21, DW_AT_name("PRU0_GPI_MODE")
	.dwattr $C$DW$21, DW_AT_TI_symbol_name("PRU0_GPI_MODE")
	.dwattr $C$DW$21, DW_AT_bit_offset(0x1e)
	.dwattr $C$DW$21, DW_AT_bit_size(0x02)
	.dwattr $C$DW$21, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$21, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$21, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$21, DW_AT_decl_line(0x45)
	.dwattr $C$DW$21, DW_AT_decl_column(0x0d)

$C$DW$22	.dwtag  DW_TAG_member
	.dwattr $C$DW$22, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$22, DW_AT_name("PRU0_GPI_CLK_MODE")
	.dwattr $C$DW$22, DW_AT_TI_symbol_name("PRU0_GPI_CLK_MODE")
	.dwattr $C$DW$22, DW_AT_bit_offset(0x1d)
	.dwattr $C$DW$22, DW_AT_bit_size(0x01)
	.dwattr $C$DW$22, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$22, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$22, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$22, DW_AT_decl_line(0x46)
	.dwattr $C$DW$22, DW_AT_decl_column(0x0d)

$C$DW$23	.dwtag  DW_TAG_member
	.dwattr $C$DW$23, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$23, DW_AT_name("PRU0_GPI_DIV0")
	.dwattr $C$DW$23, DW_AT_TI_symbol_name("PRU0_GPI_DIV0")
	.dwattr $C$DW$23, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$23, DW_AT_bit_size(0x05)
	.dwattr $C$DW$23, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$23, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$23, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$23, DW_AT_decl_line(0x47)
	.dwattr $C$DW$23, DW_AT_decl_column(0x0d)

$C$DW$24	.dwtag  DW_TAG_member
	.dwattr $C$DW$24, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$24, DW_AT_name("PRU0_GPI_DIV1")
	.dwattr $C$DW$24, DW_AT_TI_symbol_name("PRU0_GPI_DIV1")
	.dwattr $C$DW$24, DW_AT_bit_offset(0x13)
	.dwattr $C$DW$24, DW_AT_bit_size(0x05)
	.dwattr $C$DW$24, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$24, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$24, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$24, DW_AT_decl_line(0x48)
	.dwattr $C$DW$24, DW_AT_decl_column(0x0d)

$C$DW$25	.dwtag  DW_TAG_member
	.dwattr $C$DW$25, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$25, DW_AT_name("PRU0_GPI_SB")
	.dwattr $C$DW$25, DW_AT_TI_symbol_name("PRU0_GPI_SB")
	.dwattr $C$DW$25, DW_AT_bit_offset(0x12)
	.dwattr $C$DW$25, DW_AT_bit_size(0x01)
	.dwattr $C$DW$25, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$25, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$25, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$25, DW_AT_decl_line(0x49)
	.dwattr $C$DW$25, DW_AT_decl_column(0x0d)

$C$DW$26	.dwtag  DW_TAG_member
	.dwattr $C$DW$26, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$26, DW_AT_name("PRU0_GPO_MODE")
	.dwattr $C$DW$26, DW_AT_TI_symbol_name("PRU0_GPO_MODE")
	.dwattr $C$DW$26, DW_AT_bit_offset(0x11)
	.dwattr $C$DW$26, DW_AT_bit_size(0x01)
	.dwattr $C$DW$26, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$26, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$26, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$26, DW_AT_decl_line(0x4a)
	.dwattr $C$DW$26, DW_AT_decl_column(0x0d)

$C$DW$27	.dwtag  DW_TAG_member
	.dwattr $C$DW$27, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$27, DW_AT_name("PRU0_GPO_DIV0")
	.dwattr $C$DW$27, DW_AT_TI_symbol_name("PRU0_GPO_DIV0")
	.dwattr $C$DW$27, DW_AT_bit_offset(0x0c)
	.dwattr $C$DW$27, DW_AT_bit_size(0x05)
	.dwattr $C$DW$27, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$27, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$27, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$27, DW_AT_decl_line(0x4b)
	.dwattr $C$DW$27, DW_AT_decl_column(0x0d)

$C$DW$28	.dwtag  DW_TAG_member
	.dwattr $C$DW$28, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$28, DW_AT_name("PRU0_GPO_DIV1")
	.dwattr $C$DW$28, DW_AT_TI_symbol_name("PRU0_GPO_DIV1")
	.dwattr $C$DW$28, DW_AT_bit_offset(0x07)
	.dwattr $C$DW$28, DW_AT_bit_size(0x05)
	.dwattr $C$DW$28, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$28, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$28, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$28, DW_AT_decl_line(0x4c)
	.dwattr $C$DW$28, DW_AT_decl_column(0x0d)

$C$DW$29	.dwtag  DW_TAG_member
	.dwattr $C$DW$29, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$29, DW_AT_name("PRU0_GPO_SH_SEL")
	.dwattr $C$DW$29, DW_AT_TI_symbol_name("PRU0_GPO_SH_SEL")
	.dwattr $C$DW$29, DW_AT_bit_offset(0x06)
	.dwattr $C$DW$29, DW_AT_bit_size(0x01)
	.dwattr $C$DW$29, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$29, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$29, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$29, DW_AT_decl_line(0x4d)
	.dwattr $C$DW$29, DW_AT_decl_column(0x0d)

$C$DW$30	.dwtag  DW_TAG_member
	.dwattr $C$DW$30, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$30, DW_AT_name("rsvd26")
	.dwattr $C$DW$30, DW_AT_TI_symbol_name("rsvd26")
	.dwattr $C$DW$30, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$30, DW_AT_bit_size(0x06)
	.dwattr $C$DW$30, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$30, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$30, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$30, DW_AT_decl_line(0x4e)
	.dwattr $C$DW$30, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$21, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$21, DW_AT_decl_line(0x44)
	.dwattr $C$DW$T$21, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$21

$C$DW$T$41	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$41, DW_AT_type(*$C$DW$T$21)


$C$DW$T$22	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$22, DW_AT_byte_size(0x04)
$C$DW$31	.dwtag  DW_TAG_member
	.dwattr $C$DW$31, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$31, DW_AT_name("PRU1_GPI_MODE")
	.dwattr $C$DW$31, DW_AT_TI_symbol_name("PRU1_GPI_MODE")
	.dwattr $C$DW$31, DW_AT_bit_offset(0x1e)
	.dwattr $C$DW$31, DW_AT_bit_size(0x02)
	.dwattr $C$DW$31, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$31, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$31, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$31, DW_AT_decl_line(0x58)
	.dwattr $C$DW$31, DW_AT_decl_column(0x0d)

$C$DW$32	.dwtag  DW_TAG_member
	.dwattr $C$DW$32, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$32, DW_AT_name("PRU1_GPI_CLK_MODE")
	.dwattr $C$DW$32, DW_AT_TI_symbol_name("PRU1_GPI_CLK_MODE")
	.dwattr $C$DW$32, DW_AT_bit_offset(0x1d)
	.dwattr $C$DW$32, DW_AT_bit_size(0x01)
	.dwattr $C$DW$32, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$32, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$32, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$32, DW_AT_decl_line(0x59)
	.dwattr $C$DW$32, DW_AT_decl_column(0x0d)

$C$DW$33	.dwtag  DW_TAG_member
	.dwattr $C$DW$33, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$33, DW_AT_name("PRU1_GPI_DIV0")
	.dwattr $C$DW$33, DW_AT_TI_symbol_name("PRU1_GPI_DIV0")
	.dwattr $C$DW$33, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$33, DW_AT_bit_size(0x05)
	.dwattr $C$DW$33, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$33, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$33, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$33, DW_AT_decl_line(0x5a)
	.dwattr $C$DW$33, DW_AT_decl_column(0x0d)

$C$DW$34	.dwtag  DW_TAG_member
	.dwattr $C$DW$34, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$34, DW_AT_name("PRU1_GPI_DIV1")
	.dwattr $C$DW$34, DW_AT_TI_symbol_name("PRU1_GPI_DIV1")
	.dwattr $C$DW$34, DW_AT_bit_offset(0x13)
	.dwattr $C$DW$34, DW_AT_bit_size(0x05)
	.dwattr $C$DW$34, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$34, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$34, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$34, DW_AT_decl_line(0x5b)
	.dwattr $C$DW$34, DW_AT_decl_column(0x0d)

$C$DW$35	.dwtag  DW_TAG_member
	.dwattr $C$DW$35, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$35, DW_AT_name("PRU1_GPI_SB")
	.dwattr $C$DW$35, DW_AT_TI_symbol_name("PRU1_GPI_SB")
	.dwattr $C$DW$35, DW_AT_bit_offset(0x12)
	.dwattr $C$DW$35, DW_AT_bit_size(0x01)
	.dwattr $C$DW$35, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$35, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$35, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$35, DW_AT_decl_line(0x5c)
	.dwattr $C$DW$35, DW_AT_decl_column(0x0d)

$C$DW$36	.dwtag  DW_TAG_member
	.dwattr $C$DW$36, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$36, DW_AT_name("PRU1_GPO_MODE")
	.dwattr $C$DW$36, DW_AT_TI_symbol_name("PRU1_GPO_MODE")
	.dwattr $C$DW$36, DW_AT_bit_offset(0x11)
	.dwattr $C$DW$36, DW_AT_bit_size(0x01)
	.dwattr $C$DW$36, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$36, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$36, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$36, DW_AT_decl_line(0x5d)
	.dwattr $C$DW$36, DW_AT_decl_column(0x0d)

$C$DW$37	.dwtag  DW_TAG_member
	.dwattr $C$DW$37, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$37, DW_AT_name("PRU1_GPO_DIV0")
	.dwattr $C$DW$37, DW_AT_TI_symbol_name("PRU1_GPO_DIV0")
	.dwattr $C$DW$37, DW_AT_bit_offset(0x0c)
	.dwattr $C$DW$37, DW_AT_bit_size(0x05)
	.dwattr $C$DW$37, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$37, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$37, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$37, DW_AT_decl_line(0x5e)
	.dwattr $C$DW$37, DW_AT_decl_column(0x0d)

$C$DW$38	.dwtag  DW_TAG_member
	.dwattr $C$DW$38, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$38, DW_AT_name("PRU1_GPO_DIV1")
	.dwattr $C$DW$38, DW_AT_TI_symbol_name("PRU1_GPO_DIV1")
	.dwattr $C$DW$38, DW_AT_bit_offset(0x07)
	.dwattr $C$DW$38, DW_AT_bit_size(0x05)
	.dwattr $C$DW$38, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$38, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$38, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$38, DW_AT_decl_line(0x5f)
	.dwattr $C$DW$38, DW_AT_decl_column(0x0d)

$C$DW$39	.dwtag  DW_TAG_member
	.dwattr $C$DW$39, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$39, DW_AT_name("PRU1_GPO_SH_SEL")
	.dwattr $C$DW$39, DW_AT_TI_symbol_name("PRU1_GPO_SH_SEL")
	.dwattr $C$DW$39, DW_AT_bit_offset(0x06)
	.dwattr $C$DW$39, DW_AT_bit_size(0x01)
	.dwattr $C$DW$39, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$39, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$39, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$39, DW_AT_decl_line(0x60)
	.dwattr $C$DW$39, DW_AT_decl_column(0x0d)

$C$DW$40	.dwtag  DW_TAG_member
	.dwattr $C$DW$40, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$40, DW_AT_name("rsvd26")
	.dwattr $C$DW$40, DW_AT_TI_symbol_name("rsvd26")
	.dwattr $C$DW$40, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$40, DW_AT_bit_size(0x06)
	.dwattr $C$DW$40, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$40, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$40, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$40, DW_AT_decl_line(0x61)
	.dwattr $C$DW$40, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$22, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$22, DW_AT_decl_line(0x57)
	.dwattr $C$DW$T$22, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$22

$C$DW$T$43	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$43, DW_AT_type(*$C$DW$T$22)


$C$DW$T$23	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$23, DW_AT_byte_size(0x04)
$C$DW$41	.dwtag  DW_TAG_member
	.dwattr $C$DW$41, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$41, DW_AT_name("PRU0_CLK_STOP_REQ")
	.dwattr $C$DW$41, DW_AT_TI_symbol_name("PRU0_CLK_STOP_REQ")
	.dwattr $C$DW$41, DW_AT_bit_offset(0x1f)
	.dwattr $C$DW$41, DW_AT_bit_size(0x01)
	.dwattr $C$DW$41, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$41, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$41, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$41, DW_AT_decl_line(0x6b)
	.dwattr $C$DW$41, DW_AT_decl_column(0x0d)

$C$DW$42	.dwtag  DW_TAG_member
	.dwattr $C$DW$42, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$42, DW_AT_name("PRU0_CLK_STOP_ACK")
	.dwattr $C$DW$42, DW_AT_TI_symbol_name("PRU0_CLK_STOP_ACK")
	.dwattr $C$DW$42, DW_AT_bit_offset(0x1e)
	.dwattr $C$DW$42, DW_AT_bit_size(0x01)
	.dwattr $C$DW$42, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$42, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$42, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$42, DW_AT_decl_line(0x6c)
	.dwattr $C$DW$42, DW_AT_decl_column(0x0d)

$C$DW$43	.dwtag  DW_TAG_member
	.dwattr $C$DW$43, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$43, DW_AT_name("PRU0_CLK_EN")
	.dwattr $C$DW$43, DW_AT_TI_symbol_name("PRU0_CLK_EN")
	.dwattr $C$DW$43, DW_AT_bit_offset(0x1d)
	.dwattr $C$DW$43, DW_AT_bit_size(0x01)
	.dwattr $C$DW$43, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$43, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$43, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$43, DW_AT_decl_line(0x6d)
	.dwattr $C$DW$43, DW_AT_decl_column(0x0d)

$C$DW$44	.dwtag  DW_TAG_member
	.dwattr $C$DW$44, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$44, DW_AT_name("PRU1_CLK_STOP_REQ")
	.dwattr $C$DW$44, DW_AT_TI_symbol_name("PRU1_CLK_STOP_REQ")
	.dwattr $C$DW$44, DW_AT_bit_offset(0x1c)
	.dwattr $C$DW$44, DW_AT_bit_size(0x01)
	.dwattr $C$DW$44, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$44, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$44, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$44, DW_AT_decl_line(0x6e)
	.dwattr $C$DW$44, DW_AT_decl_column(0x0d)

$C$DW$45	.dwtag  DW_TAG_member
	.dwattr $C$DW$45, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$45, DW_AT_name("PRU1_CLK_STOP_ACK")
	.dwattr $C$DW$45, DW_AT_TI_symbol_name("PRU1_CLK_STOP_ACK")
	.dwattr $C$DW$45, DW_AT_bit_offset(0x1b)
	.dwattr $C$DW$45, DW_AT_bit_size(0x01)
	.dwattr $C$DW$45, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$45, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$45, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$45, DW_AT_decl_line(0x6f)
	.dwattr $C$DW$45, DW_AT_decl_column(0x0d)

$C$DW$46	.dwtag  DW_TAG_member
	.dwattr $C$DW$46, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$46, DW_AT_name("PRU1_CLK_EN")
	.dwattr $C$DW$46, DW_AT_TI_symbol_name("PRU1_CLK_EN")
	.dwattr $C$DW$46, DW_AT_bit_offset(0x1a)
	.dwattr $C$DW$46, DW_AT_bit_size(0x01)
	.dwattr $C$DW$46, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$46, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$46, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$46, DW_AT_decl_line(0x70)
	.dwattr $C$DW$46, DW_AT_decl_column(0x0d)

$C$DW$47	.dwtag  DW_TAG_member
	.dwattr $C$DW$47, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$47, DW_AT_name("INTC_CLK_STOP_REQ")
	.dwattr $C$DW$47, DW_AT_TI_symbol_name("INTC_CLK_STOP_REQ")
	.dwattr $C$DW$47, DW_AT_bit_offset(0x19)
	.dwattr $C$DW$47, DW_AT_bit_size(0x01)
	.dwattr $C$DW$47, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$47, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$47, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$47, DW_AT_decl_line(0x71)
	.dwattr $C$DW$47, DW_AT_decl_column(0x0d)

$C$DW$48	.dwtag  DW_TAG_member
	.dwattr $C$DW$48, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$48, DW_AT_name("INTC_CLK_STOP_ACK")
	.dwattr $C$DW$48, DW_AT_TI_symbol_name("INTC_CLK_STOP_ACK")
	.dwattr $C$DW$48, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$48, DW_AT_bit_size(0x01)
	.dwattr $C$DW$48, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$48, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$48, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$48, DW_AT_decl_line(0x72)
	.dwattr $C$DW$48, DW_AT_decl_column(0x0d)

$C$DW$49	.dwtag  DW_TAG_member
	.dwattr $C$DW$49, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$49, DW_AT_name("INTC_CLK_EN")
	.dwattr $C$DW$49, DW_AT_TI_symbol_name("INTC_CLK_EN")
	.dwattr $C$DW$49, DW_AT_bit_offset(0x17)
	.dwattr $C$DW$49, DW_AT_bit_size(0x01)
	.dwattr $C$DW$49, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$49, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$49, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$49, DW_AT_decl_line(0x73)
	.dwattr $C$DW$49, DW_AT_decl_column(0x0d)

$C$DW$50	.dwtag  DW_TAG_member
	.dwattr $C$DW$50, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$50, DW_AT_name("UART_CLK_STOP_REQ")
	.dwattr $C$DW$50, DW_AT_TI_symbol_name("UART_CLK_STOP_REQ")
	.dwattr $C$DW$50, DW_AT_bit_offset(0x16)
	.dwattr $C$DW$50, DW_AT_bit_size(0x01)
	.dwattr $C$DW$50, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$50, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$50, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$50, DW_AT_decl_line(0x74)
	.dwattr $C$DW$50, DW_AT_decl_column(0x0d)

$C$DW$51	.dwtag  DW_TAG_member
	.dwattr $C$DW$51, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$51, DW_AT_name("UART_CLK_STOP_ACK")
	.dwattr $C$DW$51, DW_AT_TI_symbol_name("UART_CLK_STOP_ACK")
	.dwattr $C$DW$51, DW_AT_bit_offset(0x15)
	.dwattr $C$DW$51, DW_AT_bit_size(0x01)
	.dwattr $C$DW$51, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$51, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$51, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$51, DW_AT_decl_line(0x75)
	.dwattr $C$DW$51, DW_AT_decl_column(0x0d)

$C$DW$52	.dwtag  DW_TAG_member
	.dwattr $C$DW$52, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$52, DW_AT_name("UART_CLK_EN")
	.dwattr $C$DW$52, DW_AT_TI_symbol_name("UART_CLK_EN")
	.dwattr $C$DW$52, DW_AT_bit_offset(0x14)
	.dwattr $C$DW$52, DW_AT_bit_size(0x01)
	.dwattr $C$DW$52, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$52, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$52, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$52, DW_AT_decl_line(0x76)
	.dwattr $C$DW$52, DW_AT_decl_column(0x0d)

$C$DW$53	.dwtag  DW_TAG_member
	.dwattr $C$DW$53, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$53, DW_AT_name("ECAP_CLK_STOP_REQ")
	.dwattr $C$DW$53, DW_AT_TI_symbol_name("ECAP_CLK_STOP_REQ")
	.dwattr $C$DW$53, DW_AT_bit_offset(0x13)
	.dwattr $C$DW$53, DW_AT_bit_size(0x01)
	.dwattr $C$DW$53, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$53, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$53, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$53, DW_AT_decl_line(0x77)
	.dwattr $C$DW$53, DW_AT_decl_column(0x0d)

$C$DW$54	.dwtag  DW_TAG_member
	.dwattr $C$DW$54, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$54, DW_AT_name("ECAP_CLK_STOP_ACK")
	.dwattr $C$DW$54, DW_AT_TI_symbol_name("ECAP_CLK_STOP_ACK")
	.dwattr $C$DW$54, DW_AT_bit_offset(0x12)
	.dwattr $C$DW$54, DW_AT_bit_size(0x01)
	.dwattr $C$DW$54, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$54, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$54, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$54, DW_AT_decl_line(0x78)
	.dwattr $C$DW$54, DW_AT_decl_column(0x0d)

$C$DW$55	.dwtag  DW_TAG_member
	.dwattr $C$DW$55, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$55, DW_AT_name("ECAP_CLK_EN")
	.dwattr $C$DW$55, DW_AT_TI_symbol_name("ECAP_CLK_EN")
	.dwattr $C$DW$55, DW_AT_bit_offset(0x11)
	.dwattr $C$DW$55, DW_AT_bit_size(0x01)
	.dwattr $C$DW$55, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$55, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$55, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$55, DW_AT_decl_line(0x79)
	.dwattr $C$DW$55, DW_AT_decl_column(0x0d)

$C$DW$56	.dwtag  DW_TAG_member
	.dwattr $C$DW$56, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$56, DW_AT_name("IEP_CLK_STOP_REQ")
	.dwattr $C$DW$56, DW_AT_TI_symbol_name("IEP_CLK_STOP_REQ")
	.dwattr $C$DW$56, DW_AT_bit_offset(0x10)
	.dwattr $C$DW$56, DW_AT_bit_size(0x01)
	.dwattr $C$DW$56, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$56, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$56, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$56, DW_AT_decl_line(0x7a)
	.dwattr $C$DW$56, DW_AT_decl_column(0x0d)

$C$DW$57	.dwtag  DW_TAG_member
	.dwattr $C$DW$57, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$57, DW_AT_name("IEP_CLK_STOP_ACK")
	.dwattr $C$DW$57, DW_AT_TI_symbol_name("IEP_CLK_STOP_ACK")
	.dwattr $C$DW$57, DW_AT_bit_offset(0x0f)
	.dwattr $C$DW$57, DW_AT_bit_size(0x01)
	.dwattr $C$DW$57, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$57, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$57, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$57, DW_AT_decl_line(0x7b)
	.dwattr $C$DW$57, DW_AT_decl_column(0x0d)

$C$DW$58	.dwtag  DW_TAG_member
	.dwattr $C$DW$58, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$58, DW_AT_name("IEP_CLK_EN")
	.dwattr $C$DW$58, DW_AT_TI_symbol_name("IEP_CLK_EN")
	.dwattr $C$DW$58, DW_AT_bit_offset(0x0e)
	.dwattr $C$DW$58, DW_AT_bit_size(0x01)
	.dwattr $C$DW$58, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$58, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$58, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$58, DW_AT_decl_line(0x7c)
	.dwattr $C$DW$58, DW_AT_decl_column(0x0d)

$C$DW$59	.dwtag  DW_TAG_member
	.dwattr $C$DW$59, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$59, DW_AT_name("rsvd18")
	.dwattr $C$DW$59, DW_AT_TI_symbol_name("rsvd18")
	.dwattr $C$DW$59, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$59, DW_AT_bit_size(0x0e)
	.dwattr $C$DW$59, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$59, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$59, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$59, DW_AT_decl_line(0x7d)
	.dwattr $C$DW$59, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$23, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$23, DW_AT_decl_line(0x6a)
	.dwattr $C$DW$T$23, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$23

$C$DW$T$45	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$45, DW_AT_type(*$C$DW$T$23)


$C$DW$T$24	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$24, DW_AT_byte_size(0x04)
$C$DW$60	.dwtag  DW_TAG_member
	.dwattr $C$DW$60, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$60, DW_AT_name("PRU0_IMEM_PE_RAW")
	.dwattr $C$DW$60, DW_AT_TI_symbol_name("PRU0_IMEM_PE_RAW")
	.dwattr $C$DW$60, DW_AT_bit_offset(0x1c)
	.dwattr $C$DW$60, DW_AT_bit_size(0x04)
	.dwattr $C$DW$60, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$60, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$60, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$60, DW_AT_decl_line(0x87)
	.dwattr $C$DW$60, DW_AT_decl_column(0x0d)

$C$DW$61	.dwtag  DW_TAG_member
	.dwattr $C$DW$61, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$61, DW_AT_name("PRU0_DMEM_PE_RAW")
	.dwattr $C$DW$61, DW_AT_TI_symbol_name("PRU0_DMEM_PE_RAW")
	.dwattr $C$DW$61, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$61, DW_AT_bit_size(0x04)
	.dwattr $C$DW$61, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$61, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$61, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$61, DW_AT_decl_line(0x88)
	.dwattr $C$DW$61, DW_AT_decl_column(0x0d)

$C$DW$62	.dwtag  DW_TAG_member
	.dwattr $C$DW$62, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$62, DW_AT_name("PRU1_IMEM_PE_RAW")
	.dwattr $C$DW$62, DW_AT_TI_symbol_name("PRU1_IMEM_PE_RAW")
	.dwattr $C$DW$62, DW_AT_bit_offset(0x14)
	.dwattr $C$DW$62, DW_AT_bit_size(0x04)
	.dwattr $C$DW$62, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$62, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$62, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$62, DW_AT_decl_line(0x89)
	.dwattr $C$DW$62, DW_AT_decl_column(0x0d)

$C$DW$63	.dwtag  DW_TAG_member
	.dwattr $C$DW$63, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$63, DW_AT_name("PRU1_DMEM_PE_RAW")
	.dwattr $C$DW$63, DW_AT_TI_symbol_name("PRU1_DMEM_PE_RAW")
	.dwattr $C$DW$63, DW_AT_bit_offset(0x10)
	.dwattr $C$DW$63, DW_AT_bit_size(0x04)
	.dwattr $C$DW$63, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$63, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$63, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$63, DW_AT_decl_line(0x8a)
	.dwattr $C$DW$63, DW_AT_decl_column(0x0d)

$C$DW$64	.dwtag  DW_TAG_member
	.dwattr $C$DW$64, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$64, DW_AT_name("RAM_PE_RAW")
	.dwattr $C$DW$64, DW_AT_TI_symbol_name("RAM_PE_RAW")
	.dwattr $C$DW$64, DW_AT_bit_offset(0x0c)
	.dwattr $C$DW$64, DW_AT_bit_size(0x04)
	.dwattr $C$DW$64, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$64, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$64, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$64, DW_AT_decl_line(0x8b)
	.dwattr $C$DW$64, DW_AT_decl_column(0x0d)

$C$DW$65	.dwtag  DW_TAG_member
	.dwattr $C$DW$65, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$65, DW_AT_name("rsvd20")
	.dwattr $C$DW$65, DW_AT_TI_symbol_name("rsvd20")
	.dwattr $C$DW$65, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$65, DW_AT_bit_size(0x0c)
	.dwattr $C$DW$65, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$65, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$65, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$65, DW_AT_decl_line(0x8c)
	.dwattr $C$DW$65, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$24, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$24, DW_AT_decl_line(0x86)
	.dwattr $C$DW$T$24, DW_AT_decl_column(0x14)
	.dwendtag $C$DW$T$24

$C$DW$T$47	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$47, DW_AT_type(*$C$DW$T$24)


$C$DW$T$25	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$25, DW_AT_byte_size(0x04)
$C$DW$66	.dwtag  DW_TAG_member
	.dwattr $C$DW$66, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$66, DW_AT_name("PRU0_IMEM_PE")
	.dwattr $C$DW$66, DW_AT_TI_symbol_name("PRU0_IMEM_PE")
	.dwattr $C$DW$66, DW_AT_bit_offset(0x1c)
	.dwattr $C$DW$66, DW_AT_bit_size(0x04)
	.dwattr $C$DW$66, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$66, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$66, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$66, DW_AT_decl_line(0x96)
	.dwattr $C$DW$66, DW_AT_decl_column(0x0d)

$C$DW$67	.dwtag  DW_TAG_member
	.dwattr $C$DW$67, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$67, DW_AT_name("PRU0_DMEM_PE")
	.dwattr $C$DW$67, DW_AT_TI_symbol_name("PRU0_DMEM_PE")
	.dwattr $C$DW$67, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$67, DW_AT_bit_size(0x04)
	.dwattr $C$DW$67, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$67, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$67, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$67, DW_AT_decl_line(0x97)
	.dwattr $C$DW$67, DW_AT_decl_column(0x0d)

$C$DW$68	.dwtag  DW_TAG_member
	.dwattr $C$DW$68, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$68, DW_AT_name("PRU1_IMEM_PE")
	.dwattr $C$DW$68, DW_AT_TI_symbol_name("PRU1_IMEM_PE")
	.dwattr $C$DW$68, DW_AT_bit_offset(0x14)
	.dwattr $C$DW$68, DW_AT_bit_size(0x04)
	.dwattr $C$DW$68, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$68, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$68, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$68, DW_AT_decl_line(0x98)
	.dwattr $C$DW$68, DW_AT_decl_column(0x0d)

$C$DW$69	.dwtag  DW_TAG_member
	.dwattr $C$DW$69, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$69, DW_AT_name("PRU1_DMEM_PE")
	.dwattr $C$DW$69, DW_AT_TI_symbol_name("PRU1_DMEM_PE")
	.dwattr $C$DW$69, DW_AT_bit_offset(0x10)
	.dwattr $C$DW$69, DW_AT_bit_size(0x04)
	.dwattr $C$DW$69, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$69, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$69, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$69, DW_AT_decl_line(0x99)
	.dwattr $C$DW$69, DW_AT_decl_column(0x0d)

$C$DW$70	.dwtag  DW_TAG_member
	.dwattr $C$DW$70, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$70, DW_AT_name("RAM_PE")
	.dwattr $C$DW$70, DW_AT_TI_symbol_name("RAM_PE")
	.dwattr $C$DW$70, DW_AT_bit_offset(0x0c)
	.dwattr $C$DW$70, DW_AT_bit_size(0x04)
	.dwattr $C$DW$70, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$70, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$70, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$70, DW_AT_decl_line(0x9a)
	.dwattr $C$DW$70, DW_AT_decl_column(0x0d)

$C$DW$71	.dwtag  DW_TAG_member
	.dwattr $C$DW$71, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$71, DW_AT_name("rsvd20")
	.dwattr $C$DW$71, DW_AT_TI_symbol_name("rsvd20")
	.dwattr $C$DW$71, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$71, DW_AT_bit_size(0x0c)
	.dwattr $C$DW$71, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$71, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$71, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$71, DW_AT_decl_line(0x9b)
	.dwattr $C$DW$71, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$25, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$25, DW_AT_decl_line(0x95)
	.dwattr $C$DW$T$25, DW_AT_decl_column(0x14)
	.dwendtag $C$DW$T$25

$C$DW$T$49	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$49, DW_AT_type(*$C$DW$T$25)


$C$DW$T$26	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$26, DW_AT_byte_size(0x04)
$C$DW$72	.dwtag  DW_TAG_member
	.dwattr $C$DW$72, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$72, DW_AT_name("PRU0_IMEM_PE_SET")
	.dwattr $C$DW$72, DW_AT_TI_symbol_name("PRU0_IMEM_PE_SET")
	.dwattr $C$DW$72, DW_AT_bit_offset(0x1c)
	.dwattr $C$DW$72, DW_AT_bit_size(0x04)
	.dwattr $C$DW$72, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$72, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$72, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$72, DW_AT_decl_line(0xa4)
	.dwattr $C$DW$72, DW_AT_decl_column(0x0d)

$C$DW$73	.dwtag  DW_TAG_member
	.dwattr $C$DW$73, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$73, DW_AT_name("PRU0_DMEM_PE_SET")
	.dwattr $C$DW$73, DW_AT_TI_symbol_name("PRU0_DMEM_PE_SET")
	.dwattr $C$DW$73, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$73, DW_AT_bit_size(0x04)
	.dwattr $C$DW$73, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$73, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$73, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$73, DW_AT_decl_line(0xa5)
	.dwattr $C$DW$73, DW_AT_decl_column(0x0d)

$C$DW$74	.dwtag  DW_TAG_member
	.dwattr $C$DW$74, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$74, DW_AT_name("PRU1_IMEM_PE_SET")
	.dwattr $C$DW$74, DW_AT_TI_symbol_name("PRU1_IMEM_PE_SET")
	.dwattr $C$DW$74, DW_AT_bit_offset(0x14)
	.dwattr $C$DW$74, DW_AT_bit_size(0x04)
	.dwattr $C$DW$74, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$74, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$74, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$74, DW_AT_decl_line(0xa6)
	.dwattr $C$DW$74, DW_AT_decl_column(0x0d)

$C$DW$75	.dwtag  DW_TAG_member
	.dwattr $C$DW$75, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$75, DW_AT_name("PRU1_DMEM_PE_SET")
	.dwattr $C$DW$75, DW_AT_TI_symbol_name("PRU1_DMEM_PE_SET")
	.dwattr $C$DW$75, DW_AT_bit_offset(0x10)
	.dwattr $C$DW$75, DW_AT_bit_size(0x04)
	.dwattr $C$DW$75, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$75, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$75, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$75, DW_AT_decl_line(0xa7)
	.dwattr $C$DW$75, DW_AT_decl_column(0x0d)

$C$DW$76	.dwtag  DW_TAG_member
	.dwattr $C$DW$76, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$76, DW_AT_name("RAM_PE_SET")
	.dwattr $C$DW$76, DW_AT_TI_symbol_name("RAM_PE_SET")
	.dwattr $C$DW$76, DW_AT_bit_offset(0x0c)
	.dwattr $C$DW$76, DW_AT_bit_size(0x04)
	.dwattr $C$DW$76, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$76, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$76, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$76, DW_AT_decl_line(0xa8)
	.dwattr $C$DW$76, DW_AT_decl_column(0x0d)

$C$DW$77	.dwtag  DW_TAG_member
	.dwattr $C$DW$77, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$77, DW_AT_name("rsvd20")
	.dwattr $C$DW$77, DW_AT_TI_symbol_name("rsvd20")
	.dwattr $C$DW$77, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$77, DW_AT_bit_size(0x0c)
	.dwattr $C$DW$77, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$77, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$77, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$77, DW_AT_decl_line(0xa9)
	.dwattr $C$DW$77, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$26, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$26, DW_AT_decl_line(0xa3)
	.dwattr $C$DW$T$26, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$26

$C$DW$T$51	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$51, DW_AT_type(*$C$DW$T$26)


$C$DW$T$27	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$27, DW_AT_byte_size(0x04)
$C$DW$78	.dwtag  DW_TAG_member
	.dwattr $C$DW$78, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$78, DW_AT_name("PRU0_IMEM_PE_CLR")
	.dwattr $C$DW$78, DW_AT_TI_symbol_name("PRU0_IMEM_PE_CLR")
	.dwattr $C$DW$78, DW_AT_bit_offset(0x1c)
	.dwattr $C$DW$78, DW_AT_bit_size(0x04)
	.dwattr $C$DW$78, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$78, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$78, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$78, DW_AT_decl_line(0xb3)
	.dwattr $C$DW$78, DW_AT_decl_column(0x0d)

$C$DW$79	.dwtag  DW_TAG_member
	.dwattr $C$DW$79, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$79, DW_AT_name("PRU0_DMEM_PE_CLR")
	.dwattr $C$DW$79, DW_AT_TI_symbol_name("PRU0_DMEM_PE_CLR")
	.dwattr $C$DW$79, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$79, DW_AT_bit_size(0x04)
	.dwattr $C$DW$79, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$79, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$79, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$79, DW_AT_decl_line(0xb4)
	.dwattr $C$DW$79, DW_AT_decl_column(0x0d)

$C$DW$80	.dwtag  DW_TAG_member
	.dwattr $C$DW$80, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$80, DW_AT_name("PRU1_IMEM_PE_CLR")
	.dwattr $C$DW$80, DW_AT_TI_symbol_name("PRU1_IMEM_PE_CLR")
	.dwattr $C$DW$80, DW_AT_bit_offset(0x14)
	.dwattr $C$DW$80, DW_AT_bit_size(0x04)
	.dwattr $C$DW$80, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$80, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$80, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$80, DW_AT_decl_line(0xb5)
	.dwattr $C$DW$80, DW_AT_decl_column(0x0d)

$C$DW$81	.dwtag  DW_TAG_member
	.dwattr $C$DW$81, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$81, DW_AT_name("PRU1_DMEM_PE_CLR")
	.dwattr $C$DW$81, DW_AT_TI_symbol_name("PRU1_DMEM_PE_CLR")
	.dwattr $C$DW$81, DW_AT_bit_offset(0x10)
	.dwattr $C$DW$81, DW_AT_bit_size(0x04)
	.dwattr $C$DW$81, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$81, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$81, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$81, DW_AT_decl_line(0xb6)
	.dwattr $C$DW$81, DW_AT_decl_column(0x0d)

$C$DW$82	.dwtag  DW_TAG_member
	.dwattr $C$DW$82, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$82, DW_AT_name("rsvd16")
	.dwattr $C$DW$82, DW_AT_TI_symbol_name("rsvd16")
	.dwattr $C$DW$82, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$82, DW_AT_bit_size(0x10)
	.dwattr $C$DW$82, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$82, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$82, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$82, DW_AT_decl_line(0xb7)
	.dwattr $C$DW$82, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$27, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$27, DW_AT_decl_line(0xb2)
	.dwattr $C$DW$T$27, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$27

$C$DW$T$53	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$53, DW_AT_type(*$C$DW$T$27)


$C$DW$T$28	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$28, DW_AT_byte_size(0x04)
$C$DW$83	.dwtag  DW_TAG_member
	.dwattr $C$DW$83, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$83, DW_AT_name("PMAO_PRU0")
	.dwattr $C$DW$83, DW_AT_TI_symbol_name("PMAO_PRU0")
	.dwattr $C$DW$83, DW_AT_bit_offset(0x1f)
	.dwattr $C$DW$83, DW_AT_bit_size(0x01)
	.dwattr $C$DW$83, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$83, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$83, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$83, DW_AT_decl_line(0xc4)
	.dwattr $C$DW$83, DW_AT_decl_column(0x0d)

$C$DW$84	.dwtag  DW_TAG_member
	.dwattr $C$DW$84, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$84, DW_AT_name("PMAO_PRU1")
	.dwattr $C$DW$84, DW_AT_TI_symbol_name("PMAO_PRU1")
	.dwattr $C$DW$84, DW_AT_bit_offset(0x1e)
	.dwattr $C$DW$84, DW_AT_bit_size(0x01)
	.dwattr $C$DW$84, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$84, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$84, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$84, DW_AT_decl_line(0xc5)
	.dwattr $C$DW$84, DW_AT_decl_column(0x0d)

$C$DW$85	.dwtag  DW_TAG_member
	.dwattr $C$DW$85, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$85, DW_AT_name("rsvd2")
	.dwattr $C$DW$85, DW_AT_TI_symbol_name("rsvd2")
	.dwattr $C$DW$85, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$85, DW_AT_bit_size(0x1e)
	.dwattr $C$DW$85, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$85, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$85, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$85, DW_AT_decl_line(0xc6)
	.dwattr $C$DW$85, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$28, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$28, DW_AT_decl_line(0xc3)
	.dwattr $C$DW$T$28, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$28

$C$DW$T$55	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$55, DW_AT_type(*$C$DW$T$28)


$C$DW$T$29	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$29, DW_AT_byte_size(0x04)
$C$DW$86	.dwtag  DW_TAG_member
	.dwattr $C$DW$86, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$86, DW_AT_name("OCP_EN")
	.dwattr $C$DW$86, DW_AT_TI_symbol_name("OCP_EN")
	.dwattr $C$DW$86, DW_AT_bit_offset(0x1f)
	.dwattr $C$DW$86, DW_AT_bit_size(0x01)
	.dwattr $C$DW$86, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$86, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$86, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$86, DW_AT_decl_line(0xd3)
	.dwattr $C$DW$86, DW_AT_decl_column(0x0d)

$C$DW$87	.dwtag  DW_TAG_member
	.dwattr $C$DW$87, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$87, DW_AT_name("rsvd1")
	.dwattr $C$DW$87, DW_AT_TI_symbol_name("rsvd1")
	.dwattr $C$DW$87, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$87, DW_AT_bit_size(0x1f)
	.dwattr $C$DW$87, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$87, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$87, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$87, DW_AT_decl_line(0xd4)
	.dwattr $C$DW$87, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$29, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$29, DW_AT_decl_line(0xd2)
	.dwattr $C$DW$T$29, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$29

$C$DW$T$57	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$57, DW_AT_type(*$C$DW$T$29)


$C$DW$T$30	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$30, DW_AT_byte_size(0x04)
$C$DW$88	.dwtag  DW_TAG_member
	.dwattr $C$DW$88, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$88, DW_AT_name("PRU1_PAD_HP_EN")
	.dwattr $C$DW$88, DW_AT_TI_symbol_name("PRU1_PAD_HP_EN")
	.dwattr $C$DW$88, DW_AT_bit_offset(0x1f)
	.dwattr $C$DW$88, DW_AT_bit_size(0x01)
	.dwattr $C$DW$88, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$88, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$88, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$88, DW_AT_decl_line(0xde)
	.dwattr $C$DW$88, DW_AT_decl_column(0x0d)

$C$DW$89	.dwtag  DW_TAG_member
	.dwattr $C$DW$89, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$89, DW_AT_name("XFR_SHIFT_EN")
	.dwattr $C$DW$89, DW_AT_TI_symbol_name("XFR_SHIFT_EN")
	.dwattr $C$DW$89, DW_AT_bit_offset(0x1e)
	.dwattr $C$DW$89, DW_AT_bit_size(0x01)
	.dwattr $C$DW$89, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$89, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$89, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$89, DW_AT_decl_line(0xdf)
	.dwattr $C$DW$89, DW_AT_decl_column(0x0d)

$C$DW$90	.dwtag  DW_TAG_member
	.dwattr $C$DW$90, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$90, DW_AT_name("rsvd2")
	.dwattr $C$DW$90, DW_AT_TI_symbol_name("rsvd2")
	.dwattr $C$DW$90, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$90, DW_AT_bit_size(0x1e)
	.dwattr $C$DW$90, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$90, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$90, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$90, DW_AT_decl_line(0xe0)
	.dwattr $C$DW$90, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$30, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$30, DW_AT_decl_line(0xdd)
	.dwattr $C$DW$T$30, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$30

$C$DW$T$59	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$59, DW_AT_type(*$C$DW$T$30)


$C$DW$T$31	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$31, DW_AT_byte_size(0x04)
$C$DW$91	.dwtag  DW_TAG_member
	.dwattr $C$DW$91, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$91, DW_AT_name("PIN_MUX_SEL")
	.dwattr $C$DW$91, DW_AT_TI_symbol_name("PIN_MUX_SEL")
	.dwattr $C$DW$91, DW_AT_bit_offset(0x18)
	.dwattr $C$DW$91, DW_AT_bit_size(0x08)
	.dwattr $C$DW$91, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$91, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$91, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$91, DW_AT_decl_line(0xec)
	.dwattr $C$DW$91, DW_AT_decl_column(0x0d)

$C$DW$92	.dwtag  DW_TAG_member
	.dwattr $C$DW$92, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$92, DW_AT_name("rsvd2")
	.dwattr $C$DW$92, DW_AT_TI_symbol_name("rsvd2")
	.dwattr $C$DW$92, DW_AT_bit_offset(0x00)
	.dwattr $C$DW$92, DW_AT_bit_size(0x18)
	.dwattr $C$DW$92, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$92, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$92, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$92, DW_AT_decl_line(0xed)
	.dwattr $C$DW$92, DW_AT_decl_column(0x0d)

	.dwattr $C$DW$T$31, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$31, DW_AT_decl_line(0xeb)
	.dwattr $C$DW$T$31, DW_AT_decl_column(0x13)
	.dwendtag $C$DW$T$31

$C$DW$T$61	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$61, DW_AT_type(*$C$DW$T$31)


$C$DW$T$35	.dwtag  DW_TAG_structure_type
	.dwattr $C$DW$T$35, DW_AT_byte_size(0x44)
$C$DW$93	.dwtag  DW_TAG_member
	.dwattr $C$DW$93, DW_AT_type(*$C$DW$T$38)
	.dwattr $C$DW$93, DW_AT_name("$P$T0")
	.dwattr $C$DW$93, DW_AT_TI_symbol_name("$P$T0")
	.dwattr $C$DW$93, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$93, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$93, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$93, DW_AT_decl_line(0x29)
	.dwattr $C$DW$93, DW_AT_decl_column(0x02)

$C$DW$94	.dwtag  DW_TAG_member
	.dwattr $C$DW$94, DW_AT_type(*$C$DW$T$40)
	.dwattr $C$DW$94, DW_AT_name("$P$T1")
	.dwattr $C$DW$94, DW_AT_TI_symbol_name("$P$T1")
	.dwattr $C$DW$94, DW_AT_data_member_location[DW_OP_plus_uconst 0x4]
	.dwattr $C$DW$94, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$94, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$94, DW_AT_decl_line(0x33)
	.dwattr $C$DW$94, DW_AT_decl_column(0x02)

$C$DW$95	.dwtag  DW_TAG_member
	.dwattr $C$DW$95, DW_AT_type(*$C$DW$T$42)
	.dwattr $C$DW$95, DW_AT_name("$P$T2")
	.dwattr $C$DW$95, DW_AT_TI_symbol_name("$P$T2")
	.dwattr $C$DW$95, DW_AT_data_member_location[DW_OP_plus_uconst 0x8]
	.dwattr $C$DW$95, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$95, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$95, DW_AT_decl_line(0x41)
	.dwattr $C$DW$95, DW_AT_decl_column(0x02)

$C$DW$96	.dwtag  DW_TAG_member
	.dwattr $C$DW$96, DW_AT_type(*$C$DW$T$44)
	.dwattr $C$DW$96, DW_AT_name("$P$T3")
	.dwattr $C$DW$96, DW_AT_TI_symbol_name("$P$T3")
	.dwattr $C$DW$96, DW_AT_data_member_location[DW_OP_plus_uconst 0xc]
	.dwattr $C$DW$96, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$96, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$96, DW_AT_decl_line(0x54)
	.dwattr $C$DW$96, DW_AT_decl_column(0x02)

$C$DW$97	.dwtag  DW_TAG_member
	.dwattr $C$DW$97, DW_AT_type(*$C$DW$T$46)
	.dwattr $C$DW$97, DW_AT_name("$P$T4")
	.dwattr $C$DW$97, DW_AT_TI_symbol_name("$P$T4")
	.dwattr $C$DW$97, DW_AT_data_member_location[DW_OP_plus_uconst 0x10]
	.dwattr $C$DW$97, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$97, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$97, DW_AT_decl_line(0x67)
	.dwattr $C$DW$97, DW_AT_decl_column(0x02)

$C$DW$98	.dwtag  DW_TAG_member
	.dwattr $C$DW$98, DW_AT_type(*$C$DW$T$48)
	.dwattr $C$DW$98, DW_AT_name("$P$T5")
	.dwattr $C$DW$98, DW_AT_TI_symbol_name("$P$T5")
	.dwattr $C$DW$98, DW_AT_data_member_location[DW_OP_plus_uconst 0x14]
	.dwattr $C$DW$98, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$98, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$98, DW_AT_decl_line(0x83)
	.dwattr $C$DW$98, DW_AT_decl_column(0x02)

$C$DW$99	.dwtag  DW_TAG_member
	.dwattr $C$DW$99, DW_AT_type(*$C$DW$T$50)
	.dwattr $C$DW$99, DW_AT_name("$P$T6")
	.dwattr $C$DW$99, DW_AT_TI_symbol_name("$P$T6")
	.dwattr $C$DW$99, DW_AT_data_member_location[DW_OP_plus_uconst 0x18]
	.dwattr $C$DW$99, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$99, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$99, DW_AT_decl_line(0x92)
	.dwattr $C$DW$99, DW_AT_decl_column(0x02)

$C$DW$100	.dwtag  DW_TAG_member
	.dwattr $C$DW$100, DW_AT_type(*$C$DW$T$52)
	.dwattr $C$DW$100, DW_AT_name("$P$T7")
	.dwattr $C$DW$100, DW_AT_TI_symbol_name("$P$T7")
	.dwattr $C$DW$100, DW_AT_data_member_location[DW_OP_plus_uconst 0x1c]
	.dwattr $C$DW$100, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$100, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$100, DW_AT_decl_line(0xa0)
	.dwattr $C$DW$100, DW_AT_decl_column(0x02)

$C$DW$101	.dwtag  DW_TAG_member
	.dwattr $C$DW$101, DW_AT_type(*$C$DW$T$54)
	.dwattr $C$DW$101, DW_AT_name("$P$T8")
	.dwattr $C$DW$101, DW_AT_TI_symbol_name("$P$T8")
	.dwattr $C$DW$101, DW_AT_data_member_location[DW_OP_plus_uconst 0x20]
	.dwattr $C$DW$101, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$101, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$101, DW_AT_decl_line(0xaf)
	.dwattr $C$DW$101, DW_AT_decl_column(0x02)

$C$DW$102	.dwtag  DW_TAG_member
	.dwattr $C$DW$102, DW_AT_type(*$C$DW$T$32)
	.dwattr $C$DW$102, DW_AT_name("rsvd24")
	.dwattr $C$DW$102, DW_AT_TI_symbol_name("rsvd24")
	.dwattr $C$DW$102, DW_AT_data_member_location[DW_OP_plus_uconst 0x24]
	.dwattr $C$DW$102, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$102, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$102, DW_AT_decl_line(0xbc)
	.dwattr $C$DW$102, DW_AT_decl_column(0x0b)

$C$DW$103	.dwtag  DW_TAG_member
	.dwattr $C$DW$103, DW_AT_type(*$C$DW$T$56)
	.dwattr $C$DW$103, DW_AT_name("$P$T9")
	.dwattr $C$DW$103, DW_AT_TI_symbol_name("$P$T9")
	.dwattr $C$DW$103, DW_AT_data_member_location[DW_OP_plus_uconst 0x28]
	.dwattr $C$DW$103, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$103, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$103, DW_AT_decl_line(0xc0)
	.dwattr $C$DW$103, DW_AT_decl_column(0x02)

$C$DW$104	.dwtag  DW_TAG_member
	.dwattr $C$DW$104, DW_AT_type(*$C$DW$T$33)
	.dwattr $C$DW$104, DW_AT_name("rsvd2c")
	.dwattr $C$DW$104, DW_AT_TI_symbol_name("rsvd2c")
	.dwattr $C$DW$104, DW_AT_data_member_location[DW_OP_plus_uconst 0x2c]
	.dwattr $C$DW$104, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$104, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$104, DW_AT_decl_line(0xcb)
	.dwattr $C$DW$104, DW_AT_decl_column(0x0b)

$C$DW$105	.dwtag  DW_TAG_member
	.dwattr $C$DW$105, DW_AT_type(*$C$DW$T$58)
	.dwattr $C$DW$105, DW_AT_name("$P$T10")
	.dwattr $C$DW$105, DW_AT_TI_symbol_name("$P$T10")
	.dwattr $C$DW$105, DW_AT_data_member_location[DW_OP_plus_uconst 0x30]
	.dwattr $C$DW$105, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$105, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$105, DW_AT_decl_line(0xcf)
	.dwattr $C$DW$105, DW_AT_decl_column(0x02)

$C$DW$106	.dwtag  DW_TAG_member
	.dwattr $C$DW$106, DW_AT_type(*$C$DW$T$60)
	.dwattr $C$DW$106, DW_AT_name("$P$T11")
	.dwattr $C$DW$106, DW_AT_TI_symbol_name("$P$T11")
	.dwattr $C$DW$106, DW_AT_data_member_location[DW_OP_plus_uconst 0x34]
	.dwattr $C$DW$106, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$106, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$106, DW_AT_decl_line(0xda)
	.dwattr $C$DW$106, DW_AT_decl_column(0x02)

$C$DW$107	.dwtag  DW_TAG_member
	.dwattr $C$DW$107, DW_AT_type(*$C$DW$T$34)
	.dwattr $C$DW$107, DW_AT_name("rsvd38")
	.dwattr $C$DW$107, DW_AT_TI_symbol_name("rsvd38")
	.dwattr $C$DW$107, DW_AT_data_member_location[DW_OP_plus_uconst 0x38]
	.dwattr $C$DW$107, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$107, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$107, DW_AT_decl_line(0xe5)
	.dwattr $C$DW$107, DW_AT_decl_column(0x0b)

$C$DW$108	.dwtag  DW_TAG_member
	.dwattr $C$DW$108, DW_AT_type(*$C$DW$T$62)
	.dwattr $C$DW$108, DW_AT_name("$P$T12")
	.dwattr $C$DW$108, DW_AT_TI_symbol_name("$P$T12")
	.dwattr $C$DW$108, DW_AT_data_member_location[DW_OP_plus_uconst 0x40]
	.dwattr $C$DW$108, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$108, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$108, DW_AT_decl_line(0xe8)
	.dwattr $C$DW$108, DW_AT_decl_column(0x02)

	.dwattr $C$DW$T$35, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$35, DW_AT_decl_line(0x26)
	.dwattr $C$DW$T$35, DW_AT_decl_column(0x10)
	.dwendtag $C$DW$T$35

$C$DW$T$63	.dwtag  DW_TAG_typedef
	.dwattr $C$DW$T$63, DW_AT_name("pruCfg")
	.dwattr $C$DW$T$63, DW_AT_type(*$C$DW$T$35)
	.dwattr $C$DW$T$63, DW_AT_language(DW_LANG_C)
	.dwattr $C$DW$T$63, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$63, DW_AT_decl_line(0xf0)
	.dwattr $C$DW$T$63, DW_AT_decl_column(0x03)

$C$DW$T$64	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$64, DW_AT_type(*$C$DW$T$63)


$C$DW$T$38	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$38, DW_AT_byte_size(0x04)
$C$DW$109	.dwtag  DW_TAG_member
	.dwattr $C$DW$109, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$109, DW_AT_name("REVID")
	.dwattr $C$DW$109, DW_AT_TI_symbol_name("REVID")
	.dwattr $C$DW$109, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$109, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$109, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$109, DW_AT_decl_line(0x2a)
	.dwattr $C$DW$109, DW_AT_decl_column(0x15)

$C$DW$110	.dwtag  DW_TAG_member
	.dwattr $C$DW$110, DW_AT_type(*$C$DW$T$37)
	.dwattr $C$DW$110, DW_AT_name("REVID_bit")
	.dwattr $C$DW$110, DW_AT_TI_symbol_name("REVID_bit")
	.dwattr $C$DW$110, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$110, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$110, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$110, DW_AT_decl_line(0x2e)
	.dwattr $C$DW$110, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$38, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$38, DW_AT_decl_line(0x29)
	.dwattr $C$DW$T$38, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$38


$C$DW$T$40	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$40, DW_AT_byte_size(0x04)
$C$DW$111	.dwtag  DW_TAG_member
	.dwattr $C$DW$111, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$111, DW_AT_name("SYSCFG")
	.dwattr $C$DW$111, DW_AT_TI_symbol_name("SYSCFG")
	.dwattr $C$DW$111, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$111, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$111, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$111, DW_AT_decl_line(0x34)
	.dwattr $C$DW$111, DW_AT_decl_column(0x15)

$C$DW$112	.dwtag  DW_TAG_member
	.dwattr $C$DW$112, DW_AT_type(*$C$DW$T$39)
	.dwattr $C$DW$112, DW_AT_name("SYSCFG_bit")
	.dwattr $C$DW$112, DW_AT_TI_symbol_name("SYSCFG_bit")
	.dwattr $C$DW$112, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$112, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$112, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$112, DW_AT_decl_line(0x3c)
	.dwattr $C$DW$112, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$40, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$40, DW_AT_decl_line(0x33)
	.dwattr $C$DW$T$40, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$40


$C$DW$T$42	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$42, DW_AT_byte_size(0x04)
$C$DW$113	.dwtag  DW_TAG_member
	.dwattr $C$DW$113, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$113, DW_AT_name("GPCFG0")
	.dwattr $C$DW$113, DW_AT_TI_symbol_name("GPCFG0")
	.dwattr $C$DW$113, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$113, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$113, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$113, DW_AT_decl_line(0x42)
	.dwattr $C$DW$113, DW_AT_decl_column(0x15)

$C$DW$114	.dwtag  DW_TAG_member
	.dwattr $C$DW$114, DW_AT_type(*$C$DW$T$41)
	.dwattr $C$DW$114, DW_AT_name("GPCFG0_bit")
	.dwattr $C$DW$114, DW_AT_TI_symbol_name("GPCFG0_bit")
	.dwattr $C$DW$114, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$114, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$114, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$114, DW_AT_decl_line(0x4f)
	.dwattr $C$DW$114, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$42, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$42, DW_AT_decl_line(0x41)
	.dwattr $C$DW$T$42, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$42


$C$DW$T$44	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$44, DW_AT_byte_size(0x04)
$C$DW$115	.dwtag  DW_TAG_member
	.dwattr $C$DW$115, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$115, DW_AT_name("GPCFG1")
	.dwattr $C$DW$115, DW_AT_TI_symbol_name("GPCFG1")
	.dwattr $C$DW$115, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$115, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$115, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$115, DW_AT_decl_line(0x55)
	.dwattr $C$DW$115, DW_AT_decl_column(0x15)

$C$DW$116	.dwtag  DW_TAG_member
	.dwattr $C$DW$116, DW_AT_type(*$C$DW$T$43)
	.dwattr $C$DW$116, DW_AT_name("GPCFG1_bit")
	.dwattr $C$DW$116, DW_AT_TI_symbol_name("GPCFG1_bit")
	.dwattr $C$DW$116, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$116, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$116, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$116, DW_AT_decl_line(0x62)
	.dwattr $C$DW$116, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$44, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$44, DW_AT_decl_line(0x54)
	.dwattr $C$DW$T$44, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$44


$C$DW$T$46	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$46, DW_AT_byte_size(0x04)
$C$DW$117	.dwtag  DW_TAG_member
	.dwattr $C$DW$117, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$117, DW_AT_name("CGR")
	.dwattr $C$DW$117, DW_AT_TI_symbol_name("CGR")
	.dwattr $C$DW$117, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$117, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$117, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$117, DW_AT_decl_line(0x68)
	.dwattr $C$DW$117, DW_AT_decl_column(0x15)

$C$DW$118	.dwtag  DW_TAG_member
	.dwattr $C$DW$118, DW_AT_type(*$C$DW$T$45)
	.dwattr $C$DW$118, DW_AT_name("CGR_bit")
	.dwattr $C$DW$118, DW_AT_TI_symbol_name("CGR_bit")
	.dwattr $C$DW$118, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$118, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$118, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$118, DW_AT_decl_line(0x7e)
	.dwattr $C$DW$118, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$46, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$46, DW_AT_decl_line(0x67)
	.dwattr $C$DW$T$46, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$46


$C$DW$T$48	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$48, DW_AT_byte_size(0x04)
$C$DW$119	.dwtag  DW_TAG_member
	.dwattr $C$DW$119, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$119, DW_AT_name("ISRP")
	.dwattr $C$DW$119, DW_AT_TI_symbol_name("ISRP")
	.dwattr $C$DW$119, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$119, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$119, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$119, DW_AT_decl_line(0x84)
	.dwattr $C$DW$119, DW_AT_decl_column(0x15)

$C$DW$120	.dwtag  DW_TAG_member
	.dwattr $C$DW$120, DW_AT_type(*$C$DW$T$47)
	.dwattr $C$DW$120, DW_AT_name("ISRP_bit")
	.dwattr $C$DW$120, DW_AT_TI_symbol_name("ISRP_bit")
	.dwattr $C$DW$120, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$120, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$120, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$120, DW_AT_decl_line(0x8d)
	.dwattr $C$DW$120, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$48, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$48, DW_AT_decl_line(0x83)
	.dwattr $C$DW$T$48, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$48


$C$DW$T$50	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$50, DW_AT_byte_size(0x04)
$C$DW$121	.dwtag  DW_TAG_member
	.dwattr $C$DW$121, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$121, DW_AT_name("ISP")
	.dwattr $C$DW$121, DW_AT_TI_symbol_name("ISP")
	.dwattr $C$DW$121, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$121, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$121, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$121, DW_AT_decl_line(0x93)
	.dwattr $C$DW$121, DW_AT_decl_column(0x15)

$C$DW$122	.dwtag  DW_TAG_member
	.dwattr $C$DW$122, DW_AT_type(*$C$DW$T$49)
	.dwattr $C$DW$122, DW_AT_name("ISP_bit")
	.dwattr $C$DW$122, DW_AT_TI_symbol_name("ISP_bit")
	.dwattr $C$DW$122, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$122, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$122, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$122, DW_AT_decl_line(0x9c)
	.dwattr $C$DW$122, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$50, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$50, DW_AT_decl_line(0x92)
	.dwattr $C$DW$T$50, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$50


$C$DW$T$52	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$52, DW_AT_byte_size(0x04)
$C$DW$123	.dwtag  DW_TAG_member
	.dwattr $C$DW$123, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$123, DW_AT_name("IESP")
	.dwattr $C$DW$123, DW_AT_TI_symbol_name("IESP")
	.dwattr $C$DW$123, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$123, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$123, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$123, DW_AT_decl_line(0xa1)
	.dwattr $C$DW$123, DW_AT_decl_column(0x15)

$C$DW$124	.dwtag  DW_TAG_member
	.dwattr $C$DW$124, DW_AT_type(*$C$DW$T$51)
	.dwattr $C$DW$124, DW_AT_name("IESP_bit")
	.dwattr $C$DW$124, DW_AT_TI_symbol_name("IESP_bit")
	.dwattr $C$DW$124, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$124, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$124, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$124, DW_AT_decl_line(0xaa)
	.dwattr $C$DW$124, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$52, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$52, DW_AT_decl_line(0xa0)
	.dwattr $C$DW$T$52, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$52


$C$DW$T$54	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$54, DW_AT_byte_size(0x04)
$C$DW$125	.dwtag  DW_TAG_member
	.dwattr $C$DW$125, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$125, DW_AT_name("IECP")
	.dwattr $C$DW$125, DW_AT_TI_symbol_name("IECP")
	.dwattr $C$DW$125, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$125, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$125, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$125, DW_AT_decl_line(0xb0)
	.dwattr $C$DW$125, DW_AT_decl_column(0x15)

$C$DW$126	.dwtag  DW_TAG_member
	.dwattr $C$DW$126, DW_AT_type(*$C$DW$T$53)
	.dwattr $C$DW$126, DW_AT_name("IECP_bit")
	.dwattr $C$DW$126, DW_AT_TI_symbol_name("IECP_bit")
	.dwattr $C$DW$126, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$126, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$126, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$126, DW_AT_decl_line(0xb8)
	.dwattr $C$DW$126, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$54, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$54, DW_AT_decl_line(0xaf)
	.dwattr $C$DW$T$54, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$54


$C$DW$T$56	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$56, DW_AT_byte_size(0x04)
$C$DW$127	.dwtag  DW_TAG_member
	.dwattr $C$DW$127, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$127, DW_AT_name("PMAO")
	.dwattr $C$DW$127, DW_AT_TI_symbol_name("PMAO")
	.dwattr $C$DW$127, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$127, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$127, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$127, DW_AT_decl_line(0xc1)
	.dwattr $C$DW$127, DW_AT_decl_column(0x15)

$C$DW$128	.dwtag  DW_TAG_member
	.dwattr $C$DW$128, DW_AT_type(*$C$DW$T$55)
	.dwattr $C$DW$128, DW_AT_name("PMAO_bit")
	.dwattr $C$DW$128, DW_AT_TI_symbol_name("PMAO_bit")
	.dwattr $C$DW$128, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$128, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$128, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$128, DW_AT_decl_line(0xc7)
	.dwattr $C$DW$128, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$56, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$56, DW_AT_decl_line(0xc0)
	.dwattr $C$DW$T$56, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$56


$C$DW$T$58	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$58, DW_AT_byte_size(0x04)
$C$DW$129	.dwtag  DW_TAG_member
	.dwattr $C$DW$129, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$129, DW_AT_name("IEPCLK")
	.dwattr $C$DW$129, DW_AT_TI_symbol_name("IEPCLK")
	.dwattr $C$DW$129, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$129, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$129, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$129, DW_AT_decl_line(0xd0)
	.dwattr $C$DW$129, DW_AT_decl_column(0x15)

$C$DW$130	.dwtag  DW_TAG_member
	.dwattr $C$DW$130, DW_AT_type(*$C$DW$T$57)
	.dwattr $C$DW$130, DW_AT_name("IEPCLK_bit")
	.dwattr $C$DW$130, DW_AT_TI_symbol_name("IEPCLK_bit")
	.dwattr $C$DW$130, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$130, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$130, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$130, DW_AT_decl_line(0xd5)
	.dwattr $C$DW$130, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$58, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$58, DW_AT_decl_line(0xcf)
	.dwattr $C$DW$T$58, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$58


$C$DW$T$60	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$60, DW_AT_byte_size(0x04)
$C$DW$131	.dwtag  DW_TAG_member
	.dwattr $C$DW$131, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$131, DW_AT_name("SPP")
	.dwattr $C$DW$131, DW_AT_TI_symbol_name("SPP")
	.dwattr $C$DW$131, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$131, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$131, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$131, DW_AT_decl_line(0xdb)
	.dwattr $C$DW$131, DW_AT_decl_column(0x15)

$C$DW$132	.dwtag  DW_TAG_member
	.dwattr $C$DW$132, DW_AT_type(*$C$DW$T$59)
	.dwattr $C$DW$132, DW_AT_name("SPP_bit")
	.dwattr $C$DW$132, DW_AT_TI_symbol_name("SPP_bit")
	.dwattr $C$DW$132, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$132, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$132, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$132, DW_AT_decl_line(0xe1)
	.dwattr $C$DW$132, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$60, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$60, DW_AT_decl_line(0xda)
	.dwattr $C$DW$T$60, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$60


$C$DW$T$62	.dwtag  DW_TAG_union_type
	.dwattr $C$DW$T$62, DW_AT_byte_size(0x04)
$C$DW$133	.dwtag  DW_TAG_member
	.dwattr $C$DW$133, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$133, DW_AT_name("PIN_MX")
	.dwattr $C$DW$133, DW_AT_TI_symbol_name("PIN_MX")
	.dwattr $C$DW$133, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$133, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$133, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$133, DW_AT_decl_line(0xe9)
	.dwattr $C$DW$133, DW_AT_decl_column(0x15)

$C$DW$134	.dwtag  DW_TAG_member
	.dwattr $C$DW$134, DW_AT_type(*$C$DW$T$61)
	.dwattr $C$DW$134, DW_AT_name("PIN_MX_bit")
	.dwattr $C$DW$134, DW_AT_TI_symbol_name("PIN_MX_bit")
	.dwattr $C$DW$134, DW_AT_data_member_location[DW_OP_plus_uconst 0x0]
	.dwattr $C$DW$134, DW_AT_accessibility(DW_ACCESS_public)
	.dwattr $C$DW$134, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$134, DW_AT_decl_line(0xee)
	.dwattr $C$DW$134, DW_AT_decl_column(0x05)

	.dwattr $C$DW$T$62, DW_AT_decl_file("inc/pru_cfg.h")
	.dwattr $C$DW$T$62, DW_AT_decl_line(0xe8)
	.dwattr $C$DW$T$62, DW_AT_decl_column(0x08)
	.dwendtag $C$DW$T$62

$C$DW$T$2	.dwtag  DW_TAG_unspecified_type
	.dwattr $C$DW$T$2, DW_AT_name("void")

$C$DW$T$4	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$4, DW_AT_encoding(DW_ATE_boolean)
	.dwattr $C$DW$T$4, DW_AT_name("bool")
	.dwattr $C$DW$T$4, DW_AT_byte_size(0x01)

$C$DW$T$5	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$5, DW_AT_encoding(DW_ATE_signed_char)
	.dwattr $C$DW$T$5, DW_AT_name("signed char")
	.dwattr $C$DW$T$5, DW_AT_byte_size(0x01)

$C$DW$T$6	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$6, DW_AT_encoding(DW_ATE_unsigned_char)
	.dwattr $C$DW$T$6, DW_AT_name("unsigned char")
	.dwattr $C$DW$T$6, DW_AT_byte_size(0x01)

$C$DW$T$67	.dwtag  DW_TAG_typedef
	.dwattr $C$DW$T$67, DW_AT_name("uint8_t")
	.dwattr $C$DW$T$67, DW_AT_type(*$C$DW$T$6)
	.dwattr $C$DW$T$67, DW_AT_language(DW_LANG_C)
	.dwattr $C$DW$T$67, DW_AT_decl_file("/Applications/ti/ti-cgt-pru_2.3.3/include/stdint.h")
	.dwattr $C$DW$T$67, DW_AT_decl_line(0x3d)
	.dwattr $C$DW$T$67, DW_AT_decl_column(0x1c)

$C$DW$T$7	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$7, DW_AT_encoding(DW_ATE_signed_char)
	.dwattr $C$DW$T$7, DW_AT_name("wchar_t")
	.dwattr $C$DW$T$7, DW_AT_byte_size(0x04)

$C$DW$T$8	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$8, DW_AT_encoding(DW_ATE_signed)
	.dwattr $C$DW$T$8, DW_AT_name("short")
	.dwattr $C$DW$T$8, DW_AT_byte_size(0x02)

$C$DW$T$9	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$9, DW_AT_encoding(DW_ATE_unsigned)
	.dwattr $C$DW$T$9, DW_AT_name("unsigned short")
	.dwattr $C$DW$T$9, DW_AT_byte_size(0x02)

$C$DW$T$10	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$10, DW_AT_encoding(DW_ATE_signed)
	.dwattr $C$DW$T$10, DW_AT_name("int")
	.dwattr $C$DW$T$10, DW_AT_byte_size(0x04)

$C$DW$T$11	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$11, DW_AT_encoding(DW_ATE_unsigned)
	.dwattr $C$DW$T$11, DW_AT_name("unsigned int")
	.dwattr $C$DW$T$11, DW_AT_byte_size(0x04)

$C$DW$T$32	.dwtag  DW_TAG_typedef
	.dwattr $C$DW$T$32, DW_AT_name("uint32_t")
	.dwattr $C$DW$T$32, DW_AT_type(*$C$DW$T$11)
	.dwattr $C$DW$T$32, DW_AT_language(DW_LANG_C)
	.dwattr $C$DW$T$32, DW_AT_decl_file("/Applications/ti/ti-cgt-pru_2.3.3/include/stdint.h")
	.dwattr $C$DW$T$32, DW_AT_decl_line(0x41)
	.dwattr $C$DW$T$32, DW_AT_decl_column(0x1c)


$C$DW$T$33	.dwtag  DW_TAG_array_type
	.dwattr $C$DW$T$33, DW_AT_type(*$C$DW$T$32)
	.dwattr $C$DW$T$33, DW_AT_language(DW_LANG_C)
	.dwattr $C$DW$T$33, DW_AT_byte_size(0x04)
$C$DW$135	.dwtag  DW_TAG_subrange_type
	.dwattr $C$DW$135, DW_AT_upper_bound(0x00)

	.dwendtag $C$DW$T$33


$C$DW$T$34	.dwtag  DW_TAG_array_type
	.dwattr $C$DW$T$34, DW_AT_type(*$C$DW$T$32)
	.dwattr $C$DW$T$34, DW_AT_language(DW_LANG_C)
	.dwattr $C$DW$T$34, DW_AT_byte_size(0x08)
$C$DW$136	.dwtag  DW_TAG_subrange_type
	.dwattr $C$DW$136, DW_AT_upper_bound(0x01)

	.dwendtag $C$DW$T$34

$C$DW$T$36	.dwtag  DW_TAG_volatile_type
	.dwattr $C$DW$T$36, DW_AT_type(*$C$DW$T$32)

$C$DW$T$68	.dwtag  DW_TAG_pointer_type
	.dwattr $C$DW$T$68, DW_AT_type(*$C$DW$T$36)
	.dwattr $C$DW$T$68, DW_AT_address_class(0x20)

$C$DW$T$12	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$12, DW_AT_encoding(DW_ATE_signed)
	.dwattr $C$DW$T$12, DW_AT_name("long")
	.dwattr $C$DW$T$12, DW_AT_byte_size(0x04)

$C$DW$T$13	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$13, DW_AT_encoding(DW_ATE_unsigned)
	.dwattr $C$DW$T$13, DW_AT_name("unsigned long")
	.dwattr $C$DW$T$13, DW_AT_byte_size(0x04)

$C$DW$T$14	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$14, DW_AT_encoding(DW_ATE_signed)
	.dwattr $C$DW$T$14, DW_AT_name("long long")
	.dwattr $C$DW$T$14, DW_AT_byte_size(0x08)

$C$DW$T$15	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$15, DW_AT_encoding(DW_ATE_unsigned)
	.dwattr $C$DW$T$15, DW_AT_name("unsigned long long")
	.dwattr $C$DW$T$15, DW_AT_byte_size(0x08)

$C$DW$T$16	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$16, DW_AT_encoding(DW_ATE_float)
	.dwattr $C$DW$T$16, DW_AT_name("float")
	.dwattr $C$DW$T$16, DW_AT_byte_size(0x04)

$C$DW$T$17	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$17, DW_AT_encoding(DW_ATE_float)
	.dwattr $C$DW$T$17, DW_AT_name("double")
	.dwattr $C$DW$T$17, DW_AT_byte_size(0x08)

$C$DW$T$18	.dwtag  DW_TAG_base_type
	.dwattr $C$DW$T$18, DW_AT_encoding(DW_ATE_float)
	.dwattr $C$DW$T$18, DW_AT_name("long double")
	.dwattr $C$DW$T$18, DW_AT_byte_size(0x08)

	.dwattr $C$DW$CU, DW_AT_language(DW_LANG_C)

;***************************************************************
;* DWARF CIE ENTRIES                                           *
;***************************************************************

$C$DW$CIE	.dwcie 14
	.dwcfi	cfa_register, 8
	.dwcfi	cfa_offset, 0
	.dwcfi	same_value, 8
	.dwcfi	same_value, 9
	.dwcfi	same_value, 10
	.dwcfi	same_value, 11
	.dwcfi	same_value, 16
	.dwcfi	same_value, 17
	.dwcfi	same_value, 18
	.dwcfi	same_value, 19
	.dwcfi	same_value, 20
	.dwcfi	same_value, 21
	.dwcfi	same_value, 22
	.dwcfi	same_value, 23
	.dwcfi	same_value, 24
	.dwcfi	same_value, 25
	.dwcfi	same_value, 26
	.dwcfi	same_value, 27
	.dwcfi	same_value, 28
	.dwcfi	same_value, 29
	.dwcfi	same_value, 30
	.dwcfi	same_value, 31
	.dwcfi	same_value, 32
	.dwcfi	same_value, 33
	.dwcfi	same_value, 34
	.dwcfi	same_value, 35
	.dwcfi	same_value, 36
	.dwcfi	same_value, 37
	.dwcfi	same_value, 38
	.dwcfi	same_value, 39
	.dwcfi	same_value, 40
	.dwcfi	same_value, 41
	.dwcfi	same_value, 42
	.dwcfi	same_value, 43
	.dwcfi	same_value, 44
	.dwcfi	same_value, 45
	.dwcfi	same_value, 46
	.dwcfi	same_value, 47
	.dwcfi	same_value, 48
	.dwcfi	same_value, 49
	.dwcfi	same_value, 50
	.dwcfi	same_value, 51
	.dwcfi	same_value, 52
	.dwcfi	same_value, 53
	.dwcfi	same_value, 54
	.dwcfi	same_value, 55
	.dwendentry

;***************************************************************
;* DWARF REGISTER MAP                                          *
;***************************************************************

$C$DW$137	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$137, DW_AT_name("R0_b0")
	.dwattr $C$DW$137, DW_AT_location[DW_OP_reg0]

$C$DW$138	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$138, DW_AT_name("R0_b1")
	.dwattr $C$DW$138, DW_AT_location[DW_OP_reg1]

$C$DW$139	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$139, DW_AT_name("R0_b2")
	.dwattr $C$DW$139, DW_AT_location[DW_OP_reg2]

$C$DW$140	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$140, DW_AT_name("R0_b3")
	.dwattr $C$DW$140, DW_AT_location[DW_OP_reg3]

$C$DW$141	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$141, DW_AT_name("R1_b0")
	.dwattr $C$DW$141, DW_AT_location[DW_OP_reg4]

$C$DW$142	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$142, DW_AT_name("R1_b1")
	.dwattr $C$DW$142, DW_AT_location[DW_OP_reg5]

$C$DW$143	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$143, DW_AT_name("R1_b2")
	.dwattr $C$DW$143, DW_AT_location[DW_OP_reg6]

$C$DW$144	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$144, DW_AT_name("R1_b3")
	.dwattr $C$DW$144, DW_AT_location[DW_OP_reg7]

$C$DW$145	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$145, DW_AT_name("R2_b0")
	.dwattr $C$DW$145, DW_AT_location[DW_OP_reg8]

$C$DW$146	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$146, DW_AT_name("R2_b1")
	.dwattr $C$DW$146, DW_AT_location[DW_OP_reg9]

$C$DW$147	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$147, DW_AT_name("R2_b2")
	.dwattr $C$DW$147, DW_AT_location[DW_OP_reg10]

$C$DW$148	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$148, DW_AT_name("R2_b3")
	.dwattr $C$DW$148, DW_AT_location[DW_OP_reg11]

$C$DW$149	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$149, DW_AT_name("R3_b0")
	.dwattr $C$DW$149, DW_AT_location[DW_OP_reg12]

$C$DW$150	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$150, DW_AT_name("R3_b1")
	.dwattr $C$DW$150, DW_AT_location[DW_OP_reg13]

$C$DW$151	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$151, DW_AT_name("R3_b2")
	.dwattr $C$DW$151, DW_AT_location[DW_OP_reg14]

$C$DW$152	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$152, DW_AT_name("R3_b3")
	.dwattr $C$DW$152, DW_AT_location[DW_OP_reg15]

$C$DW$153	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$153, DW_AT_name("R4_b0")
	.dwattr $C$DW$153, DW_AT_location[DW_OP_reg16]

$C$DW$154	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$154, DW_AT_name("R4_b1")
	.dwattr $C$DW$154, DW_AT_location[DW_OP_reg17]

$C$DW$155	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$155, DW_AT_name("R4_b2")
	.dwattr $C$DW$155, DW_AT_location[DW_OP_reg18]

$C$DW$156	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$156, DW_AT_name("R4_b3")
	.dwattr $C$DW$156, DW_AT_location[DW_OP_reg19]

$C$DW$157	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$157, DW_AT_name("R5_b0")
	.dwattr $C$DW$157, DW_AT_location[DW_OP_reg20]

$C$DW$158	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$158, DW_AT_name("R5_b1")
	.dwattr $C$DW$158, DW_AT_location[DW_OP_reg21]

$C$DW$159	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$159, DW_AT_name("R5_b2")
	.dwattr $C$DW$159, DW_AT_location[DW_OP_reg22]

$C$DW$160	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$160, DW_AT_name("R5_b3")
	.dwattr $C$DW$160, DW_AT_location[DW_OP_reg23]

$C$DW$161	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$161, DW_AT_name("R6_b0")
	.dwattr $C$DW$161, DW_AT_location[DW_OP_reg24]

$C$DW$162	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$162, DW_AT_name("R6_b1")
	.dwattr $C$DW$162, DW_AT_location[DW_OP_reg25]

$C$DW$163	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$163, DW_AT_name("R6_b2")
	.dwattr $C$DW$163, DW_AT_location[DW_OP_reg26]

$C$DW$164	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$164, DW_AT_name("R6_b3")
	.dwattr $C$DW$164, DW_AT_location[DW_OP_reg27]

$C$DW$165	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$165, DW_AT_name("R7_b0")
	.dwattr $C$DW$165, DW_AT_location[DW_OP_reg28]

$C$DW$166	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$166, DW_AT_name("R7_b1")
	.dwattr $C$DW$166, DW_AT_location[DW_OP_reg29]

$C$DW$167	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$167, DW_AT_name("R7_b2")
	.dwattr $C$DW$167, DW_AT_location[DW_OP_reg30]

$C$DW$168	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$168, DW_AT_name("R7_b3")
	.dwattr $C$DW$168, DW_AT_location[DW_OP_reg31]

$C$DW$169	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$169, DW_AT_name("R8_b0")
	.dwattr $C$DW$169, DW_AT_location[DW_OP_regx 0x20]

$C$DW$170	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$170, DW_AT_name("R8_b1")
	.dwattr $C$DW$170, DW_AT_location[DW_OP_regx 0x21]

$C$DW$171	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$171, DW_AT_name("R8_b2")
	.dwattr $C$DW$171, DW_AT_location[DW_OP_regx 0x22]

$C$DW$172	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$172, DW_AT_name("R8_b3")
	.dwattr $C$DW$172, DW_AT_location[DW_OP_regx 0x23]

$C$DW$173	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$173, DW_AT_name("R9_b0")
	.dwattr $C$DW$173, DW_AT_location[DW_OP_regx 0x24]

$C$DW$174	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$174, DW_AT_name("R9_b1")
	.dwattr $C$DW$174, DW_AT_location[DW_OP_regx 0x25]

$C$DW$175	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$175, DW_AT_name("R9_b2")
	.dwattr $C$DW$175, DW_AT_location[DW_OP_regx 0x26]

$C$DW$176	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$176, DW_AT_name("R9_b3")
	.dwattr $C$DW$176, DW_AT_location[DW_OP_regx 0x27]

$C$DW$177	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$177, DW_AT_name("R10_b0")
	.dwattr $C$DW$177, DW_AT_location[DW_OP_regx 0x28]

$C$DW$178	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$178, DW_AT_name("R10_b1")
	.dwattr $C$DW$178, DW_AT_location[DW_OP_regx 0x29]

$C$DW$179	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$179, DW_AT_name("R10_b2")
	.dwattr $C$DW$179, DW_AT_location[DW_OP_regx 0x2a]

$C$DW$180	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$180, DW_AT_name("R10_b3")
	.dwattr $C$DW$180, DW_AT_location[DW_OP_regx 0x2b]

$C$DW$181	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$181, DW_AT_name("R11_b0")
	.dwattr $C$DW$181, DW_AT_location[DW_OP_regx 0x2c]

$C$DW$182	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$182, DW_AT_name("R11_b1")
	.dwattr $C$DW$182, DW_AT_location[DW_OP_regx 0x2d]

$C$DW$183	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$183, DW_AT_name("R11_b2")
	.dwattr $C$DW$183, DW_AT_location[DW_OP_regx 0x2e]

$C$DW$184	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$184, DW_AT_name("R11_b3")
	.dwattr $C$DW$184, DW_AT_location[DW_OP_regx 0x2f]

$C$DW$185	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$185, DW_AT_name("R12_b0")
	.dwattr $C$DW$185, DW_AT_location[DW_OP_regx 0x30]

$C$DW$186	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$186, DW_AT_name("R12_b1")
	.dwattr $C$DW$186, DW_AT_location[DW_OP_regx 0x31]

$C$DW$187	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$187, DW_AT_name("R12_b2")
	.dwattr $C$DW$187, DW_AT_location[DW_OP_regx 0x32]

$C$DW$188	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$188, DW_AT_name("R12_b3")
	.dwattr $C$DW$188, DW_AT_location[DW_OP_regx 0x33]

$C$DW$189	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$189, DW_AT_name("R13_b0")
	.dwattr $C$DW$189, DW_AT_location[DW_OP_regx 0x34]

$C$DW$190	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$190, DW_AT_name("R13_b1")
	.dwattr $C$DW$190, DW_AT_location[DW_OP_regx 0x35]

$C$DW$191	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$191, DW_AT_name("R13_b2")
	.dwattr $C$DW$191, DW_AT_location[DW_OP_regx 0x36]

$C$DW$192	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$192, DW_AT_name("R13_b3")
	.dwattr $C$DW$192, DW_AT_location[DW_OP_regx 0x37]

$C$DW$193	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$193, DW_AT_name("R14_b0")
	.dwattr $C$DW$193, DW_AT_location[DW_OP_regx 0x38]

$C$DW$194	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$194, DW_AT_name("R14_b1")
	.dwattr $C$DW$194, DW_AT_location[DW_OP_regx 0x39]

$C$DW$195	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$195, DW_AT_name("R14_b2")
	.dwattr $C$DW$195, DW_AT_location[DW_OP_regx 0x3a]

$C$DW$196	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$196, DW_AT_name("R14_b3")
	.dwattr $C$DW$196, DW_AT_location[DW_OP_regx 0x3b]

$C$DW$197	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$197, DW_AT_name("R15_b0")
	.dwattr $C$DW$197, DW_AT_location[DW_OP_regx 0x3c]

$C$DW$198	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$198, DW_AT_name("R15_b1")
	.dwattr $C$DW$198, DW_AT_location[DW_OP_regx 0x3d]

$C$DW$199	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$199, DW_AT_name("R15_b2")
	.dwattr $C$DW$199, DW_AT_location[DW_OP_regx 0x3e]

$C$DW$200	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$200, DW_AT_name("R15_b3")
	.dwattr $C$DW$200, DW_AT_location[DW_OP_regx 0x3f]

$C$DW$201	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$201, DW_AT_name("R16_b0")
	.dwattr $C$DW$201, DW_AT_location[DW_OP_regx 0x40]

$C$DW$202	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$202, DW_AT_name("R16_b1")
	.dwattr $C$DW$202, DW_AT_location[DW_OP_regx 0x41]

$C$DW$203	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$203, DW_AT_name("R16_b2")
	.dwattr $C$DW$203, DW_AT_location[DW_OP_regx 0x42]

$C$DW$204	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$204, DW_AT_name("R16_b3")
	.dwattr $C$DW$204, DW_AT_location[DW_OP_regx 0x43]

$C$DW$205	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$205, DW_AT_name("R17_b0")
	.dwattr $C$DW$205, DW_AT_location[DW_OP_regx 0x44]

$C$DW$206	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$206, DW_AT_name("R17_b1")
	.dwattr $C$DW$206, DW_AT_location[DW_OP_regx 0x45]

$C$DW$207	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$207, DW_AT_name("R17_b2")
	.dwattr $C$DW$207, DW_AT_location[DW_OP_regx 0x46]

$C$DW$208	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$208, DW_AT_name("R17_b3")
	.dwattr $C$DW$208, DW_AT_location[DW_OP_regx 0x47]

$C$DW$209	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$209, DW_AT_name("R18_b0")
	.dwattr $C$DW$209, DW_AT_location[DW_OP_regx 0x48]

$C$DW$210	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$210, DW_AT_name("R18_b1")
	.dwattr $C$DW$210, DW_AT_location[DW_OP_regx 0x49]

$C$DW$211	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$211, DW_AT_name("R18_b2")
	.dwattr $C$DW$211, DW_AT_location[DW_OP_regx 0x4a]

$C$DW$212	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$212, DW_AT_name("R18_b3")
	.dwattr $C$DW$212, DW_AT_location[DW_OP_regx 0x4b]

$C$DW$213	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$213, DW_AT_name("R19_b0")
	.dwattr $C$DW$213, DW_AT_location[DW_OP_regx 0x4c]

$C$DW$214	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$214, DW_AT_name("R19_b1")
	.dwattr $C$DW$214, DW_AT_location[DW_OP_regx 0x4d]

$C$DW$215	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$215, DW_AT_name("R19_b2")
	.dwattr $C$DW$215, DW_AT_location[DW_OP_regx 0x4e]

$C$DW$216	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$216, DW_AT_name("R19_b3")
	.dwattr $C$DW$216, DW_AT_location[DW_OP_regx 0x4f]

$C$DW$217	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$217, DW_AT_name("R20_b0")
	.dwattr $C$DW$217, DW_AT_location[DW_OP_regx 0x50]

$C$DW$218	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$218, DW_AT_name("R20_b1")
	.dwattr $C$DW$218, DW_AT_location[DW_OP_regx 0x51]

$C$DW$219	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$219, DW_AT_name("R20_b2")
	.dwattr $C$DW$219, DW_AT_location[DW_OP_regx 0x52]

$C$DW$220	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$220, DW_AT_name("R20_b3")
	.dwattr $C$DW$220, DW_AT_location[DW_OP_regx 0x53]

$C$DW$221	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$221, DW_AT_name("R21_b0")
	.dwattr $C$DW$221, DW_AT_location[DW_OP_regx 0x54]

$C$DW$222	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$222, DW_AT_name("R21_b1")
	.dwattr $C$DW$222, DW_AT_location[DW_OP_regx 0x55]

$C$DW$223	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$223, DW_AT_name("R21_b2")
	.dwattr $C$DW$223, DW_AT_location[DW_OP_regx 0x56]

$C$DW$224	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$224, DW_AT_name("R21_b3")
	.dwattr $C$DW$224, DW_AT_location[DW_OP_regx 0x57]

$C$DW$225	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$225, DW_AT_name("R22_b0")
	.dwattr $C$DW$225, DW_AT_location[DW_OP_regx 0x58]

$C$DW$226	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$226, DW_AT_name("R22_b1")
	.dwattr $C$DW$226, DW_AT_location[DW_OP_regx 0x59]

$C$DW$227	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$227, DW_AT_name("R22_b2")
	.dwattr $C$DW$227, DW_AT_location[DW_OP_regx 0x5a]

$C$DW$228	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$228, DW_AT_name("R22_b3")
	.dwattr $C$DW$228, DW_AT_location[DW_OP_regx 0x5b]

$C$DW$229	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$229, DW_AT_name("R23_b0")
	.dwattr $C$DW$229, DW_AT_location[DW_OP_regx 0x5c]

$C$DW$230	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$230, DW_AT_name("R23_b1")
	.dwattr $C$DW$230, DW_AT_location[DW_OP_regx 0x5d]

$C$DW$231	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$231, DW_AT_name("R23_b2")
	.dwattr $C$DW$231, DW_AT_location[DW_OP_regx 0x5e]

$C$DW$232	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$232, DW_AT_name("R23_b3")
	.dwattr $C$DW$232, DW_AT_location[DW_OP_regx 0x5f]

$C$DW$233	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$233, DW_AT_name("R24_b0")
	.dwattr $C$DW$233, DW_AT_location[DW_OP_regx 0x60]

$C$DW$234	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$234, DW_AT_name("R24_b1")
	.dwattr $C$DW$234, DW_AT_location[DW_OP_regx 0x61]

$C$DW$235	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$235, DW_AT_name("R24_b2")
	.dwattr $C$DW$235, DW_AT_location[DW_OP_regx 0x62]

$C$DW$236	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$236, DW_AT_name("R24_b3")
	.dwattr $C$DW$236, DW_AT_location[DW_OP_regx 0x63]

$C$DW$237	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$237, DW_AT_name("R25_b0")
	.dwattr $C$DW$237, DW_AT_location[DW_OP_regx 0x64]

$C$DW$238	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$238, DW_AT_name("R25_b1")
	.dwattr $C$DW$238, DW_AT_location[DW_OP_regx 0x65]

$C$DW$239	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$239, DW_AT_name("R25_b2")
	.dwattr $C$DW$239, DW_AT_location[DW_OP_regx 0x66]

$C$DW$240	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$240, DW_AT_name("R25_b3")
	.dwattr $C$DW$240, DW_AT_location[DW_OP_regx 0x67]

$C$DW$241	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$241, DW_AT_name("R26_b0")
	.dwattr $C$DW$241, DW_AT_location[DW_OP_regx 0x68]

$C$DW$242	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$242, DW_AT_name("R26_b1")
	.dwattr $C$DW$242, DW_AT_location[DW_OP_regx 0x69]

$C$DW$243	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$243, DW_AT_name("R26_b2")
	.dwattr $C$DW$243, DW_AT_location[DW_OP_regx 0x6a]

$C$DW$244	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$244, DW_AT_name("R26_b3")
	.dwattr $C$DW$244, DW_AT_location[DW_OP_regx 0x6b]

$C$DW$245	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$245, DW_AT_name("R27_b0")
	.dwattr $C$DW$245, DW_AT_location[DW_OP_regx 0x6c]

$C$DW$246	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$246, DW_AT_name("R27_b1")
	.dwattr $C$DW$246, DW_AT_location[DW_OP_regx 0x6d]

$C$DW$247	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$247, DW_AT_name("R27_b2")
	.dwattr $C$DW$247, DW_AT_location[DW_OP_regx 0x6e]

$C$DW$248	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$248, DW_AT_name("R27_b3")
	.dwattr $C$DW$248, DW_AT_location[DW_OP_regx 0x6f]

$C$DW$249	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$249, DW_AT_name("R28_b0")
	.dwattr $C$DW$249, DW_AT_location[DW_OP_regx 0x70]

$C$DW$250	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$250, DW_AT_name("R28_b1")
	.dwattr $C$DW$250, DW_AT_location[DW_OP_regx 0x71]

$C$DW$251	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$251, DW_AT_name("R28_b2")
	.dwattr $C$DW$251, DW_AT_location[DW_OP_regx 0x72]

$C$DW$252	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$252, DW_AT_name("R28_b3")
	.dwattr $C$DW$252, DW_AT_location[DW_OP_regx 0x73]

$C$DW$253	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$253, DW_AT_name("R29_b0")
	.dwattr $C$DW$253, DW_AT_location[DW_OP_regx 0x74]

$C$DW$254	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$254, DW_AT_name("R29_b1")
	.dwattr $C$DW$254, DW_AT_location[DW_OP_regx 0x75]

$C$DW$255	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$255, DW_AT_name("R29_b2")
	.dwattr $C$DW$255, DW_AT_location[DW_OP_regx 0x76]

$C$DW$256	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$256, DW_AT_name("R29_b3")
	.dwattr $C$DW$256, DW_AT_location[DW_OP_regx 0x77]

$C$DW$257	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$257, DW_AT_name("R30_b0")
	.dwattr $C$DW$257, DW_AT_location[DW_OP_regx 0x78]

$C$DW$258	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$258, DW_AT_name("R30_b1")
	.dwattr $C$DW$258, DW_AT_location[DW_OP_regx 0x79]

$C$DW$259	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$259, DW_AT_name("R30_b2")
	.dwattr $C$DW$259, DW_AT_location[DW_OP_regx 0x7a]

$C$DW$260	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$260, DW_AT_name("R30_b3")
	.dwattr $C$DW$260, DW_AT_location[DW_OP_regx 0x7b]

$C$DW$261	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$261, DW_AT_name("R31_b0")
	.dwattr $C$DW$261, DW_AT_location[DW_OP_regx 0x7c]

$C$DW$262	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$262, DW_AT_name("R31_b1")
	.dwattr $C$DW$262, DW_AT_location[DW_OP_regx 0x7d]

$C$DW$263	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$263, DW_AT_name("R31_b2")
	.dwattr $C$DW$263, DW_AT_location[DW_OP_regx 0x7e]

$C$DW$264	.dwtag  DW_TAG_TI_assign_register
	.dwattr $C$DW$264, DW_AT_name("R31_b3")
	.dwattr $C$DW$264, DW_AT_location[DW_OP_regx 0x7f]

	.dwendtag $C$DW$CU

