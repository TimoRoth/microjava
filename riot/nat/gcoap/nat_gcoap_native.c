#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <net/gcoap.h>
#ifdef MODULE_VFS
#include <vfs.h>
#endif

#include "nat_gcoap.h"
#include "events.h"

// Nanocoap symbols, unused, but linker likes to have them.
const coap_resource_t coap_resources[] = { { 0 } };
const unsigned coap_resources_numof = 0;


static ssize_t riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    memcpy(pdu->payload, RIOT_BOARD, strlen(RIOT_BOARD));
    return gcoap_finish(pdu, strlen(RIOT_BOARD), COAP_FORMAT_TEXT);
}

static ssize_t uj_update_start_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    int fd = vfs_open("/main/update.ujcpak.part", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    if (vfs_close(fd) < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    return gcoap_response(pdu, buf, len, COAP_CODE_CREATED);
}

static ssize_t uj_update_abort_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    vfs_unlink("/main/update.ujcpak.part");
    vfs_unlink("/main/update.ujcpak");

    return gcoap_response(pdu, buf, len, COAP_CODE_DELETED);
}

static ssize_t uj_update_finish_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

    if (vfs_rename("/main/update.ujcpak.part", "/main/update.ujcpak") < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    if (post_event(make_event(EVT_UPD_RDY)) != 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
}

static ssize_t uj_update_write_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, void *ctx)
{
    (void)ctx;

#ifndef MODULE_VFS
    return gcoap_response(pdu, buf, len, COAP_CODE_NOT_IMPLEMENTED);
#else
    if (pdu->payload_len < 1)
        return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);

    uint8_t qs[NANOCOAP_URI_MAX];
    ssize_t qslen = coap_get_uri_query(pdu, qs);
    int offset = 0;
    if (qslen > 2)
        sscanf((char*)qs, "&%d", &offset);

    printf("Writing code update...\n");

    int fd = vfs_open("/main/update.ujcpak.part", O_WRONLY, 0);
    if (fd < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    if (vfs_lseek(fd, offset, SEEK_SET) < 0) {
        vfs_close(fd);
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }

    if (vfs_write(fd, pdu->payload, pdu->payload_len) != pdu->payload_len) {
        vfs_close(fd);
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }

    if (vfs_close(fd) < 0)
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);

    printf("Code update written. (offset %d)\n", offset);

    return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
#endif
}

// This has to be in alphabetical order!
static const coap_resource_t native_resources[] = {
    { "/riot/board", COAP_GET, riot_board_handler, NULL },
    { "/uj/update/abort", COAP_GET, uj_update_abort_handler, NULL },
    { "/uj/update/finish", COAP_GET, uj_update_finish_handler, NULL },
    { "/uj/update/start", COAP_GET, uj_update_start_handler, NULL },
    { "/uj/update/write", COAP_PUT, uj_update_write_handler, NULL },
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
