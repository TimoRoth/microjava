#include "long64.h"
#include "double64.h"

#ifdef UJ_FTR_SUPPORT_DOUBLE

	#ifdef COMPILER_HAS_DOUBLE


		Double64 d64_fromHalves(UInt32 top, UInt32 bottom){

			Double64 a;
	
			((UInt32*)&a)[1] = top;
			((UInt32*)&a)[0] = bottom;
	
			return a;
		}

		Double64 d64_froml(Int64 l){

			Double64 a;
			Boolean neg = false;
	
			if(i64_isNeg(l)){

				neg = true;
				l = u64_sub(u64_zero(), l);
			}
			a = u64_get_hi(l);
			a *= (65536.0 * 65536.0);
			a += u64_64_to_32(l);
			if(neg) a = -a;
			return a;
		}

		Int64 d64_tol(Double64 d){

			if(d64_isnan(d)) return u64_zero();
			else if(d >= 9223372036854775807.0) return u64_from_halves(0x7FFFFFFF, 0xFFFFFFFF);
			else if(d <= -9223372036854775808.0) return u64_from_halves(0x80000000, 0x00000000);
			else{

				Int64 r;
				Int32 top;
				Boolean neg = false;

				if(d < 0){
					neg = true;
					d = -d;
				}
		
				top = d64_floor(d / 4294967296.0);
				d -= ((Double64)top) * 4294967296.0;
				r = u64_from_halves(top, d);

				if(neg) r = u64_sub(u64_zero(), r);
				return r;
			}
		}

		Int32 d64_toi(Double64 d){

			if(d64_isnan(d)) return 0;
			else if(d >= 2147483647.0) return 0x7FFFFFFF;
			else if(d <= -2147483648.0) return 0x80000000;
			else{

				Int32 r;
				Boolean neg = false;

				if(d < 0){
				        neg = true;
				        d = -d;
				}

				r = d;

				if(neg) r = -r;
				return r;
			}
		}

	#else

	
		static void d64_prvLLadd(UInt64 aT, UInt64 aB, UInt64 bT, UInt64 bB, UInt64* rT, UInt64* rB){
	
			*rB = u64_add(aB, bB);
			*rT = u64_add(aT, bT);
			if(i64_isNeg(u64_sub(*rB, aB))) *rT = u64_add32(*rT, 1);
		}
	
		static void d64_prvLLsub(UInt64 aT, UInt64 aB, UInt64 bT, UInt64 bB, UInt64* rT, UInt64* rB){
	
			bT = u64_xor(bT, u64_from_halves(0xFFFFFFFFUL, 0xFFFFFFFFUL));
			bB = u64_xor(bB, u64_from_halves(0xFFFFFFFFUL, 0xFFFFFFFFUL));
			bB = u64_add32(bB, 1);
			if(u64_isZero(bB)) bT = u64_add32(bT, 1);
	
			*rB = u64_add(aB, bB);
			*rT = u64_add(aT, bT);
			if(i64_isNeg(u64_sub(*rB, aB))) *rT = u64_add32(*rT, 1);
		}
	
		static void d64_prvLLshl(UInt64 aT, UInt64 aB, UInt16 bits, UInt64* rT, UInt64* rB){
	
			if(bits >= 128){
			
				*rT = u64_zero();
				*rB = u64_zero();
			}
			if(bits >= 64){
		
				bits -= 64;
				aT = aB;
				aB = u64_zero();	
			}
	
			*rT = u64_orr(u64_shl(aT, bits), u64_shr(aB, 64 - bits));
			*rB = u64_shl(aB, bits);
		}
	
		static void d64_prvLLshr(UInt64 aT, UInt64 aB, UInt16 bits, UInt64* rT, UInt64* rB){
	
			if(bits >= 128){
			
				*rT = u64_zero();
				*rB = u64_zero();
			}
			if(bits >= 64){
		
				bits -= 64;
				aB = aT;
				aT = u64_zero();	
			}
	
			*rB = u64_orr(u64_shr(aB, bits), u64_shl(aT, 64 - bits));
			*rT = u64_shr(aT, bits);
		}
	
		static void d64_prvLLmul(UInt64 a, UInt64 b, UInt64* resultTopP, UInt64* resultBottomP){	//64x64 -> 128 multiplication
	
			d64_prvLLadd(u64_zero(), u64_umul3232(u64_64_to_32(a), u64_get_hi(b)), u64_zero(), u64_umul3232(u64_64_to_32(b), u64_get_hi(a)), resultTopP, resultBottomP);
		
			d64_prvLLshl(*resultTopP, *resultBottomP, 32, resultTopP, resultBottomP);
			d64_prvLLadd(u64_umul3232(u64_get_hi(a), u64_get_hi(b)), u64_umul3232(u64_64_to_32(a), u64_64_to_32(b)), *resultTopP, *resultBottomP, resultTopP, resultBottomP);
		}
	
		static void d64_prvLLdiv(UInt64 aT, UInt64 aB, UInt64 bT, UInt64 bB, UInt64* rTP, UInt64* rBP){		//128/128 -> 128 division
	
			UInt64 rT = u64_zero();
			UInt64 rB = u64_zero();
			UInt64 pT = u64_zero();
			UInt64 pB = u64_32_to_64(1);
			UInt64 tT, tB;
		
			while(1){
			
				if(i64_isNeg(bT)) break;			//divisor top bit set -> we cannot shift it left
				d64_prvLLsub(aT, aB, bT, bB, &tT, &tB);
				if(i64_isNeg(tT)) break;			//a < b
				d64_prvLLshl(bT, bB, 1, &bT, &bB);
				d64_prvLLshl(pT, pB, 1, &pT, &pB);
			}
		
			while(!u64_isZero(pT) || !u64_isZero(pB)){
		
				d64_prvLLsub(bT, bB, aT, aB, &tT, &tB);
				if(i64_isNeg(tT)){				//b < a
			
					d64_prvLLsub(aT, aB, bT, bB, &aT, &aB);
					rT = u64_orr(rT, pT);
					rB = u64_orr(rB, pB);
				}
				d64_prvLLshr(bT, bB, 1, &bT, &bB);
				d64_prvLLshr(pT, pB, 1, &pT, &pB);
			}
		
			*rTP = rT;
			*rBP = rB;
		}
	
		static UInt8 d64_prvLLclz(UInt64 aT, UInt64 aB){
	
			UInt32 v;
			UInt8 ret;
		
			if((v = u64_get_hi(aT)) != 0){
			
				ret = 0;
			}
			else if((v = u64_64_to_32(aT)) != 0){
			
				ret = 32;
			}
			else if((v = u64_get_hi(aB)) != 0){
			
				ret = 64;
			}
			else if((v = u64_64_to_32(aB)) != 0){
			
				ret = 96;
			}
			else return 128;
			while(!(v & 0x80000000UL)){
			
				ret++;
				v <<= 1UL;
			}
		
			return ret;
		}







		static Int16 d64_prvGetExp(UInt32 top, Boolean raw){

			Int16 ret = (top >> 20UL) & 0x7FF;
	
			if(!raw){
				if(!ret) ret = 1;
				ret -= 0x3FF;
			}
	
			return ret;
		}

		static Int64 d64_prvGetMant(Double64 a){

			UInt32 top = a.top & 0x000FFFFFUL;
			Int64 v;
	
			if(d64_prvGetExp(a.top, true)) top |= 0x00100000UL;
	
			v = u64_from_halves(top, a.bottom);
			if(a.top & 0x80000000UL) v = u64_sub(u64_zero(), v);
	
			return v;
		}

		Double64 d64_neg(Double64 a){

			a.top ^= 0x80000000UL;
	
			return a;
		}

		Boolean d64_isNeg(Double64 a){

			return (a.top & 0x80000000UL) != 0;
		}

		Boolean d64_isnan(Double64 a){

			return (((a.top >> 20UL) & 0x7FF) == 0x7FF) && !a.bottom && !(a.top & 0x000FFFFFUL);
		}

		Boolean d64_isinf(Double64 a){

			return (((a.top >> 20UL) & 0x7FF) == 0x7FF) && (a.bottom || (a.top & 0x000FFFFFUL));
		}

		Boolean d64_isEq(Double64 a, Double64 b){

			if(d64_isnan(a) || d64_isnan(b)) return false;
			if(d64_isZero(a) && d64_isZero(b)) return true;
			return a.top == b.top && a.bottom == b.bottom;
		}

		Boolean d64_isGt(Double64 a, Double64 b){

			if(d64_isnan(a) || d64_isnan(b)) return false;
	
			if((a.top & 0x80000000UL) && !(b.top & 0x80000000UL)) return false;	//a is neg b is pos
			if(!(a.top & 0x80000000UL) && (b.top & 0x80000000UL)) return true;	//a is pos b is neg
	
			return i64_isNeg(u64_sub(d64_prvGetMant(b), d64_prvGetMant(a)));
		}

		Boolean d64_isZero(Double64 a){

			return !a.bottom && !(a.top & 0x7FFFFFFFUL);	
		}
	
	
		Double64 d64_add(Double64 a, Double64 b){

			if(d64_isnan(a) || d64_isnan(b)) return d64_nan();
			else if(d64_isinf(a) && d64_isinf(b)) return (a.top & 0x80000000UL) == (b.top & 0x80000000UL) ? a : d64_nan();
			else if(d64_isinf(a)) return a;
			else if(d64_isinf(b)) return b;
			else if(d64_isZero(a) && d64_isZero(b) && d64_isNeg(a) && d64_isNeg(b)) return a;	//negatize zeroes add up to negative zero
			else if(d64_isZero(a) &&  d64_isZero(b)) return d64_zero();
			else{		//now the real thing begins - we have two non-NaN, non-INF, non-zero values
	
				Int64 am = d64_prvGetMant(a);
				Int64 bm = d64_prvGetMant(b);
				Int16 ae = d64_prvGetExp(a.top, false);
				Int16 be = d64_prvGetExp(b.top, false);
	
				//convert to same exponent
				if(ae > be){
			
					bm = u64_ashr(bm, ae - be);
				}
				else if(ae < be){
		
					am = u64_ashr(am, be - ae);
					ae = be;	
				}
		
				//add
		
				am = u64_add(am, bm);
		
				//start generating result
				a.top = 0;
				a.bottom = 0;
				if(i64_isNeg(am)){
			
					a.top = 0x80000000UL;
					am = u64_sub(u64_zero(), am);	
				}
		
				//handle zero
				if(u64_isZero(am)) return d64_zero();
		
				//normalize
				while(u64_get_hi(am) & 0xFFE00000UL){
		
					ae++;
					am = u64_shr(am, 1);	
				}
				while(!(u64_get_hi(am) & 0x00100000UL)){
		
					ae--;
					am = u64_shl(am, 1);	
				}
				if(ae > 0x3FF) return d64_inf(!!(a.top & 0x80000000UL));
				else{
		
					while(ae <= -0x3FF){
			
						ae++;
						am = u64_shr(am, 1);	
					}
			
					ae += 0x3FF;
					a.top |= ((UInt32)(ae) << 20UL);
					a.top |= u64_get_hi(am) & 0x000FFFFFUL;
					a.bottom = u64_64_to_32(am);
					
					return a;
				}
			}
		}

		static Double64 d64_prvLLtoDbl(UInt64 rT, UInt64 rB, Int16 e){

			Double64 a;
			Int16 z;
	
			//we want a CLZ of 75
			z = d64_prvLLclz(rT, rB);
			if(z > 75){
	
				e -= z - 75;
				rB = u64_shl(rB, z - 75);
			}
			else if(z < 75){
	
				e += 75 - z;
				d64_prvLLshr(rT, rB, 75 - z, &rT, &rB);
			}
			if(e > 0x3FF) a = d64_inf(true);
			else{
	
				if(e < -0x3FE){
		
					rB = u64_shr(rB, -0x3FE - e);
					e = -0x3FF;
				}
		
				if(u64_isZero(rB)) a = d64_zero();
				else{
		
					e += 0x3FF;
					if(!(u64_get_hi(rB) & 0x100000UL) && (e == 1)) e = 0;
					a.top = ((UInt32)e) << 20UL;
					a.top |= u64_get_hi(rB) &~ 0x100000UL;
					a.bottom = u64_64_to_32(rB);
				}
			}
	
			return a;	
		}

		Double64 d64_mul(Double64 a, Double64 b){

			if(d64_isnan(a) || d64_isnan(b)) return a;
			else{
		
				Boolean neg = false;
		
				if(a.top & 0x80000000UL){
			
					neg = !neg;
					a.top &=~ 0x80000000UL;
				}
				if(b.top & 0x80000000UL){
			
					neg = !neg;
					b.top &=~ 0x80000000UL;
				}
				if((d64_isinf(a) && d64_isZero(b)) || (d64_isinf(b) && d64_isZero(a))) a = d64_nan();
				else if((d64_isinf(a) && !d64_isZero(b)) || (d64_isinf(b) && !d64_isZero(a))) a = d64_inf(true);
				else if(d64_isZero(a) || d64_isZero(b)) a = d64_zero();
				else{
		
					UInt64 rT, rB;
					Int16 e = d64_prvGetExp(a.top, false) + d64_prvGetExp(b.top, false) - 52;
	
					d64_prvLLmul(d64_prvGetMant(a), d64_prvGetMant(b), &rT, &rB);
			
					a = d64_prvLLtoDbl(rT, rB, e);
				}
				if(neg) a = d64_neg(a);
				return a;
			}
		}

		Double64 d64_div(Double64 a, Double64 b){

			if(d64_isnan(a) || d64_isnan(b)) return a;
			else{
		
				Boolean neg = false;
		
				if(a.top & 0x80000000UL){
			
					neg = !neg;
					a.top &=~ 0x80000000UL;
				}
				if(b.top & 0x80000000UL){
			
					neg = !neg;
					b.top &=~ 0x80000000UL;
				}
				if(d64_isZero(b)){
		
					if(d64_isZero(a)) a = d64_nan();
					else a = d64_inf(true);
				}
				else if(d64_isinf(a)){
		
					if(d64_isinf(b)) a = d64_nan();
					else a = d64_inf(true);
				}
				else if(d64_isZero(a) || d64_isinf(b)) a = d64_zero();
				else{
		
					Int16 e = d64_prvGetExp(a.top, false) - d64_prvGetExp(b.top, false) - 12;
					UInt64 rT, rB;
			
					d64_prvLLdiv(d64_prvGetMant(a), u64_zero(), u64_zero(), d64_prvGetMant(b), &rT, &rB);
			
					a = d64_prvLLtoDbl(rT, rB, e);
				}
				if(neg) a = d64_neg(a);
				return a;
			}
		}

		UInt32 d64_getTopWord(Double64 d){

			return d.top;	
		}

		UInt32 d64_getBottomWord(Double64 d){

			return d.bottom;	
		}

		Double64 d64_floor(Double64 a){

			if(!d64_isinf(a)){

				Boolean neg = d64_isNeg(a);
				Int16 exp = d64_prvGetExp(a.top, false);
				if(exp < 0) return d64_isNeg(a) ? d64_zero(): d64_neg(d64_zero());
		
				if(exp < 52){
		
					exp = 52 - exp;
					if(exp >= 32){
						a.bottom = 0;
						a.top &= (0xFFFFFFFFUL << (UInt32)exp);
					}
					else{
						a.bottom &= (0xFFFFFFFFUL << (UInt32)exp);
					}
				}
		
				if(neg) a = d64_sub(a, d64_fromi(1));
			}
			return a;
		}

		Double64 d64_froml(Int64 l){
	
			Double64 a = {0, 0};
			Int16 e = 63;

			if(i64_isNeg(l)){

				a.top = 0x80000000UL;
				l = u64_sub(u64_zero(), l);
			}
			if(!u64_isZero(l)){

				while(!(u64_get_hi(l) & 0x80000000UL)){
					e--;
					l = u64_shl(l, 1);
				}
				l = u64_shl(l, 1);

				e += 0x3FF;
				a.top |= ((UInt32)e) << 20UL;
				a.top |= u64_get_hi(l) >> 12UL;
				a.bottom = (u64_get_hi(l) << 20UL) | (u64_64_to_32(l) >> 12UL);
			}

			return a;
		}

		UjFloat d64_tof(Double64 a){

			if(d64_isnan(a)) return 0.0/0.0;
			else{
	
				Int16 exp = d64_prvGetExp(a.top, false);
				UjFloat ret;
				Boolean neg = false;
		
				if(exp > 126) ret = 1.0/0.0;
				else{
		
					Int64 m = d64_prvGetMant(a);
			
					if(d64_isNeg(a)){
					
						neg = true;
						a = d64_neg(a);
					}
			
					exp -= 29;
			
					if(!u64_isZero(m)){
						while(u64_isZero(u64_and(m, u64_from_halves(0xFFFFFFFFUL, 0xFF800000UL)))){
				
							m = u64_shl(m, 1);
							exp--;
						}
						while(exp < -126 || !u64_isZero(u64_and(m, u64_from_halves(0xFFFFFFFFUL, 0xFF000000UL)))){
				
							m = u64_shr(m, 1);
							exp++;
						}
					}
					if(u64_isZero(m)) ret = 0.0;
					else if(exp > 126) ret = 1.0/0.0;
					else{
			
						if(u64_64_to_32(m) & 0x00800000UL){
				
							exp += 0x7F;
						}
						else{
				
							exp = 0;
						}
						*(UInt32*)&ret = u64_64_to_32(m) &~ 0x00800000UL;
						*(UInt32*)&ret |= (((UInt32)exp) << 23UL) & 0x7F800000UL;
					}
				}
		
				if(neg) ret = -ret;
				return ret;
			}
		}

		Int32 d64_toi(Double64 a){
	
			if(d64_isnan(a)) return 0;
			else{
				Int16 exp = d64_prvGetExp(a.top, false);
				if(exp < 0) return 0;
				else{
		
					UInt32 ret;
					Boolean neg = false;
			
					if(d64_isNeg(a)){
				
						neg = true;
						a = d64_neg(a);
					}
			
					if(exp >= 31){
			
						ret = neg ? 0x80000000UL : 0x7FFFFFFFUL;	
					}
					else{
				
						ret = (((a.top & 0x000FFFFFUL) | 0x00100000UL) << 11UL) | (a.bottom >> 21UL);
						exp -= 31;
						while(exp > 0){
					
							exp--;
							ret <<= 1UL;
						}
						while(exp < 0){
					
							exp++;
							ret >>= 1UL;
						}
				
						if(neg) ret = -ret;
					}
					return ret;
				}
			}
		}

		Int64 d64_tol(Double64 a){
	
			if(d64_isnan(a)) return u64_zero();
			else{
				Int16 exp = d64_prvGetExp(a.top, false);
				if(exp < 0) return u64_zero();
				else{
		
					UInt64 ret;
					Boolean neg = false;
			
					if(d64_isNeg(a)){
				
						neg = true;
						a = d64_neg(a);
					}
			
					if(exp >= 63){
			
						ret = neg ? u64_from_halves(0x80000000UL, 0x00000000UL) : u64_from_halves(0x7FFFFFFFUL, 0xFFFFFFFFUL);	
					}
					else{
				
						ret = u64_from_halves((a.top & 0x000FFFFFUL) | 0x00100000UL, a.bottom);
						exp -= 52;
						while(exp > 0){
					
							exp--;
							ret = u64_shl(ret, 1);
						}
						while(exp < 0){
					
							exp++;
							ret = u64_shr(ret, 1);
						}
				
						if(neg) ret = u64_sub(u64_zero(), ret);
					}
					return ret;
				}
			}
		}

		Double64 d64_fromf(UjFloat fv){
	
			Double64 d;
			UInt32 f = *(UInt32*)&fv;
			Int16 exp;
	
			exp = (Int16)(UInt8)(f >> 23UL);
			d.top = f & 0x80000000UL;
	
			f &= 0x007FFFFFUL;
	
			if(exp == 0){		//special case 1
		
				if(f){
			
					exp = -126;
					while(!(f & 0x00800000UL)){
				
						f <<= 1UL;
						exp--;
					}
					f &=~ 0x00800000UL;
				}
				else{	//zero
		
					d.bottom = 0;
					return d;
				}
			}
			else{			//normal case (handles inf and nan too)
	
				if(exp == 0xFF){
			
					exp = 0x400;
				}
				else{
					exp = (Int8)(exp - 0x7F);
				}
		
			}
			exp += 0x3FF;
			d.top |= ((UInt32)(exp & 0x7FF)) << 20UL;
			d.top |= f >> 3UL;
			d.bottom = f << 29UL;
	
			return d;
		}

		Double64 d64_fromi(Int32 i){

			Double64 a = {0, 0};
			Int16 e = 31;

			if(i < 0){

				a.top = 0x80000000UL;
				i = -i;
			}
			if(i){

				while(!(i & 0x80000000UL)){
					e--;
					i <<= 1UL;
				}
				i <<= 1UL;

				e += 0x3FF;
				a.top |= ((UInt32)e) << 20UL;
				a.top |= ((UInt32)i) >> 12UL;
				a.bottom = i << 20;
			}

			return a;
		}

		Double64 d64_fromHalves(UInt32 top, UInt32 bottom){

			Double64 a;
	
			a.top = top;
			a.bottom = bottom;
	
			return a;
		}

		Double64 d64_nan(void){
	
			Double64 a = {0xFFFFFFFFUL, 0xFFFFFFFF};
	
			return a;
		}

		Double64 d64_inf(Boolean pos){

			Double64 a = {0x7FF00000UL, 0x00000000UL};
	
			if(!pos) a.top |= 0x80000000UL;
	
			return a;
		}

		Double64 d64_zero(void){

			Double64 a = {0x00000000UL, 0x00000000UL};
	
			return a;	
		}

	#endif

#endif
