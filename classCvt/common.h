#ifndef _COMMON_H_
#define _COMMON_H_


typedef signed char Int8;
typedef signed short Int16;
typedef signed long Int32;
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
typedef unsigned long long UInt64;
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
