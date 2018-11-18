java_targets := $(wildcard *.java)
SRC := $(java_targets:.java=.c)

%.c: %.java
	rm -rf "$(CURDIR)/../../BuildEnv/RT_tmp"
	cp "$<" "$(CURDIR)/../../BuildEnv"
	cp -r RT/. "$(CURDIR)/../../BuildEnv/RT_tmp"
	"$(MAKE)" -C "$(CURDIR)/../../BuildEnv"
	cp "$(CURDIR)/../../BuildEnv/$(patsubst %.java,%.c,$<)" "$@"

include $(RIOTBASE)/Makefile.base
