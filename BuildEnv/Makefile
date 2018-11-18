SHELL := bash -O globstar

JAVAC    ?= javac
TOBIN    ?= $(CURDIR)/tobin.sh
CLASSCVT ?= $(CURDIR)/../classCvt/classCvt

RT_R_SOURCES = $(shell ls $(CURDIR)/RT{,_tmp}/real/**/*.java)
RT_R_CLASSES = $(RT_R_SOURCES:.java=.rtclass)
RT_R_UJC     = $(RT_R_SOURCES:.java=.rtujc)

RT_F_SOURCES = $(shell ls $(CURDIR)/RT{,_tmp}/fake/**/*.java)
RT_F_CLASSES = $(RT_F_SOURCES:.java=.rtclass)
RT_F_UJC     = $(RT_F_SOURCES:.java=.rtujc)

LOCAL_SOURCES = $(shell ls $(CURDIR)/*.java)
LOCAL_CLASSES = $(LOCAL_SOURCES:.java=.class)
LOCAL_UJC     = $(LOCAL_SOURCES:.java=.ujc)
LOCAL_C       = $(LOCAL_SOURCES:.java=.c)

all: runtime $(LOCAL_C)

classCvt:
ifeq ($(CLASSCVT),$(CURDIR)/../classCvt/classCvt)
	"$(MAKE)" -C $(CURDIR)/../classCvt
endif

%.class: %.java runtime
	"$(JAVAC)" -source 1.6 -target 1.6 -classpath "$(CURDIR)/RT/real:$(CURDIR)/RT/fake:$(CURDIR)/RT_tmp/real:$(CURDIR)/RT_tmp/fake:$(CURDIR)" "$<"

%.rtclass: %.java
	"$(JAVAC)" -source 1.6 -target 1.6 -classpath "$(CURDIR)/RT/real:$(CURDIR)/RT/fake:$(CURDIR)/RT_tmp/real:$(CURDIR)/RT_tmp/fake:$(CURDIR)" "$<"
	cp "$(patsubst %.rtclass,%.class,$@)" "$@"

%.rtujc: %.rtclass classCvt
	"$(CLASSCVT)" <"$<" >"$@"

%.ujc: %.class classCvt
	"$(CLASSCVT)" <"$<" >"$@"

%.c: %.ujc
	"$(TOBIN)" "$@" "$<" $(RT_R_UJC)
	"$(TOBIN)" "$(patsubst %.c,%.raw,$@)" "$<" $(RT_R_UJC)

runtime: $(RT_F_UJC) $(RT_R_UJC)

rtclean:
	rm -f $(RT_F_SOURCES:.java=.class) $(RT_R_SOURCES:.java=.class) $(RT_F_CLASSES) $(RT_R_CLASSES) $(RT_R_UJC) $(RT_F_UJC)

clean: rtclean
	rm -f $(LOCAL_CLASSES) $(LOCAL_UJC)

.PHONY: all runtime rtclean clean classCvt
.PRECIOUS: %.class %.rtclass %.ujc %.rtucj
