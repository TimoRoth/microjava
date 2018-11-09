#include "common.h"
#include "uc_FATFS.h"
#include "uc_SDMMC.h"
#include "uc_loader.h"


#define FAIL_TYPE_OVER_SIZE_LIMIT	0x81	// X------X
#define FAIL_TYPE_NO_CLASS_FILES	0x83	// X-----XX
#define FAIL_FLASH_WRITE		0x86	// X----XX-
#define FAIL_FILE_OPEN			0x87	// X----XXX
#define FAIL_SD_READ			0x88	// X---X---
#define FAIL_FILE_FIND_CLUSTER		0x8A	// X---X-X-
#define FAIL_FILE_FRAGMENTED		0x8F	// X---XXXX
#define FAIL_TOO_MAIN_MAINS		0x91	// X--X---X
#define FAIL_TOO_FEW_MAINS		0x93	// X--X--XX


static UInt32 gCurSec = 0xFFFFFFFFUL;
static UInt16 gCurPos = 0;


void fatfsExtFinish(){
	
	if(gCurSec != 0xFFFFFFFFUL){
	
		while(gCurPos++ - 512) spiByte(0xFF);
		sdSecReadStop();
		gCurSec = 0xFFFFFFFF;
	}
}

Boolean fatfsExtRead(UInt32 sector, UInt16 offset, UInt8 len, UInt8* buf){

	if(sector != gCurSec || offset < gCurPos){

		if(gCurSec != 0xFFFFFFFFUL){

			while(gCurPos++ != 512) spiByte(0xFF);	//fast forward to sector end
			sdSecReadStop();
			gCurSec = 0xFFFFFFFFUL;
		}
		if(!sdReadStart(gCurSec = sector)) return false;
		gCurPos = 0;
	}

	while(gCurPos != offset){	//skip to where we're needed
		gCurPos++;
		spiByte(0xFF);
	}

	gCurPos += len;
	while(len--) *buf++ = spiByte(0xFF);
	
	if(gCurPos == 512){
		sdSecReadStop();
		gCurSec = 0xFFFFFFFFUL;
	}

	return true;
}

static Boolean writeByte(UInt8 byte, UInt8* buf, UInt16* bufPosP, UInt32* pageAddrP, UInt16* pagesLeftP){
	
	UInt32 page = (*pageAddrP) &~ (FLASHROM_PAGE - 1);
	UInt16 i, bufPos = *bufPosP;
	
	
	if(*pagesLeftP == 0) return false;
	
	buf[bufPos++] = byte;
	
	if(bufPos == FLASHROM_PAGE){
	
		for(i = 0; i < FLASHROM_PAGE; i++) if(pgm_read(page + i) != buf[i]) break;
	
		if(i != FLASHROM_PAGE){
			
			flashromWrite(page, buf, FLASH_KEY);
		}
		
		bufPos = 0;
		(*pagesLeftP)--;
	}
	
	*bufPosP = bufPos;
	(*pageAddrP)++;

	return true;
}

static void putline(UInt32 addr, UInt8 line){

	#ifdef HAVE_CHAR_LCD
		clcdGotoXY(0, line);
		for(line = 0; line < 16; line++) clcdChar(pgm_read_str(addr + line));
	#endif
}


UInt32 uc_loader(void){	//return actual area size

	UInt32 addr, size;
	static const char _PROGMEM_ lineX[16] = "  POWERING  UP  ";
	
	//flash writerState state
	UInt8 buf[FLASHROM_PAGE];
	UInt16 bufPos = 0;
	UInt32 pageAddr;
	UInt16 pagesLeft;

	addr = getFlashFreeArea(&size);
	pagesLeft = size / FLASHROM_PAGE;
	
	
redo_loader:

	#ifdef BOARD_DIANA	//special case for SD flakyness
		putline(GET_ADDRESS(lineX), 0);
		_delay_ms(500);
	#endif

	if(pgm_read(addr) == 0xFF){
	
		static const char _PROGMEM_ line1[16] = " No code found! ";
		static const char _PROGMEM_ line2[16] = " insert SD now. ";
		
		putline(GET_ADDRESS(line1), 0);
		putline(GET_ADDRESS(line2), 1);
		
		while(!sdInit());	
	}
	
	//then bootload if needed
	if(sdInit()){
		
		fatfsInit();
		
		if(fatfsMount()){
			
			UInt16 n;
			UInt8 i;
			
			pageAddr = addr;
			
			
			{
				UInt32 sz = size;
				
				static const char _PROGMEM_ line1[16] = "SD->uC in X sec.";
				static const char _PROGMEM_ line2[16] = "        b avail.";
				static const char _PROGMEM_ line3[16] = "   LOADING...   ";
				
				putline(GET_ADDRESS(line1), 0);
				putline(GET_ADDRESS(line2), 1);
		
				for(i = 7; i; i--){
					
					#ifdef HAVE_CHAR_LCD
						clcdGotoXY(i, 1);
						clcdChar('0' + (sz % 10));
					#endif
					sz /= 10;
					if(!sz) break;
				}
				
				for(i = 5; i; i--){
				
					UInt32 t;
					
					#ifdef HAVE_CHAR_LCD
						clcdGotoXY(10, 0);
						clcdChar('0' + i);
					#endif
					t = getClock();
					while(getClock() - t < getClkTicksPerSec());
				}
				
				#ifdef HAVE_CHAR_LCD
					clcdClear();
				#endif
				putline(GET_ADDRESS(line3), 0);
			}
			
			
			
			//list files and verify we have some
			{
				char name[11];
				UInt32 sz, first, num;
				Boolean last;
				UInt16 id;
				UInt8 flags;
				UInt8 numMains = 0, wantedFlags = FATFS_FLAG_READONLY;
				
				
				for(i = 0; i < 2; i++, wantedFlags = 0){	//two passes
					
					
					n = 0;
					while(fatfsGetNthFile(n, name, &sz, &flags, &id)){
						
						if((flags & (FATFS_FLAG_DIR | FATFS_FLAG_READONLY)) == wantedFlags){
							
							if(((name[8] == 'C' && name[9] == 'L' && name[10] == 'A') || (name[8] == 'U' && name[9] == 'J' && name[10] == 'C')) && sz){
											
								UInt16 left;
								
								#ifdef HAVE_CHAR_LCD
									{
										UInt8 k;
										
										clcdGotoXY(0, 1);
										clcdChar('>');
										for(k = 0; k < 11; k++) clcdChar(name[k]);
									}
								#endif
		
								if(!writeByte(sz >> 16, buf, &bufPos, &pageAddr, &pagesLeft)) fail_msg(FAIL_FLASH_WRITE);
								if(!writeByte(sz >> 8, buf, &bufPos, &pageAddr, &pagesLeft)) fail_msg(FAIL_FLASH_WRITE);
								if(!writeByte(sz, buf, &bufPos, &pageAddr, &pagesLeft)) fail_msg(FAIL_FLASH_WRITE);
								
								if(!fatfsOpen(id)) fail_msg(FAIL_FILE_OPEN);
								if(!fatfsGetNextSectorRange(&first, &num, &last)) fail_msg(FAIL_FILE_FIND_CLUSTER);
								if(!last) fail_msg(FAIL_FILE_FRAGMENTED);
								
								fatfsExtFinish();
								if(!sdReadStart(first)) fail_msg(FAIL_SD_READ);
								
								left = SD_BLOCK_SIZE;
								while(1){
								
									UInt16 todo = sz > SD_BLOCK_SIZE ? SD_BLOCK_SIZE : sz;
									
									while(todo--){
									
										if(!writeByte(spiByte(0xFF), buf, &bufPos, &pageAddr, &pagesLeft)) fail_msg(FAIL_FLASH_WRITE);
										sz--;
										left--;
									}
									if(!sz) break;
									sdNextSec();
									left = SD_BLOCK_SIZE;
								}
								while(left--) spiByte(0xFF);
								sdSecReadStop();
								
								#ifdef HAVE_CHAR_LCD
									{
										UInt8 k;
										
										clcdGotoXY(0, 1);
										clcdChar('!');
										for(k = 0; k < 11; k++) clcdChar(name[k]);
									}
								#endif
								
								if(i == 0) numMains++;
							}
						}
						n++;
					}
					
					if(numMains > 1) fail_msg(FAIL_TOO_MAIN_MAINS);
					if(numMains < 1) fail_msg(FAIL_TOO_FEW_MAINS);
				}
				
				if(pageAddr == addr) fail_msg(FAIL_TYPE_NO_CLASS_FILES);
				
				//terminate the list
				for(n = 0; n < 3; n++) if(!writeByte(0, buf, &bufPos, &pageAddr, &pagesLeft)) fail_msg(FAIL_FLASH_WRITE);
				
				#ifdef HAVE_CHAR_LCD
					{
						UInt8 k;
						
						clcdGotoXY(0, 1);
						clcdChar('T');
						for(k = 0; k < 11; k++) clcdChar(' ');
					}
				#endif
				//terminate the flash page
				while(pageAddr & (FLASHROM_PAGE-1)) if(!writeByte(0xFF, buf, &bufPos, &pageAddr, &pagesLeft)) fail_msg(FAIL_FLASH_WRITE);
			}
		}
	}
	if(pgm_read(addr) == 0xFF) goto redo_loader;
	
	clcdClear();
	return addr;
}





