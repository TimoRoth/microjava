#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <thread.h>
#include <periph/pm.h>
#include <msg.h>

#ifdef MODULE_SHELL
#include <shell.h>
#endif

#ifdef MODULE_NAT_CONSTFS
#include "nat/constfs/nat_constfs.h"
#endif
#ifdef MODULE_NAT_ESP32
#include "nat/esp32/nat_esp32.h"
#endif
#ifdef MODULE_NAT_ESP8266
#include "nat/esp8266/nat_esp8266.h"
#endif
#ifdef MODULE_NAT_ESP_SPIFFS
#include "nat/esp_spiffs/nat_esp_spiffs.h"
#endif
#ifdef MODULE_NAT_LITTLEFS
#include "nat/littlefs/nat_littlefs.h"
#endif
#ifdef MODULE_NAT_GCOAP
#include "nat/gcoap/nat_gcoap.h"
#endif

#include "nat_uj.h"


static int init_hardware(void)
{
    int res = 0;

#ifdef MODULE_NAT_CONSTFS
    res = init_nat_constfs();
    if (res)
        return res;
#endif

#ifdef MODULE_NAT_ESP32
    res = init_nat_esp32();
    if (res)
        return res;
#endif

#ifdef MODULE_NAT_ESP8266
    res = init_nat_esp8266();
    if (res)
        return res;
#endif

#ifdef MODULE_NAT_ESP_SPIFFS
    res = init_nat_esp_spiffs();
    if (res)
        return res;
#endif

#ifdef MODULE_NAT_LITTLEFS
    res = init_nat_littlefs();
    if (res)
        return res;
#endif

#ifdef MODULE_NAT_GCOAP
    res = init_nat_gcoap();
    if (res)
        return res;
#endif

    return res;
}

char uj_stack[THREAD_STACKSIZE_MAIN];

void *uj_thread(void *arg)
{
    (void)arg;

    if (!run_uj())
        pm_reboot();

    return NULL;
}

#ifdef MODULE_SHELL
static const shell_command_t commands[] =
{
    { NULL, NULL, NULL }
};

static msg_t main_msg_queue[1 << 2];
#endif

int main(void)
{
    int res;

    res = init_hardware();
    if (res != 0)
        return res;

    thread_create(uj_stack, sizeof(uj_stack),
                  THREAD_PRIORITY_MAIN - 1, 0,
                  uj_thread, NULL, "uj");

#ifdef MODULE_SHELL
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    msg_init_queue(main_msg_queue, sizeof(main_msg_queue) / sizeof(main_msg_queue[0]));
    shell_run(commands, line_buf, sizeof(line_buf));
#endif

    return 0;
}
