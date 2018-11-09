#include <math.h>
#include <stdarg.h>
#include "common.h"
#include "uj.h"
#include "uc_loader.h"






extern const UjNativeClass nativeCls_UC;


#ifdef SDLOADER

	static UInt32 gFilesAddr = 0;

#else

	extern const UInt8 uj_code[] _PROGMEM_;

#endif

UInt8 ujReadClassByte(UInt32 addr, UInt32 offset){

	return pgm_read(addr + offset);
}

#ifdef UJ_LOG
	void ujLog(const char* fmtStr, ...){
	
		va_list va;
		
		va_start(va, fmtStr);
		vfprintf(stderr, fmtStr, va);
		va_end(va);	
	}
#endif

static UInt32 getNthFile(UInt8 n){

	UInt32 addr;
	UInt24 sz;
	
	#ifdef SDLOADER
	
		addr = gFilesAddr;
	#else
	
		addr = GET_ADDRESS(uj_code);
	#endif
	
	while(1){
		
		sz = pgm_read(addr++);
		sz <<= 8;
		sz += pgm_read(addr++);
		sz <<= 8;
		sz += pgm_read(addr++);
	
		if(!n--) return sz ? addr : 0;
		
		addr += sz;
	}
}

#define MAX_CLASSES	128

#define IS_SET(i)	(!!(((doneLoading[i >> 3] >> (i & 7)) & 1)))
#define DO_SET(i)	(doneLoading[i >> 3] |= 1 << (i & 7))

Boolean loadClasses(UInt8 num, struct UjClass** mainClassP){

	UInt8 i, ret;
	Boolean done;
	Boolean remaining;
	UInt8 doneLoading[(MAX_CLASSES + 7) / 8] = {0, };
	

	if(num > MAX_CLASSES){
	
		ujLog("Found %u classes, max allowed is %u -> fail\n", num, MAX_CLASSES);
		return false;	
	}

	do{
		done = false;
		remaining = false;
		for(i = 0; i < num; i++){
		
			if(!IS_SET(i)){
				remaining = true;
				
				ret = ujLoadClass(getNthFile(i), (i == 0) ? mainClassP : NULL);
				if(ret == UJ_ERR_NONE){				//success
				
					done = true;
					DO_SET(i);
				}
				else if(ret == UJ_ERR_DEPENDENCY_MISSING){	//fail: we'll try again later
				
					//nothing to do here	
				}
				else{
					
					ujLog("Failed to load class %d: %d\n", i, ret);
					return false;
				}
			}
		}
	}while(done);
	
	for(i = 0; i < num; i++) if(!IS_SET(i)){
		
		ujLog("Completely failed to load class %d\n", i);
		return false;
	}
	
	return true;
}

int main(void){
	
	UInt8 i, ret, num;
	UInt32 threadH;
	struct UjClass* mainClass = NULL;
	struct UjClass* objectClass;

	board_init();
	
	
	ujLog("uJ starting!\n");
	
	#ifdef SDLOADER

		gFilesAddr = uc_loader();

	#endif
	
	ret = ujInit(&objectClass);
	if(ret != UJ_ERR_NONE){
		ujLog("ujInit() fail\n");
		goto fail;	
	}
	ret = ujRegisterNativeClass(&nativeCls_UC, objectClass, NULL);
	if(ret != UJ_ERR_NONE){
		ujLog("ujRegisterNativeClass() fail\n");
		goto fail;	
	}
	ujLog("init stage3\r\n");
	num = 0;
	while(getNthFile(num)) num++;
	ujLog("found %d files\n", num);

	if(!loadClasses(num, &mainClass)) goto fail;
	ujLog("loaded\n");

	ret = ujInitAllClasses();
	ujLog("classes inited: %d\n", ret);
	if(ret != UJ_ERR_NONE){
		ujLog("ujInitAllClasses() fail\n");
		goto fail;	
	}
	
	//now classes are loaded, time to call the entry point
	
	threadH = ujThreadCreate(0);
	if(!threadH){
		ujLog("ujThreadCreate() fail\n");
		goto fail;	
	}
	
	i = ujThreadGoto(threadH, mainClass, "main", "()V");
	if(i == UJ_ERR_METHOD_NONEXISTENT){
	
		ujLog("Main method not found!\n");
		goto fail;
	}
	
	while(ujCanRun()){
		
		i = ujInstr();
		if(i != UJ_ERR_NONE){
		
			fail_msg(i);
		
			ujLog("Ret %d @ instr right before 0x%08lX\n", i, ujThreadDbgGetPc(threadH));
			goto fail;
		}
	}
	
	
done:
	while(1);
	
fail:
	ujLog("FAIL\n");
	goto done;
}

static UInt8 natUc_clkGet(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	return ujThreadPush(t, getClock(), false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_clkSpeed(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	return ujThreadPush(t, getClkTicksPerSec(), false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_instrsGet(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	return ujThreadPush(t, ujGetNumInstrs(), false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_gpioGetNumPorts(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	return ujThreadPush(t, GPIO_PORTS, false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_gpioGetNumPins(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	return ujThreadPush(t, GPIO_PINS_PER_PORT, false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_gpioSetOutput(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt8 port, pin;
	Boolean out;
	
	out = !!ujThreadPop(t);
	pin = ujThreadPop(t);
	port = ujThreadPop(t);
	
	if(port >= GPIO_PORTS) return UJ_ERR_ARRAY_INDEX_OOB;
	if(pin >= GPIO_PINS_PER_PORT) return UJ_ERR_ARRAY_INDEX_OOB;
	
	gpioSetOut(port, pin, out);
	
	return UJ_ERR_NONE;
}

static UInt8 natUc_gpioIsHi(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt8 port, pin;
	
	pin = ujThreadPop(t);
	port = ujThreadPop(t);
	
	if(port >= GPIO_PORTS) return UJ_ERR_ARRAY_INDEX_OOB;
	if(pin >= GPIO_PINS_PER_PORT) return UJ_ERR_ARRAY_INDEX_OOB;
	
	port = gpioGetVal(port, pin);

	return ujThreadPush(t, port, false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_gpioSetHi(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt8 port, pin;
	Boolean hi;
	
	hi = !!ujThreadPop(t);
	pin = ujThreadPop(t);
	port = ujThreadPop(t);
	
	if(port >= GPIO_PORTS) return UJ_ERR_ARRAY_INDEX_OOB;
	if(pin >= GPIO_PINS_PER_PORT) return UJ_ERR_ARRAY_INDEX_OOB;
	
	gpioSetVal(port, pin, hi);
	
	return UJ_ERR_NONE;
}

static UInt8 natUc_eepromSize(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	return ujThreadPush(t, EEPROM_SIZE, false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_eepromWrite(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt8 val = ujThreadPop(t);
	UInt32 addr = ujThreadPop(t);

	if(addr >= EEPROM_SIZE) return UJ_ERR_ARRAY_INDEX_OOB;

	eepromWrite(addr, val);

	return UJ_ERR_NONE;
}

static UInt8 natUc_eepromRead(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt32 addr = ujThreadPop(t);

	if(addr >= EEPROM_SIZE) return UJ_ERR_ARRAY_INDEX_OOB;

	return ujThreadPush(t, eepromRead(addr), false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_pwmSetBri(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt8 bri = ujThreadPop(t);

	return pwmSet(ujThreadPop(t), bri) ? UJ_ERR_NONE : UJ_ERR_ARRAY_INDEX_OOB;	
}

static UInt8 natUc_rawMemRead(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt32 addr = ujThreadPop(t);

	return ujThreadPush(t, *(volatile UInt8*)addr, false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
}

static UInt8 natUc_rawMemWrite(struct UjThread* t, _UNUSED_ struct UjClass* cls){

	UInt8 val = ujThreadPop(t);
	UInt32 addr = ujThreadPop(t);

	*(volatile UInt8*)addr = val;

	return UJ_ERR_NONE;
}

#ifdef HAVE_I2C
	static UInt8 natUc_i2cStart(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		i2cStart();
	
		return UJ_ERR_NONE;
	}
	
	static UInt8 natUc_i2cStop(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		i2cStop();
	
		return UJ_ERR_NONE;
	}
	
	static UInt8 natUc_i2cSend(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		return ujThreadPush(t, i2cSend(ujThreadPop(t)) ? 1 : 0, false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
	}
	
	static UInt8 natUc_i2cRecv(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		return ujThreadPush(t, i2cRecv(ujThreadPop(t)), false) ? UJ_ERR_NONE : UJ_ERR_STACK_SPACE;
	}
#endif

#ifdef HAVE_CHAR_LCD
	static UInt8 natUc_lcdRawWrite(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		clcdRawWrite(ujThreadPop(t));
	
		return UJ_ERR_NONE;	
	}
	
	static UInt8 natUc_lcdChar(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		clcdChar(ujThreadPop(t));
	
		return UJ_ERR_NONE;	
	}
	
	static UInt8 natUc_lcdGotoXY(struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		UInt8 y = ujThreadPop(t);
		
		clcdGotoXY(ujThreadPop(t), y);
	
		return UJ_ERR_NONE;	
	}
	
	static UInt8 natUc_lcdClear(_UNUSED_ struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		clcdClear();
		
		return UJ_ERR_NONE;
	}
#endif

#ifdef HAVE_GFX_LCD

	static UInt8 natUc_glcdClear(_UNUSED_ struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		glcdClear();
		
		return UJ_ERR_NONE;
	}
	
	static UInt8 natUc_glcdSetPixels(_UNUSED_ struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		UInt8 px = ujThreadPop(t);
		UInt8 col = ujThreadPop(t);
		
		glcdSetPixels(ujThreadPop(t), col, px);
		
		return UJ_ERR_NONE;
	}
	static UInt8 natUc_glcdSetManyPixels(_UNUSED_ struct UjThread* t, _UNUSED_ struct UjClass* cls){
	
		UInt16 numCols = ujThreadPop(t);
		UInt16 arrOfst = ujThreadPop(t);
		UInt32 arr = ujThreadPop(t);
		UInt8 col = ujThreadPop(t);
		UInt8 rowOver8 = ujThreadPop(t);
		
		glcdSetManyPixels(rowOver8, col, ((const UInt8*)ujArrayRawAccessStart(arr)) + arrOfst, numCols);
		ujArrayRawAccessFinish(arr);
		
		return UJ_ERR_NONE;
	}
#endif

const UjNativeClass nativeCls_UC =
	{
		"UC",
		0,
		0,
		NULL,
		NULL,
		
		16
	#ifdef HAVE_CHAR_LCD
		+4
	#endif
	#ifdef HAVE_GFX_LCD
		+3
	#endif
	#ifdef HAVE_I2C
		+4
	#endif
		
		,{
			{
				"clkGet",
				"()I",
				natUc_clkGet,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"clkSpeed",
				"()I",
				natUc_clkSpeed,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"instrsGet",
				"()I",
				natUc_instrsGet,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"gpioGetNumPorts",	//() -> int nump orts
				"()I",
				natUc_gpioGetNumPorts,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"gpioGetPortPins",	//() -> int num pins/port
				"()I",
				natUc_gpioGetNumPins,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"gpioSetOutput",	//(int port, int pin, boolean output) -> void
				"(IIZ)V",
				natUc_gpioSetOutput,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"gpioIsHi",		//(int port, int pin) -> boolean isHi
				"(II)Z",
				natUc_gpioIsHi,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"gpioSetHi",		//(int port, int pin, boolean setHi) -> void
				"(IIZ)V",
				natUc_gpioSetHi,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{	"eepromSize",	//()-> I
				"()I",
				natUc_eepromSize,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{	"eepromWrite",	//(IB)-> ()
				"(IB)V",
				natUc_eepromWrite,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{	"eepromRead",	//(I)-> B
				"(I)B",
				natUc_eepromRead,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
		#ifdef HAVE_CHAR_LCD
			{
				"lcdClear",	//() -> ()
				"()V",
				natUc_lcdClear,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"lcdGotoXY",	//(II) -> ()
				"(II)V",
				natUc_lcdGotoXY,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"lcdChar",	//(C) -> ()
				"(C)V",
				natUc_lcdChar,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"lcdRawWrite",	//(I) -> ()
				"(I)V",
				natUc_lcdRawWrite,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
		#endif
		#ifdef HAVE_GFX_LCD
			{
				"glcdClear",	//() -> ()
				"()V",
				natUc_glcdClear,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"glcdSetPixels",	//(IIB) -> ()
				"(IIB)V",
				natUc_glcdSetPixels,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"glcdSetManyPixels",	//(II[BII) -> ()
				"(II[BII)V",
				natUc_glcdSetManyPixels,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
		#endif
		#ifdef HAVE_I2C
			{
				"i2cStart",	//() -> ()
				"()V",
				natUc_i2cStart,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"i2cSend",	//(B) -> Z
				"(B)Z",
				natUc_i2cSend,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"i2cRecv",	//(Z) -> B
				"(Z)B",
				natUc_i2cRecv,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"i2cStop",	//() -> ()
				"()V",
				natUc_i2cStop,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
		#endif
			{
				"pwmSetBri",	//(II) -> ()
				"(II)V",
				natUc_pwmSetBri,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"rawMemRead",	//(I)-> B
				"(I)B",
				natUc_rawMemRead,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			},
			{
				"rawMemWrite",	//(IB)-> ()
				"(IB)V",
				natUc_rawMemWrite,
				JAVA_ACC_PUBLIC | JAVA_ACC_NATIVE | JAVA_ACC_STATIC
			}
		}
	};


