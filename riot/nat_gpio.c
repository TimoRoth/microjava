#include <periph/gpio.h>

#include <uJ/uj.h>

#include "nat_classes.h"


static uint8_t natGPIO_pin(UjThread* t, UjClass* cls)
{
    (void)cls;

    // Parameters are pushed onto the stack from left to right, so pop gives them to us in reverse order
    int pin = ujThreadPop(t);
    int port = ujThreadPop(t);

    if (!ujThreadPush(t, GPIO_PIN(port, pin), false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_init(UjThread* t, UjClass* cls)
{
    (void)cls;

    int mode = ujThreadPop(t);
    int gpio_pin = ujThreadPop(t);

    // This needs to be translated, as java cannot see board specific constants.
    switch(mode)
    {
    case 1:
        mode = GPIO_IN;
        break;
    case 2:
        mode = GPIO_IN_PD;
        break;
    case 3:
        mode = GPIO_IN_PU;
        break;
    case 4:
        mode = GPIO_OUT;
        break;
    case 5:
        mode = GPIO_OD;
        break;
    case 6:
        mode = GPIO_OD_PU;
        break;
    default:
        printf("Invalid GPIO mode %d\n", mode);
        return UJ_ERR_INTERNAL;
    }

    int res = gpio_init(gpio_pin, mode);

    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_clear(UjThread* t, UjClass* cls)
{
    (void)cls;

    int gpio_pin = ujThreadPop(t);

    gpio_clear(gpio_pin);

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_set(UjThread* t, UjClass* cls)
{
    (void)cls;

    int gpio_pin = ujThreadPop(t);

    gpio_set(gpio_pin);

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_toggle(UjThread* t, UjClass* cls)
{
    (void)cls;

    int gpio_pin = ujThreadPop(t);

    gpio_toggle(gpio_pin);

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_read(UjThread* t, UjClass* cls)
{
    (void)cls;

    int gpio_pin = ujThreadPop(t);

    if (!ujThreadPush(t, gpio_read(gpio_pin) ? true : false, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_write(UjThread* t, UjClass* cls)
{
    (void)cls;

    int value = ujThreadPop(t) ? 1 : 0;
    int gpio_pin = ujThreadPop(t);

    gpio_write(gpio_pin, value);

    return UJ_ERR_NONE;
}

static const UjNativeMethod nativeCls_GPIO_methods[] = {
    {
        .name = "pin",
        .type = "(II)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_pin,
    },
    {
        .name = "init",
        .type = "(II)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_init,
    },
    {
        .name = "clear",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_clear,
    },
    {
        .name = "set",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_set,
    },
    {
        .name = "toggle",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_toggle,
    },
    {
        .name = "read",
        .type = "(I)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_read,
    },
    {
        .name = "write",
        .type = "(IZ)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_write,
    },
};

static const UjNativeClass nativeCls_GPIO =
{
    .clsName = "GPIO",

    .clsDatSz = 0,
    .instDatSz = 0,
    .gcClsF = NULL,
    .gcInstF = NULL,

    .numMethods = sizeof(nativeCls_GPIO_methods) / sizeof(nativeCls_GPIO_methods[0]),
    .methods = nativeCls_GPIO_methods,
};

int register_nat_gpio(UjClass *objectClass)
{
    int res = ujRegisterNativeClass(&nativeCls_GPIO, objectClass, NULL);
    if (res != UJ_ERR_NONE)
        printf("Failed registering GPIO native class.\n");

    return res;
}