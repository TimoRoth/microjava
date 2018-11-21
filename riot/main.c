#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <periph/pm.h>
#include <msg.h>

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

    return res;
}

int main(void)
{
    int res;

    res = init_hardware();
    if (res != 0)
        return res;

    run_uj();

    pm_reboot();

    return 0;
}
