#include <stdio.h>
#include "fs/constfs.h"
#include "nat_constfs.h"

#include "java/Default.inc"


static constfs_file_t constfs_files[] = {
    {
        .path = "/" NAT_CONSTFS_DEFAULT_FILE,
        .size = sizeof(ujc_Default),
        .data = ujc_Default,
    },
};

/* this is the constfs specific descriptor */
static constfs_t constfs_desc = {
    .nfiles = sizeof(constfs_files) / sizeof(constfs_files[0]),
    .files = constfs_files,
};

static vfs_mount_t const_mount = {
    .fs = &constfs_file_system,
    .mount_point = "/const",
    .private_data = &constfs_desc,
};

int init_nat_constfs(void)
{
    int res = vfs_mount(&const_mount);
    if (res < 0) {
        printf("Failed mounting constfs.");
        return res;
    }

    return 0;
}
