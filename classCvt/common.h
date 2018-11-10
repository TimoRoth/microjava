#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

typedef int8_t Int8;
typedef int16_t Int16;
typedef int32_t Int32;
typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;
typedef unsigned char Boolean;


typedef unsigned long UInt24;

#define true	1
#define false	0
#ifndef NULL
	#define NULL	0
#endif

void* natAlloc(UInt16 sz);	//alloc/free on native heap. never returns null
void natFree(void* ptr);
void natMemZero(void* ptr, UInt16 len);

void err(const char* str);

#define _UNUSED_	__attribute__((unused))
#define DEBUG		0

#endif
