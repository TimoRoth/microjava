#include "long64.h"

#if defined(UJ_FTR_SUPPORT_LONG) || defined(UJ_FTR_SUPPORT_DOUBLE)

	UInt64 u64_from_halves(UInt32 hi, UInt32 lo){
		
		UInt64 r;
		
		r.lo = lo;
		r.hi = hi;
		
		return r;	
	}
	
	
	UInt64 u64_32_to_64(UInt32 v){
		
		UInt64 r;
		
		r.hi = 0;
		r.lo = v;
		
		return r;	
	}
	
	UInt32 u64_64_to_32(UInt64 v){
		
		return v.lo;	
	}
	
	UInt32 u64_get_hi(UInt64 v){
		
		return v.hi;	
	}
	
	UInt64 u64_add(UInt64 a, UInt64 b){
		
		UInt64 r;
		
		r.lo = a.lo + b.lo;
		r.hi = a.hi + b.hi;
		if(r.lo < a.lo) r.hi++;
		
		return r;
	}
	
	UInt64 u64_add32(UInt64 a, UInt32 b){
		
		UInt64 r;
		
		r.lo = a.lo + b;
		r.hi = a.hi;
		if(r.lo < a.lo) r.hi++;
		
		return r;	
	}
	
	
	UInt64 u64_mul(UInt64 a, UInt64 b){
	
		return	u64_add(
			
				u64_umul3232(a.lo, b.lo),
				u64_shl(
					u64_add(
						u64_umul3232(a.lo, b.hi),
						u64_umul3232(a.hi, b.lo)
					),
					32
				)
			);
	}
	
	UInt64 u64_umul3232(UInt32 a, UInt32 b){
		
		UInt64 r;
		UInt32 ah, al, bh, bl;
		
		ah = a >> 16UL;
		al = a & 0xFFFFUL;
		bh = b >> 16UL;
		bl = b & 0xFFFFUL;
		
		r = u64_shl(u64_32_to_64(ah * bh), 16);
		r = u64_add32(r, ah * bl);
		r = u64_add32(r, al * bh);
		r = u64_add32(u64_shl(r, 16), al * bl);
		
		return r;
	}
	
	UInt64 u64_smul3232(Int32 a, Int32 b){
		
		Boolean negative = false;
		UInt64 r;
		
		if(a < 0){
			a = -a;
			negative = !negative;
		}
		
		if(b < 0){
			b = -b;
			negative = !negative;
		}
		
		r = u64_umul3232(a, b);
		
		if(negative){
			
			r.hi = ~r.hi;			//negate r
			r.lo = ~r.lo;
			r = u64_inc(r);
		}
		
		return r;
	}
	
	UInt64 u64_ashr(UInt64 v, UInt16 bits){
		
		UInt64 a = v;
		
		if(bits >= 64){
		
			if(v.hi & 0x80000000UL){
				
				a.hi = 0xFFFFFFFFUL;
				a.lo = 0xFFFFFFFFUL;
			}
			else{
				a.hi = 0;
				a.lo = 0;
			}
			return a;	
		}
		
		if(bits >= 32){
			a.lo = a.hi;
			a.hi = (a.lo & 0x80000000UL) ? 0xFFFFFFFFUL : 0;
			bits -= 32;
		}
		
		a.lo = (a.lo >> ((UInt32)bits)) | (a.hi << (32UL - ((UInt32)bits)));
		a.hi = ((Int32)a.hi) >> ((UInt32)bits);
		
		return a;
	}
	
	UInt64 u64_shr(UInt64 v, UInt16 bits){
		
		UInt64 a = v;
		
		if(bits >= 64){
			
			return u64_zero();	
		}
		
		if(bits >= 32){
			a.lo = a.hi;
			a.hi = 0;
			bits -= 32;
		}
		
		a.lo = (a.lo >> ((UInt32)bits)) | (a.hi << (32UL - ((UInt32)bits)));
		a.hi >>= ((UInt32)bits);
		
		return a;
	}
	
	UInt64 u64_shl(UInt64 v, UInt16 bits){
		
		UInt64 a = v;
		
		if(bits >= 64){
			
			return u64_zero();
		}
		
		if(bits >= 32){
			a.hi = a.lo;
			a.lo = 0;
			bits -= 32;
		}
		
		a.hi = (a.hi << ((UInt32)bits)) | (a.lo >> (32UL - ((UInt32)bits)));
		a.lo <<= ((UInt32)bits);
		
		return a;
	}
	
	UInt64 i64_xtnd32(UInt64 v){
		
		UInt64 a = v;
		
		if(a.lo & 0x80000000UL) a.hi = 0xFFFFFFFFUL;
		
		return a;
	}
	
	Boolean u64_isZero(UInt64 a){
		
		return a.lo == 0 && a.hi == 0;
	}
	
	Boolean i64_isNeg(Int64 a){
	
		return (a.hi & 0x80000000UL) != 0UL;
	}
	
	UInt64 u64_inc(UInt64 v){
		
		UInt64 a = v;
		
		if(!++a.lo) a.hi++;
		
		return a;
	}
	
	UInt64 u64_and(UInt64 a, UInt64 b){
		
		UInt64 r;
		
		r.lo = a.lo & b.lo;
		r.hi = a.hi & b.hi;
		
		return r;	
	}
	
	UInt64 u64_orr(UInt64 a, UInt64 b){
		
		UInt64 r;
		
		r.lo = a.lo | b.lo;
		r.hi = a.hi | b.hi;
		
		return r;	
	}
	
	UInt64 u64_xor(UInt64 a, UInt64 b){
		
		UInt64 r;
		
		r.lo = a.lo ^ b.lo;
		r.hi = a.hi ^ b.hi;
		
		return r;	
	}
	
	UInt64 u64_zero(void){
		
		UInt64 r;
		
		r.lo = 0;
		r.hi = 0;
		
		return r;	
	}
	
	
	UInt64 u64_sub(UInt64 a, UInt64 b){
		
		UInt64 bn;
		
		bn.lo = ~b.lo;
		bn.hi = ~b.hi;
		
		bn = u64_inc(bn);
		
		return u64_add(a, bn);
	}
	
	static UInt64 u64_divmod(UInt64 a, UInt64 b, Boolean div){
	
		UInt64 ret = u64_zero();
		UInt64 mask = u64_32_to_64(1);
		UInt8 numShifts = 0;
		
		while(i64_isNeg(u64_sub(b, a)) && (numShifts < 63)){	//while b < a
		
			mask = u64_shl(mask, 1);
			b = u64_shl(b, 1);
			numShifts++;
		}
		
		while(!u64_isZero(mask)){
		
			if(!i64_isNeg(u64_sub(a, b))){			//a >= b	
				
				ret = u64_orr(ret, mask);
				a = u64_sub(a, b);
			}
			mask = u64_shr(mask, 1);
			b = u64_shr(b, 1);
		}
		
		return div ? ret : a;
	}
	
	UInt64 u64_div(UInt64 a, UInt64 b){
	
		return u64_divmod(a, b, true);
	}
	
	UInt64 u64_mod(UInt64 a, UInt64 b){
	
		return u64_divmod(a, b, false);
	}
	
	Int64 i64_div(Int64 a, Int64 b){
		
		Boolean a_neg = false;
		Boolean b_neg = false;
		
		if(i64_isNeg(a)){
			
			a_neg = true;
			a = u64_sub(u64_zero(), a);
		}
	
		if(i64_isNeg(b)){
			
			b_neg = true;
			b = u64_sub(u64_zero(), b);
		}
	
		a = u64_divmod(a, b, true);
		if(a_neg != b_neg) a = u64_sub(u64_zero(), a);
		
		return a;
	}
	
	Int64 i64_mod(Int64 a, Int64 b){
		
		Boolean a_neg = false;
		Boolean b_neg = false;
		
		if(i64_isNeg(a)){
			
			a_neg = true;
			a = u64_sub(u64_zero(), a);
		}
	
		if(i64_isNeg(b)){
			
			b_neg = true;
			b = u64_sub(u64_zero(), b);
		}
	
		a = u64_divmod(a, b, false);
		if(a_neg) a = u64_sub(u64_zero(), a);
		
		return a;
	}

#endif
