#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <vfs.h>
#include <uJ/uj.h>
#include <assert.h>

#ifdef MODULE_NAT_CONSTFS
#include "nat/constfs/nat_constfs.h"
#endif

#include "events.h"
#include "nat_classes.h"
#include "nat_uj.h"


static inline uint8_t rdByte(int fd)
{
    uint8_t res = 0;
    vfs_read(fd, &res, 1);
    return res;
}

uint8_t ujReadClassByte(void *userData, uint32_t offset)
{
    int fd = ((intptr_t)userData) >> 24;
    offset += ((intptr_t)userData) & 0xFFFFFF;
    vfs_lseek(fd, offset, SEEK_SET);
    return rdByte(fd);
}

static int loadPackedUjcClasses(UjClass **mainClass, int *fdP)
{
    int fd = -1;
    int i, sz, done, class_count, offset;

    fd = vfs_open("/main/update.ujcpak", O_RDONLY, 0);
    if (fd >= 0)
    {
        vfs_close(fd);
        printf("Code Update detected, applying.\n");
        vfs_unlink("/main/default.ujcpak");
        if (vfs_rename("/main/update.ujcpak", "/main/default.ujcpak") < 0)
            printf("Update failed.\n");
    }

    fd = vfs_open("/main/default.ujcpak", O_RDONLY, 0);

#ifdef MODULE_NAT_CONSTFS
    if (fd < 0)
    {
        printf("Using builtin source.\n");
        fd = vfs_open(NAT_CONSTFS_DEFAULT_PATH, O_RDONLY, 0);
    }
#endif

    if (fd < 0)
    {
        printf("Failed opening java source.\n");
        return UJ_ERR_INTERNAL;
    }

    // We pack it with the offset into a potentially 32bit pointer, so it can't use more than 8 bits.
    assert(fd <= 0xFF);

    class_count = 0;
    for (;;)
    {
        sz  = rdByte(fd) << 16;
        sz |= rdByte(fd) <<  8;
        sz |= rdByte(fd) <<  0;

        if (!sz)
            break;

        vfs_lseek(fd, sz, SEEK_CUR);
        class_count += 1;
    }

    uint8_t class_loaded[class_count];
    memset(class_loaded, 0, class_count * sizeof(uint8_t));

    do
    {
        done = 0;

        for (i = 0, offset = 0; i < class_count; i++, offset += sz)
        {
            // ujLoadClass _will_ mess up our file position, so we need to keep track manually.
            vfs_lseek(fd, offset, SEEK_SET);

            sz  = rdByte(fd) << 16;
            sz |= rdByte(fd) <<  8;
            sz |= rdByte(fd) <<  0;
            offset += 3;

            if (!sz)
                break;

            if (class_loaded[i])
                continue;

            int res = ujLoadClass((void*)(intptr_t)((fd << 24) | (offset & 0xFFFFFF)), i ? NULL : mainClass);

            if (res == UJ_ERR_NONE)
            {
                class_loaded[i] = 1;
                done = 1;
            }
            else if (res != UJ_ERR_DEPENDENCY_MISSING)
            {
                vfs_close(fd);
                printf("Failed to load class at %d: %d\n", offset, res);
                return res;
            }
        }
    } while(done);

    for (i = 0; i < class_count; i++)
    {
        if (!class_loaded[i])
        {
            vfs_close(fd);
            printf("Could not resolve all class dependencies.\n");
            return UJ_ERR_DEPENDENCY_MISSING;
        }
    }

    *fdP = fd;
    return UJ_ERR_NONE;
}

int run_uj(void)
{
    UjClass *objectClass = NULL;
    UjClass *mainClass = NULL;
    int res, fd = -1;

    init_events();

    res = ujInit(&objectClass);
    if (res != UJ_ERR_NONE)
    {
        printf("ujInit failed: %d\n", res);
        return -1;
    }

    res = register_nat_all(objectClass);
    if (res != UJ_ERR_NONE)
        return -1;

    res = loadPackedUjcClasses(&mainClass, &fd);
    if (res != UJ_ERR_NONE)
    {
        return -1;
    }

    res = ujInitAllClasses();
    if (res != UJ_ERR_NONE)
    {
        vfs_close(fd);
        printf("ujInitAllClasses failed: %d\n", res);
        return -1;
    }

    // Half of the heap will be used as stack
    HANDLE threadH = ujThreadCreate(UJ_HEAP_SZ / 2);
    if (!threadH)
    {
        vfs_close(fd);
        printf("ujThreadCreate failed\n");
        return -1;
    }

    res = ujThreadGoto(threadH, mainClass, "main", "()V");
    if (res != UJ_ERR_NONE)
    {
        vfs_close(fd);
        printf("ujThreadGoto failed: %d\n", res);
        return -1;
    }

    while (ujCanRun()) {
        res = ujInstr();
        if (res != UJ_ERR_NONE)
        {
            vfs_close(fd);
            printf("ujInstr failed: %d\n", res);
            return -1;
        }
    }

    vfs_close(fd);

    printf("Program ended\n");

    return 0;
}
