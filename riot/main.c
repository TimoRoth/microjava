#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <uJ/uj.h>

#include "class_loader.h"

extern unsigned char ujc_Example[];

void ujLog(const char *fmtStr, ...)
{
    va_list va;

    va_start(va, fmtStr);
    vfprintf(stdout, fmtStr, va);
    fflush(stdout);
    va_end(va);
}

static uint8_t natRIOT_sayHello(UjThread* t, UjClass* cls)
{
    (void)t;
    (void)cls;

    printf("Hello From native method!\n");

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_printString(UjThread* t, UjClass* cls)
{
    (void)cls;

    //TODO: Need to peek at this
    HANDLE strHandle = (HANDLE)ujThreadPop(t);

    uint32_t sz = ujStringGetBytes(strHandle, NULL, 0);

    char buf[sz];
    sz = ujStringGetBytes(strHandle, (uint8_t*)buf, sz);

    printf("%s\n", buf);

    if (!ujThreadPush(t, sz, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_getSomeString(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE stringHandle;
    int res;

    res = ujStringFromBytes(&stringHandle, (uint8_t*)"String from natRIOT_getSomeString (äöü)", 0);
    if (res != UJ_ERR_NONE)
        return res;

    if (!ujThreadPush(t, stringHandle, true))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static const UjNativeMethod nativeCls_RIOT_methods[] = {
    {
        .name = "sayHello",
        .type = "()V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_sayHello,
    },
    {
        .name = "printString",
        .type = "(Ljava/lang/String;)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_printString,
    },
    {
        .name = "getSomeString",
        .type = "()Ljava/lang/String;",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_getSomeString,
    },
};

static const UjNativeClass nativeCls_RIOT =
{
    .clsName = "RIOT",

    .clsDatSz = 0,
    .instDatSz = 0,
    .gcClsF = NULL,
    .gcInstF = NULL,

    .numMethods = sizeof(nativeCls_RIOT_methods) / sizeof(nativeCls_RIOT_methods[0]),
    .methods = nativeCls_RIOT_methods,
};

int main(void)
{
    struct UjClass *objectClass = NULL;
    struct UjClass *mainClass = NULL;
    int res;

    res = ujInit(&objectClass);
    if (res != UJ_ERR_NONE)
    {
        printf("ujInit failed: %d\n", res);
        return -1;
    }

    res = ujRegisterNativeClass(&nativeCls_RIOT, objectClass, NULL);
    if (res != UJ_ERR_NONE)
    {
        printf("Failed registering RIOT native class.\n");
        return -1;
    }

    res = loadPackedUjcClasses(ujc_Example, &mainClass);
    if (res != UJ_ERR_NONE)
    {
        return -1;
    }

    res = ujInitAllClasses();
    if (res != UJ_ERR_NONE)
    {
        printf("ujInitAllClasses failed: %d\n", res);
        return -1;
    }

    HANDLE threadH = ujThreadCreate(UJ_HEAP_SZ / 2);
    if (!threadH)
    {
        printf("ujThreadCreate failed\n");
        return -1;
    }

    res = ujThreadGoto(threadH, mainClass, "main", "()V");
    if (res != UJ_ERR_NONE)
    {
        printf("ujThreadGoto failed: %d\n", res);
        return -1;
    }

    while (ujCanRun()) {
        res = ujInstr();
        if (res != UJ_ERR_NONE)
        {
            printf("ujInstr failed: %d\n", res);
            return -1;
        }
    }

    printf("Program ended\n");

    return 0;
}
