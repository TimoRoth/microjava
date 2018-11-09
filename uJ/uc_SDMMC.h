#ifndef _UC_SDMMC_H_
#define _UC_SDMMC_H_

#include "common.h"


#define SD_BLOCK_SIZE		512

Boolean sdInit();

Boolean sdReadStart(UInt24 sec);
void sdNextSec();
void sdSecReadStop();



#endif
