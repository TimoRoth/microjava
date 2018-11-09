#include "uc_SDMMC.h"

#define FLAG_TIMEOUT		0x80
#define FLAG_PARAM_ERR		0x40
#define FLAG_ADDR_ERR		0x20
#define FLAG_ERZ_SEQ_ERR	0x10
#define FLAG_CMD_CRC_ERR	0x08
#define FLAG_ILLEGAL_CMD	0x04
#define FLAG_ERZ_RST		0x02
#define FLAG_IN_IDLE_MODE	0x01


/*
	this is a very simplified SD/MMC diver [which is adapted from PowerSDHC sources :) ]
	it supports reads in stream mode. i removed support for block reads/writes and SDHC/MMCplus support since it is not needed.
	also removed is ability to read card size since it is also unused (it saves us 300 instructions)

*/

#define sdChipSelect(/*Boolean*/ active) 	//NO NEED! CS is active low on cards, btw



static UInt8 sdCrc7(UInt8* chr,UInt8 cnt,UInt8 crc){

	UInt8 i, a;
	UInt8 Data;

	for(a = 0; a < cnt; a++){
		
		Data = chr[a];
		
		for(i = 0; i < 8; i++){
			
			crc <<= 1;

			if( (Data & 0x80) ^ (crc & 0x80) ) crc ^= 0x09;
			
			Data <<= 1;
		}
	}
	
	return crc & 0x7F;
}

static inline void sdPrvSendCmd(UInt8 cmd, UInt32 param){
	
	UInt8 send[6];
	
	send[0] = cmd | 0x40;
	send[1] = param >> 24;
	send[2] = param >> 16;
	send[3] = param >> 8;
	send[4] = param;
	send[5] = (sdCrc7(send, 5, 0) << 1) | 1;
	
	for(cmd = 0; cmd < sizeof(send); cmd++){
		spiByte(send[cmd]);
	}
}

static inline UInt8 sdPrvReadResp(void){	//if return has FLAG_TIMEOUT bit_ set, we timed out
	
	UInt8 v, i = 0;
	
	do{		//our max wait time is 128 byte clocks (1024 clock ticks)
		
		v = spiByte(0xFF);
		
	}while(i++ < 128 && (v == 0xFF));
	
	return v;
}

static UInt8 sdPrvSimpleCommand(UInt8 cmd, UInt32 param, Boolean cmdDone){	//do a command, return R1 reply
	
	UInt8 ret;
	
	sdChipSelect(true);
	spiByte(0xFF);
	sdPrvSendCmd(cmd, param);
	ret = sdPrvReadResp();
	if(cmdDone){
		
		sdChipSelect(false);
	}
	
	return ret;
}

static UInt8 sdPrvACMD(UInt8 cmd, UInt32 param){
	
	UInt8 ret;
	
	ret = sdPrvSimpleCommand(55, 0, true);
	if(ret & FLAG_TIMEOUT) return ret;
	if(ret & FLAG_ILLEGAL_CMD) return ret;
	
	return sdPrvSimpleCommand(cmd, param, true);
}

Boolean sdPrvCardInit(Boolean sd){
	
	UInt16 time = 0;
	UInt8 resp;
	Boolean first = true;
	UInt32 param;
	
	param = 0;
	
	while(time++ < 500UL){	//retry 10..0 times
	
		resp = sd ? sdPrvACMD(41, param) : sdPrvSimpleCommand(1, param, true);
		
		if(resp & FLAG_TIMEOUT) break;
		
		if(first){
			
			param |= 0x00200000UL;
			first = false;
		}
		else{
			
			if(!(resp & FLAG_IN_IDLE_MODE)) return true;
		}
	}

	return false;
}

UInt32 sdPrvGetBits(UInt8* data, UInt8 numBytesInArray, UInt8 startBit, UInt8 len){//for CID and CSD data..
	
	UInt8 bitWrite = 0;
	UInt8 numBitsInArray = numBytesInArray * 8;
	UInt32 ret = 0;
	
	do{
		
		UInt8 bit_,byte;
		
		bit_ = numBitsInArray - startBit - 1;
		byte = bit_ >> 3;
		bit_ = 7 - (bit_ & 7);
		
		ret |= ((data[byte] >> bit_) & 1) << (bitWrite++);
		
		startBit++;
	}while(--len);
	
	return ret;
}

Boolean sdInit(){
	
	UInt8 v, tries = 0;
	Boolean SD;
	
	SD = false;
	

	spiClockSpeed(false);
	sdChipSelect(false);
	for(v = 0; v < 20; v++) spiByte(0xFF);	//lots of clocks with CS not asserted to give card time to init
	
	//with CS tied low, we get here with clock sync a bit weird, so we need to re-sync it, we do so here, since we know for sure what the valid RESP for CMD0 is
	do{
		spiSingleClock();
		v = sdPrvSimpleCommand(0, 0, true);
		//resync usage makes this bad, so i comment it out: if(v & FLAG_TIMEOUT) return false;
		tries++;
		if(tries > 30) return false;
	}while(v != 0x01);

	v = sdPrvSimpleCommand(55, 0, true);		//see if this is SD or MMC
	if(v & FLAG_TIMEOUT) return false;
	SD = !(v & FLAG_ILLEGAL_CMD);
	
	if(!sdPrvCardInit(SD))return false;
	
	v = sdPrvSimpleCommand(16, 512, true);		//sec sector size
	if(v) return false;

	v = sdPrvSimpleCommand(59, 0, true);		//crc off
	if(v) return false;


	spiClockSpeed(true);
	
	return true;
}

//stream mode
Boolean sdReadStart(UInt24 sec){

	UInt8 v;

	v = sdPrvSimpleCommand(18, ((UInt32)sec) << 9, false);
	if(v & FLAG_TIMEOUT) { return false; }

	do{
		v = spiByte(0xFF);
	}while(v == 0xFF);
	if(v != 0xFE) { return false;}

	return true;
}

void sdNextSec(){

	UInt8 v;

	spiByte(0xFF);	//skip crc
	spiByte(0xFF);

	do{
		v = spiByte(0xFF);
	}while(v == 0xFF);
}

void sdSecReadStop(){

	//cancel read
	sdPrvSimpleCommand(12, 0, true);
}
