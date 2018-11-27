#include <periph_conf.h>
#include <periph/i2c.h>

#include "nat_i2c.h"
#include "events.h"


static uint8_t natI2C_dev(UjThread* t, UjClass* cls)
{
    (void)cls;

    int32_t input = ujThreadPop(t);

    i2c_t dev = I2C_DEV(input);

    if (!ujThreadPush(t, (int32_t)dev, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_init(UjThread* t, UjClass* cls)
{
    (void)cls;

    i2c_t dev = (int32_t)ujThreadPop(t);

    i2c_init(dev);

    return UJ_ERR_NONE;
}

static uint8_t natI2C_acquire(UjThread* t, UjClass* cls)
{
    (void)cls;

    i2c_t dev = (int32_t)ujThreadPop(t);

    int res = i2c_acquire(dev);

    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_release(UjThread* t, UjClass* cls)
{
    (void)cls;

    i2c_t dev = (int32_t)ujThreadPop(t);

    int res = i2c_release(dev);

    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static inline uint8_t cvt_flags_val(int val)
{
    uint8_t res = 0;

    if (val & 1)
        res |= I2C_ADDR10;
    if (val & 2)
        res |= I2C_REG16;
    if (val & 4)
        res |= I2C_NOSTOP;
    if (val & 8)
        res |= I2C_NOSTART;

    return res;
}

static uint8_t natI2C_read_reg(UjThread* t, UjClass* cls)
{
    (void)cls;

    int32_t flags = cvt_flags_val(ujThreadPop(t));
    int32_t reg = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);

    int8_t data;
    int res = i2c_read_reg(dev, addr, reg, &data, flags);

    if (!ujThreadPush(t, res ? 0 : data, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_read_regs(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    int32_t len = ujThreadPop(t);
    int32_t reg = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);

    HANDLE arr = 0;
    int ret = ujArrayNew('B', len, &arr);
    if (ret != UJ_ERR_NONE)
        return ret;

    int8_t *data = ujArrayRawAccessStart(arr);
    int res = i2c_read_regs(dev, addr, reg, data, len, flags);
    ujArrayRawAccessFinish(arr);

    if (!ujThreadPush(t, res ? 0 : arr, res ? false : true))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_write_reg(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    int8_t data = ujThreadPop(t);
    int32_t reg = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);

    int res = i2c_write_reg(dev, addr, reg, data, flags);

    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_write_regs(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    HANDLE arr = ujThreadPop(t);
    int32_t reg = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);
    int res = -1;

    if (!arr)
        goto err;

    uint32_t len = ujArrayLen(arr);

    if (!len)
        goto err;

    int8_t *data = ujArrayRawAccessStart(arr);
    res = i2c_write_regs(dev, addr, reg, data, len, flags);
    ujArrayRawAccessFinish(arr);

err:
    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_read_byte(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);

    int8_t data;
    int res = i2c_read_byte(dev, addr, &data, flags);

    if (!ujThreadPush(t, res ? 0 : data, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_read_bytes(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    int32_t len = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);

    HANDLE arr = 0;
    int ret = ujArrayNew('B', len, &arr);
    if (ret != UJ_ERR_NONE)
        return ret;

    int8_t *data = ujArrayRawAccessStart(arr);
    int res = i2c_read_bytes(dev, addr, data, len, flags);
    ujArrayRawAccessFinish(arr);

    if (!ujThreadPush(t, res ? 0 : arr, res ? false : true))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_write_byte(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    int8_t data = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);

    int res = i2c_write_byte(dev, addr, data, flags);

    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static uint8_t natI2C_write_bytes(UjThread* t, UjClass* cls)
{
    (void)cls;

    int flags = cvt_flags_val(ujThreadPop(t));
    HANDLE arr = ujThreadPop(t);
    int32_t addr = ujThreadPop(t);
    i2c_t dev = (int32_t)ujThreadPop(t);
    int res = -1;

    if (!arr)
        goto err;

    uint32_t len = ujArrayLen(arr);

    if (!len)
        goto err;

    int8_t *data = ujArrayRawAccessStart(arr);
    res = i2c_write_bytes(dev, addr, data, len, flags);
    ujArrayRawAccessFinish(arr);

err:
    if (!ujThreadPush(t, res ? false : true, false))
        return UJ_ERR_STACK_SPACE;

    return UJ_ERR_NONE;
}

static const UjNativeMethod nativeCls_I2C_methods[] = {
    {
        .name = "dev",
        .type = "(I)I",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_dev,
    },
    {
        .name = "init",
        .type = "(I)V",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_init,
    },
    {
        .name = "acquire",
        .type = "(I)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_acquire,
    },
    {
        .name = "release",
        .type = "(I)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_release,
    },
    {
        .name = "read_reg",
        .type = "(IIII)B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_read_reg,
    },
    {
        .name = "read_regs",
        .type = "(IIIII)[B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_read_regs,
    },
    {
        .name = "write_reg",
        .type = "(IIIBI)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_write_reg,
    },
    {
        .name = "write_regs",
        .type = "(III[BI)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_write_regs,
    },
    {
        .name = "read_byte",
        .type = "(III)B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_read_byte,
    },
    {
        .name = "read_bytes",
        .type = "(IIII)[B",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_read_bytes,
    },
    {
        .name = "write_byte",
        .type = "(IIBI)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_write_byte,
    },
    {
        .name = "write_bytes",
        .type = "(II[BI)Z",
        .flags = JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC,

        .func = natI2C_write_bytes,
    },
};

static const UjNativeClass nativeCls_I2C =
{
    .clsName = "I2C",

    .clsDatSz = 0,
    .instDatSz = 0,
    .gcClsF = NULL,
    .gcInstF = NULL,

    .numMethods = sizeof(nativeCls_I2C_methods) / sizeof(nativeCls_I2C_methods[0]),
    .methods = nativeCls_I2C_methods,
};

int register_nat_i2c(UjClass *objectClass)
{
    int res = ujRegisterNativeClass(&nativeCls_I2C, objectClass, NULL);
    if (res != UJ_ERR_NONE)
        printf("Failed registering I2C native class.\n");

    return res;
}

int init_nat_i2c(void)
{
    return 0;
}
