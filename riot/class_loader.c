#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <uJ/uj.h>

#include "class_loader.h"

uint8_t ujReadClassByte(void *userData, uint32_t offset)
{
    uint8_t *data = (uint8_t*)userData;
    return data[offset];
}

int loadPackedUjcClasses(unsigned char *data, UjClass **mainClass)
{
    int i, sz, done, offset, class_count;

    offset = 0;
    class_count = 0;
    for (;;)
    {
        sz  = data[offset++] << 16;
        sz |= data[offset++] <<  8;
        sz |= data[offset++] <<  0;

        if (!sz)
            break;

        offset += sz;
        class_count += 1;
    }

    uint8_t class_loaded[class_count];
    memset(class_loaded, 0, class_count * sizeof(uint8_t));

    do
    {
        done = 0;

        for (i = 0, offset = 0; i < class_count; i++, offset += sz)
        {
            sz  = data[offset++] << 16;
            sz |= data[offset++] <<  8;
            sz |= data[offset++] <<  0;

            if (!sz)
                break;

            if (class_loaded[i])
                continue;

            int res = ujLoadClass(data + offset, i ? NULL : mainClass);

            if (res == UJ_ERR_NONE)
            {
                class_loaded[i] = 1;
                done = 1;
            }
            else if (res != UJ_ERR_DEPENDENCY_MISSING)
            {
                printf("Failed to load class at %d: %d\n", offset, res);
                return res;
            }
        }
    } while(done);

    for (i = 0; i < class_count; i++)
    {
        if (!class_loaded[i])
        {
            printf("Could not resolve all class dependencies.\n");
            return UJ_ERR_DEPENDENCY_MISSING;
        }
    }

    return UJ_ERR_NONE;
}
