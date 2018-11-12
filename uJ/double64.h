#ifndef _DOUBLE_H_
#define _DOUBLE_H_

#include "common.h"
#include "long64.h"

#ifdef UJ_FTR_SUPPORT_DOUBLE

#ifdef COMPILER_HAS_DOUBLE

#include <math.h>

typedef double Double64;

#define d64_neg(a) (-(a))
#define d64_isNeg(a) ((a) < 0.0)
#define d64_isnan(a) ((a) != (a))
#define d64_isinf(a) ((a) != 0 && ((a)*2.0 == (a)))
#define d64_isEq(a, b) ((a) == (b))
#define d64_isGt(a, b) ((a) > (b))
#define d64_isLt(a, b) ((a) < (b))
#define d64_isZero(a) ((a) == 0.0)
#define d64_add(a, b) ((a) + (b))
#define d64_sub(a, b) ((a) - (b))
#define d64_div(a, b) ((a) / (b))
#define d64_mul(a, b) ((a) * (b))
#define d64_nan() ((Double64)(0.0 / 0.0))
#define d64_inf(pos) ((Double64)((pos ? 1.0 : -1.0) / 0.0))
#define d64_zero() ((Double64)(0.0))
#define d64_fromi(i) ((Double64)(i))
Int64 d64_toLong(Double64 a);
#define d64_getTopWord(d) (((uint32_t *)(&d))[1])
#define d64_getBottomWord(d) (((uint32_t *)(&d))[0])
Double64 d64_fromHalves(uint32_t top, uint32_t bottom);
#define d64_floor(a) floor(a)
#define d64_tof(d) ((float)(d))
#define d64_fromf(f) ((Double64)(f))
Double64 d64_froml(Int64 l);
Int64 d64_tol(Double64 d);
int32_t d64_toi(Double64 d);

#else

typedef struct {
    uint32_t bottom, top;
} Double64;

Double64 d64_neg(Double64 a);
bool d64_isNeg(Double64 a);
bool d64_isnan(Double64 a);
bool d64_isinf(Double64 a);
bool d64_isEq(Double64 a, Double64 b);
bool d64_isGt(Double64 a, Double64 b);
#define d64_isLt(a, b) d64_isGt(b, a)
bool d64_isZero(Double64 a);
Double64 d64_add(Double64 a, Double64 b);
#define d64_sub(a, b) d64_add(a, d64_neg(b))
Double64 d64_div(Double64 a, Double64 b);
Double64 d64_mul(Double64 a, Double64 b);
Double64 d64_nan(void);
Double64 d64_inf(bool pos);
Double64 d64_zero(void);
Int64 d64_toLong(Double64 a);
Double64 d64_fromi(int32_t a);
uint32_t d64_getTopWord(Double64 d);
uint32_t d64_getBottomWord(Double64 d);
Double64 d64_fromHalves(uint32_t top, uint32_t bottom);
Double64 d64_floor(Double64 a);
float d64_tof(Double64 d);
Double64 d64_fromf(float f);
Double64 d64_froml(Int64 l);
Int64 d64_tol(Double64 d);
int32_t d64_toi(Double64 d);

#endif
#endif

#endif
