#include <uJ/uj.h>

#ifdef MODULE_NAT_GPIO
#include "nat/gpio/nat_gpio.h"
#endif
#ifdef MODULE_NAT_I2C
#include "nat/i2c/nat_i2c.h"
#endif
#ifdef MODULE_NAT_GCOAP
#include "nat/gcoap/nat_gcoap.h"
#endif

#include "nat_classes.h"

int register_nat_all(UjClass *objectClass)
{
    int res = register_nat_riot(objectClass);
    if (res != UJ_ERR_NONE)
        return res;

#ifdef MODULE_NAT_GPIO
    res = register_nat_gpio(objectClass);
    if (res != UJ_ERR_NONE)
        return res;
#endif

#ifdef MODULE_NAT_I2C
    res = register_nat_i2c(objectClass);
    if (res != UJ_ERR_NONE)
        return res;
#endif

#ifdef MODULE_NAT_GCOAP
    res = register_nat_gcoap(objectClass);
    if (res != UJ_ERR_NONE)
        return res;
#endif

    return res;
}