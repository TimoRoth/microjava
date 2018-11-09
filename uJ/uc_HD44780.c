#include <math.h>
#include <stdarg.h>
#include <avr/io.h>
#include "common.h"
#include "uj.h"
#include "uc_HD44780.h"
#include <util/delay.h>



static void lcdPrvStrobe(void){
	_delay_us(30);			//might need to be 20
	PORTA |= 0b00010000;		//raise EN pin
	_delay_us(30);			//might need to be 20
	PORTA &=~ 0b00010000;		//lower EN pin
}

static void lcdPrvBits(UInt8 b){
	
	UInt8 t = 0;
	
	if(b & 1) t |= 4;
	if(b & 2) t |= 2;
	if(b & 4) t |= 1;
	if(b & 8) t |= 8; 
	
	PORTA = (PORTA & 0xF0) | t;
}

void lcdRawWriteCmd(UInt8 c)
{
	lcdPrvBits(c >> 4);
	lcdPrvStrobe();
	lcdPrvBits(c & 0x0F);
	lcdPrvStrobe();
	_delay_us(40);
}

void lcdInit(){
	
	DDRA = 0xFF;	//porta is output
	PORTA = 0x00;	//all pins low
	
	lcdPrvBits(0);
		
	_delay_ms(50);	// wait after power applied
			//this sequece is neede if atempt to operate LCD outside supported voltage range
	
	lcdPrvBits(3);
	lcdPrvStrobe();
	_delay_ms(10);
	lcdPrvStrobe();
	_delay_us(500);
	lcdPrvStrobe();
	_delay_us(500);
	
	lcdPrvBits(2);
	lcdPrvStrobe();
	_delay_ms(10);
	
	lcdRawWriteCmd(0x28); // Set interface length
	lcdRawWriteCmd(0x08); // Display Off
	lcdRawWriteCmd(0x0F); // Display On, Cursor On, Cursor Blink On
	lcdRawWriteCmd(0x0C); // Display On, Cursor Off, Cursor Blink Off
	lcdRawWriteCmd(0x06); // Set entry Mode
	lcdClear();	// Clear screen
}

void lcdClear(){
	lcdRawWriteCmd(0x1);
	_delay_us(1600);
}

void lcdGotoXY(UInt8 x,UInt8 y){
	lcdRawWriteCmd(0x80 + x + (y << 6));
	_delay_us(80);
}

void lcdPutc(char c){
	
	PORTA |= 0b00100000;			//toggle RS pin
	lcdRawWriteCmd(c);
	PORTA &=~ 0b00100000;			//toggle RS pin
}