#ifndef _UC_HD44780_H_
#define _UC_HD44780_H_

#include "common.h"

void lcdInit();
void lcdClear();
void lcdGotoXY(UInt8 x,UInt8 y);
void lcdPutc(char c);
void lcdRawWriteCmd(UInt8 bytes);



#endif
