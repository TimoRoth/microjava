#ifndef _DOUBLE_H_
#define _DOUBLE_H_

#include "common.h"
#include "long64.h"


#ifdef UJ_FTR_SUPPORT_DOUBLE


	#ifdef COMPILER_HAS_DOUBLE

		#include <math.h>

		typedef double Double64;

		#define d64_neg(a)		(-(a))
		#define d64_isNeg(a)		((a) < 0.0)
		#define d64_isnan(a)		((a) != (a))
		#define d64_isinf(a)		((a) != 0 && ((a) * 2.0 == (a)))
		#define d64_isEq(a, b)		((a) == (b))
		#define d64_isGt(a, b)		((a) > (b))
		#define d64_isLt(a, b)		((a) < (b))
		#define d64_isZero(a)		((a) == 0.0)
		#define d64_add(a, b)		((a) + (b))
		#define d64_sub(a, b)		((a) - (b))
		#define d64_div(a, b)		((a) / (b))
		#define d64_mul(a, b)		((a) * (b))
		#define d64_nan()		((Double64)(0.0 / 0.0))
		#define d64_inf(pos)		((Double64)((pos ? 1.0 : -1.0) / 0.0))
		#define d64_zero()		((Double64)(0.0))
		#define d64_fromi(i)		((Double64)(i))
		Int64 d64_toLong(Double64 a);
		#define d64_getTopWord(d)	(((UInt32*)(&d))[1])
		#define d64_getBottomWord(d)	(((UInt32*)(&d))[0])
		Double64 d64_fromHalves(UInt32 top, UInt32 bottom);
		#define d64_floor(a)		floor(a)
		#define d64_tof(d)		((UjFloat)(d))
		#define d64_fromf(f)		((Double64)(f))
		Double64 d64_froml(Int64 l);
		Int64 d64_tol(Double64 d);
		Int32 d64_toi(Double64 d);
	

	#else

		typedef struct{
		
			UInt32 bottom, top;
		}Double64;
	
		Double64 d64_neg(Double64 a);
		Boolean d64_isNeg(Double64 a);
		Boolean d64_isnan(Double64 a);
		Boolean d64_isinf(Double64 a);
		Boolean d64_isEq(Double64 a, Double64 b);
		Boolean d64_isGt(Double64 a, Double64 b);
		#define d64_isLt(a, b)	d64_isGt(b, a)
		Boolean d64_isZero(Double64 a);
		Double64 d64_add(Double64 a, Double64 b);
		#define d64_sub(a, b) d64_add(a, d64_neg(b))
		Double64 d64_div(Double64 a, Double64 b);
		Double64 d64_mul(Double64 a, Double64 b);
		Double64 d64_nan(void);
		Double64 d64_inf(Boolean pos);
		Double64 d64_zero(void);
		Int64 d64_toLong(Double64 a);
		Double64 d64_fromi(Int32 a);
		UInt32 d64_getTopWord(Double64 d);
		UInt32 d64_getBottomWord(Double64 d);
		Double64 d64_fromHalves(UInt32 top, UInt32 bottom);
		Double64 d64_floor(Double64 a);
		UjFloat d64_tof(Double64 d);
		Double64 d64_fromf(UjFloat f);
		Double64 d64_froml(Int64 l);
		Int64 d64_tol(Double64 d);
		Int32 d64_toi(Double64 d);
	

	#endif
#endif



#endif

