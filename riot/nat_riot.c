#include <stdlib.h>
#include <string.h>
#include <xtimer.h>

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

static event_t *cur_event = NULL;

static uint8_t natRIOT_waitEvent(UjThread* t, UjClass* cls)
{
    (void)cls;

    int timeout_us = ujThreadPop(t);
    int res;

    free_event(&cur_event);
    cur_event = wait_event(timeout_us);

    if (cur_event)
        res = cur_event->id;
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

    if (cur_event->id > 0 && idx < cur_event->num_params && cur_event->params[idx].type == EPT_Int)
        res = cur_event->params[idx].val.int_val;

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

    if (cur_event->id > 0 && idx < cur_event->num_params && cur_event->params[idx].type == EPT_String && cur_event->params[idx].val.str_val.str) {
        ret = ujStringFromBytes(&res, (uint8_t*)cur_event->params[idx].val.str_val.str, 0);
        if (ret != UJ_ERR_NONE)
            return ret;
    }

    if (!ujThreadPush(t, res, res ? true : false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_replyEventBuf(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE arrHandle = ujThreadPop(t);
    int dataType = ujThreadPop(t);
    int replType = ujThreadPop(t);

    int numParams = 2;
    int arrLen = 0;
    char *databuf = NULL;

    if (arrHandle) {
        numParams += 1;
        arrLen = ujArrayLen(arrHandle);
        databuf = malloc(arrLen);
        if (!databuf)
            return UJ_ERR_OUT_OF_MEMORY;

        char *data = ujArrayRawAccessStart(arrHandle);
        memcpy(databuf, data, arrLen);
        ujArrayRawAccessFinish(arrHandle);
    }

    event_t *event = make_event_raw(replType, numParams);

    event->params[0].type = EPT_Int;
    event->params[0].val.int_val = dataType;

    event->params[1].type = EPT_Int;
    event->params[1].val.int_val = arrLen;

    if (databuf) {
        event->params[2].type = EPT_String;
        event->params[2].val.str_val.str = databuf;
        event->params[2].val.str_val.needs_free = true;
    }

    int res = reply_last_event(event);
    if (res)
        free(databuf);

    if (!ujThreadPush(t, res, res ? false : true))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_usleep(UjThread* t, UjClass* cls)
{
    (void)cls;

    int usec = ujThreadPop(t);

    xtimer_usleep(usec);

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
    {
        .name = "replyEvent",
        .type = "(II[B)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_replyEventBuf,
    },
    {
        .name = "usleep",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_usleep,
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
