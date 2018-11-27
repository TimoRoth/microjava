#include <periph_conf.h>
#include <periph/spi.h>

#include "nat_spi.h"


static uint8_t natSPI_dev(UjThread* t, UjClass* cls)
{
    (void)cls;

    int32_t input = ujThreadPop(t);

    spi_t dev = SPI_DEV(input);

    if (!ujThreadPush(t, (int32_t)dev, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natSPI_hwcs(UjThread* t, UjClass* cls)
{
    (void)cls;

    int32_t input = ujThreadPop(t);

    spi_cs_t dev = SPI_HWCS(input);
    (void)input; // SPI_HWCS might be a no-op

    if (!ujThreadPush(t, (dev == SPI_CS_UNDEF) ? ((int32_t)-1) : ((int32_t)dev), false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static inline spi_t conv_spi_dev(int32_t in)
{
    return (spi_t)in;
}

static inline spi_cs_t conv_spi_cs(int32_t in)
{
    if (in == -1)
        return SPI_CS_UNDEF;
    return (spi_cs_t)in;
}

static inline spi_mode_t conv_spi_mode(int32_t in)
{
    switch (in)
    {
        case 0:
            return SPI_MODE_0;
        case 1:
            return SPI_MODE_1;
        case 2:
            return SPI_MODE_2;
        case 3:
            return SPI_MODE_3;
        default:
            printf("Invalid spi mode\n");
            return -1;
    }
}

static inline spi_clk_t conv_spi_clk(int32_t in)
{
    switch (in)
    {
        case 0:
            return SPI_CLK_100KHZ;
        case 1:
            return SPI_CLK_400KHZ;
        case 2:
            return SPI_CLK_1MHZ;
        case 3:
            return SPI_CLK_5MHZ;
        case 4:
            return SPI_CLK_10MHZ;
        default:
            printf("Invalid spi clk\n");
            return -1;
    }
}

static uint8_t natSPI_init(UjThread* t, UjClass* cls)
{
    (void)cls;

    spi_t bus = conv_spi_dev(ujThreadPop(t));

    spi_init(bus);

    return UJ_ERR_NONE;
}

static uint8_t natSPI_init_pins(UjThread* t, UjClass* cls)
{
    (void)cls;

    spi_t bus = conv_spi_dev(ujThreadPop(t));

    spi_init_pins(bus);

    return UJ_ERR_NONE;
}

static uint8_t natSPI_init_cs(UjThread* t, UjClass* cls)
{
    (void)cls;

    spi_cs_t cs = conv_spi_cs(ujThreadPop(t));
    spi_t bus = conv_spi_dev(ujThreadPop(t));

    int res = spi_init_cs(bus, cs);

    if (!ujThreadPush(t, res == SPI_OK, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natSPI_acquire(UjThread* t, UjClass* cls)
{
    (void)cls;

    spi_clk_t clk = conv_spi_clk(ujThreadPop(t));
    spi_mode_t mode = conv_spi_mode(ujThreadPop(t));
    spi_cs_t cs = conv_spi_cs(ujThreadPop(t));
    spi_t bus = conv_spi_dev(ujThreadPop(t));

    int res = spi_acquire(bus, cs, mode, clk);

    if (!ujThreadPush(t, res == SPI_OK, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natSPI_release(UjThread* t, UjClass* cls)
{
    (void)cls;

    spi_t bus = conv_spi_dev(ujThreadPop(t));

    spi_release(bus);

    return UJ_ERR_NONE;
}

static uint8_t natSPI_transfer_byte(UjThread* t, UjClass* cls)
{
    (void)cls;

    uint8_t out = ujThreadPop(t);
    bool cont = ujThreadPop(t);
    spi_cs_t cs = conv_spi_cs(ujThreadPop(t));
    spi_t bus = conv_spi_dev(ujThreadPop(t));

    uint8_t res = spi_transfer_byte(bus, cs, cont, out);

    if (!ujThreadPush(t, res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natSPI_transfer_bytes(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE arr = ujThreadPop(t);
    bool cont = ujThreadPop(t);
    spi_cs_t cs = conv_spi_cs(ujThreadPop(t));
    spi_t bus = conv_spi_dev(ujThreadPop(t));

    HANDLE resArr = 0;

    if (!arr)
        goto err;

    uint32_t len = ujArrayLen(arr);
    uint8_t res = ujArrayNew('B', len, &resArr);
    if (res != UJ_ERR_NONE)
        return res;

    void *out = ujArrayRawAccessStart(arr);
    void *in = ujArrayRawAccessStart(resArr);

    spi_transfer_bytes(bus, cs, cont, out, in, len);

    ujArrayRawAccessFinish(resArr);
    ujArrayRawAccessFinish(arr);

err:
    if (!ujThreadPush(t, resArr, !!resArr))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natSPI_transfer_reg(UjThread* t, UjClass* cls)
{
    (void)cls;

    uint8_t out = ujThreadPop(t);
    uint8_t reg = ujThreadPop(t);
    spi_cs_t cs = conv_spi_cs(ujThreadPop(t));
    spi_t bus = conv_spi_dev(ujThreadPop(t));

    uint8_t res = spi_transfer_reg(bus, cs, reg, out);

    if (!ujThreadPush(t, res, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natSPI_transfer_regs(UjThread* t, UjClass* cls)
{
    (void)cls;

    HANDLE arr = ujThreadPop(t);
    uint8_t reg = ujThreadPop(t);
    spi_cs_t cs = conv_spi_cs(ujThreadPop(t));
    spi_t bus = conv_spi_dev(ujThreadPop(t));

    HANDLE resArr = 0;

    if (!arr)
        goto err;

    uint32_t len = ujArrayLen(arr);
    uint8_t res = ujArrayNew('B', len, &resArr);
    if (res != UJ_ERR_NONE)
        return res;

    void *out = ujArrayRawAccessStart(arr);
    void *in = ujArrayRawAccessStart(resArr);

    spi_transfer_regs(bus, cs, reg, out, in, len);

    ujArrayRawAccessFinish(resArr);
    ujArrayRawAccessFinish(arr);

err:
    if (!ujThreadPush(t, resArr, !!resArr))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static const UjNativeMethod nativeCls_SPI_methods[] = {
    {
        .name = "dev",
        .type = "(I)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_dev,
    },
    {
        .name = "hwcs",
        .type = "(I)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_hwcs,
    },
    {
        .name = "init",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_init,
    },
    {
        .name = "init_pins",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_init_pins,
    },
    {
        .name = "init_cs",
        .type = "(II)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_init_cs,
    },
    {
        .name = "acquire",
        .type = "(IIII)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_acquire,
    },
    {
        .name = "release",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_release,
    },
    {
        .name = "transfer_byte",
        .type = "(IIZB)B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_transfer_byte,
    },
    {
        .name = "transfer_bytes",
        .type = "(IIZ[B)[B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_transfer_bytes,
    },
    {
        .name = "transfer_reg",
        .type = "(IIIB)B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_transfer_reg,
    },
    {
        .name = "transfer_regs",
        .type = "(III[B)[B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natSPI_transfer_regs,
    },
};

static const UjNativeClass nativeCls_SPI =
{
    .clsName = "SPI",

    .clsDatSz = 0,
    .instDatSz = 0,
    .gcClsF = NULL,
    .gcInstF = NULL,

    .numMethods = sizeof(nativeCls_SPI_methods) / sizeof(nativeCls_SPI_methods[0]),
    .methods = nativeCls_SPI_methods,
};

int register_nat_spi(UjClass *objectClass)
{
    int res = ujRegisterNativeClass(&nativeCls_SPI, objectClass, NULL);
    if (res != UJ_ERR_NONE)
        printf("Failed registering SPI native class.\n");

    return res;
}

int init_nat_spi(void)
{
    return 0;
}
