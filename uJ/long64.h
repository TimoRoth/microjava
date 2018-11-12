#ifndef _MATH_64_H_
#define _MATH_64_H_

#include "common.h"

#if defined(UJ_FTR_SUPPORT_LONG) || defined(UJ_FTR_SUPPORT_DOUBLE)

#ifdef COMPILER_SUPPORTS_LONG_LONG

typedef uint64_t UInt64;
typedef int64_t Int64;

#else

typedef struct {
    uint32_t hi, lo;
} UInt64;

typedef UInt64 Int64;

#endif

UInt64 u64_from_halves(uint32_t hi, uint32_t lo);
UInt64 u64_32_to_64(uint32_t v);
uint32_t u64_64_to_32(UInt64 v);
uint32_t u64_get_hi(UInt64 v);
UInt64 u64_add(UInt64 a, UInt64 b);
UInt64 u64_mul(UInt64 a, UInt64 b);
UInt64 u64_umul3232(uint32_t a, uint32_t b);
UInt64 u64_smul3232(int32_t a, int32_t b);
UInt64 u64_ashr(UInt64 a, uint16_t bits);
UInt64 u64_shr(UInt64 a, uint16_t bits);
UInt64 u64_shl(UInt64 a, uint16_t bits);
UInt64 u64_add32(UInt64 a, uint32_t b);
UInt64 i64_xtnd32(UInt64 a);
bool u64_isZero(UInt64 a);
bool i64_isNeg(Int64 a);
UInt64 u64_inc(UInt64 a);
UInt64 u64_zero(void);
UInt64 u64_sub(UInt64 a, UInt64 b);
UInt64 u64_and(UInt64 a, UInt64 b);
UInt64 u64_orr(UInt64 a, UInt64 b);
UInt64 u64_xor(UInt64 a, UInt64 b);
UInt64 u64_div(UInt64 a, UInt64 b);
UInt64 u64_mod(UInt64 a, UInt64 b);
Int64 i64_div(Int64 a, Int64 b);
Int64 i64_mod(Int64 a, Int64 b);

#endif

#endif
