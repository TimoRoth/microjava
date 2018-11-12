#ifndef _UJ_HEAP_H_
#define _UJ_HEAP_H_

#include "common.h"

// init & debug
#ifndef UJ_HEAP_SZ
#define UJ_HEAP_SZ 1024
#endif

#define UJ_HEAP_MAX_HANDLES (UJ_HEAP_SZ / 8)

#if UJ_HEAP_MAX_HANDLES < (1UL << 8)
#define HANDLE uint8_t
#define HANDLE_SZ HEAP_ALIGN
#elif UJ_HEAP_MAX_HANDLES < (1UL << 16)
#define HANDLE uint16_t
#define HANDLE_SZ (2 > HEAP_ALIGN ? 2 : HEAP_ALIGN)
#elif UJ_HEAP_MAX_HANDLES < (1UL << 32)
#define HANDLE uint32_t
#define HANDLE_SZ (4 > HEAP_ALIGN ? 4 : HEAP_ALIGN)
#else
#error "too many heap handles possible!"
#endif

void ujHeapInit(void);
void ujHeapDebug(void);

HANDLE ujHeapHandleNew(uint16_t sz);
void ujHeapHandleFree(HANDLE handle);
void *ujHeapAllocNonmovable(uint16_t sz);

void *ujHeapHandleLock(HANDLE handle);
void ujHeapHandleRelease(HANDLE handle);
void *ujHeapHandleIsLocked(HANDLE handle); // return pointer if already locked, else NULL

void ujHeapUnmarkAll(void);
void ujHeapFreeUnmarked(void);
HANDLE ujHeapFirstMarked(uint8_t markVal); // get first handle with a given mark value
void ujHeapMark(HANDLE handle, uint8_t mark); // will only increase the mark value
uint8_t ujHeapGetMark(HANDLE handle);

#endif
