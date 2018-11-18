#include <uJ/uj.h>

#include "nat_classes.h"

static uint8_t natRIOT_printString(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE strHandle = (HANDLE)ujThreadPeek(t, 0);
    uint32_t sz = ujStringGetBytes(strHandle, NULL, 0);

    char buf[sz];
    sz = ujStringGetBytes(strHandle, (uint8_t*)buf, sz);

    printf("%s\n", buf);

    ujThreadPop(t);

    if (!ujThreadPush(t, sz, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static const UjNativeMethod nativeCls_RIOT_methods[] = {
    {
        .name = "printString",
        .type = "(Ljava/lang/String;)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_printString,
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

int register_nat_riot(UjClass *objectClass)
{
    int res = ujRegisterNativeClass(&nativeCls_RIOT, objectClass, NULL);
    if (res != UJ_ERR_NONE)
        printf("Failed registering RIOT native class.\n");

    return res;
}