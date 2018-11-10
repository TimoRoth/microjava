SHELL := bash -O globstar

JAVAC    ?= javac
TOBIN    ?= ./tobin.sh
CLASSCVT ?= ../classCvt/classCvt

RT_SOURCES = $(shell ls RT/**/*.java)
RT_CLASSES = $(RT_SOURCES:.java=.rtclass)
RT_UJC     = $(RT_SOURCES:.java=.rtujc)

LOCAL_SOURCES = $(shell ls *.java)
LOCAL_CLASSES = $(LOCAL_SOURCES:.java=.class)
LOCAL_UJC     = $(LOCAL_SOURCES:.java=.ujc)
LOCAL_C       = $(LOCAL_SOURCES:.java=.c)

all: runtime $(LOCAL_C)

classCvt:
ifeq ($(CLASSCVT),../classCvt/classCvt)
	$(MAKE) -C ../classCvt
endif

%.class: %.java runtime
	"$(JAVAC)" -source 1.8 -target 1.8 -classpath "$(PWD)/RT/real:$(PWD)/RT/fake:$(PWD)" "$<"

%.rtclass: %.java
	"$(JAVAC)" -source 1.8 -target 1.8 -classpath "$(PWD)/RT/real:$(PWD)/RT/fake:$(PWD)" "$<"
	cp "$(patsubst %.rtclass,%.class,$@)" "$@"

%.rtujc: %.rtclass classCvt
	"$(CLASSCVT)" <"$<" >"$@"

%.ujc: %.class classCvt
	"$(CLASSCVT)" <"$<" >"$@"

%.c: %.ujc
	"$(TOBIN)" "$@" "$<" $(RT_UJC)
	"$(TOBIN)" "$(patsubst %.c,%.raw.ujc,$@)" "$<" $(RT_UJC)

runtime: $(RT_UJC)

rtclean:
	rm -f $(RT_SOURCES:.java=.class) $(RT_CLASSES) $(RT_UJC)

clean: rtclean
	rm -f $(LOCAL_CLASSES) $(LOCAL_UJC)

.PHONY: all runtime rtclean clean classCvt
.PRECIOUS: %.class %.rtclass %.ujc %rtucj
