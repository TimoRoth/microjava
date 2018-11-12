#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdalign.h>

// Special type. Some AVR chips actually have a 24bit type.
typedef uint32_t UInt24;

#ifndef NAN
#define NAN (0.0f / 0.0f)
#endif

void err(const char *str);

#define TL(...) // fprintf(stderr, "**UL** " __VA_ARGS__)

#define DEBUG 1

#define _UNUSED_ __attribute__((unused))
#define _INLINE_ __attribute__((always_inline)) inline

#define HEAP_ALIGN alignof(uintptr_t)
#define _HEAP_ATTRS_

#endif
