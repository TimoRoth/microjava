java_targets := $(wildcard *.java)
SRC := $(java_targets:.java=.c)

%.c: %.java
	@cp $< $(CURDIR)/../../BuildEnv
	"$(MAKE)" -C $(CURDIR)/../../BuildEnv
	@cp $(CURDIR)/../../BuildEnv/$(patsubst %.java,%.c,$<) $@

include $(RIOTBASE)/Makefile.base
