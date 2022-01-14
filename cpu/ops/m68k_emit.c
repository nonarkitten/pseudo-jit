#include "m68k_common.h"
#include "m68k_emitters.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

extern const char* m68k_disasm(uint16_t op);

int avoid_register_saving = 0;

int lines;
int debug;
int last_opcode;

static char* opcodes[65536] = { 0 };
static int opcode_len[65536] = { 0 };

static const char* header = 
"/*\n"
" * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project\n"
" *\n"
" * This is part of PJIT the Pseudo-JIT 68K emulator.\n"
" *\n"
" * This file has been auto generated; do not manually edit. \n"
" *\n"
" * PJIT is distributed in the hope that it will be useful,\n"
" * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
" * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
" *\n"
" * PJIT is licensed under a Creative Commons\n"
" * Attribution-NonCommercial-ShareAlike 4.0 International License.\n"
" *\n"
" * Under the terms of this license you are free copy and redistribute\n"
" * the material in any medium or format as well as remix, transform,\n"
" * and build upon the material.\n"
" *\n"
" * You must give appropriate credit, provide a link to the license,\n"
" * and indicate if changes were made. You may do so in any reasonable\n"
" * manner, but not in any way that suggests the licensor endorses you\n"
" * or your use.\n"
" *\n"
" * You may not use the material for commercial purposes.\n"
" *\n"
" * If you remix, transform, or build upon the material, you must\n"
" * distribute your contributions under the same license as the original.\n"
" *\n"
" * You may not apply legal terms or technological measures that legally\n"
" * restrict others from doing anything the license permits.\n"
" *\n"
" * Portions of PJIT have been derived from the following:\n"
" *\n"
" * Castaway (formerly FAST)\n"
" * Copyright (c) 1994-2002 Martin Doering, Joachim Hˆnig\n"
" *\n"
" * Cyclone 68K\n"
" * Copyright (c) 2004,2011 FinalDave\n"
" * Copyright (c) 2005-2011 Graûvydas \"notaz\" Ignotas\n"
" *\n"
" * Both of which were distributed under GPL version 2.\n"
" *\n"
" */\n\n";

// There are three basic kinds of opcodes:

// special inline	These are opcodes like branch that need to be
//					handled at the parser level (computed branch)
//					These are left as empty fields in the opcode table.

// general inline	These opcodes may be generated here and are just
//					copy-pasted into the PJIT cache; these are included
//					in the opcode table with a LEN == 1; note that any
//					opcode with LEN == 1 is inlined with a NOP

// cannot inline	These cannot be inlined and also have to be
//					handled at the parser level (computed branch)/


// Any ultimately illegal opcode should trigger an illegal exception
// since dealing with doubly-relative branches, we'll implement the
// whole 68000 exception table as a software interrupt.

// In all cases, this is a common and generic inlined operation

// static const struct op_synonyms {
// 
// } opsyn[] = {
	// PJIT ADDITIONS
	//
	// alu.bwl	Dx,Dy (ea)	alu.bwl		Dx,Dy (reg)	
	// any abs.w			abs.l						Immediate is precalc
	// any d8(Xn,An)		d!6(An)						Index is precalc
	//
	// STANDARD LIST
	//
	// USE OF IMMEDIATE FORMS (DONE)
	// add.bwl	#imm,Dn		addi.bwl	#imm.Dn
	// and.bwl	#imm,Dn		andi.bwl 	#imm,An
	// cmp.bwl	#imm,Dn		cmpi.bwl 	#imm.Dn
	// or.bwl	#imm,Dn		ori.bwl		#imm,Dn
	// sub.bwl	#imm,Dn		subi.bwl	#imm,Dn
	//
	// QUICK EQUIVALENT (DONE)
	// addq.w	#imm,An		addq.l 		#imm,An
	// subq.w	#imm,Dn		subq.l 		#imm,Dn
	//
	// NOT DONE
	// adda.w	#imm,An	 	lea.l 		imm(An),An
	// asl.bwl	#1,Dn		add.bwl 	Dn,Dn
	// bra.s	*+4			dbt.w 		Dx,xxxx (32 bit NOP)
	// bra.w	d16			jmp 		d16(PC)
	// bsr.w	d16			jsr 		d16(PC)
	// clr.l	Dn			moveq.l 	#0,Dn
	// lea.l	(As),Ad		movea.l 	As,Ad
	// lea.l	abs.w,An	movea.w 	#imm16,An
	// lea.l	abs.l,An	movea.l 	#imm32,An
	// movea.l	(A7),A7		unlk 		a7
	//
	
//}



static const struct op_details {
    uint16_t base, bits;
    int (*emit)(char*,uint16_t);
} optab[] = {
    { 0x003C, 0x0000, emit_ORI_TO_CCR    },
    { 0x007C, 0x0000, emit_ORI_TO_SR     },
    { 0x0000, 0x00FF, emit_ORI           },
    { 0x023C, 0x0000, emit_ANDI_TO_CCR   },
    { 0x027C, 0x0000, emit_ANDI_TO_SR    },
    { 0x0200, 0x00FF, emit_ANDI          },
    { 0x0400, 0x00FF, emit_SUBI          },
    { 0x0600, 0x00FF, emit_ADDI          },
    { 0x0A3C, 0x0000, emit_EORI_TO_CCR   },
    { 0x0A7C, 0x0000, emit_EORI_TO_SR    },
    { 0x0A00, 0x00FF, emit_EORI          },
    { 0x0C00, 0x00FF, emit_CMPI          },
    { 0x0800, 0x003F, emit_BTSTI         },
    { 0x0840, 0x003F, emit_BCHGI         },
    { 0x0880, 0x003F, emit_BCLRI         },
    { 0x08C0, 0x003F, emit_BSETI         },
    { 0x0100, 0x0E3F, emit_BTST          },
    { 0x0140, 0x0E3F, emit_BCHG          },
    { 0x0180, 0x0E3F, emit_BCLR          },
    { 0x01C0, 0x0E3F, emit_BSET          },
    { 0x0108, 0x0EC7, emit_MOVEP         },
    { 0x1000, 0x0FFF, emit_MOVEB         },
    { 0x2000, 0x0FFF, emit_MOVEL         },
    { 0x3000, 0x0FFF, emit_MOVEW         },
    { 0x40C0, 0x003F, emit_MOVE_FROM_SR  },
    { 0x44C0, 0x003F, emit_MOVE_TO_CCR   },
    { 0x46C0, 0x003F, emit_MOVE_TO_SR    },
    { 0x4000, 0x00FF, emit_NEGX          },
    { 0x4200, 0x00FF, emit_CLR           },
    { 0x4400, 0x00FF, emit_NEG           },
    { 0x4600, 0x00FF, emit_NOT           },
    { 0x4880, 0x0047, emit_EXT           },
    { 0x4800, 0x003F, emit_NBCD          },
    { 0x4840, 0x0007, emit_SWAP          },
    { 0x4840, 0x003F, emit_PEA           },
    { 0x4AFC, 0x0000, emit_ILLEGAL       },
    { 0x4AC0, 0x003F, emit_TAS           },
    { 0x4A00, 0x00FF, emit_TST           },
    { 0x4E40, 0x000F, emit_TRAP          },
    { 0x4E50, 0x0007, emit_LINK          },
    { 0x4E58, 0x0007, emit_UNLK          },
    { 0x4E60, 0x000F, emit_MOVE_USP      },
    { 0x4E70, 0x0000, emit_RESET         },
    { 0x4E71, 0x0000, emit_NOP           },
    { 0x4E72, 0x0000, emit_STOP          },
    { 0x4E73, 0x0000, emit_RTE           },
    { 0x4E75, 0x0000, emit_RTS           },
    { 0x4E76, 0x0000, emit_TRAPV         },
    { 0x4E77, 0x0000, emit_RTR           },
    { 0x4E80, 0x003F, emit_JSR           },
    { 0x4EC0, 0x003F, emit_JMP           },
    { 0x4880, 0x047F, emit_MOVEM         },
    { 0x41C0, 0x0E3F, emit_LEA           },
    { 0x4180, 0x0E3F, emit_CHK           },
    { 0x50C8, 0x0F07, emit_DBCC          },
    { 0x50C0, 0x0F3F, emit_SCC           },
    { 0x5000, 0x0EFF, emit_ADDQ          },
    { 0x5100, 0x0EFF, emit_SUBQ          },
    { 0x6000, 0x00FF, emit_BRA           },
    { 0x6100, 0x00FF, emit_BSR           },
    { 0x6000, 0x0FFF, emit_BCC           },
    { 0x7000, 0x0EFF, emit_MOVEQ         },
    { 0x80C0, 0x0E3F, emit_DIVU          },
    { 0x81C0, 0x0E3F, emit_DIVS          },
    { 0x8100, 0x0E0F, emit_SBCD          },
    { 0x8000, 0x0FFF, emit_OR            },
    { 0x90C0, 0x0F3F, emit_SUBA          },
    { 0x9100, 0x0ECF, emit_SUBX          },
    { 0x9000, 0x0FFF, emit_SUB           },
    { 0xB0C0, 0x0F3F, emit_CMPA          },
    { 0xB108, 0x0EC7, emit_CMPM          },
    { 0xB100, 0x0EFF, emit_EOR           },
    { 0xB000, 0x0EFF, emit_CMP           },
    { 0xC0C0, 0x0E3F, emit_MULU          },
    { 0xC1C0, 0x0E3F, emit_MULS          },
    { 0xC100, 0x0E0F, emit_ABCD          },
    { 0xC100, 0x0ECF, emit_EXG           },
    { 0xC000, 0x0FFF, emit_AND           },
    { 0xD0C0, 0x0F3F, emit_ADDA          },
    { 0xD100, 0x0ECF, emit_ADDX          },
    { 0xD000, 0x0FFF, emit_ADD           },
    { 0xE0C0, 0x013F, emit_ASD_EA        },
    { 0xE2C0, 0x013F, emit_LSD_EA        },
    { 0xE4C0, 0x013F, emit_ROXD_EA       },
    { 0xE6C0, 0x013F, emit_ROD_EA        },
    { 0xE000, 0x0FE7, emit_ASD           },
    { 0xE008, 0x0FE7, emit_LSD           },
    { 0xE010, 0x0FE7, emit_ROXD          },
    { 0xE018, 0x0FE7, emit_ROD           },
    { 0xA000, 0x0FFF, emit_A_LINE        },
    { 0xF000, 0x0FFF, emit_F_LINE        },
    
    /* Ensure emit_INVALID is last */
    { 0x0000, 0xFFFF, emit_INVALID       },
};
static const uint32_t optab_size = sizeof(optab) / sizeof(struct op_details);

//static int invalid = 0;

// Opcode emitters should return the number of lines they emit

// Opcode emitters should not include either the label or return

// Opcode emitters should return the negative of the root opcode if they're
// an alias of another opcode
static int emit_SVC(char *buffer, int exception) {
	emit_reset( buffer );
	emit("\tsvc     #%d\n", exception);
	return 1;
}

int emit_ILLEGAL(char *buffer, uint16_t opcode) {
	return emit_SVC(buffer, ILLINSTR);
}
int emit_INVALID(char *buffer, uint16_t opcode) {
//	invalid++;
	return -0x4AFC;
}
int emit_A_LINE(char *buffer, uint16_t opcode) {
	if(opcode != 0xA000) return -0xA000;
	else return emit_SVC(buffer, LINE_A);
}
int emit_F_LINE(char *buffer, uint16_t opcode) {
	if(opcode != 0xF000) return -0xF000;
	else return emit_SVC(buffer, LINE_F);
}
int emit_RESET(char *buffer, uint16_t opcode) {
	return emit_SVC(buffer, RESET_SP);
}
int emit_TRAP(char *buffer, uint16_t opcode) {
	return emit_SVC(buffer, TRAP0 + (opcode & 0xF));
}
int emit_TRAPV(char *buffer, uint16_t opcode) {
	emit_reset( buffer );
	emit("\tsvcvs   #%d\n", TRAPV);
	return 1;
}
int emit_NOP(char *buffer, uint16_t opcode) {
	emit_reset( buffer );
	emit("\tnop\n");
	return 1;
}

int main(int argc, char ** argv) {
//	const char *omitted = "// omitted\n";
	char buffer[16384];
	int line_counts[256] = { 0 };
	int total_arm = 0;
	int total_68k = 0;
	int total_alias = 0;
	int total_nops = 0;
	int total_invalid = 0;
		
	fprintf(stderr, "Initializing disassembler...\n"); 
	m68k_disasm(0);
	
	if(argc == 2) {
		uint16_t opcode = strtoul(argv[1], 0, 0);
		debug = 1;
		printf("@ %s", m68k_disasm(opcode));
try_again:
		for(int i=0; i<optab_size; i++) {
			if((opcode & ~optab[i].bits) == optab[i].base) {
				int n = optab[i].emit(buffer, opcode);
				if(n == -1) continue;
				else if(n == -0x4AFC) break;
				else if(n == 0) {
					printf("@ Opcode is NOP\n");
				} else if(n < 0) {
					printf("@ Opcode %04X is alias of %04X\n", opcode, -n);
					opcode = -n;
					//n = optab[i].emit(buffer, -n);
					goto try_again;
				}
				if((n & NO_BX_LR) != NO_BX_LR) {
					emit("\tbx      lr\n");
					n++;
				}
				printf("%s\n@ Lines: %d (flags %04X)\n", buffer, n & 0xFF, n & 0xFF00);
				return 0;
			}
		}
		printf("@ Opcode %04X appears invalid.\n", opcode);
		return -1;
	}

	for(int i=0; i<0x10000; i++)
		opcode_len[i] = 0;
		
	// 0 nothing
	// <0 alias for -n
	// >0 actual length + flags
		
	// Generate all the opcodes first
	fprintf(stderr, "Generating opcodes...\n"); 
	for(int i=0; i<optab_size; i++) {
		uint16_t base = optab[i].base, bits = 0, opcode;
		do {
			opcode = bits | base;
			last_opcode = opcode; 
			
			bits = (bits - optab[i].bits) & optab[i].bits;
			
			if(opcode_len[opcode] == 0) {
				int n = optab[i].emit(buffer, opcode);
				if(n == -1) continue;
				else if(n < 0) opcode_len[opcode] = n; // alias
				else if(n == 0) opcode_len[opcode] = -0x4E71; // no length, NOP
				else opcodes[opcode] = strdup(buffer), opcode_len[opcode] = n; // actual opcode
			}
			
		} while(bits);
	}

	fprintf(stderr, "Scanning table for invalid opcodes...\n"); 
	for(int i=0; i<0x10000; i++) {
		int opcode = i;
		int len = opcode_len[opcode];
		
		if(len == 0) { 
			fprintf(stderr, "Failure at opcode 0x%04X\n", i); 
			exit(1);
		}
		else if(len > 0) {
			// implemented opcode
			line_counts[len & 0xFF]++;
			total_arm += len & 0xFF;
			if(!(len & NO_BX_LR)) total_arm++;
			total_68k++;
		}
		else if(len < 0) {
			// look for real opcode
			while(len < 0) opcode = -len, len = opcode_len[-len];
			opcode_len[i] = -opcode;
			if(opcode == 0x4E71) total_nops++;
			else if(opcode == 0x4AFC) total_invalid++;
			total_alias++;
		}
	}
	
	// Emit the opcode functions in sixteen files
	for(int i=0; i<0x10000; i+=0x1000) {
		char filename[16];		
		sprintf(filename, "pjit_ops_%04x.s", i);
		FILE * file = fopen( filename, "w" );

		fprintf(stderr, "Generating file '%s'...\n", filename); 
				
		fprintf(file, "%s", header);
	
		fprintf(file, "\t.text\n\n");
		fprintf(file, "\t.code 32\n\n");
				
		for(int j=0; j<0x1000; j++) {
			uint16_t opcode = i + j;
			const char *m68k_op = m68k_disasm(opcode);
			bool valid = (opcode == 0x4AFC) || memcmp(m68k_op, "ILLEGAL", 7);
			
			// print out all the aliases for this
			if(opcode_len[opcode] > 0) {
				char *op = "\tnop\n";
				int len = opcode_len[opcode] & 0xFF;
				if(len > 0) op = opcodes[opcode];
				fprintf(file, "\t.global opcode_%04x\n", opcode);
				fprintf(file, "\t.syntax unified\n");
				fprintf(file, "\t@ %s", m68k_op);
				fprintf(file, "opcode_%04x:\n%s", opcode, op);
				
				if((opcode_len[opcode] & NO_BX_LR) != NO_BX_LR)
					fprintf(file, "\tbx      lr\n");
				fprintf(file, "\n");
				if(!valid) {
					fprintf(stderr, "ERR: opcode emitted for invalid pattern %04X\n", opcode);
				}
			} else if(opcode_len[opcode] == 0) {
				if(valid) {
					fprintf(stderr, "ERR: opcode skipped for valid pattern %04X: %s", opcode, m68k_op);
				}
			}
		}
		fprintf(file, "\tnop\n\n");		
		fclose(file);
	}
	
	// Emit the op lengths
	{
		char filename[16];
		sprintf(filename, "pjit_oplen.s");
		FILE * file = fopen( filename, "w" );

		fprintf(stderr, "Generating file '%s'...\n", filename); 
		
		fprintf(file, "%s", header);
		
		fprintf(file, "\t.text\n\t.global oplen\n");
		fprintf(file, "oplen:");
		
		for(int i=0; i<0x10000; i++) {
			if((i & 4095) == 0) fprintf(file, "\n;// 0x%04X\n\t.hword ", i);
			else if((i & 7) == 0) fprintf(file, "\t;// 0x%04X\n\t.hword ", i - 8);
			
			char sepr = (i & 7) ? ',' : ' ';
			
			int len = opcode_len[i];
			if (len < 0) len = opcode_len[-len];
			fprintf(file, "%c 0x%04X", sepr, (uint16_t)len);
		}
		
		fprintf(file, "\n\n");

		fclose(file);
	}

	{
		char filename[16];
		sprintf(filename, "pjit_optab.s");
		FILE * file = fopen( filename, "w" );
		
		fprintf(stderr, "Generating file '%s'...\n", filename); 
		
		fprintf(file, "%s", header);
		
		fprintf(file, "\t.text\n\t.global optab\n");
		fprintf(file, "optab:");
		
		for(int i=0; i<0x10000; i++) {
			if((i & 4095) == 0) fprintf(file, "\n// %04X\n\t.word ", i);
			else if((i & 3) == 0) fprintf(file, "\t// %04X\n\t.word ", i - 4);
			
			char sepr = (i & 3) ? ',' : ' ';

			int len = opcode_len[i];
			uint16_t opcode = (len < 0) ? -len : i;
			fprintf(file, "%c opcode_%04x", sepr, opcode);
		}
		
		fprintf(file, "\n\n");

		fclose(file);
	}
	
	// Emit all the extension modes
	{
		char branches[16384];
		char buffer[256];
		char filename[16];
		
		sprintf(filename, "pjit_extword.s");
		FILE * file = fopen( filename, "w" );
		fprintf(stderr, "Generating file '%s'...\n", filename); 
		
		for(int r=1; r<3; r++) {
			char *b = branches;
			for(int i=0; i<128; i++) {
				b += sprintf(b, "\tmov     r%d, #%d\n\tb       0f\n", r, i);
			}
			for(int i=-128; i; i++) {
				b += sprintf(b, "\tmvn     r%d, #%d\n", r, ~i);
				if(i < -1) b += sprintf(b, "\tb       0f\n");
			}
			
			fprintf(file, "\t.text\n\t.thumb_func\n\t.syntax unified\n");

			for(int s=0; s<2; s++) {
				for(int d=0; d<16; d++) {
					char label[256];
					uint8_t rD;

					sprintf(label, "extword_%s_%X%d00", ((r==1) ? "src" : "dst" ), d, (s * 8));
					
					fprintf(file, "\t.global %s\n", label);
					fprintf(file, "%s:\n", label);
					fprintf(file, "%s", branches);
				
					buffer[0] = 0;
					emit_reset( buffer );
					
					reg_alloc_arm(r);
					reg_alloc_68k(&rD, d, s ? 4 : 2); // todo bail?
				
					fprintf(file, "0:\n%s", buffer);
					fprintf(file, "\tadd     r%d, r%d\n", r, rD);				
					fprintf(file, "\tbx      lr\n\n");
					reg_flush();
				}
			}
		}
	
		fclose(file);
	}

	// Make the header file too
	{
		char filename[16];		
		sprintf(filename, "pjit_extword.h");
		FILE * file = fopen( filename, "w" );
		
		fprintf(stderr, "Generating file '%s'...\n", filename); 
		
		for(int r=1; r<3; r++) {			
			for(int s=0; s<2; s++) {
				for(int d=0; d<16; d++) {
					fprintf(file, "extern void extword_%s_%X%d00(void);\n", ((r==1) ? "src" : "dst" ), d, (s * 8));
				}
			}
		}
	
		fclose(file);
	}
	
	printf("\n");
	printf("Total real 68k instructions: %d\n", total_68k );
	printf("Total aliased 68k instructions: %d\n", total_alias );
	printf("Total invalid 68k instructions: %d\n", total_invalid );
	printf("Total opcode count is %s\n", (((total_68k + total_alias) == 65536) ? "valid" : "invalid"));
	printf("\n");	
	printf("Total arm instructions: %d (%d bytes)\n", total_arm, total_arm * 4);
	
	float average = 0.0f;
	for(int i=1; i<255; i++) {
		if(line_counts[i] == 0) continue;
		//if(i < 20) 
		printf("functions with %d lines: %d\n", i, line_counts[i]);
		average += i * line_counts[i];
		//if(i > 8 && (line_counts[i] == 0) && (line_counts[i + 1] == 0)) break;
	}
	average /= (float)total_68k;
	printf("line average: %f\n", average);
	
}