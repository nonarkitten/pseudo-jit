#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* opcodes[] = {
    "ORI.BW #,%E",
    "ORI.BW #,%E",
    "ORI.BW #,%E",
    "ORI.BW #,%E",
    "ORI.BW #,%E",
    "ORI to CCR",
    "ORI to SR",
    "ORI.L #,%E",
    "ORI.L #,%E",
    "ORI.L #,%E",
    "ORI.L #,%E",
    "ORI.L #,%E",
    "ANDI.BW #,%E",
    "ANDI.BW #,%E",
    "ANDI.BW #,%E",
    "ANDI.BW #,%E",
    "ANDI.BW #,%E",
    "ANDI to CCR",
    "ANDI to SR",
    "ANDI.L #,%E",
    "ANDI.L #,%E",
    "ANDI.L #,%E",
    "ANDI.L #,%E",
    "ANDI.L #,%E",
    "SUBI.BW #,%E",
    "SUBI.BW #,%E",
    "SUBI.BW #,%E",
    "SUBI.BW #,%E",
    "SUBI.BW #,%E",
    "SUBI.L #,%E",
    "SUBI.L #,%E",
    "SUBI.L #,%E",
    "SUBI.L #,%E",
    "SUBI.L #,%E",
    "ADDI.BW #,%E",
    "ADDI.BW #,%E",
    "ADDI.BW #,%E",
    "ADDI.BW #,%E",
    "ADDI.BW #,%E",
    "ADDI.L #,%E",
    "ADDI.L #,%E",
    "ADDI.L #,%E",
    "ADDI.L #,%E",
    "ADDI.L #,%E",
    "EORI.BW #,%E",
    "EORI.BW #,%E",
    "EORI.BW #,%E",
    "EORI.BW #,%E",
    "EORI.BW #,%E",
    "EORI to CCR",
    "EORI to SR",
    "EORI.L #,%E",
    "EORI.L #,%E",
    "EORI.L #,%E",
    "EORI.L #,%E",
    "EORI.L #,%E",
    "CMPI.BW #,%E",
    "CMPI.BW #,%E",
    "CMPI.BW #,%E",
    "CMPI.BW #,%E",
    "CMPI.BW #,%E",
    "CMPI.L #,%E",
    "CMPI.L #,%E",
    "CMPI.L #,%E",
    "CMPI.L #,%E",
    "CMPI.L #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCHG #,%E",
    "BCHG #,%E",
    "BCHG #,%E",
    "BCHG #,%E",
    "BCHG #,%E",
    "BCHG D%N,%E",
    "BCHG D%N,%E",
    "BCHG D%N,%E",
    "BCHG D%N,%E",
    "BCHG D%N,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR #,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BCLR D%N,%E",
    "BSET #,%E",
    "BSET #,%E",
    "BSET #,%E",
    "BSET #,%E",
    "BSET #,%E",
    "BSET D%N,%E",
    "BSET D%N,%E",
    "BSET D%N,%E",
    "BSET D%N,%E",
    "BSET D%N,%E",
    "MOVEP %E,D%N",
    "MOVEP D%N,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.B %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVEA.L %E,%E",
    "MOVEA.L %E,%E",
    "MOVEA.L %E,%E",
    "MOVEA.L %E,%E",
    "MOVEA.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.L %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVEA.W %E,%E",
    "MOVEA.W %E,%E",
    "MOVEA.W %E,%E",
    "MOVEA.W %E,%E",
    "MOVEA.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE.W %E,%E",
    "MOVE SR,%E",
    "MOVE SR,%E",
    "MOVE SR,%E",
    "MOVE SR,%E",
    "MOVE SR,%E",
    "MOVE %E,CCR",
    "MOVE %E,CCR",
    "MOVE %E,CCR",
    "MOVE %E,CCR",
    "MOVE %E,CCR",
    "MOVE %E,CCR",
    "MOVE %E,SR",
    "MOVE %E,SR",
    "MOVE %E,SR",
    "MOVE %E,SR",
    "MOVE %E,SR",
    "MOVE %E,SR",
    "NEGX.BW %E",
    "NEGX.BW %E",
    "NEGX.BW %E",
    "NEGX.BW %E",
    "NEGX.BW %E",
    "NEGX.L %E",
    "NEGX.L %E",
    "NEGX.L %E",
    "NEGX.L %E",
    "NEGX.L %E",
    "CLR.BW %E",
    "CLR.BW %E",
    "CLR.BW %E",
    "CLR.BW %E",
    "CLR.BW %E",
    "CLR.L %E",
    "CLR.L %E",
    "CLR.L %E",
    "CLR.L %E",
    "CLR.L %E",
    "NEG.BW %E",
    "NEG.BW %E",
    "NEG.BW %E",
    "NEG.BW %E",
    "NEG.BW %E",
    "NEG.L %E",
    "NEG.L %E",
    "NEG.L %E",
    "NEG.L %E",
    "NEG.L %E",
    "NOT.BW %E",
    "NOT.BW %E",
    "NOT.BW %E",
    "NOT.BW %E",
    "NOT.BW %E",
    "NOT.L %E",
    "NOT.L %E",
    "NOT.L %E",
    "NOT.L %E",
    "NOT.L %E",
    "EXT.W %E",
    "EXT.L %E",
    "NBCD #,%E",
    "NBCD #,%E",
    "NBCD #,%E",
    "NBCD #,%E",
    "NBCD #,%E",
    "SWAP %E",
    "PEA %E",
    "PEA %E",
    "PEA %E",
    "PEA %E",
    "ILLEGAL",
    "TAS %E",
    "TAS %E",
    "TAS %E",
    "TAS %E",
    "TAS %E",
    "TST.BW %E",
    "TST.BW %E",
    "TST.BW %E",
    "TST.BW %E",
    "TST.BW %E",
    "TST.L %E",
    "TST.L %E",
    "TST.L %E",
    "TST.L %E",
    "TST.L %E",
    "TRAP #%R",
    "TRAP #8",
    "TRAP #9",
    "TRAP #10",
    "TRAP #11",
    "TRAP #12",
    "TRAP #13",
    "TRAP #14",
    "TRAP #15",
    "LINK A%R",
    "UNLK A%R",
    "MOVE A%R,USP",
    "MOVE USP,A%R",
    "RESET",
    "NOP",
    "STOP",
    "RTE",
    "RTS",
    "TRAPV",
    "RTR",
    "JSR %E",
    "JSR %E",
    "JSR %E",
    "JSR %E",
    "JMP %E",
    "JMP %E",
    "JMP %E",
    "JMP %E",
    "MOVEM <list>,%E",
    "MOVEM <list>,%E",
    "MOVEM <list>,%E",
    "MOVEM <list>,%E",
    "MOVEM %E,<list>",
    "MOVEM %E,<list>",
    "MOVEM %E,<list>",
    "MOVEM %E,<list>",
    "LEA %E,A%R",
    "LEA %E,A%R",
    "LEA %E,A%R",
    "LEA %E,A%R",
    "CHK %E,D%N",
    "CHK %E,D%N",
    "CHK %E,D%N",
    "CHK %E,D%N",
    "CHK %E,D%N",
    "ADDQ.BW #%N,%E",
    "ADDQ.BW #%N,%E",
    "ADDQ.BW #%N,%E",
    "ADDQ.BW #%N,%E",
    "ADDQ.BW #%N,%E",
    "ADDQ.L #%N,%E",
    "ADDQ.L #%N,%E",
    "ADDQ.L #%N,%E",
    "ADDQ.L #%N,%E",
    "SUBQ.BW #%N,%E",
    "SUBQ.BW #%N,%E",
    "SUBQ.BW #%N,%E",
    "SUBQ.BW #%N,%E",
    "SUBQ.BW #%N,%E",
    "SUBQ.L #%N,%E",
    "SUBQ.L #%N,%E",
    "SUBQ.L #%N,%E",
    "SUBQ.L #%N,%E",
    "SCC %E",
    "SCC %E",
    "SCC %E",
    "SCC %E",
    "SCC %E",
    "DBRA D%R,label",
    "DBcc D%R,label",
    "BRA label",
    "BSR label",
    "Bcc label",
    "Bcc label",
    "Bcc label",
    "MOVEQ #Immd,D%N",
    "DIVU %E,D%N",
    "DIVU %E,D%N",
    "DIVU %E,D%N",
    "DIVU %E,D%N",
    "DIVU %E,D%N",
    "DIVU %E,D%N",
    "DIVS %E,D%N",
    "DIVS %E,D%N",
    "DIVS %E,D%N",
    "DIVS %E,D%N",
    "DIVS %E,D%N",
    "DIVS %E,D%N",
    "SBCD D%R,D%N",
    "SBCD -(A%R),-(A%N)",
    "OR.BW %E,D%N"	,
    "OR.BW %E,D%N",
    "OR.BW %E,D%N",
    "OR.BW %E,D%N",
    "OR.BW %E,D%N",
    "OR.BW %E,D%N",
    "OR.W A%N,%E",
    "OR.BW D%N,%E"	,
    "OR.BW D%N,%E",
    "OR.BW D%N,%E",
    "OR.BW D%N,%E",
    "OR.L %E,D%N",
    "OR.L %E,D%N",
    "OR.L %E,D%N",
    "OR.L %E,D%N",
    "OR.L %E,D%N",
    "OR.L %E,D%N",
    "OR.L %E,D%N",
    "OR.L D%N,%E",
    "OR.L D%N,%E",
    "OR.L D%N,%E",
    "OR.L D%N,%E",
    "SUB.BW %E,D%N"	,
    "SUB.BW %E,D%N",
    "SUB.BW %E,D%N",
    "SUB.BW %E,D%N",
    "SUB.BW %E,D%N",
    "SUB.BW %E,D%N",
    "SUB.W A%N,%E",
    "SUB.BW D%N,%E"	,
    "SUB.BW D%N,%E",
    "SUB.BW D%N,%E",
    "SUB.BW D%N,%E",
    "SUB.L %E,D%N",
    "SUB.L %E,D%N",
    "SUB.L %E,D%N",
    "SUB.L %E,D%N",
    "SUB.L %E,D%N",
    "SUB.L %E,D%N",
    "SUB.L %E,D%N",
    "SUB.L D%N,%E",
    "SUB.L D%N,%E",
    "SUB.L D%N,%E",
    "SUB.L D%N,%E",
    "SUBX.BW D%R,D%N",
    "SUBX.BW -(A%R),-(A%N)",
    "SUBX.L D%R,D%N",
    "SUBX.L -(A%R),-(A%N)",
    "SUBA.W %E,A%N",
    "SUBA.W %E,A%N",
    "SUBA.W %E,A%N",
    "SUBA.W %E,A%N",
    "SUBA.W %E,A%N",
    "SUBA.L %E,A%N",
    "SUBA.L %E,A%N",
    "SUBA.L %E,A%N",
    "SUBA.L %E,A%N",
    "SUBA.L %E,A%N",
    "ALINE Exception",
    "CMP.B %E,D%N",
    "CMP.B %E,D%N",
    "CMP.W %E,D%N",
    "CMP.BW %E,D%N",
    "CMP.BW %E,D%N",
    "CMP.BW %E,D%N",
    "CMP.BW %E,D%N",
    "CMP.L %E,D%N",
    "CMP.L %E,D%N",
    "CMP.L %E,D%N",
    "CMP.L %E,D%N",
    "CMP.L %E,D%N",
    "CMPA.W %E,A%N",
    "CMPA.W %E,A%N",
    "CMPA.W %E,A%N",
    "CMPA.W %E,A%N",
    "CMPA.W %E,A%N",
    "CMPA.L %E,A%N",
    "CMPA.L %E,A%N",
    "CMPA.L %E,A%N",
    "CMPA.L %E,A%N",
    "CMPA.L %E,A%N",
    "CMPM.B (A%R)+,(A%N)+",
    "CMPM.W (A%R)+,(A%N)+",
    "CMPM.L (A%R)+,(A%N)+",
    "EOR.BW %E,D%N",
    "EOR.BW %E,D%N",
    "EOR.BW %E,D%N",
    "EOR.BW %E,D%N",
    "EOR.BW %E,D%N",
    "EOR.L D%N,%E",
    "EOR.L D%N,%E",
    "EOR.L D%N,%E",
    "EOR.L D%N,%E",
    "EOR.L D%N,%E",
    "AND.BW %E,D%N"	,
    "AND.BW %E,D%N",
    "AND.BW %E,D%N",
    "AND.BW %E,D%N",
    "AND.BW %E,D%N",
    "AND.BW %E,D%N",
    "AND.W A%N,%E",
    "AND.BW D%N,%E"	,
    "AND.BW D%N,%E",
    "AND.BW D%N,%E",
    "AND.BW D%N,%E",
    "AND.L %E,D%N",
    "AND.L %E,D%N",
    "AND.L %E,D%N",
    "AND.L %E,D%N",
    "AND.L %E,D%N",
    "AND.L %E,D%N",
    "AND.L %E,D%N",
    "AND.L D%N,%E",
    "AND.L D%N,%E",
    "AND.L D%N,%E",
    "AND.L D%N,%E",
    "MULU %E,D%N",
    "MULU %E,D%N",
    "MULU %E,D%N",
    "MULU %E,D%N",
    "MULU %E,D%N",
    "MULU %E,D%N",
    "MULS %E,D%N",
    "MULS %E,D%N",
    "MULS %E,D%N",
    "MULS %E,D%N",
    "MULS %E,D%N",
    "MULS %E,D%N",
    "ABCD D%R, D%N",
    "ABCD -(A%R),-(A%N)",
    "EXG D%N,D%R",
    "EXG A%N,A%R",
    "EXG D%N,A%R",
    "ADD.BW %E,D%N"	,
    "ADD.BW %E,D%N",
    "ADD.BW %E,D%N",
    "ADD.BW %E,D%N",
    "ADD.BW %E,D%N",
    "ADD.BW %E,D%N",
    "ADD.W A%N,%E",
    "ADD.BW D%N,%E"	,
    "ADD.BW D%N,%E",
    "ADD.BW D%N,%E",
    "ADD.BW D%N,%E",
    "ADD.L %E,D%N",
    "ADD.L %E,D%N",
    "ADD.L %E,D%N",
    "ADD.L %E,D%N",
    "ADD.L %E,D%N",
    "ADD.L %E,D%N",
    "ADD.L %E,D%N",
    "ADD.L D%N,%E",
    "ADD.L D%N,%E",
    "ADD.L D%N,%E",
    "ADD.L D%N,%E",
    "ADDX.BW D%R,D%N",
    "ADDX.BW -(A%R),-(A%N)",
    "ADDX.L D%R,D%N",
    "ADDX.L -(A%R),-(A%N)",
    "ADDA.W %E,A%N",
    "ADDA.W %E,A%N",
    "ADDA.W %E,A%N",
    "ADDA.W %E,A%N",
    "ADDA.W %E,A%N",
    "ADDA.L %E,A%N",
    "ADDA.L %E,A%N",
    "ADDA.L %E,A%N",
    "ADDA.L %E,A%N",
    "ADDA.L %E,A%N",
    "ASd %E",
    "ASd %E",
    "ASd %E",
    "ASd %E",
    "LSd %E",
    "LSd %E",
    "LSd %E",
    "LSd %E",
    "ROXd %E",
    "ROXd %E",
    "ROXd %E",
    "ROXd %E",
    "ROd %E",
    "ROd %E",
    "ROd %E",
    "ROd %E",
    "ASd.BW %N,D%R",
    "ASd.L %N,D%R",
    "LSd.BW %N,D%R",
    "LSd.L %N,D%R",
    "ROXd.BW %N,D%R",
    "ROXd.L %N,D%R",
    "ROd.BW %N,D%R",
    "ROd.L %N,D%R",
    "ASd.BW D%N,D%R",
    "ASd.L D%N,D%R",
    "LSd.BW D%N,D%R",
    "LSd.L D%N,D%R",
    "ROXd.BW D%N,D%R",
    "ROXd.L D%N,D%R",
    "ROd.BW D%N,D%R",
    "ROd.L D%N,D%R",
    "FLINE Exception",
};

//const int OPCODE_COUNT = sizeof(opcodes) / sizeof(char*);
#define OPCODE_COUNT (sizeof(opcodes) / sizeof(char*))

// typedef struct {
// 	uint16_t match;
// 	uint16_t equal;
// } opcode_t;

char *ea(uint16_t ea) {
	static char b1[32], b2[32], *buffer;
	if(buffer == b1) buffer = b2; else buffer = b1;

	switch(ea & 0x38) {
	case 0x00: sprintf( buffer, "D%d", ea & 7 ); break;
	case 0x08: sprintf( buffer, "A%d", ea & 7 ); break;
	case 0x10: sprintf( buffer, "(A%d)", ea & 7 ); break;
	case 0x18: sprintf( buffer, "(A%d)+", ea & 7 ); break;
	case 0x20: sprintf( buffer, "-(A%d)", ea & 7 ); break;
	case 0x28: sprintf( buffer, "(d16,A%d)", ea & 7 ); break;
	case 0x30: sprintf( buffer, "(d8,A%d,Xn)", ea & 7 ); break;
	case 0x38:
		switch(ea & 7) {
		case 0: sprintf(buffer, "ABS.W"); break;		
		case 1: sprintf(buffer, "ABS.L"); break;		
		case 2: sprintf(buffer, "(d16,PC)"); break;		
		case 3: sprintf(buffer, "(d8,PC,Xn)"); break;
		case 4: sprintf(buffer, "#Immd"); break;	
		}
		break;
	} // end switch
	return buffer;
}

const char* m68k_disasm(uint16_t op) {
    typedef struct {
        uint16_t match, equal;
        void (*emit)(uint32_t**, uint16_t);
    } op_details_t;
	static bool inited = false;
    extern const op_details_t optab_68000[];

	// static opcode_t ops[OPCODE_COUNT] = { 0 };
	static char buffer[64];
	static char buffer2[64];
	
	// if(!inited) {
	// 	int total_ops = 0;
	// 	inited = true;
	// 	for(uint32_t i=0; i<OPCODE_COUNT; i++) {
	// 		uint16_t match = 0, equal = 0, count = 0;
	// 		for(int b=0; b<16; b++) {
	// 			uint16_t bit = 0x8000 >> b;
	// 			char c = opcodes[i][b];
	// 			if(c != 'x') {
	// 				match |= bit;
	// 				if(c == '1') equal |= bit;
	// 			} else {
	// 				count++;
	// 			}
	// 		}
	// 		ops[i].match = match;
	// 		ops[i].equal = equal;
	// 	}
	// 	printf("%d total opcodes loaded.\n", total_ops);
	// }


	//extern int debug;
	
	for(uint32_t i=0; i<OPCODE_COUNT; i++) {
		if((op & optab_68000[i].match) == optab_68000[i].equal) {
			sprintf( buffer, "%s", opcodes[i] ); // e.g., "ORI.BW #,%E"

			//int x = strlen(ops[i].op) - 5; // back up over \n
			//if(debug) printf("@ Disassembling opcode %04hX\n", op);
			char* ea1 = strstr(buffer, "%E");
			char* ea2 = ea1 ? strstr(ea1 + 1, "%E") : 0;
			char* imm = strstr(buffer, "%N");
			char* reg = strstr(buffer, "%R");
			
			int ea1_off = ea1 - buffer;
			int ea2_off = ea2 - buffer;
			int imm_off = imm - buffer;
			int reg_off = reg - buffer;

			unsigned _i = (op >> 9) & 7;
			unsigned _r = (op & 7);
			
			if(ea1) buffer[ea1_off + 1] = 's';
			if(ea2) buffer[ea2_off + 1] = 's';
			if(imm) buffer[imm_off + 1] = 'd';
			if(reg) buffer[reg_off + 1] = 'd';
			
			if(ea1 && ea2) {
				char* _ea1 = ea(op & 0x3F);
				char* _ea2 = ea(((op >> 9) & 0x07) | ((op >> 3) & 0x38));
				sprintf(buffer2, buffer, _ea1, _ea2);
				
			} else if(ea1) {
				char* _ea = ea(op & 0x3F);
				if(imm && reg) {
					if (ea1 < imm) {
						if (imm < reg)      sprintf(buffer2, buffer, _ea,_i,_r);  //return [ea1,imm,reg];
						else if (ea1 < reg) sprintf(buffer2, buffer, _ea,_r,_i);  //return [ea1,reg,imm];
						else                sprintf(buffer2, buffer, _r,_ea,_i);  //return [reg,ea1,imm];
					} else {
						if (ea1 < reg)      sprintf(buffer2, buffer, _i,_ea,_r);  //return [imm,ea1,reg];
						else if (imm < reg) sprintf(buffer2, buffer, _i,_r,_ea);  //return [imm,reg,ea1];
						else                sprintf(buffer2, buffer, _r,_i,_ea);  //return [reg,imm,ea1];
					}
				} else if(imm) {
					if (ea1 < imm)          sprintf(buffer2, buffer, _ea,_i);  //return [reg,ea1,imm];
					else                    sprintf(buffer2, buffer, _i,_ea);  //return [reg,imm,ea1];
					
				} else if(reg) {
					if (ea1 < reg)          sprintf(buffer2, buffer, _ea,_r);  //return [reg,ea1,imm];
					else                    sprintf(buffer2, buffer, _r,_ea);  //return [reg,imm,ea1];
					
				} else                      sprintf(buffer2, buffer, _ea);  //return [reg,imm,ea1];
				
			} else {
				if(imm && reg) {
					if (imm < reg)          sprintf(buffer2, buffer, _i,_r);  //return [ea1,imm,reg];
					else                    sprintf(buffer2, buffer, _r,_i);  //return [reg,ea1,imm];
					
				} 
				else if(imm)                sprintf(buffer2, buffer, _i);  //return [reg,ea1,imm];
				else if(reg)                sprintf(buffer2, buffer, _r);  //return [reg,ea1,imm];
				else                        sprintf(buffer2, buffer);  //return [reg,imm,ea1];
				
			}
			return buffer2;
		}
	}
	
	return "ILLEGAL";
}






