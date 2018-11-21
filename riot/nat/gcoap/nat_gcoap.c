#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <net/gcoap.h>
#ifdef MODULE_VFS
#include <vfs.h>
#endif

#include "nat_gcoap.h"


static const UjNativeMethod nativeCls_COAP_methods[] = {
    {
        .name = "pin",
        .type = "(II)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = NULL,
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
    int res = ujRegisterNativeClass(&nativeCls_COAP, objectClass, NULL);
    if (res != UJ_ERR_NONE)
        printf("Failed registering COAP native class.\n");

    return res;
}

static ssize_t riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memcpy(pdu->payload, RIOT_BOARD, strlen(RIOT_BOARD));
    return gcoap_finish(pdu, strlen(RIOT_BOARD), COAP_FORMAT_TEXT);
}

static ssize_t uj_update_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

#ifndef MODULE_VFS
    return gcoap_response(pdu, buf, len, COAP_CODE_NOT_IMPLEMENTED);
#else
    if (pdu->payload_len < 3)
        return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);

    printf("Writing code update...\n");

    int fd = vfs_open("/main/update.ujcpak", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_PATH_NOT_FOUND);

    if (vfs_write(fd, pdu->payload, pdu->payload_len) != pdu->payload_len) {
        vfs_close(fd);
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }

    vfs_close(fd);

    printf("Code update written.\n");

    return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
#endif
}

static const coap_resource_t native_resources[] = {
    { "/riot/board", COAP_GET, riot_board_handler, NULL },
    { "/uj/update", COAP_PUT, uj_update_handler, NULL },
};

static gcoap_listener_t native_listeners = {
    native_resources,
    sizeof(native_resources) / sizeof(native_resources[0]),
    NULL
};

int init_nat_gcoap(void)
{
    gcoap_register_listener(&native_listeners);

    return 0;
}
