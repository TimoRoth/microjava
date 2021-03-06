#include <stdlib.h>
#include <string.h>
#include <xtimer.h>

#ifdef MODULE_PERIPH_HWRNG
#include <periph/hwrng.h>
#include <bitarithm.h>
#else
#include <random.h>
#endif

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

    if (cur_event && cur_event->id > 0 && idx < cur_event->num_params && cur_event->params[idx].type == EPT_Int)
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

    if (cur_event && cur_event->id > 0 && idx < cur_event->num_params && (cur_event->params[idx].type == EPT_String || cur_event->params[idx].type == EPT_Bytes) && cur_event->params[idx].val.str_val.str) {
        ret = ujStringFromBytes(&res, (uint8_t*)cur_event->params[idx].val.str_val.str, cur_event->params[idx].val.str_val.len);
        if (ret != UJ_ERR_NONE)
            return ret;
    }

    if (!ujThreadPush(t, res, res ? true : false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_getEventParamBytes(UjThread* t, UjClass* cls)
{
    (void)cls;

    unsigned int idx = ujThreadPop(t);
    HANDLE res = 0;
    int ret;

    if (cur_event && cur_event->id > 0 && idx < cur_event->num_params && (cur_event->params[idx].type == EPT_String || cur_event->params[idx].type == EPT_Bytes) && cur_event->params[idx].val.str_val.str) {
        const char *data = cur_event->params[idx].val.str_val.str;
        int len = cur_event->params[idx].val.str_val.len;
        if (!len)
            len = strlen(data);

        ret = ujArrayNew('B', len, &res);
        if (ret != UJ_ERR_NONE)
            return ret;

        char *arr_data = ujArrayRawAccessStart(res);
        memcpy(arr_data, data, len);
        ujArrayRawAccessFinish(res);
    }

    if (!ujThreadPush(t, res, res ? true : false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t replyEventGeneric(UjThread* t, bool isArray)
{
    HANDLE handle = ujThreadPop(t);
    int param2 = ujThreadPop(t);
    int param1 = ujThreadPop(t);
    int replType = ujThreadPop(t);

    int numParams = 2;
    int arrLen = 0;
    char *databuf = NULL;

    if (handle) {
        numParams += 1;
        arrLen = isArray ? ujArrayLen(handle) : ujStringGetBytes(handle, NULL, 0);
        databuf = malloc(arrLen);
        if (!databuf)
            return UJ_ERR_OUT_OF_MEMORY;

        if (isArray) {
            char *data = ujArrayRawAccessStart(handle);
            memcpy(databuf, data, arrLen);
            ujArrayRawAccessFinish(handle);
        } else {
            ujStringGetBytes(handle, (uint8_t*)databuf, arrLen);
        }
    }

    event_t *event = make_event_raw(replType, numParams);

    event->params[0].type = EPT_Int;
    event->params[0].val.int_val = param1;

    event->params[1].type = EPT_Int;
    event->params[1].val.int_val = param2;

    if (databuf) {
        event->params[2].type = isArray ? EPT_Bytes : EPT_String;
        event->params[2].val.str_val.str = databuf;
        event->params[2].val.str_val.needs_free = true;
        event->params[2].val.str_val.len = arrLen - (isArray ? 0 : 1); // remove final null byte for strings from length
    }

    int res = reply_last_event(event);
    if (res != 0)
        free(databuf);

    if (!ujThreadPush(t, res, res ? false : true))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_replyEventBuf(UjThread* t, UjClass* cls)
{
    (void)cls;

    return replyEventGeneric(t, true);
}

static uint8_t natRIOT_replyEventStr(UjThread* t, UjClass* cls)
{
    (void)cls;

    return replyEventGeneric(t, false);
}

typedef struct timer_helper_st
{
    event_t event; // has to be first memeber, as this is what gets passed to free()
    event_param_t params[1];
    msg_t msg;
    xtimer_t timer;
} timer_helper_t;

static uint8_t setTimeoutGeneric(uint64_t timeout_us, int userdata)
{
    timer_helper_t *timer_helper = calloc(1, sizeof(timer_helper_t));
    xtimer_t *timer = &timer_helper->timer;
    msg_t *msg = &timer_helper->msg;
    event_t *event = &timer_helper->event;

    msg->type = EVT_MSG_TYPE;
    msg->content.ptr = event;

    event->id = EVT_TIMER;
    event->num_params = sizeof(timer_helper->params) / sizeof(timer_helper->params[0]);
    event->params = timer_helper->params;

    event->params[0].type = EPT_Int;
    event->params[0].val.int_val = userdata;

    xtimer_set_msg64(timer, timeout_us, msg, get_event_pid());

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_setTimeoutUS(UjThread* t, UjClass* cls)
{
    (void)cls;

    int userdata = ujThreadPop(t);
    int usec = ujThreadPop(t);

    return setTimeoutGeneric(usec, userdata);
}

static uint8_t natRIOT_setTimeoutS(UjThread* t, UjClass* cls)
{
    (void)cls;

    int userdata = ujThreadPop(t);
    int sec = ujThreadPop(t);

    return setTimeoutGeneric((uint64_t)sec * (uint64_t)1000000, userdata);
}

static uint8_t natRIOT_usleep(UjThread* t, UjClass* cls)
{
    (void)cls;

    int usec = ujThreadPop(t);

    xtimer_usleep(usec);

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_random(UjThread* t, UjClass* cls)
{
    (void)cls;

    uint32_t res;

#ifdef MODULE_PERIPH_HWRNG
    hwrng_read(&res, sizeof(res));
#else
    res = random_uint32();
#endif

    if (!ujThreadPush(t, res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natRIOT_randomRange(UjThread* t, UjClass* cls)
{
    (void)cls;

    uint32_t b = ujThreadPop(t);
    uint32_t a = ujThreadPop(t);

    uint32_t res;

    if (a > b) {
        res = a;
        a = b;
        b = res;
    }

#ifdef MODULE_PERIPH_HWRNG
    // logic taken from random_uint32_range implementation
    uint32_t divisor, range = b - a;
    uint8_t range_msb = bitarithm_msb(range);

    if (!(range & (range - 1)))
        divisor = (1 << range_msb) - 1;
    else if (range_msb < 31)
        divisor = (1 << (range_msb + 1)) -1;
    else
        divisor = UINT32_MAX;

    do {
        hwrng_read(&res, sizeof(res));
        res &= divisor;
    } while (res >= range);

    res += a;
#else
    res = random_uint32_range(a, b);
#endif

    if (!ujThreadPush(t, res, false))
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
    {
        .name = "getEventParamBytes",
        .type = "(I)[B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_getEventParamBytes,
    },
    {
        .name = "replyEvent",
        .type = "(III[B)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_replyEventBuf,
    },
    {
        .name = "replyEvent",
        .type = "(IIILjava/lang/String;)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_replyEventStr,
    },
    {
        .name = "setTimeoutUS",
        .type = "(II)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_setTimeoutUS,
    },
    {
        .name = "setTimeoutS",
        .type = "(II)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_setTimeoutS,
    },
    {
        .name = "usleep",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_usleep,
    },
    {
        .name = "random",
        .type = "()I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_random,
    },
    {
        .name = "randomRange",
        .type = "(II)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natRIOT_randomRange,
    }
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
