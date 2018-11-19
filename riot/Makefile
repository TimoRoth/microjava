APPLICATION = microjava_host

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

FEATURES_REQUIRED += periph_gpio
FEATURES_OPTIONAL += periph_gpio_irq
USEMODULE += sema xtimer

# Java to UJC target
EXTERNAL_MODULE_DIRS += $(CURDIR)/java_gen
USEMODULE += java_gen

# uJ itself
EXTERNAL_MODULE_DIRS += $(CURDIR)/uJ
USEMODULE += uJ

# Basic uJ settings
CFLAGS += -ggdb -DUJ_LOG -DUJ_FTR_SUPPORT_UJC_FORMAT -DUJ_OPT_CLASS_SEARCH -DUJ_OPT_RAM_STRINGS -DUJ_FTR_STRING_FEATURES -DUJ_FTR_SYNCHRONIZATION -DUJ_FTR_SUPPORT_EXCEPTIONS

# uJ Debug Helpers
CFLAGS += -DUJ_DBG_HELPERS -DDEBUG_HEAP

# uJ Heap Size
CFLAGS += -DUJ_HEAP_SZ=4096

include $(RIOTBASE)/Makefile.include
