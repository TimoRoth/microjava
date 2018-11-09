#include "long64.h"

#if defined(UJ_FTR_SUPPORT_LONG) || defined(UJ_FTR_SUPPORT_DOUBLE)
	
	UInt64 u64_from_halves(UInt32 hi, UInt32 lo)
	{
		return (((UInt64)hi) << 32ULL) | ((UInt64)lo);
	}

	UInt64 u64_32_to_64(UInt32 v)
	{
		return (UInt64)v;
	}

	UInt32 u64_64_to_32(UInt64 v)
	{
		return (UInt32)v;
	}

	UInt32 u64_get_hi(UInt64 v)
	{
		return (UInt32)(v >> 32ULL);
	}

	UInt64 u64_add(UInt64 a, UInt64 b)
	{
		return a + b;
	}

	UInt64 u64_add32(UInt64 a, UInt32 b)
	{
		return a + (UInt64)b;
	}

	UInt64 u64_mul(UInt64 a, UInt64 b)
	{
		return a * b;
	}

	UInt64 u64_umul3232(UInt32 a, UInt32 b)	//sad but true: gcc has no u32xu32->64 multiply
	{
		return ((UInt64)a) * ((UInt64)b);
	}

	UInt64 u64_smul3232(Int32 a, Int32 b)	//sad but true: gcc has no s32xs32->64 multiply
	{
		return ((signed long long)a) * ((signed long long)b);
	}

	UInt64 u64_ashr(UInt64 a, UInt16 bits)
	{
		return bits >= 64U ? (((Int64)a) < 0 ? -1LL:1LL) : (((Int64)a) >> (Int64)bits);
	}

	UInt64 u64_shr(UInt64 a, UInt16 bits)
	{
		return bits >= 64U ? 0 : (a >> (UInt64)bits);
	}

	UInt64 u64_shl(UInt64 a, UInt16 bits)
	{
		return bits >= 64U ? 0 : (a << (UInt64)bits);
	}

	UInt64 i64_xtnd32(UInt64 a)
	{
		if(a & 0x80000000UL) a |= (((UInt64)-1) << 32ULL); return a;
	}

	Boolean u64_isZero(UInt64 a)
	{
		return a == 0;
	}

	Boolean i64_isNeg(Int64 a)
	{
		return ((Int64)a) < 0;
	}

	UInt64 u64_inc(UInt64 a)
	{
		return a + 1ULL;
	}

	UInt64 u64_and(UInt64 a, UInt64 b)
	{
		return a & b;
	}

	UInt64 u64_orr(UInt64 a, UInt64 b)
	{
		return a | b;
	}

	UInt64 u64_xor(UInt64 a, UInt64 b)
	{
		return a ^ b;
	}

	UInt64 u64_zero(void)
	{
		return 0;
	}

	UInt64 u64_sub(UInt64 a, UInt64 b)
	{
		return a - b;
	}

	UInt64 u64_div(UInt64 a, UInt64 b)
	{
		return a / b;
	}

	Int64 i64_div(Int64 a, Int64 b)
	{
		return a / b;
	}

	UInt64 u64_mod(UInt64 a, UInt64 b)
	{
		return a % b;
	}

	Int64 i64_mod(Int64 a, Int64 b)
	{
		return a % b;
	}

#endif

