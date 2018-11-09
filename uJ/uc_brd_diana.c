#include <avr/wdt.h>
#include "common.h"
#include "uc_HD44780.h"


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define SD_BIT_MOSI		1
#define SD_BIT_CLK		2
#define SD_BIT_MISO		0
#define SD_PIN_MOSI		(1 << SD_BIT_MOSI)
#define SD_PIN_CLK		(1 << SD_BIT_CLK)
#define SD_PIN_MISO		(1 << SD_BIT_MISO)
#define SD_REGx(name, type)	name ## type
#define SD_REG(name)		SD_REGx(name, B)

#define BYTES_PER_WORD	2UL
#define WORDS_PER_PAGE	128UL
#define BYTES_PER_PAGE	(WORDS_PER_PAGE * BYTES_PER_WORD)


static int uart_putchar(char c, _UNUSED_ FILE *stream);
static int uart_getchar(_UNUSED_ FILE *stream);

static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
static UInt16 gTime;
static Boolean gSlow;
extern UInt8 __bss_end[];

extern char __data_load_end[];
extern char _etext[];


static void _putchar(char c){

	#if defined(SIM)
	
		(*((volatile char *)SIM)) = c;
		
	#else
	
		while(!(UCSR0A & (1 << UDRE0)));
		UDR0 = c;
	
	#endif	
}

static int uart_putchar(char c, _UNUSED_ FILE *stream){

	if(c == '\n') _putchar('\r');
	_putchar(c);
	
	return 0;
}

static int uart_getchar(_UNUSED_ FILE *stream){
	
	return _FDEV_EOF;
}

void __rt_div0(void){

	ujLog("Division by zero\n");
	while(1);
}

void board_init(void){

	//wdt
	{		
		asm("cli");
		wdt_reset();
		wdt_disable();
	}
	
	//clock
	{
		CLKPR = 0x80;
		CLKPR = 0;
		OSCCAL = 0x7F;
	}
	
	//uart
	{
		#ifndef CPU_ATMEGA64
			//UART config
			UBRR0H = UBRRH_VALUE;
			UBRR0L = UBRRL_VALUE;
			UCSR0A = USE_2X ? (1 << U2X0) : 0;
			UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
			UCSR0B = (1 << RXEN0) | (1 << TXEN0);
		#endif
	}
	
	//timer
	{
		TCCR1A = 0;
		TCCR1B = 5;	//clk/1024
		TIMSK1 = 1;
		sei();
		gTime = 0;
	}
	
	//IO
	{
		stdout = stderr = &uart_str;
	}
	
	//LCD
	{
		lcdInit();
		lcdClear();
		lcdGotoXY(0,0);
	}
	
	//PWM
	{
		TCCR0A = 0xA3;			//setup pwms on A and B pinc
		TCCR0B = 0x04;			//PWM freq: 305HZ @ 20MHz
		DDRB |= (1 << 3) | (1 << 4);
	
		OCR0A = 0;
		OCR0B = 0;
	}
	
	//SPI
	{
		ACSR = 0x80;	//disable comparator
		MCUSR &= 0xEF;
	
		
		SD_REG(DDR) &=~ SD_PIN_MISO;	//all inputs
		SD_REG(DDR) |= SD_PIN_MOSI | SD_PIN_CLK;
		
		
		SD_REG(PORT) &=~ (SD_PIN_MOSI | SD_PIN_CLK | SD_PIN_MISO);	//all not pulled up and low
		
		gSlow = true;
	}
	
	//i2c
	{
		TWSR = 0x00;							//no prescaler
		TWBR = (((F_CPU + 400000 - 1) / 400000) - 16) / 2;		//re rounding: err on the side of slower
		TWCR = 0x04;							//i2 on
	}
	
	//heap protection
	{
		__bss_end[HEAP_SZ] = 0xFA;
	}
}

ISR(TIMER1_OVF_vect){
	
	gTime++;
	if(__bss_end[HEAP_SZ] != 0xFA) fail_msg(0xEE);
}

UInt32 getClock(void){
	
	UInt16 lo, hi;
	
	do{
		lo = TCNT1;
		hi = gTime;
	}while(lo > TCNT1);
	
	return (((UInt32)hi) << 16) | lo;
}

UInt32 getClkTicksPerSec(void){

	return (F_CPU + 512UL) >> 10UL;
}

void fail_msg(UInt8 var){

	lcdClear();
	lcdGotoXY(0,0);
	lcdPutc('F');
	lcdPutc('A');
	lcdPutc('I');
	lcdPutc('L');
	lcdPutc(' ');
	lcdPutc((var / 100) + '0');
	var %= 100;
	lcdPutc((var / 10) + '0');
	var %= 10;
	lcdPutc(var + '0');
	lcdPutc('!');
}

void gpioSetOut(UInt8 port, UInt8 pin, Boolean isOutput){

	volatile UInt8* portP = (volatile UInt8*)(0x21 + port * 3);

	if(isOutput) (*portP) |= (1 << pin);
	else (*portP) &=~ (1 << pin);
}

void gpioSetVal(UInt8 port, UInt8 pin, Boolean isHi){

	volatile UInt8* portP = (volatile UInt8*)(0x22 + port * 3);

	if(isHi) (*portP) |= (1 << pin);
	else (*portP) &=~ (1 << pin);
}

Boolean gpioGetVal(UInt8 port, UInt8 pin){

	volatile UInt8* portP = (volatile UInt8*)(0x20 + port * 3);

	return ((*portP) >> pin) & 1;
}

UInt8 eepromRead(UInt16 addr){

	return eeprom_read_byte((uint8_t*)addr);
}

void eepromWrite(UInt16 addr, UInt8 val){
	
	eeprom_update_byte((uint8_t*)addr, val);
	while(EECR & 2);	//wait for write
}

void clcdClear(void){
	
	lcdClear();
}

void clcdGotoXY(UInt8 x, UInt8 y){

	lcdGotoXY(x, y);
}

void clcdChar(char c){

	lcdPutc(c);	
}

void clcdRawWrite(UInt8 val){

	lcdRawWriteCmd(val);	
}

Boolean pwmSet(UInt8 which, UInt8 bri){

	switch(which){
		
		case PWM_WHITE_LED:
			
			OCR0B = bri;
			break;
		
		case PWM_LCD_BL:
			
			OCR0A = bri;
			break;
		
		default:
		
			return false;
	}
	return true;
}

__attribute__ ((section (".lastpage"))) void flashromWrite(UInt32 addr, const UInt8* buf, UInt16 key){
	
	UInt16 i;
	UInt8 sreg;
	
	if(key != FLASH_KEY) return;
	
	sreg = SREG;
	cli();
	
	while(EECR & 2);	//wait for eeprom to finish writing
	
	#if (FLASHROM_SIZE > 64UL*1024UL)
		RAMPZ = addr >> 16UL;
	#endif
	
	asm volatile(					//do the for erase
			"mov r30, %0\n\t"
			"mov r31, %1\n\t"
			"sts 0x57, %2\n\t"
			"spm\n\t"
			::
				"r"((UInt8)(addr)),
				"r"((UInt8)(addr >> 8)),
				"r"(3)
			:"r30","r31"
		);
	
	while(SPMCSR & 2);	//wait for erase
	
	for(i = 0; i < BYTES_PER_PAGE; i+= 2){
	
		#if (FLASHROM_SIZE > 64UL*1024UL)
			RAMPZ = (addr + i) >> 16UL;
		#endif
		
		asm volatile(					//fill buffer
			"mov r0, %3\n\t"
			"mov r1, %4\n\t"
			"mov r30, %0\n\t"
			"mov r31, %1\n\t"
			"sts 0x57, %2\n\t"
			"spm\n\t"
			"clr r1\n\t"
			::
				"r"((UInt8)(addr + i)),
				"r"((UInt8)((addr + i) >> 8)),
				"r"(1),
				"r"(buf[i + 0]),
				"r"(buf[i + 1])
			:"r0","r1","r30","r31"
		);
		
		while(SPMCSR & 1);	//wait for buffer write
	}
	
	#if (FLASHROM_SIZE > 64UL*1024UL)
		RAMPZ = addr >> 16UL;
	#endif
	
	asm volatile(						//do the write
			"mov r30, %0\n\t"
			"mov r31, %1\n\t"
			"sts 0x57, %2\n\t"
			"spm"
			::
				"r"((UInt8)(addr)),
				"r"((UInt8)(addr >> 8)),
				"r"(5)
			:"r30","r31"
		);
	
	while(SPMCSR & 4);	//wait for write
	
	
	asm volatile(						//re-enable reads there
			"mov r30, %0\n\t"
			"mov r31, %1\n\t"
			"sts 0x57, %2\n\t"
			"spm"
			::
				"r"((UInt8)(addr)),
				"r"((UInt8)(addr >> 8)),
				"r"(17)
			:"r30","r31"
		);
	
	
	while(SPMCSR & 1);
	SREG = sreg;
}

UInt32 getFlashFreeArea(UInt32* szP){

	UInt32 t, addr, size;
	
	size = FLASHROM_SIZE;
	size -= BYTES_PER_PAGE;		//last page is reserved for bootloader-like code that does our writes

	t = GET_FAR_ADDRESS(__data_load_end);
	addr = GET_FAR_ADDRESS(_etext);
	
	if(addr < t) addr = t;
	size -= addr;
	
	//round the area to page size
		
	if((t = addr & (FLASHROM_PAGE - 1))){
	
		t = FLASHROM_PAGE - t;
		
		addr += t;
		size -= t;
	}
	
	size -= (size & (FLASHROM_PAGE - 1));
	
	*szP = size;
	
	ujLog("FLASH area: 0x%04X+0x%04X\n", (UInt16)addr, (UInt16)size);
	
	return addr;
}

void spiClockSpeed(Boolean fast){

	gSlow = !fast;	
}

static _INLINE_ void pinHi(UInt8 bit){

//	SD_REG(DDR) &=~ bit;	//make input
//	SD_REG(PORT) |= bit;	//pullup
	SD_REG(DDR) |= bit;	//make input
	SD_REG(PORT) |= bit;	//pullup
}

static _INLINE_ void pinLo(UInt8 bit){

	SD_REG(PORT) &=~ bit;	//no pullup
	SD_REG(DDR) |= bit;	//make output
}

UInt8 spiByte(UInt8 v){

	UInt8 i;
	
	for(i = 0; i < 8; i++){
	
		if(v & 0x80) pinHi(SD_PIN_MOSI);
		else pinLo(SD_PIN_MOSI);
		
		if(gSlow) _delay_us(0.5);
		
		pinHi(SD_PIN_CLK);
		
		_delay_us(0.8);
		if(gSlow) _delay_us(1.6);
		
		v <<= 1;
		if(SD_REG(PIN) & SD_PIN_MISO) v++;
		pinLo(SD_PIN_CLK);
		
		if(gSlow) _delay_us(0.2);
	}
	
	pinLo(SD_PIN_MOSI);
	
	return v;
}

void spiSingleClock(void){

	pinHi(SD_PIN_MOSI);
	_delay_us(1);
	pinHi(SD_PIN_CLK);
	_delay_us(1);
	pinLo(SD_PIN_CLK);
	_delay_us(1);
}

void i2cStart(void){

	TWCR = 0xA4;
	while(!(TWCR & 0x80));
}

Boolean i2cSend(UInt8 v){

	TWDR = v;
	TWCR = 0x84;
	while(!(TWCR & 0x80));
	
	return (TWSR & 0xF8) == 0x18;
}

UInt8 i2cRecv(Boolean ack){

	TWCR = ack ? 0xC4 : 0x84;
	while(!(TWCR & 0x80));
	
	return TWDR;
}

void i2cStop(void){

	TWCR = 0x94;
	while(!(TWCR & 0x80));
}
