#include "common.h"
#include "uc_GLCD.h"


//TODO: setup SPLIM register and thus limit stack and make heap space bigger

#ifdef CPU_DSPIC
	_FBS(BWRP_WRPROTECT_OFF &  BSS_NO_BOOT_CODE & RBS_NO_RAM);
	_FSS(RSS_NO_RAM & SSS_NO_FLASH & SWRP_WRPROTECT_OFF);
	_FGS(GSS_OFF & GCP_OFF & GWRP_OFF);
	_FOSCSEL(FNOSC_FRC & IESO_OFF);
	_FOSC(FCKSM_CSECMD & IOL1WAY_OFF & OSCIOFNC_ON & POSCMD_NONE);
	_FWDT(FWDTEN_OFF & WINDIS_OFF & WDTPOST_PS32768 & WDTPRE_PR128);
	_FPOR(ALTI2C_OFF & FPWRT_PWR32);
	_FICD(JTAGEN_OFF & ICS_PGD2);
#endif
#ifdef CPU_PIC24
	_CONFIG2(IESO_OFF & SOSCSEL_LPSOSC & FNOSC_FRC & FCKSM_CSECMD & OSCIOFNC_ON & IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_NONE);
	_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & BKBUG_OFF & COE_OFF & ICS_PGx2 & FWDTEN_OFF);
#endif

#define ISNAN(x) (							\
{									\
	union u { double d; unsigned int i[2]; } *t = (union u *)&(x);	\
	((t->i[1] & 0x7ff00000) == 0x7ff00000) &&			\
	(t->i[0] != 0 || (t->i[1] & 0xfffff) != 0);			\
})


int __unorddf2(double a, double b){ return ISNAN(a) || ISNAN(b); }


//i2c pins
#define SDA_TRIS	TRISBbits.TRISB9
#define SCL_TRIS	TRISBbits.TRISB8
#define SDA_LAT		LATBbits.LATB9
#define SCL_LAT		LATBbits.LATB8
#define SDA_PORT	PORTBbits.RB9
#define SCL_PORT	PORTBbits.RB8
#define SCL_PU		CNPU2bits.CN22PUE
#define SDA_PU		CNPU2bits.CN21PUE


//spi pins
#define MOSI_TRIS	TRISBbits.TRISB13
#define SCLK_TRIS	TRISBbits.TRISB14
#define MISO_TRIS	TRISBbits.TRISB15
#define MOSI_LAT	LATBbits.LATB13
#define SCLK_LAT	LATBbits.LATB14
#define MISO_LAT	LATBbits.LATB15
#define MISO_PORT	PORTBbits.RB15

#define CHAR_WIDTH		6
#define CHAR_HEIGHT 		8

#define CHARS_WIDTH		(GLCD_COLS / CHAR_WIDTH)
#define CHARS_HEIGHT		(GLCD_ROWS / CHAR_HEIGHT)


#define EEPROM_PAGE_SIZE	0x400	//in words
#define EEPROM_PAGE_BYTES	1536	//in bytes
#define EEPROM_MAGIC		0xEE

static volatile UInt16 gTime = 0;
static UInt8 gCurRow, gCurCol;
static void charDraw(char ch, UInt8 x, UInt8 y);
static Boolean gSlow;

extern __attribute__((space(prog))) short _JAVA;
short __attribute__((section("__JAVA.sec"),space(prog))) _JAVA = 0xFFFF;


static UInt8 eepromPrvRawRead(UInt8 page, UInt16 ofst);
static void eepromPrvRawWrite(UInt8 page, UInt16 ofst, UInt8 val);
static void eepromPrvRawErase(UInt8 page);


void board_init(){

	//WDT
	RCONbits.SWDTEN = 0;

	//clock & basic IO
	#ifdef CPU_DSPIC
	{
		//setup
		CLKDIV = 0x0000;	//remove all dividers
		PLLFBD = 201;		//106.865 MHz //C_PLLDIV
		CLKDIVbits.PLLPOST = C_PLLPOST;
		CLKDIVbits.PLLPRE = C_PLLPRE;
		
		//switch
		__builtin_write_OSCCONH(0x01);
		__builtin_write_OSCCONL(0x01);
		
		//wait for switch
		while (OSCCONbits.COSC != 0b01);
		
		//wait for pll lock
		while(OSCCONbits.LOCK != 1);

		AD1PCFGL = 0xFFFF;	//all pins digital
	}
	#endif
	
	#ifdef CPU_PIC24
	{
		//setup
		CLKDIV = 0x0000;	//remove all dividers
		
		//switch
		__builtin_write_OSCCONH(0x01);
		__builtin_write_OSCCONL(0x01);
		
		//wait for switch
		while (OSCCONbits.COSC != 0b01);
		
		//wait for pll lock
		while(OSCCONbits.LOCK != 1);

		AD1PCFG = 0x1FFF;	//all pins digital
	}
	#endif
	
	//uart
	{
		U1MODE = 0x8000;	//8N1, slow mode
		U1STA = 0x0500;		//std config
		U1BRG = (F_CPU + 16 * 38400) / (2 * 16 * 38400) - 1;  //38470 bps (0.038% error)

		RPOR3bits.RP6R = 3;	//Enable U1TX function on pin RP6 (PORTB6, pin15)
	}

	//timer
	{
		T1CON = 0x8030;		//Timer1 at Fosc/2/256
		PR1 = 0xFFFF;
		
		IPC0bits.T1IP = 6;	//high-priority
		IEC0bits.T1IE = 1;
		gTime = 0;
	}
	
	//i2c
	{
		SDA_TRIS = 1;
		SCL_TRIS = 1;
 		SDA_LAT = 0;
 		SCL_LAT = 0;
 		SCL_PU = 1;
		SDA_PU = 1;
 	}
 	
 	//spi
 	{
 		MISO_LAT = 1;
		MISO_TRIS = 0;
		MOSI_LAT = 0;
		SCLK_LAT = 0;
		MOSI_TRIS = 0;
		SCLK_TRIS = 0;
		MISO_TRIS = 1;
		gSlow = true;
 	}
 	
 	//charlieplexing timer & interrupt
 	{
 		PR2 = 8191;
 		TMR2 = 0;
 		T2CON = 0x8010;
 		OC1R = 8192;
 		OC1RS = 8192;
 		OC1CON = 0x0006;	//use TMR2 as source
 		RPOR2bits.RP5R = 18;
 	}
 	
 	i2cGlcdInit();
 	
	gCurRow = 0;
	gCurCol = 0;
}


static void _putchar(char c){

	while(U1STAbits.UTXBF);
	U1TXREG = (UInt8)c;
}

int write(int handle, void *buffer, unsigned int len){

	unsigned ret = len;
	char* b = buffer;

	while(len--){

		char c = *b++;
		if(c == '\n') _putchar('\r');
		_putchar(c);
	}
	return ret;
}

void fail_msg(UInt8 var){

	clcdClear();
	clcdGotoXY(0,0);
	clcdChar('F');
	clcdChar('A');
	clcdChar('I');
	clcdChar('L');
	clcdChar(' ');
	clcdChar((var / 100) + '0');
	var %= 100;
	clcdChar((var / 10) + '0');
	var %= 10;
	clcdChar(var + '0');
	clcdChar('!');
}

void __attribute__((__interrupt__,shadow,no_auto_psv)) _T1Interrupt(void){
	
	IFS0bits.T1IF = 0;
	gTime++;
}

UInt32 getClock(void){
	
	UInt16 lo, hi;
	
	do{
		lo = TMR1;
		hi = gTime;
	}while(lo > TMR1);
	
	return (((UInt32)hi) << 16) | lo;
}

UInt32 getClkTicksPerSec(void){

	return ((F_CPU / 2) + 128) >> 8UL;	
}

static void i2cDelay(void){
	volatile UInt16 i;

	for(i = 0; i < 30; i+=2) i--;
}

static void i2cWriteBit(Boolean one){

	SDA_TRIS = one ? 1 : 0;
	i2cDelay();
	SCL_TRIS = 1;
	i2cDelay();
	SCL_TRIS = 0;
}

static Boolean i2cReadBit(void){

	Boolean ret;

	i2cDelay();
	SDA_TRIS = 1;
	i2cDelay();
	SCL_TRIS = 1;
	i2cDelay();
	ret = SDA_PORT;
	SCL_TRIS = 0;
	i2cDelay();
	
	return ret;
}

void i2cStart(void){

	
	i2cDelay();
	SCL_TRIS = 1;
	i2cDelay();
	SDA_TRIS = 1;
	i2cDelay();
	SDA_TRIS = 0;
	i2cDelay();
	SCL_TRIS = 0;
	i2cDelay();
}

Boolean i2cSend(UInt8 v){

	UInt8 i;

	for(i = 0; i < 8; i++){

		i2cWriteBit(!!(v & 0x80));
		v <<= 1;
	}

	return !i2cReadBit();
}

UInt8 i2cRecv(Boolean ack){

	UInt8 i, v = 0;

	for(i = 0; i < 8; i++){

		v <<= 1;
		if(i2cReadBit()) v++;
	}

	i2cWriteBit(!ack);

	return v;
}

void i2cStop(void){

	
	i2cDelay();
	SCL_TRIS = 1;
	i2cDelay();
	SDA_TRIS = 1;
	i2cDelay();
}

void glcdClear(void){

	i2cGlcdClear();	
}

void glcdSetPixels(UInt8 rowOver8, UInt8 c, UInt8 vals){

	i2cGlcdSetPixels(rowOver8, c, vals);	
}

void glcdSetManyPixels(UInt8 rowOver8, UInt8 c, const UInt8* vals, UInt16 nCols){

	i2cGlcdSetManyPixels(rowOver8, c, vals, nCols);	
}

void spiClockSpeed(Boolean fast){

	gSlow = !fast;
}

UInt8 spiByte(UInt8 v){

	volatile UInt32 x;
	UInt8 i;
	
	for(i = 0; i < 8; i++){
	
		if(v & 0x80) MOSI_LAT = 1;
		else MOSI_LAT = 0;
		
		Nop();
		if(gSlow) for(x = 0; x < 5; x+= 2) x--;
		
		SCLK_LAT = 1;
		
		Nop();
		if(gSlow) for(x = 0; x < 5; x+= 2) x--;
		
		v <<= 1;
		if(MISO_PORT) v++;
		
		SCLK_LAT = 0;
		
		Nop();
		if(gSlow) for(x = 0; x < 5; x+= 2) x--;
	}
	
	return v;
}

void spiSingleClock(void){

	volatile UInt32 x;

	MOSI_LAT = 1;
	SCLK_LAT = 0;
	for(x = 0; x < 5; x+= 2) x--;
	SCLK_LAT = 1;
	for(x = 0; x < 5; x+= 2) x--;
	SCLK_LAT = 0;
}


/*
	we are expected to enable pullups if a pin in written "hi" while input - do that here as required
*/

static const UInt8 portsToCnPins[2][16] = {//	 0  1  2   3   4  5     6     7     8     9     10    11    12    13    14    15
				/* PORTA */	{2, 3, 30, 29, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
				/* PORTB */	{4, 5, 6,  7,  1, 27,   24,   23,   22,   21,   16,   15,   14,   13,   12,   11}
					};

static volatile UInt16* gpioGetReg(UInt8 port, UInt8 regIdx){
	
	volatile UInt16* reg = (volatile UInt16*)&TRISA;
	
	reg += (port << 2);
	
	return reg + regIdx;
}

static void gpioHandlePullupWork(UInt8 port, UInt8 pin){

	UInt8 pu = portsToCnPins[port][pin];
	volatile UInt16* pup = (volatile UInt16*)&CNPU1;
	UInt16 pum;
	
	//find th epullup number, if any
	if(pu == 0xFF) return;		//no more we can do :(
	
	//locate the port and bit mask we'll use
	if(pu >= 16){
		
		pu -= 16;
		pup++;
	}
	pum = 1 << pu;
	
	if((*gpioGetReg(port, 0)) & (1 << pin)){		//pin is input
	
		if((*gpioGetReg(port, 2)) & (1 << pin)){	//pin LAT is hi
			
			(*pup) |= pum;	//pullup on
		}
		else{						//pin LAT is lo
		
			(*pup) &=~ pum;	//pullup off
		}
	}
	else{							//pin is output
	
		(*pup) &=~ pum;	//pullup off
	}
}

void gpioSetOut(UInt8 port, UInt8 pin, Boolean isOutput){

	volatile UInt16* p = gpioGetReg(port, 0);
	UInt16 mask = 1 << pin;
	
	if(isOutput)(*p) &=~ mask;
	else (*p) |= mask;
	
	gpioHandlePullupWork(port, pin);
}

void gpioSetVal(UInt8 port, UInt8 pin, Boolean isHi){
	
	volatile UInt16* p = gpioGetReg(port, 2);
	UInt16 mask  = 1 << pin;	
	
	if(isHi) (*p) |= mask;
	else (*p) &=~ mask;
	
	gpioHandlePullupWork(port, pin);
}

Boolean gpioGetVal(UInt8 port, UInt8 pin){

	volatile UInt16* p = gpioGetReg(port, 1);
	
	return ((*p) >> pin) & 1;	
}

//#define EEPROM_PAGE_SIZE	0x400	//in words
//#define EEPROM_PAGE_BYTES	1536	//in bytes
//#define EEPROM_MAGIC		0xEE

static UInt8 eepromPrvRawRead(UInt8 page, UInt16 ofst){
	
	UInt16 word = ofst / 3;
	UInt8 which = ofst % 3;
	UInt32 addr = EEPROM_EMUL_ADDR + ((UInt32)page * EEPROM_PAGE_SIZE) + word;
	
	TBLPAG = addr >> 16;
	switch(which){
		
		case 0: return __builtin_tblrdh((UInt16)addr);
		case 1: return __builtin_tblrdl((UInt16)addr) >> 8;
		case 2: return __builtin_tblrdl((UInt16)addr);
	}
}

static void eepromPrvRawWrite(UInt8 page, UInt16 ofst, UInt8 val){

	UInt16 word = ofst / 3;
	UInt8 which = ofst % 3;
	UInt32 addr = EEPROM_EMUL_ADDR + ((UInt32)page * EEPROM_PAGE_SIZE) + word;
	UInt8 wrhi = 0xFF;
	UInt16 wrlo;
	
	TBLPAG = addr >> 16;
	
	switch(which){
		
		case 0:
			
			wrhi = val;
			wrlo = 0xFF;
			break;
		
		case 1:
			
			wrlo = (((UInt16)val) << 8) | 0xFF;
			break;
		
		case 2:
			
			wrlo = 0xFF00 | val;
			break;
	}
	
	__builtin_tblwtl((UInt16)addr, wrlo);
	__builtin_tblwth((UInt16)addr, wrhi);
	
	NVMCON = 0x4003;	//word write
	__builtin_write_NVM();
	while(NVMCONbits.WR);	//wait
}

static void eepromPrvRawErase(UInt8 page){

	UInt32 addr = EEPROM_EMUL_ADDR + ((UInt32)page * EEPROM_PAGE_SIZE);
	
	TBLPAG = addr >> 16;
	__builtin_tblwtl((UInt16)addr, 0);
	__builtin_tblwth((UInt16)addr, 0);
	
	NVMCON = 0x4042;	//erase
	__builtin_write_NVM();
	while(NVMCONbits.WR);	//wait
}

UInt8 eepromRead(UInt16 addr){
	
	
}

void eepromWrite(UInt16 addr, UInt8 val){
	
	//
}

Boolean pwmSet(UInt8 which, UInt8 bri){
	
	if(which == PWM_WHITE_LED){
		
		OC1R = OC1RS = (255 - bri) << 5;
		
		return true;
	}
	return false;
}

#ifdef CPU_DSPIC
	#define JAVA_START_ADDR_RAW	0x0000B800UL
	#define JAVA_END_ADDR_RAW	0x00015800UL
#endif
#ifdef CPU_PIC24
	#define JAVA_START_ADDR_RAW	0x00000400UL
	#define JAVA_END_ADDR_RAW	0x00001000UL
#endif


UInt32 getFlashFreeArea(UInt32* sz){	//return addr

	*sz = (JAVA_END_ADDR_RAW - JAVA_START_ADDR_RAW) * 3 / 2;	//in bytes
	return JAVA_START_ADDR_RAW;
}

void flashromWrite(UInt32 addr, const UInt8* data, UInt16 key){
	
	UInt16 i;
	
	if(key != FLASH_KEY) return;
	
	if(addr >= JAVA_END_ADDR_RAW){	//no need to erase here, but we're writing two pages of high bytes now
		
		addr -= JAVA_END_ADDR_RAW;
		addr <<= 1;
		addr += JAVA_START_ADDR_RAW;
		
		//we do not erase here - it's not needed
		for(i = 0; i < FLASHROM_PAGE * 2 /* we use 2 pages now per 'page' */; i += 2){
		
			TBLPAG = (addr + i) >> 16;
			__builtin_tblwtl((UInt16)(addr + i), 0xFFFF);
			__builtin_tblwth((UInt16)(addr + i), *data++);
			
			NVMCON = 0x4003;	//word write
			__builtin_write_NVM();
			while(NVMCONbits.WR);	//wait
		}
	}
	else{		//now page - write it like we mean it
	
		TBLPAG = addr >> 16;
		__builtin_tblwtl((UInt16)addr, 0);
		__builtin_tblwth((UInt16)addr, 0);
		
		NVMCON = 0x4042;	//erase
		__builtin_write_NVM();
		while(NVMCONbits.WR);	//wait
		
		for(i = 0; i < FLASHROM_PAGE; i += 2){
		
			UInt16 v;
			
			v = *data++;
			v <<= 8;
			v += *data++;
			
			TBLPAG = (addr + i) >> 16;
			__builtin_tblwtl((UInt16)(addr + i), v);
			__builtin_tblwth((UInt16)(addr + i), 0xFF);
			
			NVMCON = 0x4003;	//word write
			__builtin_write_NVM();
			while(NVMCONbits.WR);	//wait
		}
	}
}

UInt8 pgm_read(UInt32 addr){

	UInt8 ret;

	if(addr >= JAVA_END_ADDR_RAW){	//use high bytes
	
		addr -= JAVA_END_ADDR_RAW;
		addr <<= 1;			//addressed work weirdly when using high bytes :)
		addr += JAVA_START_ADDR_RAW;
		
		TBLPAG = addr >> 16;
		ret = __builtin_tblrdh((UInt16)addr);
	}
	else{
	
		UInt16 v;
		
		TBLPAG = addr >> 16;
		v = __builtin_tblrdl((UInt16)(addr &~ 1));
		
		if(!(addr & 1)) v >>= 8;
		
		ret = v;
	}
	
	return ret;
}




void clcdGotoXY(UInt8 x, UInt8 y){

	if(x >= CHARS_WIDTH) x = 0;
	if(y >= CHARS_HEIGHT) y = 0;
	
	gCurRow = y;
	gCurCol = x;
}

void clcdChar(char c){

	charDraw(c, gCurCol++, gCurRow);
	
	if(gCurCol == CHARS_WIDTH){
		
		gCurCol = 0;
		if(++gCurRow == CHARS_HEIGHT) gCurRow = 0;
	}
}

void clcdClear(){

	i2cGlcdClear();
	gCurRow = 0;
	gCurCol = 0;
}

void clcdRawWrite(UInt8 cmd){

	//TODO: maybe implement this?	
}


























#define FIRST_VALID_CHAR	0x20
#define LAST_VALID_CHAR		0x83
#define MISSING_CHAR_CHAR	0x7F

#define CHAR_ARROW_UP		0x80
#define CHAR_ARROW_DOWN		0x81
#define CHAR_ARROW_RIGHT	0x82
#define CHAR_ARROW_LEFT		0x83





#define _	0
#define W	1
	


#define _	0
#define W	1
	


#define R(a0,a1,a2,a3,a4,a5,b0,b1,b2,b3,b4,b5,c0,c1,c2,c3,c4,c5,d0,d1,d2,d3,d4,d5)	\
		((((UInt32)((a0 << 0) | (b0 << 1) | (c0 << 2) | (d0 << 3))) << 20) | 	\
		 (((UInt32)((a1 << 0) | (b1 << 1) | (c1 << 2) | (d1 << 3))) << 16) | 	\
		 (((UInt32)((a2 << 0) | (b2 << 1) | (c2 << 2) | (d2 << 3))) << 12) | 	\
		 (((UInt32)((a3 << 0) | (b3 << 1) | (c3 << 2) | (d3 << 3))) <<  8) | 	\
		 (((UInt32)((a4 << 0) | (b4 << 1) | (c4 << 2) | (d4 << 3))) <<  4) | 	\
		 (((UInt32)((a5 << 0) | (b5 << 1) | (c5 << 2) | (d5 << 3))) <<  0))
		


#define S(t, b)							\
		(((t >> 20) & 0x0F) | ((b >> 16) & 0xF0)),	\
		(((t >> 16) & 0x0F) | ((b >> 12) & 0xF0)),	\
		(((t >> 12) & 0x0F) | ((b >>  8) & 0xF0)),	\
		(((t >>  8) & 0x0F) | ((b >>  4) & 0xF0)),	\
		(((t >>  4) & 0x0F) | ((b >>  0) & 0xF0)),	\
		(((t >>  0) & 0x0F) | ((b <<  4) & 0xF0))
		

const static UInt8 chars[]=

{


//' ',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),



//'!',

S(R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,_,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),




//'"',

S(R(_,W,_,W,_,_,
  _,W,_,W,_,_,
  _,W,_,W,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),



//'#',

S(R(_,W,_,W,_,_,
  _,W,_,W,_,_,
  W,W,W,W,W,_,
  _,W,_,W,_,_),
R(W,W,W,W,W,_,
  _,W,_,W,_,_,
  _,W,_,W,_,_,
  _,_,_,_,_,_)),



//'$',

S(R(_,_,W,_,_,_,
  _,W,W,W,W,_,
  W,_,W,_,_,_,
  _,W,W,W,_,_),
R(_,_,W,_,W,_,
  W,W,W,W,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),




//'%',

S(R(W,W,_,_,_,_,
  W,W,_,_,W,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_),
R(_,W,_,_,_,_,
  W,_,_,W,W,_,
  _,_,_,W,W,_,
  _,_,_,_,_,_)),




//'&',

S(R(_,W,_,_,_,_,
  W,_,W,_,_,_,
  W,_,W,_,_,_,
  _,W,_,_,_,_),
R(W,_,W,_,W,_,
  W,_,_,W,_,_,
  _,W,W,_,W,_,
  _,_,_,_,_,_)),




//''',

S(R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),




//'(',

S(R(_,_,W,_,_,_,
  _,W,_,_,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_),
R(W,_,_,_,_,_,
  _,W,_,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//')',

S(R(_,_,W,_,_,_,
  _,_,_,W,_,_,
  _,_,_,_,W,_,
  _,_,_,_,W,_),
R(_,_,_,_,W,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//'*',

S(R(_,_,_,_,_,_,
  _,_,W,_,_,_,
  W,_,W,_,W,_,
  _,W,W,W,_,_),
R(W,_,W,_,W,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'+',

S(R(_,_,_,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  W,W,W,W,W,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//',',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,W,W,_,_,_,
  _,_,W,_,_,_,
  _,W,_,_,_,_)),

  
  
//'-',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  W,W,W,W,W,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'.',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,W,W,_,_,_,
  _,W,W,_,_,_,
  _,_,_,_,_,_)),


  
//'/',

S(R(_,_,_,_,_,_,
  _,_,_,_,W,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_),
R(_,W,_,_,_,_,
  W,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'0',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,W,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'1',

S(R(_,_,W,_,_,_,
  _,W,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'2',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  _,_,_,_,W,_,
  _,_,_,W,_,_),
R(_,_,W,_,_,_,
  _,W,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),

  
  
//'3',

S(R(W,W,W,W,W,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_,
  _,_,_,W,_,_),
R(_,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
  
//'4',

S(R(_,_,_,W,_,_,
  _,_,W,W,_,_,
  _,W,_,W,_,_,
  W,_,_,W,_,_),
R(W,W,W,W,W,_,
  _,_,_,W,_,_,
  _,_,_,W,_,_,
  _,_,_,_,_,_)),

  
  
//'5',

S(R(W,W,W,W,W,_,
  W,_,_,_,_,_,
  W,W,W,W,_,_,
  _,_,_,_,W,_),
R(_,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
  
//'6',

S(R(_,_,W,W,_,_,
  _,W,_,_,_,_,
  W,_,_,_,_,_,
  W,W,W,W,_,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'7',

S(R(W,W,W,W,W,_,
  _,_,_,_,W,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_),
R(_,W,_,_,_,_,
  _,W,_,_,_,_,
  _,W,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'8',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'9',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,W,_),
R(_,_,_,_,W,_,
  _,_,_,W,_,_,
  _,W,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//':',

S(R(_,_,_,_,_,_,
  _,W,W,_,_,_,
  _,W,W,_,_,_,
  _,_,_,_,_,_),
R(_,W,W,_,_,_,
  _,W,W,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),




//';',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,W,W,_,_,
  _,_,W,W,_,_),
R(_,_,_,_,_,_,
  _,_,W,W,_,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_)),




//'<',

S(R(_,_,_,W,_,_,
  _,_,W,_,_,_,
  _,W,_,_,_,_,
  W,_,_,_,_,_),
R(_,W,_,_,_,_,
  _,_,W,_,_,_,
  _,_,_,W,_,_,
  _,_,_,_,_,_)),

  
  
//'=',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_),
R(W,W,W,W,W,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),

  
//'>',

S(R(_,W,_,_,_,_,
  _,_,W,_,_,_,
  _,_,_,W,_,_,
  _,_,_,_,W,_),
R(_,_,_,W,_,_,
  _,_,W,_,_,_,
  _,W,_,_,_,_,
  _,_,_,_,_,_)),
  
   
//'?',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  _,_,_,_,W,_,
  _,_,_,W,_,_),
R(_,_,W,_,_,_,
  _,_,_,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),




//'@',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,W,_,W,_,
  W,_,W,W,W,_),
R(W,_,W,_,_,_,
  W,_,_,_,_,_,
  _,W,W,W,W,_,
  _,_,_,_,_,_)),




//'A',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'B',

S(R(W,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,_,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'C',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_),
R(W,_,_,_,_,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'D',

S(R(W,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'E',

S(R(W,W,W,W,W,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,W,W,W,_,_),
R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),

  
  
//'F',

S(R(W,W,W,W,W,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,W,W,W,_,_),
R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'G',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,_,_,
  W,_,W,W,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'H',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'I',

S(R(_,W,W,W,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'J',

S(R(_,_,W,W,W,_,
  _,_,_,W,_,_,
  _,_,_,W,_,_,
  _,_,_,W,_,_),
R(_,_,_,W,_,_,
  W,_,_,W,_,_,
  _,W,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//'K',

S(R(W,_,_,_,W,_,
  W,_,_,W,_,_,
  W,_,W,_,_,_,
  W,W,_,_,_,_),
R(W,_,W,_,_,_,
  W,_,_,W,_,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'L',

S(R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_),
R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),

  
  
//'M',

S(R(W,_,_,_,W,_,
  W,W,_,W,W,_,
  W,_,W,_,W,_,
  W,_,W,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'N',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,_,_,W,_,
  W,_,W,_,W,_),
R(W,_,_,W,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'O',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'P',

S(R(W,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,_,_),
R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
  
//'Q',

S(R(_,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,W,_,W,_,
  W,_,_,W,_,_,
  _,W,W,_,W,_,
  _,_,_,_,_,_)),

  
  
//'R',

S(R(W,W,W,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,_,_),
R(W,_,W,_,_,_,
  W,_,_,W,_,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
  
//'S',

S(R(_,W,W,W,W,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  _,W,W,W,_,_),
R(_,_,_,_,W,_,
  _,_,_,_,W,_,
  W,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
  
//'T',

S(R(W,W,W,W,W,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),

  
  
  
//'U',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'V',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  _,W,_,W,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//'W',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,W,_,W,_,
  W,_,W,_,W,_,
  _,W,_,W,_,_,
  _,_,_,_,_,_)),

  
  
//'X',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,_,W,_,_,
  _,_,W,_,_,_),
R(_,W,_,W,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'Y',

S(R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,_,W,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//'Z',

S(R(W,W,W,W,W,_,
  _,_,_,_,W,_,
  _,_,_,W,_,_,
  _,_,W,_,_,_),
R(_,W,_,_,_,_,
  W,_,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),


  
//'[',

S(R(_,_,W,W,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,W,_,_,
  _,_,_,_,_,_)),

  

//'\',

S(R(_,_,_,_,_,_,
  W,_,_,_,_,_,
  _,W,_,_,_,_,
  _,_,W,_,_,_),
R(_,_,_,W,_,_,
  _,_,_,_,W,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//']',

S(R(_,_,W,W,_,_,
  _,_,_,W,_,_,
  _,_,_,W,_,_,
  _,_,_,W,_,_),
R(_,_,_,W,_,_,
  _,_,_,W,_,_,
  _,_,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'^',

S(R(_,_,W,_,_,_,
  _,W,_,W,_,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),




//'_',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),




//'`',

S(R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,W,_,_,
  _,_,_,_,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),




//'a',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,W,_,_,
  _,_,_,_,W,_),
R(_,W,W,W,W,_,
  W,_,_,_,W,_,
  _,W,W,W,W,_,
  _,_,_,_,_,_)),

  
  
//'b',

S(R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,W,W,_,_,
  W,W,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'c',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,W,_,_,
  W,_,_,_,_,_),
R(W,_,_,_,_,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'d',

S(R(_,_,_,_,W,_,
  _,_,_,_,W,_,
  _,W,W,_,W,_,
  W,_,_,W,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,W,_,
  _,_,_,_,_,_)),

  
  
//'e',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,W,_,_,
  W,_,_,_,W,_),
R(W,W,W,W,W,_,
  W,_,_,_,_,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'f',

S(R(_,_,W,W,_,_,
  _,W,_,_,W,_,
  _,W,_,_,_,_,
  W,W,W,_,_,_),
R(_,W,_,_,_,_,
  _,W,_,_,_,_,
  _,W,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'g',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,W,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  _,W,W,W,W,_,
  _,_,_,_,W,_,
  _,W,W,W,_,_)),

  
  
//'h',

S(R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,W,W,_,_,
  W,W,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'i',

S(R(_,_,W,_,_,_,
  _,_,_,_,_,_,
  _,W,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'j',

S(R(_,_,_,W,_,_,
  _,_,_,_,_,_,
  _,_,W,W,_,_,
  _,_,_,W,_,_),
R(_,_,_,W,_,_,
  _,_,_,W,_,_,
  W,_,_,W,_,_,
  _,W,W,_,_,_)),

  
  
//'k',

S(R(_,W,_,_,_,_,
  _,W,_,_,_,_,
  _,W,_,_,W,_,
  _,W,_,W,_,_),
R(_,W,W,_,_,_,
  _,W,_,W,_,_,
  _,W,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'l',

S(R(_,W,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'m',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,W,_,W,_,_,
  W,_,W,_,W,_),
R(W,_,W,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'n',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,W,W,_,_,
  W,W,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'o',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,W,_,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  _,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'p',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,W,W,W,_,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,W,W,W,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_)),

  
  
//'q',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,_,W,_,
  W,_,_,W,W,_),
R(W,_,_,_,W,_,
  _,W,W,W,W,_,
  _,_,_,_,W,_,
  _,_,_,_,W,_)),

  
  
//'r',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,W,W,_,_,
  W,W,_,_,W,_),
R(W,_,_,_,_,_,
  W,_,_,_,_,_,
  W,_,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'s',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,W,W,W,_,_,
  W,_,_,_,_,_),
R(_,W,W,W,_,_,
  _,_,_,_,W,_,
  W,W,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'t',

S(R(_,W,_,_,_,_,
  _,W,_,_,_,_,
  W,W,W,_,_,_,
  _,W,_,_,_,_),
R(_,W,_,_,_,_,
  _,W,_,_,W,_,
  _,_,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'u',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,W,W,_,
  _,W,W,_,W,_,
  _,_,_,_,_,_)),

  
  
//'v',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  _,W,_,W,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),

  
  
//'w',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,W,_,W,_,
  W,_,W,_,W,_,
  _,W,_,W,_,_,
  _,_,_,_,_,_)),

  
  
//'x',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,_,_,W,_,
  _,W,_,W,_,_),
R(_,_,W,_,_,_,
  _,W,_,W,_,_,
  W,_,_,_,W,_,
  _,_,_,_,_,_)),

  
  
//'y',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  _,W,W,W,W,_,
  _,_,_,_,W,_,
  _,W,W,W,_,_)),

  
  
//'z',

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,W,_,_),
R(_,_,W,_,_,_,
  _,W,_,_,_,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),

  
  
//'{',

S(R(_,_,W,W,_,_,
  _,W,_,_,_,_,
  _,W,_,_,_,_,
  W,_,_,_,_,_),
R(_,W,_,_,_,_,
  _,W,_,_,_,_,
  _,_,W,W,_,_,
  _,_,_,_,_,_)),

  
  
//'|',

S(R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),




//'}',

S(R(W,W,_,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,_,W,_,_),
R(_,_,W,_,_,_,
  _,_,W,_,_,_,
  W,W,_,_,_,_,
  _,_,_,_,_,_)),

  
  
//'~',

S(R(_,_,_,_,_,_,
  _,W,_,_,_,_,
  W,_,W,_,W,_,
  _,_,_,W,_,_),
R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,_,_,_)),




// '\0x7F',	//- no char

S(R(W,W,W,W,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,_,_,_,W,_),
R(W,_,_,_,W,_,
  W,_,_,_,W,_,
  W,W,W,W,W,_,
  _,_,_,_,_,_)),




//UP arrow	UP_ARROW_CHAR

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,W,_,_,_,
  _,W,W,W,_,_),
R(W,_,W,_,W,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_)),




//DOWN arrow	DOWN_ARROW_CHAR

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,W,_,_,_,
  _,_,W,_,_,_),
R(_,_,W,_,_,_,
  W,_,W,_,W,_,
  _,W,W,W,_,_,
  _,_,W,_,_,_)),




//RIGHT arrow	RIGHT_ARROW_CHAR

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,_,W,_,_,
  _,_,_,_,W,_),
R(W,W,W,W,W,W,
  _,_,_,_,W,_,
  _,_,_,W,_,_,
  _,_,_,_,_,_)),
  
  

//LEFT arrow	LEFT_ARROW_CHAR

S(R(_,_,_,_,_,_,
  _,_,_,_,_,_,
  _,_,W,_,_,_,
  _,W,_,_,_,_),
R(W,W,W,W,W,W,
  _,W,_,_,_,_,
  _,_,W,_,_,_,
  _,_,_,_,_,_)),
  
};



static void charDraw(char ch, UInt8 x, UInt8 y){
	
	UInt32 offset;
	UInt32 i,num;
	const UInt8* chImg;
	UInt32 chr = (unsigned char)ch;
	
	chImg = chars;
	
	if(chr < FIRST_VALID_CHAR || chr > LAST_VALID_CHAR){
		
		chr = MISSING_CHAR_CHAR;
	}
	
	chr -= FIRST_VALID_CHAR;
	chImg += (chr * 6);
	
	x *= 6;
	i2cGlcdSetManyPixels(y, x, chImg, 6);
}
