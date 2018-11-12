#ifndef _COMMON_H_
#define _COMMON_H_

#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Special type some AVR chips actually use
typedef uint32_t UInt24;

void* natAlloc(uint16_t sz);	//alloc/free on native heap. never returns null
void natFree(void* ptr);
void natMemZero(void* ptr, uint16_t len);

void err(const char* str);

#define _UNUSED_	__attribute__((unused))
#define DEBUG		0

#endif
