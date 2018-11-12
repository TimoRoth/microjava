#ifndef _UJ_H_
#define _UJ_H_

#include "common.h"
#include "ujHeap.h"

// callback types
uint8_t ujReadClassByte(void *userData, uint32_t offset);

#ifdef UJ_LOG
void ujLog(const char *fmtStr, ...);
#else
#define ujLog(...)
#endif

// api

#define UJ_DEFAULT_STACK_SIZE 256

// some of these will throw an exception at a higher level, perhaps?
#define UJ_ERR_NONE  0 // not an error (may mean true)
#define UJ_ERR_FALSE 1 // also not an error - just means false

#define UJ_ERR_INVALID_OPCODE 2     // InternalError [?]
#define UJ_ERR_METHOD_NONEXISTENT 3 // UnknownError [?]		ujThreadGoto to an invalid place
#define UJ_ERR_DEPENDENCY_MISSING 4 // NoClassDefFoundError ujClassLoad fails because needed
                                    //                      superlcass is missing or otherwise class not found

#define UJ_ERR_STACK_SPACE 16           // StackOverflowError
#define UJ_ERR_METHOD_FLAGS_MISMATCH 17 // UnknownError [?]
#define UJ_ERR_DIV_BY_ZERO 18           // ArithmeticException
#define UJ_ERR_INVALID_CAST 19          // ClassCastException
#define UJ_ERR_OUT_OF_MEMORY 20         // OutOfMemoryError
#define UJ_ERR_ARRAY_INDEX_OOB 21       // IndexOutOfBoundsException	index out of bounds
#define UJ_ERR_FIELD_NOT_FOUND 22       // UnknownError [?]		such a field doe snot exist (by name)
#define UJ_ERR_NULL_POINTER 23          // NullPointerException
#define UJ_ERR_MON_STATE_ERR 24         // IllegalMonitorStateException	monitor state exception
#define UJ_ERR_NEG_ARR_SZ 25            // NegativeArraySizeException

#define UJ_ERR_RETRY_LATER 50 // not an error, just retry later

#define UJ_ERR_USER_EXCEPTION 99 // in case of exceptions disabled... or uncaught
#define UJ_ERR_INTERNAL 100      // UnknownError [?] bad internal error

#define UJ_THREAD_QUANTUM 10 // instrs

typedef struct UjClass UjClass;
typedef struct UjThread UjThread;
typedef struct UjInstance UjInstance;

uint8_t ujInit(UjClass **objectClsP);

uint8_t ujLoadClass(void *readD, UjClass **clsP);

uint8_t ujInitAllClasses(void);

HANDLE ujThreadCreate(uint16_t stackSz /*zero for default*/);
uint32_t ujThreadDbgGetPc(HANDLE threadH);
uint8_t ujThreadGoto(HANDLE threadH, UjClass *cls, const char *methodNamePtr, const char *methodTypePtr); // static call only (used to call main or some such thing)
bool ujCanRun(void);
uint8_t ujInstr(void); // return UJ_ERR_*
uint8_t ujThreadDestroy(HANDLE threadH);
uint8_t ujGC(void); // called by heap manager
uint32_t ujGetNumInstrs(void);

// some flags
#define JAVA_ACC_PUBLIC       0x0001 // Declared public; may be accessed from outside its package.
#define JAVA_ACC_PRIVATE      0x0002 // Declared private; accessible only within the defining class.
#define JAVA_ACC_PROTECTED    0x0004 // Declared protected; may be accessed within subclasses.
#define JAVA_ACC_STATIC       0x0008 // Declared static.
#define JAVA_ACC_FINAL        0x0010 // Declared final; may not be overridden.
#define JAVA_ACC_SYNCHRONIZED 0x0020 // Declared synchronized; invocation is wrapped in a monitor lock.
#define JAVA_ACC_NATIVE       0x0100 // Declared native; implemented in a language other than Java.
#define JAVA_ACC_INTERFACE    0x0200 // Is an interface, not a class.
#define JAVA_ACC_ABSTRACT     0x0400 // Declared abstract; no implementation is provided.
#define JAVA_ACC_STRICT       0x0800 // Declared strictfp; floating-point mode is FP-strict

uintptr_t ujThreadPop(UjThread *t);
bool ujThreadPush(UjThread *t, uintptr_t v, bool isRef);
uint32_t ujArrayLen(uint32_t arr);
uint32_t ujArrayGetByte(uint32_t arr, uint32_t idx);
uint32_t ujArrayGetShort(uint32_t arr, uint32_t idx);
uint32_t ujArrayGetInt(uint32_t arr, uint32_t idx);
void *ujArrayRawAccessStart(uint32_t arr);
void ujArrayRawAccessFinish(uint32_t arr);
uint16_t ujStringGetBytes(HANDLE handle, uint8_t *buf, uint32_t bufsize);
uint8_t ujStringFromBytes(HANDLE *handleP, uint8_t *str, uint16_t len); // if len is 0 str is assumed to be 0 terminated

// native classes

typedef uint8_t (*ujNativeMethodF)(UjThread *, UjClass *);
typedef void (*ujNativeGcInstF)(UjClass *cls, UjInstance *inst);
typedef void (*ujNativeGcClsF)(UjClass *cls);

typedef struct {
    const char *name;
    const char *type;
    ujNativeMethodF func;
    uint16_t flags;
} UjNativeMethod;

typedef struct {
    const char *clsName;
    uint16_t clsDatSz;
    uint16_t instDatSz;

    ujNativeGcClsF gcClsF; // called once per gc to mark all used handles to level 1
    ujNativeGcInstF gcInstF; // called once per gc per object to mark all used handles to 1

    uint16_t numMethods;
    const UjNativeMethod *methods;
} UjNativeClass;

uint8_t ujRegisterNativeClass(const UjNativeClass *nCls /*references and must remain valid forever*/, UjClass *super, UjClass **clsP);

#endif
