java_sources := $(wildcard *.java)

all: $(java_sources:.java=.inc)

%.inc: %.java
	rm -rf "$(CURDIR)/../../BuildEnv/RT_tmp"
	cp "$<" "$(CURDIR)/../../BuildEnv"
	cp -r RT/. "$(CURDIR)/../../BuildEnv/RT_tmp"
	"$(MAKE)" -C "$(CURDIR)/../../BuildEnv"
	cp "$(CURDIR)/../../BuildEnv/$(patsubst %.java,%.c,$<)" "$@"
	cp "$(CURDIR)/../../BuildEnv/$(patsubst %.java,%.raw,$<)" "$(patsubst %.java,%.ujcpak,$<)"
	sed -i "s/unsigned/static const unsigned/" "$@"

.PHONY: all