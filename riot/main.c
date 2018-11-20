#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <periph/pm.h>

#include <uJ/uj.h>

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

#include "nat_classes.h"
#include "class_loader.h"

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

static int run_uj(void)
{
    UjClass *objectClass = NULL;
    UjClass *mainClass = NULL;
    int res;

    res = ujInit(&objectClass);
    if (res != UJ_ERR_NONE)
    {
        printf("ujInit failed: %d\n", res);
        return -1;
    }

    res = register_nat_all(objectClass);
    if (res != UJ_ERR_NONE)
        return -1;

    res = loadPackedUjcClasses(&mainClass);
    if (res != UJ_ERR_NONE)
    {
        return -1;
    }

    res = ujInitAllClasses();
    if (res != UJ_ERR_NONE)
    {
        printf("ujInitAllClasses failed: %d\n", res);
        return -1;
    }

    // Half of the heap will be used as stack
    HANDLE threadH = ujThreadCreate(UJ_HEAP_SZ / 2);
    if (!threadH)
    {
        printf("ujThreadCreate failed\n");
        return -1;
    }

    res = ujThreadGoto(threadH, mainClass, "main", "()V");
    if (res != UJ_ERR_NONE)
    {
        printf("ujThreadGoto failed: %d\n", res);
        return -1;
    }

    while (ujCanRun()) {
        res = ujInstr();
        if (res != UJ_ERR_NONE)
        {
            printf("ujInstr failed: %d\n", res);
            return -1;
        }
    }

    printf("Program ended\n");

    return 0;
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
