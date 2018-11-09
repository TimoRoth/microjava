#include <avr/wdt.h>
#include "common.h"
#include "uc_HD44780.h"


static UInt16 gTime;
extern UInt8 __bss_end[];


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
		//UART config
		UBRR0H = UBRRH_VALUE;
		UBRR0L = UBRRL_VALUE;
		UCSR0A = USE_2X ? (1 << U2X0) : 0;
		UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
		UCSR0B = (1 << RXEN0) | (1 << TXEN0);
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
		static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);
	
		stdout = stderr = &uart_str;
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

void fail_msg(UInt8 v){

	_putchar('\r');
	_putchar('\n');
	_putchar('F');
	_putchar('A');
	_putchar('I');
	_putchar('L');
	_putchar((v / 100) + '0');
	_putchar((v % 100) / 10 + '0');
	_putchar((v % 10) + '0');
	while(1);
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

Boolean pwmSet(UInt8 which, UInt8 bri){

	return false;
}
