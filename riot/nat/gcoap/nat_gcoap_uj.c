#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <net/gcoap.h>
#ifdef MODULE_VFS
#include <vfs.h>
#endif

#include "nat_gcoap.h"
#include "events.h"


static gcoap_listener_t uj_listeners = {
    NULL,
    0,
    NULL
};

static ssize_t uj_gcoap_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    event_t *coap_event = make_event_raw(EVT_COAP_REQ, pdu->payload_len ? 3 : 2);

    coap_event->params[0].type = EPT_Int;
    coap_event->params[0].val.int_val = (uintptr_t)ctx;

    coap_event->params[1].type = EPT_Int;
    coap_event->params[1].val.int_val = coap_method2flag(coap_get_code_detail(pdu));

    if (pdu->payload_len) {
        coap_event->params[2].type = EPT_Bytes;
        coap_event->params[2].val.str_val.str = (char*)pdu->payload;
        coap_event->params[2].val.str_val.len = pdu->payload_len;
        coap_event->params[2].val.str_val.needs_free = 0;
    }

    if (post_event_receive(coap_event, &coap_event) < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    if (coap_event->id != EVT_COAP_REPLY || coap_event->num_params < 1 || coap_event->num_params > 3) {
        printf("Got invalid CoAP reply event %d len %d!\n", (int)coap_event->id, (int)coap_event->num_params);
        free_event(&coap_event);
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }

    int code = coap_event->params[0].val.int_val;

    if (coap_event->num_params < 3) {
        free_event(&coap_event);
        return gcoap_response(pdu, buf, len, code);
    }

    int fmt = coap_event->params[1].val.int_val;

    const char *str = coap_event->params[2].val.str_val.str;
    size_t slen = coap_event->params[2].val.str_val.len;
    if (!slen)
        slen = strlen(str);

    gcoap_resp_init(pdu, buf, len, code);

    if (slen > pdu->payload_len) {
        printf("Truncating CoAP response, got %d, max %d\n", (int)slen, (int)pdu->payload_len);
        slen = pdu->payload_len;
    }

    memcpy(pdu->payload, str, slen);

    free_event(&coap_event);

    return gcoap_finish(pdu, slen, fmt);
}

static uint8_t natCOAP_registerResource(UjThread* t, UjClass* cls)
{
    (void)cls;

    int methods = ujThreadPop(t);
    HANDLE pathHandle = ujThreadPop(t);

    if (!pathHandle)
        return UJ_ERR_NONE;

    uint16_t pathsize = ujStringGetBytes(pathHandle, NULL, 0);

    uj_listeners.resources = realloc((void*)uj_listeners.resources, (uj_listeners.resources_len + 1) * sizeof(coap_resource_t));
    coap_resource_t *res = (coap_resource_t*)&uj_listeners.resources[uj_listeners.resources_len];
    uj_listeners.resources_len += 1;

    res->path = malloc(pathsize);
    if (!res->path)
        return UJ_ERR_OUT_OF_MEMORY;
    ujStringGetBytes(pathHandle, (uint8_t*)res->path, pathsize);

    res->methods = methods;
    res->handler = uj_gcoap_handler;
    res->context = (void*)(uintptr_t)uj_listeners.resources_len;

    if (!ujThreadPush(t, uj_listeners.resources_len, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natCOAP_finishRegistration(UjThread* t, UjClass* cls)
{
    (void)t;
    (void)cls;

    gcoap_register_listener(&uj_listeners);

    return UJ_ERR_NONE;
}

static const UjNativeMethod nativeCls_COAP_methods[] = {
    {
        .name = "registerResource",
        .type = "(Ljava/lang/String;I)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natCOAP_registerResource,
    },
    {
        .name = "finishRegistration",
        .type = "()V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natCOAP_finishRegistration,
    },
};

static const UjNativeClass nativeCls_COAP =
{
    .clsName = "COAP",

    .clsDatSz = 0,
    .instDatSz = 0,
    .gcClsF = NULL,
    .gcInstF = NULL,

    .numMethods = sizeof(nativeCls_COAP_methods) / sizeof(nativeCls_COAP_methods[0]),
    .methods = nativeCls_COAP_methods,
};

int register_nat_gcoap(UjClass *objectClass)
{
    for (size_t i = 0; i < uj_listeners.resources_len; i++)
        free((void*)uj_listeners.resources[i].path);
    free((void*)uj_listeners.resources);

    uj_listeners.resources_len = 0;
    uj_listeners.resources = NULL;

    int res = ujRegisterNativeClass(&nativeCls_COAP, objectClass, NULL);
    if (res != UJ_ERR_NONE)
        printf("Failed registering COAP native class.\n");

    return res;
}
