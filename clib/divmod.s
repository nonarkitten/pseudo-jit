		// use vfp
		.global __aeabi_uidivmod
		.global __aeabi_idivmod
		.global __aeabi_uidiv
		.global __aeabi_idiv

		.global rev32

        .text

        @ This code is assembled for ARM instructions
        .code 32

__aeabi_uidivmod:
		vmov			s0, r0
		vcvt.f64.u32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.u32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.u32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		vcvt.f64.u32	d2, s4 @ (double)q
		vmls.f64   		d0, d1, d2 @ num - (q * n)
		vcvt.u32.f64	s2, d0
		vmov			r1, s2

		bx				lr

__aeabi_idivmod:
		vmov			s0, r0
		vcvt.f64.s32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.s32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.s32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		vcvt.f64.s32	d2, s4 @ (double)q
		vmls.f64   		d0, d1, d2 @ num - (q * n)
		vcvt.s32.f64	s2, d0
		vmov			r1, s2

		bx				lr

__aeabi_uidiv:
		vmov			s0, r0
		vcvt.f64.u32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.u32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.u32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		bx				lr

__aeabi_idiv:
		vmov			s0, r0
		vcvt.f64.s32	d0, s0 @ num
		vmov			s2, r1
		vcvt.f64.s32	d1, s2 @ denom

		vdiv.f64		d2, d0, d1
		vcvt.s32.f64	s4, d2
		vmov			r0, s4 @ q = int(num / denom)

		bx				lr

rev32:
		rev				r0, r0
		bx				lr

		.end

