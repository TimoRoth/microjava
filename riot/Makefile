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

USEMODULE += xtimer

# Using VFS for file access
USEMODULE += vfs

# ESP-NOW settings
ESPNOW_PASS ?= ThisistheRIOTporttoESP
ESPNOW_CHANNEL ?= 6

# Board Specifics
ifneq (,$(findstring esp32,$(BOARD)))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp32
  USEMODULE += nat_esp32

  CFLAGS += "-DESP_NOW_SOFT_AP_PASS=\"$(ESPNOW_PASS)\"" -DESP_NOW_CHANNEL=$(ESPNOW_CHANNEL)
  USEMODULE += esp_now auto_init_gnrc_netif gnrc_ipv6_default gnrc_icmpv6_echo

  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp_spiffs
  USEMODULE += esp_spiffs mtd spiffs nat_esp_spiffs

  WITH_COAP_SUPPORT ?= 1
  WITH_I2C_SUPPORT ?= 1
  WITH_SPI_SUPPORT ?= 1
else ifneq (,$(findstring wittycloud,$(BOARD))$(findstring esp8266,$(BOARD)))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp8266
  USEMODULE += nat_esp8266

  CFLAGS += "-DESP_NOW_SOFT_AP_PASS=\"$(ESPNOW_PASS)\"" -DESP_NOW_CHANNEL=$(ESPNOW_CHANNEL)
  USEMODULE += esp_now auto_init_gnrc_netif gnrc_ipv6_default gnrc_icmpv6_echo

  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/esp_spiffs
  USEMODULE += esp_spiffs mtd spiffs nat_esp_spiffs

  WITH_COAP_SUPPORT ?= 1
  WITH_I2C_SUPPORT ?= 1
  WITH_SPI_SUPPORT ?= 1
else ifneq (,$(findstring bluepill,$(BOARD)))
  WITH_I2C_SUPPORT ?= 1
  WITH_SPI_SUPPORT ?= 1
else ifeq (native,$(BOARD))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/littlefs
  USEMODULE += mtd littlefs nat_littlefs
  CFLAGS += -DVFS_FILE_BUFFER_SIZE=56 -DVFS_DIR_BUFFER_SIZE=44

  USEMODULE += gnrc_netdev_default auto_init_gnrc_netif gnrc_ipv6_default gnrc_icmpv6_echo

  WITH_COAP_SUPPORT ?= 1
endif

EMBED_DEFAULT_JAVA ?= 1
ifneq (0,$(EMBED_DEFAULT_JAVA))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/constfs
  USEMODULE += constfs nat_constfs
endif

WITH_GPIO_SUPPORT ?= 1
ifneq (0,$(WITH_GPIO_SUPPORT))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/gpio
  USEMODULE += nat_gpio
  FEATURES_REQUIRED += periph_gpio
  FEATURES_OPTIONAL += periph_gpio_irq
endif

WITH_I2C_SUPPORT ?= 0
ifneq (0,$(WITH_I2C_SUPPORT))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/i2c
  USEMODULE += nat_i2c
  FEATURES_REQUIRED += periph_i2c
endif

WITH_SPI_SUPPORT ?= 0
ifneq (0,$(WITH_SPI_SUPPORT))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/spi
  USEMODULE += nat_spi
  FEATURES_REQUIRED += periph_spi
endif

WITH_COAP_SUPPORT ?= 0
ifneq (0,$(WITH_COAP_SUPPORT))
  EXTERNAL_MODULE_DIRS += $(CURDIR)/nat/gcoap
  USEMODULE += gcoap nat_gcoap
endif

WITH_SHELL_SUPPORT ?= 1
ifneq (0,$(WITH_SHELL_SUPPORT))
  USEMODULE += shell shell_commands ps
endif

include $(RIOTBASE)/Makefile.include
