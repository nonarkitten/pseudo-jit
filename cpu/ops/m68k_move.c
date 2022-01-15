#include "m68k_common.h"
#include "m68k_cpu.h"
#include "m68k_emit_ea.h"
#include "m68k_registers.h"

//extern int emit_alu(char *buffer, uint16_t size, uint16_t sEA, uint16_t dEA);

static void fixup_rors(char *buffer, uint8_t sRR, uint16_t size) {
	char *ror = strstr(buffer, "ror");
	if((size == 4) && ror) {
		if(debug) printf("@ fixup ror -> rors\n");
		memcpy(ror, "rors", 4);
	} else {
		emit("\tcmp     r%d, #0\n", sRR);	
	}
}

int emit_move(char *buffer, uint16_t size, uint16_t opcode) {
	uint16_t dEA = ((opcode & 0xE00) >> 9) | ((opcode & 0x1C0) >> 3);
	uint16_t sEA = (opcode & 0x3F);

	uint8_t dR=0, dRR=0, sR=0, sRR=0, tRR=0;
	uint8_t org_sEA = sEA;//, org_dEA = dEA;

	if(size == 3 || size > 4) return -1;
	
	if(debug) printf("@ emit_move, sEA %02X, dEA %02X, size %d\n", sEA, dEA, size);

	sR = sEA & 7; sEA = (sEA >> 3) & 7; if(sEA == 7) sEA += sR; if(sEA) sR += 8;
	dR = dEA & 7; dEA = (dEA >> 3) & 7; if(dEA == 7) dEA += dR; if(dEA) dR += 8;
	
	// Universal EA exceptions
	// Cannot be byte and an address register
	if((size == 1) && (sEA == EA_AREG)) return -1;
	if((size == 1) && (dEA == EA_AREG)) return -1;
	// PC-relative and Immediate are disallowed as destinations
	if((dEA == EA_PDIS) || (dEA == EA_PIDX) || (dEA == EA_IMMD)) return -1;
	// Exclude obvious not-real addressing modes above immediate
	if((sEA > EA_IMMD) || (dEA > EA_IMMD)) return -1;
	
	// Guard standard aliasing of addressing modes -- this is an error
	// Source PC-Relative should be in R1 as an absolute
	// Both ABS.W and ABS.L should also be in R1 as an absolute
	// And finally, indexed and displacement are the same modes
	//if((sEA == EA_ABSW) || (sEA == EA_ADIS) || (sEA == EA_PDIS) || (sEA == EA_PIDX)) return -1;
	//printf("@ %d\n", __LINE__);
	// Destination PC modes are already illegal, but R2
	//if((dEA == EA_ABSW) || (dEA == EA_ADIS)) return -1;
	//printf("@ %d\n", __LINE__);
	
	//
	if(sR == dR) {
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
		if(sEA == EA_AINC && dEA == EA_ADDR) {
			// movea.wl	(an)+,an	movea.wl	(an),an	
			return -(opcode & 0xFFF7);
		}
	}

	lines = 0;
	emit_reset( buffer );

	// hyper-optimize register-to-register conditions
	if((sEA <= EA_AREG) && (dEA <= EA_AREG)) { // move reg to reg
		if((dEA == EA_AREG) && (size == 4) && (dR == sR)) return 0; // NOP
		
		if((reg_raw(sR) == 0xFF) && (reg_raw(dR) == 0xFF)) {
			// dynamic to dynamic
			if(debug) printf("@ dyn->dyn optimization\n");
			reg_alloc_temp(&tRR);
			emit("\t%s   r%d, [r12, #%d]\n", ldx(size), tRR, sR * 4);
			if(dEA == EA_AREG) size = 4;
			else emit("\tcmp     r%d, #0\n", tRR);
			if(sR != dR) emit("\t%s    r%d, [r12, #%d]\n", stx(size), tRR, dR * 4);
			
		} else if(reg_raw(sR) == 0xFF) {
			// dynamic into fixed
			if(debug) printf("@ dyn->fixed optimization\n");
			reg_alloc_68k(&dRR, dR, 4);

			if((size == 4) || (dEA == EA_AREG)) {
				if(sR != dR) emit("\t%s   r%d, [r12, #%d]\n", ldx(size), dRR, sR * 4);
				if(dEA != EA_AREG) emit("\tcmp     r%d, #0\n", dRR);
			} else {
				reg_alloc_temp(&tRR);
				emit("\t%s   r%d, [r12, #%d]\n", ldx(size), tRR, sR * 4);
				emit("\tcmp     r%d, #0\n", tRR);
				if(sR != dR) emit("\tbfi     r%d, r%d, $0, #%d\n", dRR, tRR, size * 8);
			}

		} else if(reg_raw(dR) == 0xFF) {
			// fixed into dynamic
			if(debug) printf("@ fixed->dyn optimization\n");
			reg_alloc_68k(&sRR, sR, size);
			
			if(dEA == EA_AREG) size = 4;
			else emit("\tcmp     r%d, #0\n", sRR);

			if(sR != dR) emit("\t%s    r%d, [r12, #%d]\n", stx(size), sRR, dR * 4);

		} else {
			// fixed into fixed
			if(debug) printf("@ fixed->fixed optimization\n");
			reg_alloc_68k(&sRR, sR, 4);
			reg_alloc_68k(&dRR, dR, 4);
			if(dEA == EA_AREG) {
				if(size == 4) emit("\tmov     r%d, r%d\n", dRR, sRR);
				else emit("\tsxth    r%d, r%d\n", dRR, sRR);
			} else {
				if(size == 4) emit("\tmovs    r%d, r%d\n", dRR, sRR);
				else {
					reg_alloc_temp(&tRR);
					emit("\tsxt%c    r%d, r%d\n", (size == 1) ? 'b' : 'h', tRR, sRR);
					emit("\tcmp     r%d, #0\n", tRR);
					if(sR != dR) emit("\tbfi     r%d, r%d, $0, #%d\n", dRR, tRR, size * 8);
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
		get_source_data(&sRR, sEA, sR, size);
		
		if(dEA == EA_AREG) size = 4;
		else fixup_rors(buffer, sRR, size);
		
		if(reg_raw(dR) == 0xFF) emit("\t%s    r%d, [r12, #%d]\n", stx(size), sRR, dR * 4); // dynamic
		else if(size == 4) {
			char *rors = strstr(buffer, "rors");
			if(rors) {
				if(debug) printf("@ fixup rors destination");
				dRR = reg_raw(dR);
				rors += 9;
				if(dRR > 9) *rors++ = '1';
				*rors = '0' + (dRR % 10);
			} else {
				// fixed, mov
				emit("\tmov     r%d, r%d\n", reg_raw(dR), sRR); 
			}
		}
		else emit("\tbfi     r%d, r%d, #0, #%d\n", reg_raw(dR), sRR, size * 8); // fixed, bfi

		reg_flush();
	} 
	else { // mem -> mem
		if(debug) printf("@ mem->mem\n");
		get_source_data(&sRR, sEA, sR, size);

		fixup_rors(buffer, sRR, size);

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




