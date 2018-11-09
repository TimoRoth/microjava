#include "common.h"
#include "uc_GLCD.h"

#define GLCD_I2C_ADDR	0x3C




static void sendCommand(UInt8 cmd){

	i2cStart();
	i2cSend(GLCD_I2C_ADDR << 1);
	i2cSend(0x80);
	i2cSend(cmd);
	i2cStop();
}

void i2cGlcdInit(void){

	volatile UInt32 x;

	sendCommand(0xAE);	//display off
	for(x = 0; x < 0x1000; x+=2) x--;
	sendCommand(0xAF);	//display on
	for(x = 0; x < 0x1000; x+=2) x--;
	sendCommand(0x20);	//page addressing mode
	sendCommand(0x02);
	i2cGlcdClear();
}

void i2cGlcdManyThings(const UInt8* cmds, UInt8 numCmds, const UInt8* data, UInt16 numData){

	i2cStart();
	i2cSend(GLCD_I2C_ADDR << 1);
	while(numCmds--){
		i2cSend(0x80);
		i2cSend(*cmds++);
	}
	if(numData){

		i2cSend(0x40);
		while(numData--) i2cSend(*data++);
	}
	i2cStop();
}

void i2cGlcdClear(void){

	UInt8 r, c;
	UInt8 cmds[3] = {0, 0x00, 0x10};

	for(r = 0; r < GLCD_ROWS / 8; r++){
		
		cmds[0] = 0xB0 | r;
		i2cGlcdManyThings(cmds, 3, NULL, 0);
		i2cStart();
		i2cSend(GLCD_I2C_ADDR << 1);
		i2cSend(0x40);
		for(c = 0; c < GLCD_COLS; c++){

			i2cSend(0x00);
		}
		i2cStop();
	}
}


void i2cGlcdSetPixels(UInt8 r, UInt8 c, UInt8 vals){

	glcdSetManyPixels(r, c, &vals, 1);
}

void i2cGlcdSetManyPixels(UInt8 r, UInt8 c, const UInt8* vals, UInt16 nCols){

	UInt8 cmds[3];

	cmds[0] = 0xB0 | r;
	cmds[1] = 0x00 | (c & 15);
	cmds[2] = 0x10 | (c >> 4);

	i2cGlcdManyThings(cmds, 3, vals, nCols);
}







