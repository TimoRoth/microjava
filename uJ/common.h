#ifndef _COMMON_H_
#define _COMMON_H_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdalign.h>

typedef int8_t Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef unsigned char Boolean;
typedef float UjFloat;
typedef uint32_t UInt24;

#define true 1
#define false 0
#ifndef NULL
#define NULL 0
#endif
#ifndef NAN
#define NAN (0.0f / 0.0f)
#endif

void err(const char *str);

#define TL(...) fprintf(stderr, "**UL** " __VA_ARGS__)

#define DEBUG 1

#define _UNUSED_ __attribute__((unused))
#define _INLINE_ __attribute__((always_inline)) inline

#define HEAP_ALIGN alignof(uintptr_t)
#define _HEAP_ATTRS_
#define HEAP_SZ 4096

#ifdef UJ_LOG
void ujLog(const char *fmtStr, ...);
#else
#define ujLog(...)
#endif

#endif
