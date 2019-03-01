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

    if (post_event_receive(coap_event, &coap_event) < 0) {
        free_event(&coap_event);
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }

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

static int fillRemoteInfo(sock_udp_ep_t *remote, HANDLE addrHandle, int port)
{
    if (port <= 0 || port > 65535)
        port = 5683;

    uint16_t sz = ujStringGetBytes(addrHandle, NULL, 0);
    char addrStr[sz];
    sz = ujStringGetBytes(addrHandle, (uint8_t*)addrStr, sz);

    remote->family = AF_INET6;
    remote->port = port;

    int iface = ipv6_addr_split_iface(addrStr);
    if (iface == -1) {
        if (gnrc_netif_numof() == 1) {
            /* assign the single interface found in gnrc_netif_numof() */
            remote->netif = (uint16_t)gnrc_netif_iter(NULL)->pid;
        } else {
            remote->netif = SOCK_ADDR_ANY_NETIF;
        }
    } else {
        if (gnrc_netif_get_by_pid(iface) == NULL) {
            puts("coap_req: interface not valid");
            return -1;
        }
        remote->netif = iface;
    }

    /* parse destination address */
    ipv6_addr_t addr;
    if (ipv6_addr_from_str(&addr, addrStr) == NULL) {
        puts("coap_req: unable to parse destination address");
        return -1;
    }
    if ((remote->netif == SOCK_ADDR_ANY_NETIF) && ipv6_addr_is_link_local(&addr)) {
        puts("coap_req: must specify interface for link local target");
        return -1;
    }
    memcpy(&remote->addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    return 0;
}

static void handleRequestResponse(unsigned req_state, coap_pkt_t *pdu, sock_udp_ep_t *remote)
{
    (void)remote;

    if (req_state == GCOAP_MEMO_TIMEOUT || req_state == GCOAP_MEMO_ERR) {
        event_t *coap_event = make_event_ii(EVT_COAP_RESP, coap_get_id(pdu), req_state);
        if (post_event(coap_event) != 0) {
            puts("coap_req: failed posting req reply error event");
            free_event(&coap_event);
        }
        return;
    }

    event_t *coap_event = make_event_raw(EVT_COAP_RESP, pdu->payload_len ? 5 : 4);

    coap_event->params[0].type = EPT_Int;
    coap_event->params[0].val.int_val = coap_get_id(pdu);

    coap_event->params[1].type = EPT_Int;
    coap_event->params[1].val.int_val = req_state;

    coap_event->params[2].type = EPT_Int;
    coap_event->params[2].val.int_val = coap_get_code_raw(pdu);

    coap_event->params[3].type = EPT_Int;
    coap_event->params[3].val.int_val = coap_get_content_type(pdu);

    if (pdu->payload_len) {
        char *tgt = malloc(pdu->payload_len);
        memcpy(tgt, pdu->payload, pdu->payload_len);

        coap_event->params[4].type = EPT_Bytes;
        coap_event->params[4].val.str_val.str = tgt;
        coap_event->params[4].val.str_val.len = pdu->payload_len;
        coap_event->params[4].val.str_val.needs_free = 1;
    }

    if (post_event(coap_event) != 0) {
        puts("coap_req: failed posting req reply event");
        free_event(&coap_event);
    }
}

static uint8_t natCOAP_sendRequest_generic(UjThread* t, int format, HANDLE strPl, HANDLE arrPl)
{
    HANDLE pathHandle = ujThreadPop(t);
    int port = ujThreadPop(t);
    HANDLE addrHandle = ujThreadPop(t);
    int method = ujThreadPop(t);

    int ret = -1;

    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;

    ssize_t sz = ujStringGetBytes(pathHandle, NULL, 0);
    char pathStr[sz];
    sz = ujStringGetBytes(pathHandle, (uint8_t*)pathStr, sz);

    sock_udp_ep_t remote;
    if (fillRemoteInfo(&remote, addrHandle, port) < 0)
        goto err;

    if (gcoap_req_init(&pdu, buf, sizeof(buf), method, pathStr) < 0)
        goto err;

    if (strPl) {
        sz = ujStringGetBytes(strPl, NULL, 0);
        if (pdu.payload_len > sz) {
            ujStringGetBytes(strPl, pdu.payload, sz);
        } else {
            puts("coap_req: msg buffer too small for payload");
            goto err;
        }
    } else if (arrPl) {
        sz = ujArrayLen(arrPl);
        if (pdu.payload_len > sz) {
            char *data = ujArrayRawAccessStart(arrPl);
            memcpy(pdu.payload, data, sz);
            ujArrayRawAccessFinish(arrPl);
        } else {
            puts("coap_req: msg buffer too small for payload");
            goto err;
        }
    } else {
        format = COAP_FORMAT_NONE;
        sz = 0;
    }

    coap_hdr_set_type(pdu.hdr, COAP_TYPE_NON);

    sz = gcoap_finish(&pdu, sz, format);
    if (sz < 0) {
        puts("coap_req: finish failed");
        goto err;
    }

    if (gcoap_req_send2(buf, sz, &remote, &handleRequestResponse) <= 0) {
        puts("coap_req: send2 failed");
        goto err;
    }

    ret = coap_get_id(&pdu);

err:
    if (!ujThreadPush(t, ret, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natCOAP_sendRequest(UjThread* t, UjClass* cls)
{
    (void)cls;

    return natCOAP_sendRequest_generic(t, 0, 0, 0);
}

static uint8_t natCOAP_sendRequestS(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE strPl = ujThreadPop(t);
    int format = ujThreadPop(t);

    return natCOAP_sendRequest_generic(t, format, strPl, 0);
}

static uint8_t natCOAP_sendRequestB(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE arrPl = ujThreadPop(t);
    int format = ujThreadPop(t);

    return natCOAP_sendRequest_generic(t, format, 0, arrPl);
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
    {
        .name = "sendRequest",
        .type = "(ILjava/lang/String;ILjava/lang/String;)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natCOAP_sendRequest,
    },
    {
        .name = "sendRequest",
        .type = "(ILjava/lang/String;ILjava/lang/String;ILjava/lang/String;)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natCOAP_sendRequestS,
    },
    {
        .name = "sendRequest",
        .type = "(ILjava/lang/String;ILjava/lang/String;I[B)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natCOAP_sendRequestB,
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
