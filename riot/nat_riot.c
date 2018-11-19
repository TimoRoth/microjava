#include <uJ/uj.h>

#include "events.h"
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

static event_t cur_event = { 0 };

static uint8_t natRIOT_waitEvent(UjThread* t, UjClass* cls)
{
    (void)cls;

    int timeout_us = ujThreadPop(t);

    free_event(&cur_event);
    int res = wait_event(timeout_us, &cur_event);

    if (res == 0)
        res = cur_event.id;
    else
        res = -1;

    if (!ujThreadPush(t, res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_getEventParam(UjThread* t, UjClass* cls)
{
    (void)cls;

    unsigned int idx = ujThreadPop(t);
    int res = 0;

    if (cur_event.id > 0 && idx < MAX_EVENT_PARAMS && cur_event.params[idx].type == EPT_Int)
        res = cur_event.params[idx].val.int_val;

    if (!ujThreadPush(t, res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_getEventParamStr(UjThread* t, UjClass* cls)
{
    (void)cls;

    unsigned int idx = ujThreadPop(t);
    HANDLE res = 0;
    int ret;

    if (cur_event.id > 0 && idx < MAX_EVENT_PARAMS && cur_event.params[idx].type == EPT_String && cur_event.params[idx].val.str_val.str) {
        ret = ujStringFromBytes(&res, (uint8_t*)cur_event.params[idx].val.str_val.str, 0);
        if (ret != UJ_ERR_NONE)
            return ret;
    }

    if (!ujThreadPush(t, res, res ? true : false))
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
    {
        .name = "waitEvent",
        .type = "(I)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_waitEvent,
    },
    {
        .name = "getEventParam",
        .type = "(I)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_getEventParam,
    },
    {
        .name = "getEventParamStr",
        .type = "(I)Ljava/lang/String;",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_getEventParamStr,
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