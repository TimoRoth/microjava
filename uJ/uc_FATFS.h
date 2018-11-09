#ifndef _UC_FATFS_H_
#define _UC_FATFS_H_

#include "common.h"

#define FATFS_DISK_SECTOR_SZ		512

#define FATFS_FLAG_READONLY		1
#define FATFS_FLAG_HIDDEN		2
#define FATFS_FLAG_SYSTEM		4
#define FATFS_FLAG_DIR			16
#define FATFS_FLAG_ARCHIVE		32

//externally required function(s)
Boolean fatfsExtRead(UInt32 sector, UInt16 offset, UInt8 len, UInt8* buf);

void fatfsInit(void);													//init fs driver
Boolean fatfsMount(void);													//try mounting a volume
Boolean fatfsGetNthFile(UInt16 n, char* name, UInt32* sz, UInt8* flags, UInt16* id);	//in root directory only, false for no more
Boolean fatfsOpen(UInt16 id);													//in root directory only
Boolean fatfsGetNextSectorRange(UInt32* first, UInt32* len, Boolean* lastP);						//for currently opened file, false for "no more"

#endif
