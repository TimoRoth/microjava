
BUILD	?= PC


#
#	Size approximations
#	
#					------------------AVR----------------
#	FEATURE				+CodeSz		+DataSz		Notes
#	UJ_OPT_CLASS_SEARCH		486		0		-
#	UJ_FTR_SUPPORT_FLOAT		1086		264		-
#	UJ_FTR_SUPPORT_LONG		7536		0		-
#	UJ_FTR_SUPPORT_DOUBLE 		25356		24		-
#	UJ_FTR_SUPPORT_CLASS_FORMAT	2768		6		less if toge-



#VM optimizations
VMOPTS		= -DUJ_OPT_CLASS_SEARCH -DUJ_FTR_SYNCHRONIZATION -DUJ_FTR_STRING_FEATURES


APP	= uJ
OBJS	= main.o uj.o ujHeap.o long64.o double64.o


ifeq ($(BUILD), PC)

	#debug
	VMFEATURES	= -DUJ_FTR_SUPPORT_EXCEPTIONS -DUJ_FTR_SUPPORT_UJC_FORMAT -DUJ_FTR_SUPPORT_CLASS_FORMAT -DUJ_FTR_SUPPORT_LONG -DUJ_FTR_SUPPORT_FLOAT -DUJ_FTR_SUPPORT_DOUBLE -DUJ_OPT_RAM_STRINGS
	CCFLAGS		+= -O0 -g -ggdb -ggdb3 -DUJ_DBG_HELPERS -DDEBUG_HEAP -DUJ_LOG
	LDFLAGS		+= -O0 -g -ggdb -ggdb3
	MAINFILE	= pc_main.c
	MATH64_SRC	= long64_soft.c
	DOUBLE_SRC	= double64_soft.c
	CPU		= X86
	BOARD		= PC
	CC	= gcc
	LD	= gcc

else ifeq ($(BUILD), avr)

	#avr
	
	OBJS += uc_HD44780.o
	
	ifeq ($(ATMEGA64), yes)
		
		VMFEATURES	= -DUJ_FTR_SUPPORT_EXCEPTIONS -DUJ_FTR_SUPPORT_UJC_FORMAT -DUJ_FTR_SUPPORT_CLASS_FORMAT -DUJ_FTR_SUPPORT_FLOAT
		CPU		= ATMEGA64
		OBJS		+= uc_loader.o uc_FATFS.o uc_SDMMC.o
		BOARD		= DIANA
		CCDEVICE	= atmega644p
		PROGDEVICE	= ATmega644
		SIMDEVICE	= atmega644a
		LD_EXTRA	= -Wl,-section-start=.lastpage=0xFF00 
		OPT		= -Os -mcall-prologues
		BOARDFILE	= uc_brd_diana
	else
		
		VMFEATURES	= -DUJ_FTR_SUPPORT_EXCEPTIONS -DUJ_FTR_SUPPORT_UJC_FORMAT -DUJ_FTR_SUPPORT_CLASS_FORMAT -DUJ_FTR_SUPPORT_LONG -DUJ_FTR_SUPPORT_FLOAT -DUJ_FTR_SUPPORT_DOUBLE -DUJ_OPT_RAM_STRINGS
		CPU		= ATMEGA128
		BOARD		= MEGA128
		CCDEVICE	= atmega1284p
		PROGDEVICE	= ATmega1284p
		SIMDEVICE	= atmega128
		OPT		= -O2
		BOARDFILE	= uc_brd_mega128
	endif
	
	OBJS += $(BOARDFILE).o
	
	ifeq ($(SIM), yes)
		AVR_CMD		= simulavr -d $(SIMDEVICE) -f uJ -W 0x20,- #-g
		CCFLAGS		= -DSIM=0x20 -DUJ_LOG -DUJ_DBG_HELPERS -DDEBUG_HEAP
		OPT		= -O0 -g -ggdb3
	else
		AVR_CMD		= sudo avrdude -p $(PROGDEVICE) -c avrisp2 -P usb -U flash:w:$(APP).hex:i
	endif

	#for smaller code that is slower, change "-O2" into "-Os -mcall-prologues"

	CCFLAGS		+= $(OPT) -mmcu=$(CCDEVICE) -fomit-frame-pointer -I/usr/lib/avr/include -ffunction-sections
	LDFLAGS		+= $(OPT) -mmcu=$(CCDEVICE) -Wl,--gc-sections -lm $(LD_EXTRA)
	CC		= avr-gcc
	LD		= avr-gcc
	CCFEATURES	= -DCOMPILER_SUPPORTS_LONG_LONG
	MAINFILE	= uc_main.c
	EXTRA		= avr-size -Ax $(APP) && avr-objcopy -j .text -j .data -j .lastpage -O ihex $(APP) $(APP).hex
	CODE		= __code.c
	EOBJ		= __code.o
	ECODE		= $(CC) $(CCFLAGS) -o $(EOBJ) -c $(CODE)
	MATH64_SRC	= long64_avr.S
	DOUBLE_SRC	= double64_soft.c
	
	ifeq ($(ATMEGA64), yes)
	
		ECODE	= 
		EOBJ	= 
		CODE	= 
	endif
	
	ifeq ($(SIM), yes)
		CC_FLAGS += -DSIM=0x20
	endif
endif

LDFLAGS += -Wall -Wextra -lm
CCFLAGS += -Wall -Wextra $(VMFEATURES) $(VMOPTS) $(CCFEATURES) -DBOARD_$(BOARD) -DCPU_$(CPU)



$(APP): $(OBJS) $(CODE)

	$(ECODE)
	$(LD) $(LDFLAGS) -o $(APP) $(OBJS) $(EOBJ)
	$(EXTRA)
	
AVR:	$(APP)
	$(AVR_CMD)

__code.c:	Test.java
	make -C BUILDENV CLASS=Test OUTPUT=__code.c

$(BOARDFILE).o: $(BOARDFILE).c common.h
	$(CC) $(CCFLAGS) -o $(BOARDFILE).o -c $(BOARDFILE).c

long64.o: $(MATH64_SRC) long64.h common.h
	$(CC) $(CCFLAGS) -o long64.o -c $(MATH64_SRC)

uc_HD44780.o: uc_HD44780.h uc_HD44780.c common.h
	$(CC) $(CCFLAGS) -o uc_HD44780.o -c uc_HD44780.c

uc_SDMMC.o: uc_SDMMC.h uc_SDMMC.c common.h
	$(CC) $(CCFLAGS) -o uc_SDMMC.o -c uc_SDMMC.c

uc_FATFS.o: uc_FATFS.h uc_FATFS.c common.h
	$(CC) $(CCFLAGS) -o uc_FATFS.o -c uc_FATFS.c

uc_loader.o: uc_loader.h uc_loader.c common.h
	$(CC) $(CCFLAGS) -o uc_loader.o -c uc_loader.c

main.o: $(MAINFILE) common.h uj.h ujHeap.h
	$(CC) $(CCFLAGS) -o main.o -c $(MAINFILE)

ujHeap.o: ujHeap.c ujHeap.h uj.h common.h
	$(CC) $(CCFLAGS) -o ujHeap.o -c ujHeap.c
	
uj.o: uj.c uj.h common.h long64.h ujHeap.h double64.h
	$(CC) $(CCFLAGS) -o uj.o -c uj.c

double64.o: $(DOUBLE_SRC) double64.h long64.h common.h
	$(CC) $(CCFLAGS) -o double64.o -c $(DOUBLE_SRC)

clean:
	rm -f $(APP) *.o *.class __code.c

test: $(APP)
	make -C BUILDENV CLASS=Test BUILD=just_filelist OUTPUT=filelist.txt
	time -p ./$(APP) `cat filelist.txt`

test2: $(APP)
	make -C BUILDENV CLASS=Test BUILD=just_filelist OUTPUT=filelist.txt SKIPSELF=yes
	../classCvt/classCvt < Test.class > Test.ujc
	rm Test.class
	time -p ./$(APP) Test.ujc `cat filelist.txt`

RT:
	make -C BUILDENV rtclean
	make -C BUILDENV runtime

classes:
	CLASSPATH=java_classes
	find java_classes -iname "*.class" | rm -f
	find java_classes -iname "*.java" | xargs javac

