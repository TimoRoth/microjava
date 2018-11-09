#ifndef _UC_GLCD_H_
#define _UC_GLCD_H_

#include "common.h"

#define GLCD_ROWS	64
#define GLCD_COLS	128

void i2cGlcdInit(void);
void i2cGlcdClear(void);
void i2cGlcdSetPixels(UInt8 rowOver8, UInt8 c, UInt8 vals);	//8 pixels, vertically. LSB = top
void i2cGlcdSetManyPixels(UInt8 rowOver8, UInt8 c, const UInt8* vals, UInt16 nCols);	//8 pixels, vertically. LSB = top



#endif
