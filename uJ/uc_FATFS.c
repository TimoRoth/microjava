#include "uc_FATFS.h"

#define FATFS_FLAG_VOLUME_LABEL	8
#define FATFS_FLAG_DEVICE		64
#define FATFS_FLAG_RESERVED		128
#define FATFS_FLAG_LFN			(FATFS_FLAG_VOLUME_LABEL | FATFS_FLAG_READONLY | FATFS_FLAG_HIDDEN | FATFS_FLAG_SYSTEM)

#define EOC_16					0xfff8
#define EOC_12					0xff8
#define CLUS_INVALID				0xffff

//fat16 only, very very limited

static UInt32 diskOffset = 0;		//to beginning of fs
static UInt8 secPerClus;
static UInt16 rootDirEntries;
static UInt16 sectorsPerFat;
static UInt16 fatSec;		//where fat begin
static UInt16 rootSec;		//where root directory begins
static UInt16 dataSec;		//where data begins
static UInt16 curClus = CLUS_INVALID;



static Boolean fatfsParsePartitionTable(void){

	UInt8 record[16];
	UInt16 offset;

	if(diskOffset) return false;	//partitions inside partitions do not exist, probbay no fat FS on this disk - bail out

	for(offset = 0x1BE; offset < 0x1FE; offset += 16){

		if(!fatfsExtRead(0, offset, 16, record)) return false;
		if(record[4] != 1 && record[4] != 4 && record[4] != 6 && record[4] != 0x0B && record[4] != 0x0C && record[4] != 0x0E) continue;	//not FAT parition

		//we now have a contender - try to mount it
		diskOffset = record[11];
		diskOffset = (diskOffset << 8) | record[10];
		diskOffset = (diskOffset << 8) | record[9];
		diskOffset = (diskOffset << 8) | record[8];
		if(fatfsMount()) return true;
	}
	//if we got here, we failed - give up and cry
	return false;
}

static UInt16 fatfsGetU16(UInt8* v, UInt8 idx){

	v += idx;
	return (((UInt16)v[1]) << 8) | ((UInt16)v[0]);
}

static UInt32 fatfsGetU32(UInt8* v, UInt8 idx){

	v += idx;
	return (((UInt32)v[3]) << 24UL) | (((UInt32)v[2]) << 16UL) | (((UInt32)v[1]) << 8UL) | ((UInt32)v[0]);
}

void fatfsInit(void){

	diskOffset = 0;
}

Boolean fatfsMount(void){

	UInt8 buf[13];

	if(!fatfsExtRead(diskOffset, 0x36, 4, buf)) return false;
	if(buf[0] !='F' || buf[1] !='A' || buf[2] != 'T' || buf[3] != '1'){	//may be a partition table

		return fatfsParsePartitionTable();
	}

	if(!fatfsExtRead(diskOffset, 0x0B, 13, buf)) return false;
	if(fatfsGetU16(buf, 0x0B - 0x0B) != 512) return false;		//only 512 bytes/sector FSs supported
	secPerClus = buf[0x0D - 0x0B];
	fatSec = fatfsGetU16(buf, 0x0E - 0x0B);	//"reserved sectors" = sectors before first fat
	rootDirEntries = fatfsGetU16(buf, 0x11 - 0x0B);
	sectorsPerFat = fatfsGetU16(buf, 0x16 - 0x0B);
	
	rootSec = fatSec + sectorsPerFat * (UInt16)(buf[0x10 - 0x0B]);
	dataSec = rootSec + (((UInt32)rootDirEntries) * 32 + FATFS_DISK_SECTOR_SZ - 1) / FATFS_DISK_SECTOR_SZ;

	return true;
}

Boolean fatfsGetNthFile(UInt16 n, char* name, UInt32* sz, UInt8* flags, UInt16* id){

	UInt16 i;
	UInt32 sec = diskOffset + rootSec;
	UInt16 offset = 0;
	UInt8 buf[4];

	for(i = 0; i < rootDirEntries; i++){

		if(!fatfsExtRead(sec, offset, 1, buf)) return false;
		if(buf[0] == 0) break;	//no more entries
		if(buf[0] != 0xE5 && buf[0] != 0x2E){		//we process only non-deleted, non "." and ".." entries

			if(name){

				name[0] = (buf[0] == 0x05) ? 0xE5 : buf[0];
				if(!fatfsExtRead(sec, offset + 1, 10, (UInt8*)(name + 1))) return false;
			}
			
			if(!fatfsExtRead(sec, offset + 0x0B, 1, buf)) return false;
			
			if(buf[0] != FATFS_FLAG_LFN){
				
				if(!n--){		//we found it
				
					if(flags){
	
						*flags = buf[0];
					}
	
					if(id){
	
						if(!fatfsExtRead(sec, offset + 0x1A, 2, buf)) return false;
						*id = fatfsGetU16(buf, 0);
					}
	
					if(sz){
	
						if(!fatfsExtRead(sec, offset + 0x1C, 4, buf)) return false;
						*sz = fatfsGetU32(buf, 0);
					}
	
					return true;
				}
			}
		}
		offset += 32;
		if(offset == FATFS_DISK_SECTOR_SZ){
			offset = 0;
			sec++;
		}
	}

	//we fail
	return false;
}

Boolean fatfsOpen(UInt16 id){

	curClus = id;
	return true;
}

UInt16 fatfsGetNextClus(UInt16 clus){

	UInt8 buf[2];
	UInt32 sec = diskOffset + fatSec;
	UInt16 offset;

	sec += clus / (FATFS_DISK_SECTOR_SZ / 2);
	offset = (clus % (FATFS_DISK_SECTOR_SZ / 2)) * 2;

	if(!fatfsExtRead(sec, offset, 2, buf)) return CLUS_INVALID;

	clus = fatfsGetU16(buf, 0);
	if(clus >= EOC_16) return CLUS_INVALID;

	return clus;
}
Boolean fatfsGetNextSectorRange(UInt32* first, UInt32* len, Boolean* lastP){

	UInt16 next = curClus, prev;
	UInt32 t;


	if(curClus == CLUS_INVALID) return false;

	do{

		prev = next;
		next = fatfsGetNextClus(prev);
	}while(next == prev + 1 && next != CLUS_INVALID);

	//prev is now the last cluster in this chain that is in sequence with previous ones
	//next is now the next cluster (not in sequence - fragment)

	t = prev + 1 - curClus;
	t *= secPerClus;
	*len = t;

	t = (curClus - 2);
	t *= secPerClus;
	t += dataSec;
	t += diskOffset;
	*first = t;

	curClus = next;
	if(lastP) *lastP = curClus == CLUS_INVALID;

	return true;
}

