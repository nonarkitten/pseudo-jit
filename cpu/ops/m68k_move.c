#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"


int emit_move(char *buffer, uint16_t size, uint16_t opcode) {
	uint16_t dEA = ((opcode & 0xE00) >> 9) | ((opcode & 0x1C0) >> 3);
	uint16_t sEA = (opcode & 0x3F);

	uint8_t dR=0, dRR=0, sR=0, sRR=0, tRR=0;
	uint8_t org_sEA = sEA;//, org_dEA = dEA;

	if(size == 3 || size > 4) return -1;
	
	if(debug) printf("@ emit_move, sEA %02X, dEA %02X, size %d\n", sEA, dEA, size);

	sR = sEA & 7; sEA = (sEA >> 3) & 7; if(sEA == 7) sEA += sR; if(sEA) sR += 8;
	dR = dEA & 7; dEA = (dEA >> 3) & 7; if(dEA == 7) dEA += dR; if(dEA) dR += 8;
	
	// EA exceptions for MOVE
	// Cannot be byte and an address register
	if((size == 1) && (sEA == EA_AREG)) return -1;
	if((size == 1) && (dEA == EA_AREG)) return -1;
	// PC-relative and Immediate are disallowed as destinations
	if((dEA == EA_PDIS) || (dEA == EA_PIDX) || (dEA == EA_IMMD)) return -1;
	// Exclude obvious not-real addressing modes above immediate
	if((sEA > EA_IMMD) || (dEA > EA_IMMD)) return -1;

	// EA Duplicates
	// Both absolute opcodes are the same with destination
	if (dEA == EA_ABSL) return -(opcode & 0xFF3F);
	// Absolute and PC-relative modes are all the same with source
	if((sEA == EA_ABSL) || (sEA == EA_PDIS) || (sEA == EA_PIDX)) return -(opcode & 0xFFF8);
	// Source Indexed and Displacement are the same modes
	if (dEA == EA_ADIS) return -(opcode ^ 0x00C0);
	if (sEA == EA_ADIS) return -(opcode ^ 0x0018);
	// Source and destination are the same, change to TST
	if(sR == dR) {
		if(sEA == EA_AREG && dEA == EA_AREG && size == 4) {
			return 0; // NOP
		}
		if(sEA == EA_DREG && dEA == EA_DREG) {
			// move.bwl	dn,dn		tst.bwl		dn
			return -(0x4A00 | ((size / 2) << 6) | org_sEA);
		}
		if(sEA == EA_ADDR && dEA == EA_ADDR) {
			// move.bwl (an),(an)	tst.bwl		(an)
			return -(0x4A00 | ((size / 2) << 6) | org_sEA);
		}
		if(sEA == EA_ADEC && dEA == EA_ADDR) {
			// move.bwl -(an),(an)	tst.bwl		-(an)
			return -(0x4A00 | ((size / 2) << 6) | org_sEA);
		}
		if(sEA == EA_AINC && dEA == EA_ADEC) {
			// move.bwl (an)+,-(an)	tst.bwl		(an)
			return -(0x4A10 | ((size / 2) << 6) | (org_sEA & 7));
		}
		if(sEA == EA_AINC && dEA == EA_AREG) {
			// movea.wl	(an)+,an	movea.wl	(an),an	
			return -(opcode & 0xFFF7);
		}
	}

	lines = 0;
	emit_reset( buffer );

	// hyper-optimize register-to-register conditions
	if((sEA <= EA_AREG) && (dEA <= EA_AREG)) { // move reg to reg		
		if((reg_raw(sR) == 0xFF) && (reg_raw(dR) == 0xFF)) {
			// dynamic to dynamic
			if(debug) printf("@ dyn->dyn optimization\n");
			reg_alloc_temp(&tRR);
			emit("\t%s   r%d, [" CPU ", #%d]\n", ldx(size), tRR, sR * 4);
			if(dEA == EA_AREG) size = 4;
			else emit("\tcmp     r%d, #0\n", tRR);
			emit("\t%s    r%d, [" CPU ", #%d]\n", stx(size), tRR, dR * 4);
			
		} else if(reg_raw(sR) == 0xFF) {
			// dynamic into fixed
			if(debug) printf("@ dyn->fixed optimization\n");
			reg_alloc_68k(&dRR, dR, 4);

			if((size == 4) || (dEA == EA_AREG)) {
				emit("\t%s   r%d, [" CPU ", #%d]\n", ldx(size), dRR, sR * 4);
				if(dEA != EA_AREG) emit("\tcmp     r%d, #0\n", dRR);
			} else {
				reg_alloc_temp(&tRR);
				emit("\t%s   r%d, [" CPU ", #%d]\n", ldx(size), tRR, sR * 4);
				emit("\tcmp     r%d, #0\n", tRR);
				emit("\tbfi     r%d, r%d, $0, #%d\n", dRR, tRR, size * 8);
			}

		} else if(reg_raw(dR) == 0xFF) {
			// fixed into dynamic
			if(debug) printf("@ fixed->dyn optimization\n");
			reg_alloc_68k(&sRR, sR, size);
			
			if(dEA == EA_AREG) size = 4;
			else emit("\tcmp     r%d, #0\n", sRR);

			emit("\t%s    r%d, [" CPU ", #%d]\n", stx(size), sRR, dR * 4);

		} else {
			// fixed into fixed
			if(debug) printf("@ fixed->fixed optimization\n");
			reg_alloc_68k(&sRR, sR, 4);
			reg_alloc_68k(&dRR, dR, 4);
			if(dEA == EA_AREG) {
				if(size == 4) emit("\tmov     r%d, r%d\n", dRR, sRR);
				else emit("\tsxth    r%d, r%d\n", dRR, sRR);
			} else {
				if(size == 4) {
					emit("\tmovs    r%d, r%d\n", dRR, sRR);
				} else {
					reg_alloc_temp(&tRR);
					emit("\tsxt%c    r%d, r%d\n", (size == 1) ? 'b' : 'h', tRR, sRR);
					emit("\tcmp     r%d, #0\n", tRR);
					emit("\tbfi     r%d, r%d, $0, #%d\n", dRR, tRR, size * 8);
				}
			}
		}
		reg_flush();
	} 
	else if(sEA <= EA_AREG) { // reg -> mem
		if(debug) printf("@ reg->mem optimization\n");
		reg_alloc_68k(&sRR, sR, size);
		emit("\tcmp     r%d, #0\n", sRR);	
		
		get_destination_data(&dRR, NULL, dEA, dR, size);
		set_destination_data(&dRR, &sRR, dEA, size);

	} 
	else if(dEA <= EA_AREG) { // mem -> reg
		if(debug) printf("@ mem->reg optimization\n");

		if(reg_raw(dR) == 0xFF) {
			// dynamic, stuff into state
			get_source_data(&sRR, sEA, sR, size);
			if(dEA == EA_AREG) size = 4;
			else emit("\tcmp     r%d, #0\n", sRR);
			emit("\t%s    r%d, [" CPU ", #%d]\n", stx(size), sRR, dR * 4); 

		} else if((size == 4) || (dEA == EA_AREG)) {
			// fixed, but can load directly (saves mov)
			switch(sEA) {
			case EA_ADDR:
				emit("\t%s   r%d, [r%d]\n", ldx(size), reg_raw(dR), reg_raw(sR)); break;
			case EA_AINC:
				emit("\t%s   r%d, [r%d], #4\n", ldx(size), reg_raw(dR), reg_raw(sR)); break;
			case EA_ADEC:
				if(sR == dR) emit("\t%s   r%d, [r%d, #4]\n", ldx(size), reg_raw(dR), reg_raw(sR));
				else emit("\t%s   r%d, [r%d, #4]!\n", ldx(size), reg_raw(dR), reg_raw(sR));
				break;
			case EA_ADIS: case EA_AIDX:
				emit("\t%s   r%d, [r%d, r1]\n", ldx(size), reg_raw(dR), reg_raw(sR)); break;
			case EA_PIDX: case EA_PDIS: case EA_ABSW: case EA_ABSL:
				emit("\t%s   r%d, [r1]\n", ldx(size), reg_raw(dR)); break;
			case EA_IMMD:
				if(size==4) emit("\tmovs    r%d, r%d\n", reg_raw(dR), reg_raw(sR));
				else emit("\tbfi     r%d, r%d, #0, #%d\n", reg_raw(dR), reg_raw(sR), size * 8);
				break;
			}
			if(dEA != EA_AREG && dEA != EA_IMMD) emit("\tcmp     r%d, #0\n", sRR);

		} else {
			// fixed, bfi
			get_source_data(&sRR, sEA, sR, size);
			if(dEA == EA_AREG) {
				emit("\tmov     r%d, r%d\n", reg_raw(dR), sRR);
			} else {
				emit("\tcmp     r%d, #0\n", sRR);
				emit("\tbfi     r%d, r%d, #0, #%d\n", reg_raw(dR), sRR, size * 8); 
			}
		}
		reg_flush();
	} 
	else { // mem -> mem
		if(debug) printf("@ mem->mem\n");
		get_source_data(&sRR, sEA, sR, size);
		get_destination_data(&dRR, NULL, dEA, dR, size);
		set_destination_data(&dRR, &sRR, dEA, size);
	}
	
	return lines_ext(lines, sEA, dEA, size);
}

int emit_MOVEB(char *buffer, uint16_t opcode) {
	uint16_t size = 1;
	return emit_move(buffer, size, opcode);
}

int emit_MOVEW(char *buffer, uint16_t opcode) {
	uint16_t size = 2;
	return emit_move(buffer, size, opcode);
}

int emit_MOVEL(char *buffer, uint16_t opcode) {
	uint16_t size = 4;
	return emit_move(buffer, size, opcode);
}

int emit_MOVEP(char *buffer, uint16_t opcode) {
	uint16_t size = (opcode & 0x0040) ? 4 : 2;
	uint8_t aR, aRR, dR, dRR, tRR;

	lines = 0;
	emit_reset( buffer );	

	reg_alloc_arm(1); // displacement (always)

	aR = (opcode & 0x0007) | 8;
	dR = (opcode & 0x0E00) > 9;
	// if(opcode & 0x0080) {
	// 	// register to memory
	// } else {
	// 	aR = (opcode & 0x0007) | 8;
	// 	dR = (opcode & 0x0E00) > 9;
	// }

	// add to our displacement and free Ax
	if(!emit_get_reg( &aRR, aR, 4 )) return -1;
	emit("\tadd     r1 , r1, r%d\n", aRR);
	//reg_modified(aRR); 
	reg_free(aRR);

	// allocate our accumulator
	reg_alloc_temp(&tRR);
	
	// get our data register -- note, in long-load
	// there's no need to load the actual data
	if(!emit_get_reg( &dRR, dR, 4 )) return -1;

	if(opcode & 0x0080) {
		// register to memory
		emit("\tstrb    r%d, [r1, #0]\n", dRR);
		emit("\tror     r%d, r%d, #8\n", dRR, dRR);
		emit("\tstrb    r%d, [r1, #2]\n", dRR);
		if(size == 4) {
			emit("\tror     r%d, r%d, #8\n", dRR, dRR);
			emit("\tstrb    r%d, [r1, #4]\n", dRR);
			emit("\tror     r%d, r%d, #8\n", dRR, dRR);
			emit("\tstrb    r%d, [r1, #6]\n", dRR);
			if(dRR >= REG_MAP_COUNT) {
				emit("\tror     r%d, r%d, #8\n", dRR, dRR);
			}
		} else if(dRR >= REG_MAP_COUNT) {
			emit("\tror     r%d, r%d, #24\n", dRR, dRR);
		}
	} else {
		// memory to register
		emit("\tldrb    r%d, [r1, #0]\n", 	  tRR);
		emit("\tbfi     r%d, r%d, #0, #8\n",  dRR, tRR);
		emit("\tldrb    r%d, [r1, #2]\n",     tRR);
		emit("\tbfi     r%d, r%d, #8, #8\n",  dRR, tRR);
		if(size == 4) {
			emit("\tldrb    r%d, [r1, #4]\n",     tRR);
			emit("\tbfi     r%d, r%d, #16, #8\n", dRR, tRR);
			emit("\tldrb    r%d, [r1, #6]\n",     tRR);
			emit("\tbfi     r%d, r%d, #24, #8\n", dRR, tRR);
		}
		reg_modified(dRR);
	}
	reg_flush();
	return lines | EXT_WORD_SRC_16B;
}

int emit_MOVEM(char *buffer, uint16_t opcode) {
	char *ldx, *stx;
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
	
	uint8_t sR = ((opcode & 0x0007) >> 0) | 8;
	uint8_t sRR = 2;
	
	if((sEA == EA_AIDX) || (sEA == EA_ADIS)) {
		emit("\tadd     r2, r%d, r1\n", reg_raw(sR));
	} else if(sEA == EA_ADDR) {
		emit("\tmov     r2, r%d\n", reg_raw(sR));
	} else if((sEA == EA_AINC) || (sEA == EA_ADEC)) {
		sRR = reg_raw(sR);
	} // else r2 already has absolute register

	// If the addressing register is also loaded from 
	// memory, the memory value is ignored	
	if (sEA == EA_AINC) {
		// memory to register-list, normal reg list D0-D7, A0-A7
		// address register should point to the next long address
		if(size == 2) {
			ldx = "ldrhcs"; stx = "strh  ";
		} else {
			ldx = "ldrcs "; stx = "str   ";
		}
		// copy memory into CPU register struct
		// r0 = temp
		// r1 = mask
		// r2, r6-r13 = register memory address
		// r3 = CPU state address
        emit("    stm     r5!, {r3-r4}  ;@ save d0,d1\n");
		emit("    add     r3, r5, #32\n");
		emit("    stm     r3!, {r6-r13} ;@ save a0-a7\n");
		if(size == 2) emit("    add     r3, r5, #2\n");
		emit("    cmp     r1, #0\n");
		emit("0:  beq     1f\n");
		emit("    lsrs    r1, r1, #1\n");
		emit("    %s  r0, [r%d], #%d\n", ldx, sRR, size);
		emit("    %s  r0, [r3], #4\n", stx);
		emit("    b       0b\n");
		emit("1:  ldm     r5!, {r3,r4}  ;@ restore d0,d1\n");
		emit("    add     r2, r5, #32\n");
		emit("    ldm     r2!, {r6-r13} ;@ restore a0-a7\n");
		
	} else if (sEA == EA_ADEC) {
		// register-list to memory, reverse reg list A7-A0, D7-D0
		// address register should point to the last long address written
		if(size == 2) {
			ldx = "ldrh  "; stx = "strhcs";
		} else {
			ldx = "ldr   "; stx = "strcs ";
		}
		// copy memory into CPU register struct
		// r0 = temp
		// r1 = mask
		// r2, r6-r13 = register memory address
		// r3 = CPU state address
        emit("    stm     r5!, {r3-r4}  ;@ save d0,d1\n");
		emit("    add     r3, r5, #32\n");
		emit("    stm     r3!, {r6-r13} ;@ save a0-a7\n");
		if(size == 2) emit("    add     r3, r5, #2\n");
		emit("    lsls    r1, r1, #16\n");
		emit("0:  beq     1f\n");
		emit("    lsls    r1, r1, #1\n");
		emit("    %s  r0, [r3, #-4]!\n", ldx);
		emit("    %s  r0, [r%d, #-%d]!\n", stx, sRR, size);
		emit("    b       0b\n");
		emit("1:  ldm     r5!, {r3,r4}  ;@ restore d0,d1\n");
		emit("    add     r2, r5, #32\n");
		emit("    ldm     r2!, {r6-r13} ;@ restore a0-a7\n");
		
	} else if(opcode & 0x0400) {
		// memory to register-list, normal reg list D0-D7, A0-A7
		// address register remains unchanged
		static bool wrote_movem_word = 0;
		static bool wrote_movem_long = 0;
		if(size == 2) {
			if(wrote_movem_word) {
				emit("\tb       movem_word_m2r\n");
				return lines | EXT_WORD_SRC_16B | NO_BX_LR;
			}
			wrote_movem_word = 1;
			ldx = "ldrhcs"; stx = "strh  ";
		} else {
			if(wrote_movem_long) {
				emit("\tb       movem_long_m2r\n");
				return lines | EXT_WORD_SRC_16B | NO_BX_LR;
			}
			wrote_movem_long = 1;
			ldx = "ldrcs "; stx = "str   ";
		}
		// copy memory into CPU register struct
		// r0 = temp
		// r1 = mask
		// r2, r6-r13 = register memory address
		// r3 = CPU state address
		emit("movem_%s_m2r:\n", (size == 4) ? "long" : "word");
        emit("    stm     r5!, {r3-r4}  ;@ save d0,d1\n");
		emit("    add     r3, r5, #32\n");
		emit("    stm     r3!, {r6-r13} ;@ save a0-a7\n");
		if(size == 2) emit("    add     r3, r5, #2\n");
		emit("    cmp     r1, #0\n");
		emit("0:  beq     1f\n");
		emit("    lsrs    r1, r1, #1\n");
		emit("    %s  r0, [r%d], #%d\n", ldx, sRR, size);
		emit("    %s  r0, [r3], #4\n", stx);
		emit("    b       0b\n");
		emit("1:  ldm     r5!, {r3,r4}  ;@ restore d0,d1\n");
		emit("    add     r2, r5, #32\n");
		emit("    ldm     r2!, {r6-r13} ;@ restore a0-a7\n");

	} else {
		// register-list to memory, normal reg list D0-D7, A0-A7
		// address register remains unchanged
		static bool wrote_movem_word = 0;
		static bool wrote_movem_long = 0;
		if(size == 2) {
			if(wrote_movem_word) {
				emit("\tb       movem_word_r2m\n");
				return lines | EXT_WORD_SRC_16B | NO_BX_LR;
			}
			wrote_movem_word = 1;
			ldx = "ldrh  "; stx = "strhcs";
		} else {
			if(wrote_movem_long) {
				emit("\tb       movem_long_r2m\n");
				return lines | EXT_WORD_SRC_16B | NO_BX_LR;
			}
			wrote_movem_long = 1;
			ldx = "ldr   "; stx = "strcs ";
		}
		// copy memory into CPU register struct
		// r0 = temp
		// r1 = mask
		// r2, r6-r13 = register memory address
		// r3 = CPU state address
		emit("movem_%s_r2m:\n", (size == 4) ? "long" : "word");
        emit("    stm     r5!, {r3-r4}  ;@ save d0,d1\n");
		emit("    add     r3, r5, #32\n");
		emit("    stm     r3!, {r6-r13} ;@ save a0-a7\n");
		if(size == 2) emit("    add     r3, r5, #2\n");
		emit("    cmp     r1, #0\n");
		emit("0:  beq     1f\n");
		emit("    lsrs    r1, r1, #1\n");
		emit("    %s  r0, [r3], #4\n", stx);
		emit("    %s  r0, [r%d], #%d\n", ldx, sRR, size);
		emit("    b       0b\n");
		emit("1:  ldm     r5!, {r3,r4}  ;@ restore d0,d1\n");
		emit("    add     r2, r5, #32\n");
		emit("    ldm     r2!, {r6-r13} ;@ restore a0-a7\n");

	}

	reg_flush();
	return lines | EXT_WORD_SRC_16B;
/*
*/

}




