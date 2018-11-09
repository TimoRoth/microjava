#if defined(UJ_FTR_SUPPORT_LONG) || defined(UJ_FTR_SUPPORT_DOUBLE)
	
	.globl u64_from_halves
	u64_from_halves:
		
		ret
	
	
	
	
	
	.globl u64_32_to_64
	u64_32_to_64:
		
		movw r20,r24
		movw r18,r22
		clr r25
		clr r24
		movw r22, r24
		ret
	
	
	
	
	
	.globl u64_64_to_32
	u64_64_to_32:
		
		movw r24, r20
		movw r22, r18
		ret
	
	
	
	
	
	.globl u64_get_hi
	u64_get_hi:
		
		ret
	
	
	
	
	
	.globl u64_add
	u64_add:
		
		add r18, r10
		adc r19, r11
		adc r20, r12
		adc r21, r13
		adc r22, r14
		adc r23, r15
		adc r24, r16
		adc r25, r17
		ret
	
	
	
	
	
	.globl u64_sub
	u64_sub:
		
		sub r18, r10
		sbc r19, r11
		sbc r20, r12
		sbc r21, r13
		sbc r22, r14
		sbc r23, r15
		sbc r24, r16
		sbc r25, r17
		ret
	
	
	
	
	
	.globl u64_add32
	u64_add32:
		
		add r18, r14
		adc r19, r15
		adc r20, r16
		adc r21, r17
		adc r22, r1
		adc r23, r1
		adc r24, r1
		adc r25, r1
		ret
	
	
	
	
	
	.globl u64_inc
	u64_inc:
		
		sec
		adc r18, r1
		adc r19, r1
		adc r20, r1
		adc r21, r1
		adc r22, r1
		adc r23, r1
		adc r24, r1
		adc r25, r1
		ret
	
	
	
	
	
	.globl u64_and
	u64_and:
		
		and r18, r10
		and r19, r11
		and r20, r12
		and r21, r13
		and r22, r14
		and r23, r15
		and r24, r16
		and r25, r17
		ret
	
	
	
	
	
	.globl u64_orr
	u64_orr:
		
		or r18, r10
		or r19, r11
		or r20, r12
		or r21, r13
		or r22, r14
		or r23, r15
		or r24, r16
		or r25, r17
		ret
	
	
	
	
	
	.globl u64_xor
	u64_xor:
		
		eor r18, r10
		eor r19, r11
		eor r20, r12
		eor r21, r13
		eor r22, r14
		eor r23, r15
		eor r24, r16
		eor r25, r17
		ret
	
	
	
	
	
	.globl u64_zero
	u64_zero:
		
		clr r18
		clr r19
		movw r20, r18
		movw r22, r18
		movw r24, r18
		ret
	
	
	
	
	
	.globl i64_xtnd32
	i64_xtnd32:
		
		clr r25
		sbrc r21, 7
		com r25
		mov r24, r25
		movw r22, r24
		ret
	
	
	
	
	
	.globl u64_isZero
	u64_isZero:
		
		or r25, r24
		or r25, r23
		or r25, r22
		or r25, r21
		or r25, r20
		or r25, r19
		or r25, r18
		ldi r24, 1
		cpse r25, r1
		clr r24
		ret

	
	
	
	
	.globl i64_isNeg
	i64_isNeg:
		
		clr r24
		sbrc r25, 7
		ldi r24, 1
		ret
	
	
	
	
	
	.globl u64_ashr
	u64_ashr:
		
		cpi r17, 0
		brne u64_ashr_overshift
		mov r0, r16
		andi r16, 0xC0
		brne u64_ashr_overshift
	

		;hack
		mov r16, r0
		jmp __ashrdi3
	
		cpse r0, r1
		rjmp u64_ashr_loop_body
		ret
	
	u64_ashr_loop_body:
		
		asr r25
	
	u64_ashr_loop_body_after_one_instr:
	
		ror r24
		ror r23
		ror r22
		ror r21
		ror r20
		ror r19
		ror r18
		
		dec r0
		brne u64_ashr_loop_body
		ret
	
	u64_ashr_overshift:
		
		clr r18
		sbrc r25, 7
		com r18
		mov r19, r18
		movw r20, r18
		movw r22, r18
		movw r24, r18
		ret
	
	
	
	
	
	.globl u64_shr
	u64_shr:
		
		tst r17
		brne u64_zero
		mov r0, r16
		andi r16, 0xC0
		brne u64_zero
	

		;hack!!!!!!
		mov r16, r0
		jmp __lshrdi3

	u64_shr_loop:
		
		cpse r0, r1
		rjmp u64_shr_loop_body
		ret
	
	u64_shr_loop_body:
		
		lsr r25
		rjmp u64_ashr_loop_body_after_one_instr
	
	
	
	
	u64_shl_overshift:
		jmp u64_zero
	
	.globl u64_shl
	u64_shl:
		
		cpi r17, 0
		brne u64_shl_overshift
		mov r30, r16
		andi r16, 0xC0
		brne u64_shl_overshift
		
		;hack!!!!
		mov r16, r30
		jmp __ashldi3

		ldi r31, 8
		
	u64_shl_big:
		cpi r30, 8
		brcc u64_shl_small
		sub r30, r31
		
		mov r25, r24
		mov r24, r23
		mov r23, r22
		mov r22, r21
		mov r21, r20
		mov r20, r19
		mov r19, r18
		clr r18
		rjmp u64_shl_big
		
		cpse r0, r1
		rjmp u64_shl_loop_body
		
	u64_shl_small:
		
		tst r30
		breq u64_shl_return
	
	u64_shl_loop_body:
		
		
		lsl r18
		rol r19
		rol r20
		rol r21
		rol r22
		rol r23
		rol r24
		rol r25
		
		dec r30
		brne u64_shl_loop_body
	u64_shl_return:
		ret
	
	
	
	
	
	
	
	.globl u64_umul3232
	u64_umul3232:
		
		; r25..r22 -> a
		; r21..r18 -> b
		; r17..r10 -> result
		
		;push used regs
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15
		push r16
		push r17
		
		;do the hard work
		;(r25 << 24 + r24 << 16 + r23 << 8 + r22) * (r21 << 24 + r20 << 16 + r19 << 8 + r18) = 
		;
		; (r25 * r21) << 48					+
		; (r25 * r20 + r24 * r21) << 40				+
		; (r25 * r19 + r24 * r20 + r23 * r21) << 32		+
		; (r25 * r18 + r24 * r19 + r23 * r20 + r22 * r21) << 24	+
		; (r24 * r18 + r23 * r19 + r22 * r20) << 16		+
		; (r23 * r18 + r22 * r19) << 8				+
		; (r22 * r18) << 0
		
		;first steps - cheating a bit - for each 16-bit part, premultiply a piece to save us on the "adc"s later
		mul r22, r18
		movw r10, r0
		mul r24, r18
		movw r12, r0
		mul r25, r19
		movw r14, r0
		mul r25, r21
		movw r16, r0
		
		;step (<< 8)
			mul r23, r18
			rcall umul3232_add_propagate_7bytes
			mul r22, r19
			rcall umul3232_add_propagate_7bytes
		
		;step (<< 16)
			mul r23, r19
			rcall umul3232_add_propagate_6bytes
			mul r22, r20
			rcall umul3232_add_propagate_6bytes
			
		;step (<< 24)
			mul r25, r18
			rcall umul3232_add_propagate_5bytes
			mul r24, r19
			rcall umul3232_add_propagate_5bytes
			mul r23, r20
			rcall umul3232_add_propagate_5bytes
			mul r22, r21
			rcall umul3232_add_propagate_5bytes
		
		;step (<< 32)
			mul r24, r20
			rcall umul3232_add_propagate_4bytes
			mul r23, r21
			rcall umul3232_add_propagate_4bytes
		
		;step (<< 40)
			mul r25, r20
			rcall umul3232_add_propagate_3bytes
			mul r24, r21
			rcall umul3232_add_propagate_3bytes
		
		;move result to final destination
		movw r24, r16
		movw r22, r14
		movw r20, r12
		movw r18, r10
		
		;pop saved regs, then return
		pop r17
		pop r16
		pop r15
		pop r14
		pop r13
		pop r12
		pop r11
		pop r10
		ret
	
	umul3232_add_propagate_7bytes:
		
		add r11, r0
		adc r12, r1
		clr r1
		rjmp umul3232_add_propagate_chain_r13
	
	umul3232_add_propagate_6bytes:
		
		add r12, r0
		adc r13, r1
		clr r1
		rjmp umul3232_add_propagate_chain_r14
	
	umul3232_add_propagate_5bytes:
		
		add r13, r0
		adc r14, r1
		clr r1
		rjmp umul3232_add_propagate_chain_r15
	
	umul3232_add_propagate_4bytes:
		
		add r14, r0
		adc r15, r1
		clr r1
		rjmp umul3232_add_propagate_chain_r16
	
	umul3232_add_propagate_3bytes:
		
		add r15, r0
		adc r16, r1
		clr r1
		rjmp umul3232_add_propagate_chain_r17
	
	umul3232_add_propagate_chain_r13:
		adc r13, r1
	umul3232_add_propagate_chain_r14:
		adc r14, r1
	umul3232_add_propagate_chain_r15:
		adc r15, r1
	umul3232_add_propagate_chain_r16:
		adc r16, r1
	umul3232_add_propagate_chain_r17:
		adc r17, r1
		ret
	
	
	
	
	
	.globl u64_smul3232
	u64_smul3232:
		
		push r10
		clr r10
		sbrs r25, 7
		rjmp u64_smul3232_a_is_pos
		com r10
		com r25
		com r24
		com r23
		com r22
		sec
		adc r22,r1
		adc r23,r1
		adc r24,r1
		adc r25,r1
	
	u64_smul3232_a_is_pos:
		
		sbrs r21, 7
		rjmp u64_smul3232_b_is_pos
		com r10
		com r21
		com r20
		com r19
		com r18
		sec
		adc r18,r1
		adc r19,r1
		adc r20,r1
		adc r21,r1
	
	u64_smul3232_b_is_pos:
		
		rcall u64_umul3232
		mov r0,r10
		pop r10
	
		sbrs r0, 0
		ret
	
	;if we're here, we need to negate the result
		
		rjmp u64_neg
	
	
	
	
	.globl u64_mul
	u64_mul:
		
		push r2
		push r3
		push r4
		push r5
		push r6
		push r7
		push r8
		push r9
		
		;r18..r25 = UInt32 a
		;r10..r17 = UInt32 b
		;(r25<<56 + r24<<48 + r23<<40 + r22<<32 + r21<<24 + r20<<16 + r19<<8 + r18) * (r17<<56 + r16<<48 + r15<<40 + r14<<32 + r13<<24 + r12<<16 + r11<<8 + r10)
	
		;result is the sum of the following terms
		; (r25*r10 + r24*r11 + r23*r12 + r22*r13 + r21*r14 + r20*r15 + r19*r16 + r18*r17) << 56
		; (r24*r10 + r23*r11 + r22*r12 + r21*r13 + r20*r14 + r19*r15 + r18*r16) << 48
		; (r23*r10 + r22*r11 + r21*r12 + r20*r13 + r19*r14 + r18*r15) << 40
		; (r22*r10 + r21*r11 + r20*r12 + r19*r13 + r18*r14) << 32
		; (r21*r10 + r20*r11 + r19*r12 + r18*r13) << 24
		; (r20*r10 + r19*r11 + r18*r12) << 16
		; (r19*r10 + r18*r11) << 8
		; (r18*r10) << 0
		
		;precalculate parts of the result ahead of time
		
		mul r24, r10
		movw r8, r0
		mul r22, r10
		movw r6, r0
		mul r20, r10
		movw r4, r0
		mul r18, r10
		movw r2, r0
		
		;step (<< 8)
			mul r19, r10
			rcall umul_add_propagate_7bytes
			mul r18, r11
			rcall umul_add_propagate_7bytes
		
		;step (<< 16)
			mul r19, r11
			rcall umul_add_propagate_6bytes
			mul r18, r12
			rcall umul_add_propagate_6bytes
		
		;step (<< 24)
			mul r21, r10
			rcall umul_add_propagate_5bytes
			mul r20, r11
			rcall umul_add_propagate_5bytes
			mul r19, r12
			rcall umul_add_propagate_5bytes
			mul r18, r13
			rcall umul_add_propagate_5bytes
		
		;step (<< 32)
			mul r21, r11
			rcall umul_add_propagate_4bytes
			mul r20, r12
			rcall umul_add_propagate_4bytes
			mul r19, r13
			rcall umul_add_propagate_4bytes
			mul r18, r14
			rcall umul_add_propagate_4bytes
		
		;step (<< 40)
			mul r23, r10
			rcall umul_add_propagate_3bytes
			mul r22, r11
			rcall umul_add_propagate_3bytes
			mul r21, r12
			rcall umul_add_propagate_3bytes
			mul r20, r13
			rcall umul_add_propagate_3bytes
			mul r19, r14
			rcall umul_add_propagate_3bytes
			mul r18, r15
			rcall umul_add_propagate_3bytes
			
		;step (<< 48)
			mul r23, r11
			rcall umul_add_propagate_2bytes
			mul r22, r12
			rcall umul_add_propagate_2bytes
			mul r21, r13
			rcall umul_add_propagate_2bytes
			mul r20, r14
			rcall umul_add_propagate_2bytes
			mul r19, r15
			rcall umul_add_propagate_2bytes
			mul r18, r16
			rcall umul_add_propagate_2bytes
			
		;step (<< 56)
			mul r25, r10
			add r9, r0
			mul r24, r11
			add r9, r0
			mul r23, r12
			add r9, r0
			mul r22, r13
			add r9, r0
			mul r21, r14
			add r9, r0
			mul r20, r15
			add r9, r0
			mul r19, r16
			add r9, r0
			mul r18, r17
			add r9, r0
			clr r1
		
		;move result to final destination
		movw r24, r8
		movw r22, r6
		movw r20, r4
		movw r18, r2
		
		;pop saved regs, then return
		pop r9
		pop r8
		pop r7
		pop r6
		pop r5
		pop r4
		pop r3
		pop r2
		ret
	
	umul_add_propagate_7bytes:
		
		add r3, r0
		adc r4, r1
		clr r1
		rjmp umul_add_propagate_chain_r5
	
	umul_add_propagate_6bytes:
		
		add r4, r0
		adc r5, r1
		clr r1
		rjmp umul_add_propagate_chain_r6
	
	umul_add_propagate_5bytes:
		
		add r5, r0
		adc r6, r1
		clr r1
		rjmp umul_add_propagate_chain_r7
	
	umul_add_propagate_4bytes:
		
		add r6, r0
		adc r7, r1
		clr r1
		rjmp umul_add_propagate_chain_r8
	
	umul_add_propagate_3bytes:
		
		add r7, r0
		adc r8, r1
		clr r1
		rjmp umul_add_propagate_chain_r9
		
	umul_add_propagate_2bytes:
		
		add r8, r0
		adc r9, r1
		clr r1
		ret
	
	umul_add_propagate_chain_r5:
		adc r5, r1
	umul_add_propagate_chain_r6:
		adc r6, r1
	umul_add_propagate_chain_r7:
		adc r7, r1
	umul_add_propagate_chain_r8:
		adc r8, r1
	umul_add_propagate_chain_r9:
		adc r9, r1
		ret
	
	
	
	
	
	u64_div_zero_bottom:
		jmp __rt_div0
	
	.globl u64_div
	u64_div:		
		;r31		UInt8 mask_byte
		;r30		UInt8 mask_bit
		;r18..r25	UInt64 top
		;r10..r17	UInt64 bottom
		;r2 ..r9	UInt64 ret
		
		ldi r30, 1
		ldi r31, 1
		
	;step 1 - normallize botton
	;step1.1 - coarse normalize bottom
	u64_div_coarse_norm_bot:
	
		tst r17
		brne u64_div_fine_norm_bot
	
		mov r17, r16
		mov r16, r15
		mov r15, r14
		mov r14, r13
		mov r13, r12
		mov r12, r11
		mov r11, r10
		clr r10
	
		lsl r31
		brcs u64_div_zero_bottom
		rjmp u64_div_coarse_norm_bot
	
	;nonzero bottom - we're committed - push stuff
	u64_div_fine_norm_bot:
	
		push r9
		push r8
		push r7
		push r6
		push r5
		push r4
		push r3
		push r2
		
		clr r2
		clr r3
		movw r4, r2
		movw r6, r2
		movw r8, r2
	
	;step 1.2 - fine normalize bottom
	u64_div_fine_norm_bot_loop:
	
		tst r17
		brmi u64_div_loop
	
		lsl r10
		rol r11
		rol r12
		rol r13
		rol r14
		rol r15
		rol r16
		rol r17
		lsl r30
		rjmp u64_div_fine_norm_bot_loop
	
	;step 2: perform division
	u64_div_loop:
		
		;compare
		cp  r18, r10
		cpc r19, r11
		cpc r20, r12
		cpc r21, r13
		cpc r22, r14
		cpc r23, r15
		cpc r24, r16
		cpc r25, r17
		brcs u64_div_mask_decr
		
		;subtract
		sub r18, r10
		sbc r19, r11
		sbc r20, r12
		sbc r21, r13
		sbc r22, r14
		sbc r23, r15
		sbc r24, r16
		sbc r25, r17
		
		;add to result
		sbrc r31, 0
		or r2, r30
		sbrc r31, 1
		or r3, r30
		sbrc r31, 2
		or r4, r30
		sbrc r31, 3
		or r5, r30
		sbrc r31, 4
		or r6, r30
		sbrc r31, 5
		or r7, r30
		sbrc r31, 6
		or r8, r30
		sbrc r31, 7
		or r9, r30
			
	u64_div_mask_decr:
		
		lsr r30
		brcc u64_div_mask_shift
		ldi r30, 0x80
		lsr r31
		brcs u64_div_done
	
	u64_div_mask_shift:
	
		lsr r17
		ror r16
		ror r15
		ror r14
		ror r13
		ror r12
		ror r11
		ror r10
	
		rjmp u64_div_loop
	
	u64_div_done:
	
		movw r24, r8
		movw r22, r6
		movw r20, r4
		movw r18, r2
	
		pop r2
		pop r3
		pop r4
		pop r5
		pop r6
		pop r7
		pop r8
		pop r9
		ret
	
	
	
	
	
	
	
	
	.globl u64_mod
	u64_mod:		
		;r31		UInt8 mask_byte
		;r30		UInt8 mask_bit
		;r18..r25	UInt64 top
		;r10..r17	UInt64 bottom
		
		ldi r30, 1
		ldi r31, 1
		
	;step 1 - normallize botton
	;step1.1 - coarse normalize bottom
	u64_mod_coarse_norm_bot:
	
		tst r17
		brne u64_mod_fine_norm_bot_loop
	
		mov r17, r16
		mov r16, r15
		mov r15, r14
		mov r14, r13
		mov r13, r12
		mov r12, r11
		mov r11, r10
		clr r10
	
		lsl r31
		brcs u64_mod_zero_bottom
		rjmp u64_mod_coarse_norm_bot
	
	;step 1.2 - fine normalize bottom
	u64_mod_fine_norm_bot_loop:
	
		tst r17
		brmi u64_mod_loop
	
		lsl r10
		rol r11
		rol r12
		rol r13
		rol r14
		rol r15
		rol r16
		rol r17
		lsl r30
		rjmp u64_mod_fine_norm_bot_loop
	
	;step 2: perform modision
	u64_mod_loop:
		
		;compare
		cp  r18, r10
		cpc r19, r11
		cpc r20, r12
		cpc r21, r13
		cpc r22, r14
		cpc r23, r15
		cpc r24, r16
		cpc r25, r17
		brcs u64_mod_mask_decr
		
		;subtract
		sub r18, r10
		sbc r19, r11
		sbc r20, r12
		sbc r21, r13
		sbc r22, r14
		sbc r23, r15
		sbc r24, r16
		sbc r25, r17
			
	u64_mod_mask_decr:
		
		lsr r30
		brcc u64_mod_mask_shift
		ldi r30, 0x80
		lsr r31
		brcs u64_mod_done
	
	u64_mod_mask_shift:
	
		lsr r17
		ror r16
		ror r15
		ror r14
		ror r13
		ror r12
		ror r11
		ror r10
	
		rjmp u64_mod_loop
	
	u64_mod_done:
	
		ret
		
	u64_mod_zero_bottom:
		jmp __rt_div0
	
	
	
	
	
	.globl u64_neg
	u64_neg:
		com r25
		com r24
		com r23
		com r22
		com r21
		com r20
		com r19
		com r18
		sec
		adc r18,r1
		adc r19,r1
		adc r20,r1
		adc r21,r1
		adc r22,r1
		adc r23,r1
		adc r24,r1
		adc r25,r1
		ret
	
	
	
	
	
	.globl i64_mod
	i64_mod:
	
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15
		push r16
		push r17
		
		clr r30
		
		sbrs r25, 7
		rjmp i64_mod_a_is_pos
		com r30
		com r25
		com r24
		com r23
		com r22
		com r21
		com r20
		com r19
		com r18
		sec
		adc r18,r1
		adc r19,r1
		adc r20,r1
		adc r21,r1
		adc r22,r1
		adc r23,r1
		adc r24,r1
		adc r25,r1
	i64_mod_a_is_pos:
	
		sbrs r17, 7
		rjmp i64_mod_b_is_pos
		com r17
		com r16
		com r15
		com r14
		com r13
		com r12
		com r11
		com r10
		sec
		adc r10,r1
		adc r11,r1
		adc r12,r1
		adc r13,r1
		adc r14,r1
		adc r15,r1
		adc r16,r1
		adc r17,r1
	i64_mod_b_is_pos:
	
		push r30
		rcall u64_mod
		pop r30

                pop r17
                pop r16
                pop r15
                pop r14
                pop r13
                pop r12
                pop r11
                pop r10

		sbrc r30, 0
		rjmp u64_neg
		ret
	
	
	
	
	
	.globl i64_div
	i64_div:
	
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15
		push r16
		push r17
		
		clr r30
		
		sbrs r25, 7
		rjmp i64_div_a_is_pos
		com r30
		com r25
		com r24
		com r23
		com r22
		com r21
		com r20
		com r19
		com r18
		sec
		adc r18,r1
		adc r19,r1
		adc r20,r1
		adc r21,r1
		adc r22,r1
		adc r23,r1
		adc r24,r1
		adc r25,r1
	i64_div_a_is_pos:
	
		sbrs r17, 7
		rjmp i64_div_b_is_pos
		com r30
		com r17
		com r16
		com r15
		com r14
		com r13
		com r12
		com r11
		com r10
		sec
		adc r10,r1
		adc r11,r1
		adc r12,r1
		adc r13,r1
		adc r14,r1
		adc r15,r1
		adc r16,r1
		adc r17,r1
	i64_div_b_is_pos:
	
		push r30
		rcall u64_div
		pop r30

                pop r17
                pop r16
                pop r15
                pop r14
                pop r13
                pop r12
                pop r11
                pop r10

		sbrc r30,0
		rjmp u64_neg
		ret

	
#endif
