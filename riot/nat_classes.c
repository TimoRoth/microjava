#include <uJ/uj.h>

#include "nat_classes.h"

int register_nat_all(UjClass *objectClass)
{
    int res = register_nat_riot(objectClass);
    if (res != UJ_ERR_NONE)
        return res;

    res = register_nat_gpio(objectClass);
    if (res != UJ_ERR_NONE)
        return res;

    return res;
}