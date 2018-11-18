#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include <uJ/uj.h>

#include "nat_classes.h"
#include "class_loader.h"

extern unsigned char ujc_Example[];

void ujLog(const char *fmtStr, ...)
{
    va_list va;

    va_start(va, fmtStr);
    vfprintf(stdout, fmtStr, va);
    fflush(stdout);
    va_end(va);
}



int main(void)
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

    res = loadPackedUjcClasses(ujc_Example, &mainClass);
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
