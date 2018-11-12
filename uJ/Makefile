#
#	Size approximations
#	
#					------------------AVR----------------
#	FEATURE				+CodeSz		+DataSz		Notes
#	UJ_OPT_CLASS_SEARCH		486		0		-
#	UJ_FTR_SUPPORT_FLOAT		1086		264		-
#	UJ_FTR_SUPPORT_LONG		7536		0		-
#	UJ_FTR_SUPPORT_DOUBLE 		25356		24		-
#	UJ_FTR_SUPPORT_CLASS_FORMAT	2768		6		less if together

#VM optimizations
VMOPTS = -DUJ_OPT_CLASS_SEARCH -DUJ_FTR_SYNCHRONIZATION -DUJ_FTR_STRING_FEATURES
VMFEATURES = -DUJ_FTR_SUPPORT_EXCEPTIONS -DUJ_FTR_SUPPORT_UJC_FORMAT -DUJ_FTR_SUPPORT_CLASS_FORMAT -DUJ_FTR_SUPPORT_LONG -DUJ_FTR_SUPPORT_FLOAT -DUJ_FTR_SUPPORT_DOUBLE -DUJ_OPT_RAM_STRINGS

APP = uJ
OBJS = main.o uj.o ujHeap.o long64.o double64.o

CFLAGS     += -Og -g -ggdb -ggdb3 -DUJ_DBG_HELPERS -DDEBUG_HEAP -DUJ_LOG
LDFLAGS    += -Og -g -ggdb -ggdb3
MATH64_SRC  = long64_soft.c
DOUBLE_SRC  = double64_soft.c

CC = gcc
LD = gcc

LDFLAGS += -Wall -Wextra -pedantic -lm
CFLAGS  += -std=c11 -fno-strict-aliasing -Wall -Wextra -pedantic $(VMFEATURES) $(VMOPTS)

all: $(APP)

$(APP): $(OBJS)
	$(ECODE)
	$(LD) -o $(APP) $(OBJS) $(LDFLAGS)

%.o: %.c common.h uj.h ujHeap.h
	$(CC) $(CFLAGS) -o $@ -c $<

long64.o: long64_soft.o
	cp $< $@

double64.o: double64_soft.o
	cp $< $@

clean:
	rm -f *.o

.PHONY: all clean
