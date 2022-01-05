#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

extern int emit_alu(char *buffer, uint16_t size, uint16_t sEA, uint16_t dEA, ALU_OP_t alu_op);

int emit_MOVEB(char *buffer, uint16_t opcode) {
	uint16_t dEA = ((opcode & 0xE00) >> 9) | ((opcode & 0x1C0) >> 3);
	uint16_t sEA = (opcode & 0x3F);
	uint16_t size = 1;
	return emit_alu(buffer, size, sEA, dEA, ALU_OP_MOVE);
}

int emit_MOVEW(char *buffer, uint16_t opcode) {
	uint16_t dEA = ((opcode & 0xE00) >> 9) | ((opcode & 0x1C0) >> 3);
	uint16_t sEA = (opcode & 0x3F);
	uint16_t size = 2;
	return emit_alu(buffer, size, sEA, dEA, ALU_OP_MOVE);
}

int emit_MOVEL(char *buffer, uint16_t opcode) {
	uint16_t dEA = ((opcode & 0xE00) >> 9) | ((opcode & 0x1C0) >> 3);
	uint16_t sEA = (opcode & 0x3F);
	uint16_t size = 4;
	return emit_alu(buffer, size, sEA, dEA, ALU_OP_MOVE);
}

int emit_MOVEP(char *buffer, uint16_t opcode) {
	uint16_t size = (opcode & 0x0040) ? 4 : 2;

	lines = 0;
	emit_reset( buffer );	
	
	reg_alloc_arm(1); // displacement
	
	uint16_t sEA = (opcode & 0x0080) ? EA_DREG : EA_AIDX;
	uint8_t sR = ((opcode & 0x0007) >> 0) | (sEA ? 0x8 : 0x0);
	uint8_t sRR;
	if(!emit_get_reg( &sRR, sR, size )) return -1;

	uint16_t dEA = (opcode & 0x0080) ? EA_AIDX : EA_DREG; 	
	uint8_t dR = ((opcode & 0x0E00) > 9) | (dEA ? 0x8 : 0x0);
	uint8_t dRR;
	if(!emit_get_reg( &dRR, dR, size )) return -1;
	
	if(opcode & 0x0080) {
		// register to memory
		emit("\tstrb    r%d, [r%d, #0]\n", sRR, dRR);
		emit("\tror     r%d, r%d, #8\n", sRR, sRR);
		emit("\tstrb    r%d, [r%d, #2]\n", sRR, dRR);
		emit("\tror     r%d, r%d, #8\n", sRR, sRR);
		if(size == 4) {
			emit("\tstrb    r%d, [r%d, #4]\n", sRR, dRR);
			emit("\tror     r%d, r%d, #8\n", sRR, sRR);
			emit("\tstrb    r%d, [r%d, #6]\n", sRR, dRR);
			if(sRR > 3)
			emit("\tror     r%d, r%d, #8\n", sRR, sRR);
		} else {
			if(sRR > 3)
			emit("\tror     r%d, r%d, #16\n", sRR, sRR);
		}
				
	} else {
		uint8_t tRR;
		reg_alloc_temp( &tRR );
		
		// memory to register
		emit("\tldrb    r%d, [r%d, #0]\n", tRR, sRR);
		emit("\tbfi     r%d, r%d, #0, #8\n", dRR, tRR);
		emit("\tldrb    r%d, [r%d, #2]\n", tRR, sRR);
		emit("\tbfi     r%d, r%d, #8, #8\n", dRR, tRR);
		emit("\tldrb    r%d, [r%d, #4]\n", tRR, sRR);
		emit("\tbfi     r%d, r%d, #16, #8\n", dRR, tRR);
		emit("\tldrb    r%d, [r%d, #6]\n", tRR, sRR);
		emit("\tbfi     r%d, r%d, #24, #8\n", dRR, tRR);
		
		reg_modified(dRR);
	}
	reg_flush();
	return lines | EXT_WORD_SRC_16B;
}

int emit_MOVEM(char *buffer, uint16_t opcode) {
	char *ldx = (opcode & 0x0040) ? "ldrne " : "ldrhne";
	char *stx = (opcode & 0x0040) ? "strne " : "strhne";
	uint16_t size = (opcode & 0x0040) ? 4 : 2;
	uint16_t sEA = (opcode & 0x38) >> 3;
	if(sEA == 7) sEA += (opcode & 0x0007) >> 0;
	
	if(sEA <= EA_AREG) return -1;
	if(opcode & 0x0400) { // M->R
		if(sEA == EA_ADEC || sEA == EA_IMMD) return -1;
	} else {
		if(sEA == EA_AINC || sEA > EA_ABSL) return -1;
	}	

	if(sEA >  EA_ABSW)	return -(opcode & 0xFFF8); // change to EA_ABSW
	else if(sEA == EA_ADIS) return -(opcode ^ 0x0018); // change to EA_AIDX

	lines = 0;
	emit_reset( buffer );	
	
	reg_alloc_arm(0);
	reg_alloc_arm(1); // register list
	reg_alloc_arm(2);
	
	uint8_t sR = ((opcode & 0x0007) >> 0) | (sEA ? 0x8 : 0x0);
	uint8_t sRR;
	if(!emit_get_reg( &sRR, sR, size )) return -1;
	
	switch(sEA) {
	case EA_ADDR:
		emit("\tmov     r2, r%d\n", sRR);
		break;
	case EA_AIDX: case EA_ADIS:
		emit("\tadd     r2, r%d, r1\n", sRR);
		break;
	case EA_ABSW: case EA_ABSL: case EA_PDIS: case EA_PIDX:
		emit("\tmov     r2, r1\n");
		break;
	}
	reg_free(1);
	
	if (sEA == EA_AINC) {
		// memory to register-list, normal reg list D0-D7, A0-A7
		// address register should point to the next long address
	
		emit("\ttst     r1, #0x8000\n"); emit("\t%s  r4, [r%d], #4\n", ldx, sRR );
		emit("\ttst     r1, #0x4000\n"); emit("\t%s  r5, [r%d], #4\n", ldx, sRR );
		emit("\ttst     r1, #0x2000\n"); emit("\t%s  r6, [r%d], #4\n", ldx, sRR );
		emit("\ttst     r1, #0x1000\n"); emit("\t%s  r7, [r%d], #4\n", ldx, sRR );

		emit("\ttst     r1, #0x0800\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #16]\n", stx );
		emit("\ttst     r1, #0x0400\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #20]\n", stx );
		emit("\ttst     r1, #0x0200\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #24]\n", stx );
		emit("\ttst     r1, #0x0100\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #28]\n", stx );

		emit("\ttst     r1, #0x0080\n"); emit("\t%s  r7, [r%d], #4\n", ldx, sRR );
		emit("\ttst     r1, #0x0040\n"); emit("\t%s  r8, [r%d], #4\n", ldx, sRR );
		emit("\ttst     r1, #0x0020\n"); emit("\t%s  r9, [r%d], #4\n", ldx, sRR );
	
		emit("\ttst     r1, #0x0010\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #44]\n", stx );
		emit("\ttst     r1, #0x0008\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #48]\n", stx );
		emit("\ttst     r1, #0x0004\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #52]\n", stx );
		emit("\ttst     r1, #0x0002\n"); emit("\t%s  r0, [r%d], #4\n", ldx, sRR); emit("\t%s  r0, [r12, #56]\n", stx );
	
		emit("\ttst     r1, #0x0001\n"); emit("\t%s  r10, [r%d], #4\n", ldx, sRR );
		reg_modified(sRR);
			
	} else if (sEA == EA_ADEC) {
		// register-list to memory, reverse reg list A7-A0, D7-D0
		// address register should point to the last long address written
		
		emit("\ttst     r1, #0x8000\n"); emit("\t%s  r4, [r%d, #-4]!\n", stx, sRR );
		emit("\ttst     r1, #0x4000\n"); emit("\t%s  r5, [r%d, #-4]!\n", stx, sRR );
		emit("\ttst     r1, #0x2000\n"); emit("\t%s  r6, [r%d, #-4]!\n", stx, sRR );
		emit("\ttst     r1, #0x1000\n"); emit("\t%s  r7, [r%d, #-4]!\n", stx, sRR );

		emit("\ttst     r1, #0x0800\n"); emit("\t%s  r0, [r12, #16]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
		emit("\ttst     r1, #0x0400\n"); emit("\t%s  r0, [r12, #20]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
		emit("\ttst     r1, #0x0200\n"); emit("\t%s  r0, [r12, #24]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
		emit("\ttst     r1, #0x0100\n"); emit("\t%s  r0, [r12, #28]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );

		emit("\ttst     r1, #0x0080\n"); emit("\t%s  r7, [r%d, #-4]!\n", stx, sRR );
		emit("\ttst     r1, #0x0040\n"); emit("\t%s  r8, [r%d, #-4]!\n", stx, sRR );
		emit("\ttst     r1, #0x0020\n"); emit("\t%s  r9, [r%d, #-4]!\n", stx, sRR );
	
		emit("\ttst     r1, #0x0010\n"); emit("\t%s  r0, [r12, #44]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
		emit("\ttst     r1, #0x0008\n"); emit("\t%s  r0, [r12, #48]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
		emit("\ttst     r1, #0x0004\n"); emit("\t%s  r0, [r12, #52]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
		emit("\ttst     r1, #0x0002\n"); emit("\t%s  r0, [r12, #56]\n\t%s  r0, [r%d, #-4]!\n", ldx, stx, sRR );
	
		emit("\ttst     r1, #0x0001\n"); emit("\t%s  r10, [r%d, #-4]!\n", stx, sRR );
		
	} else if(opcode & 0x0400) {
		// memory to register-list, normal reg list D0-D7, A0-A7
		// address register remains unchanged
		static bool wrote_movem_m2r_word = false;
		static bool wrote_movem_m2r_long = false;

		if((wrote_movem_m2r_word && size == 2) || (wrote_movem_m2r_long && size == 4)) {
			emit("\tb       movem_m2r_%s\n", (size == 2) ? "word" : "long");
			
		} else {
			emit("movem_m2r_%s:\n", (size == 2) ? "word" : "long");
			emit("\ttst     r1, #0x8000\n"); emit("\t%s  r4, [r2], #4\n", ldx );
			emit("\ttst     r1, #0x4000\n"); emit("\t%s  r5, [r2], #4\n", ldx );
			emit("\ttst     r1, #0x2000\n"); emit("\t%s  r6, [r2], #4\n", ldx );
			emit("\ttst     r1, #0x1000\n"); emit("\t%s  r7, [r2], #4\n", ldx );

			emit("\ttst     r1, #0x0800\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #16]\n", ldx, stx );
			emit("\ttst     r1, #0x0400\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #20]\n", ldx, stx );
			emit("\ttst     r1, #0x0200\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #24]\n", ldx, stx );
			emit("\ttst     r1, #0x0100\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #28]\n", ldx, stx );

			emit("\ttst     r1, #0x0080\n"); emit("\t%s  r7, [r2], #4\n", ldx );
			emit("\ttst     r1, #0x0040\n"); emit("\t%s  r8, [r2], #4\n", ldx );
			emit("\ttst     r1, #0x0020\n"); emit("\t%s  r9, [r2], #4\n", ldx );
		
			emit("\ttst     r1, #0x0010\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #44]\n", ldx, stx );
			emit("\ttst     r1, #0x0008\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #48]\n", ldx, stx );
			emit("\ttst     r1, #0x0004\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #52]\n", ldx, stx );
			emit("\ttst     r1, #0x0002\n"); emit("\t%s  r0, [r2], #4\n\t%s  r0, [r12, #56]\n", ldx, stx );
		
			emit("\ttst     r1, #0x0001\n"); emit("\t%s  r10, [r2], #4\n", ldx );
			
			if(size == 2) wrote_movem_m2r_word = true;
			if(size == 4) wrote_movem_m2r_long = true;
		}

	} else {
		// register-list to memory, normal reg list D0-D7, A0-A7
		// address register remains unchanged
		static bool wrote_movem_r2m_word = false;
		static bool wrote_movem_r2m_long = false;

		if((wrote_movem_r2m_word && size == 2) || (wrote_movem_r2m_long && size == 4)) {
			emit("\tb       movem_r2m_%s\n", (size == 2) ? "word" : "long");
			
		} else {
			emit("movem_r2m_%s:\n", (size == 2) ? "word" : "long");

			emit("\ttst     r1, #0x8000\n"); emit("\t%s  r4, [r2], #4\n", stx );
			emit("\ttst     r1, #0x4000\n"); emit("\t%s  r5, [r2], #4\n", stx );
			emit("\ttst     r1, #0x2000\n"); emit("\t%s  r6, [r2], #4\n", stx );
			emit("\ttst     r1, #0x1000\n"); emit("\t%s  r7, [r2], #4\n", stx );

			emit("\ttst     r1, #0x0800\n"); emit("\t%s  r0, [r12, #16]\n\t%s  r0, [r2], #4\n", ldx, stx );
			emit("\ttst     r1, #0x0400\n"); emit("\t%s  r0, [r12, #20]\n\t%s  r0, [r2], #4\n", ldx, stx );
			emit("\ttst     r1, #0x0200\n"); emit("\t%s  r0, [r12, #24]\n\t%s  r0, [r2], #4\n", ldx, stx );
			emit("\ttst     r1, #0x0100\n"); emit("\t%s  r0, [r12, #28]\n\t%s  r0, [r2], #4\n", ldx, stx );

			emit("\ttst     r1, #0x0080\n"); emit("\t%s  r7, [r2], #4\n", stx );
			emit("\ttst     r1, #0x0040\n"); emit("\t%s  r8, [r2], #4\n", stx );
			emit("\ttst     r1, #0x0020\n"); emit("\t%s  r9, [r2], #4\n", stx );
		
			emit("\ttst     r1, #0x0010\n"); emit("\t%s  r0, [r12, #44]\n\t%s  r0, [r2], #4\n", ldx, stx );
			emit("\ttst     r1, #0x0008\n"); emit("\t%s  r0, [r12, #48]\n\t%s  r0, [r2], #4\n", ldx, stx );
			emit("\ttst     r1, #0x0004\n"); emit("\t%s  r0, [r12, #52]\n\t%s  r0, [r2], #4\n", ldx, stx );
			emit("\ttst     r1, #0x0002\n"); emit("\t%s  r0, [r12, #56]\n\t%s  r0, [r2], #4\n", ldx, stx );
		
			emit("\ttst     r1, #0x0001\n"); emit("\t%s  r10, [r2], #4\n", stx );
			
			if(size == 2) wrote_movem_r2m_word = true;
			if(size == 4) wrote_movem_r2m_long = true;
		}
	}

	reg_flush();
	return lines | EXT_WORD_SRC_16B;
/*
*/

}




