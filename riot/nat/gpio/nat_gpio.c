#include <periph_conf.h>
#include <periph/gpio.h>

#include "nat_gpio.h"
#include "events.h"


static uint8_t natGPIO_pin(UjThread* t, UjClass* cls)
{
    (void)cls;

    // Parameters are pushed onto the stack from left to right, so pop gives them to us in reverse order
    int32_t pin = ujThreadPop(t);
    int32_t port = ujThreadPop(t);

    if (!ujThreadPush(t, (int32_t)GPIO_PIN(port, pin), false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static inline gpio_t convert_gpio_pin(int32_t in)
{
    return (gpio_t)in;
}

static inline int convert_gpio_mode(int32_t mode)
{
    // This needs to be translated, as java cannot see board specific constants.
    switch(mode)
    {
    case 1:
        return GPIO_IN;
    case 2:
        return GPIO_IN_PD;
    case 3:
        return GPIO_IN_PU;
    case 4:
        return GPIO_OUT;
    case 5:
        return GPIO_OD;
    case 6:
        return GPIO_OD_PU;
    default:
        printf("Invalid GPIO mode %d\n", (int)mode);
        return -1;
    }
}

#ifdef MODULE_PERIPH_GPIO_IRQ
static inline int convert_gpio_flank(int32_t mode)
{
    switch(mode)
    {
    case 1:
        return GPIO_FALLING;
    case 2:
        return GPIO_RISING;
    case 3:
        return GPIO_BOTH;
    default:
        printf("Invalid GPIO flank mode %d\n", (int)mode);
        return -1;
    }
}

static void gpio_int_cb(void *arg)
{
    int32_t gpio_pin = (intptr_t)arg;

    int res = post_event(make_event_i(EVT_GPIO, gpio_pin));
    if (res < 0)
        printf("Failed posting GPIO event for %d: %d\n", (int)gpio_pin, res);
}
#endif

static uint8_t natGPIO_init(UjThread* t, UjClass* cls)
{
    (void)cls;

    int32_t mode = ujThreadPop(t);
    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

    mode = convert_gpio_mode(mode);
    if (mode < 0)
        return UJ_ERR_INTERNAL;

    int res = gpio_init(gpio_pin, mode);

    if (!ujThreadPush(t, !res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_init_int(UjThread* t, UjClass* cls)
{
    (void)cls;

    int32_t flank = ujThreadPop(t);
    int32_t mode = ujThreadPop(t);
    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));
    int res = -1;

#ifdef MODULE_PERIPH_GPIO_IRQ
    mode = convert_gpio_mode(mode);
    if (mode < 0)
        return UJ_ERR_INTERNAL;

    flank = convert_gpio_flank(flank);
    if (flank < 0)
        return UJ_ERR_INTERNAL;

    res = gpio_init_int(gpio_pin, mode, flank, &gpio_int_cb, (void*)(intptr_t)gpio_pin);
#else
    (void)gpio_pin;
    (void)mode;
    (void)flank;
#endif

    if (!ujThreadPush(t, !res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_irq_disable(UjThread* t, UjClass* cls)
{
    (void)cls;

    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

#ifdef MODULE_PERIPH_GPIO_IRQ
    gpio_irq_disable(gpio_pin);
#else
    (void)gpio_pin;
#endif

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_irq_enable(UjThread* t, UjClass* cls)
{
    (void)cls;

    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

#ifdef MODULE_PERIPH_GPIO_IRQ
    gpio_irq_enable(gpio_pin);
#else
    (void)gpio_pin;
#endif

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_clear(UjThread* t, UjClass* cls)
{
    (void)cls;

    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

    gpio_clear(gpio_pin);

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_set(UjThread* t, UjClass* cls)
{
    (void)cls;

    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

    gpio_set(gpio_pin);

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_toggle(UjThread* t, UjClass* cls)
{
    (void)cls;

    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

    gpio_toggle(gpio_pin);

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_read(UjThread* t, UjClass* cls)
{
    (void)cls;

    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

    if (!ujThreadPush(t, !!gpio_read(gpio_pin), false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natGPIO_write(UjThread* t, UjClass* cls)
{
    (void)cls;

    int value = ujThreadPop(t) ? 1 : 0;
    gpio_t gpio_pin = convert_gpio_pin(ujThreadPop(t));

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
        .name = "init_int",
        .type = "(III)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_init_int,
    },
    {
        .name = "irq_disable",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_irq_disable,
    },
    {
        .name = "irq_enable",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natGPIO_irq_enable,
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

int init_nat_gpio(void)
{
    return 0;
}
