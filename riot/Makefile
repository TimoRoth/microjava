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

INCLUDES += -I$(CURDIR)
DIRS += $(CURDIR)/java

# uJ itself
EXTERNAL_MODULE_DIRS += $(CURDIR)/uJ
USEMODULE += uJ
# Basic uJ settings
CFLAGS += -ggdb -DUJ_LOG -DUJ_FTR_SUPPORT_UJC_FORMAT -DUJ_OPT_CLASS_SEARCH -DUJ_OPT_RAM_STRINGS -DUJ_FTR_STRING_FEATURES -DUJ_FTR_SYNCHRONIZATION -DUJ_FTR_SUPPORT_EXCEPTIONS
# uJ Debug Helpers
CFLAGS += -DUJ_DBG_HELPERS -DDEBUG_HEAP
# uJ Heap Size
CFLAGS += -DUJ_HEAP_SZ=4096

# Using VFS for file access
USEMODULE += vfs

# Wifi AP settings, used for esp_wifi on esp32 and esp_now on esp8266(only pw)
STA_SSID ?= RIOT_AP
STA_PASSWORD ?= RIOT_ESP_PW

# Board Specifics
ifneq (,$(findstring esp32,$(BOARD)))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp32
  USEMODULE += nat_esp32

  CFLAGS += "-DCONFIG_WIFI_STA_SSID=\"$(STA_SSID)\"" "-DCONFIG_WIFI_STA_PASSWORD=\"$(STA_PASSWORD)\""
  USEMODULE += esp_wifi

  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp_spiffs
  USEMODULE += esp_spiffs mtd spiffs nat_esp_spiffs
else ifneq (,$(findstring wittycloud,$(BOARD))$(findstring esp8266,$(BOARD)))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp8266
  USEMODULE += nat_esp8266

  CFLAGS += "-DESP_NOW_SOFT_AP_PASS=\"$(STA_PASSWORD)\""
  USEMODULE += esp_now

  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp_spiffs
  USEMODULE += esp_spiffs mtd spiffs nat_esp_spiffs
endif

EMBED_DEFAULT_JAVA ?= 1
ifneq (0,$(EMBED_DEFAULT_JAVA))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/constfs
  USEMODULE += constfs nat_constfs
endif

include $(RIOTBASE)/Makefile.include