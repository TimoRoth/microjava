CC	= gcc
LD	= gcc


CC_FLAGS	= -O0 -g -ggdb -ggdb3 -D_FILE_OFFSET_BITS=64 -D__USE_LARGEFILE64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
LD_FLAGS	= -O0 -g -ggdb -ggdb3

LDFLAGS = $(LD_FLAGS) -Wall -Wextra
CCFLAGS = $(CC_FLAGS) -Wall -Wextra

APP1	= classCvt
APP2	= tobin
APPS	= $(APP1) $(APP2)
OBJS1	= $(EXTRA_OBJS) main.o classAccess.o bb.o classOptimizer.o
OBJS2	= tobin.c

default: $(APPS)

$(APP1): $(OBJS1)
	$(LD) $(LDFLAGS) -o $(APP1) $(OBJS1)

$(APP2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $(APP2) $(OBJS2)

main.o: main.c common.h class.h classAccess.h bb.h classOptimizer.h
	$(CC) $(CCFLAGS) -o main.o -c main.c

classAccess.o: classAccess.c classAccess.h class.h common.h
	$(CC) $(CCFLAGS) -o classAccess.o -c classAccess.c
	
classOptimizer.o: classOptimizer.c classOptimizer.h class.h common.h
	$(CC) $(CCFLAGS) -o classOptimizer.o -c classOptimizer.c
	
bb.o: bb.c bb.h
	$(CC) $(CCFLAGS) -o bb.o -c bb.c
	
clean:
	rm -f $(APP) *.o *.class *.ujc

test: $(APP)
	./$(APP) < ../uJ/Test.class > Test.ujc
	cat Test.ujc | wc
	cat ../uJ/Test.class | wc


