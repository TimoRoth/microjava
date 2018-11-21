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

int init_nat_gcoap(void)
{
    return 0;
}
