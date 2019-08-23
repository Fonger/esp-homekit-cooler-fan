PROGRAM = esp-homekit-cooler-fan
PROGRAM_SRC_DIR = src

EXTRA_COMPONENTS = \
	extras/http-parser \
	extras/dht \
	extras/pwm \
	extras/ssd1306 extras/i2c extras/fonts \
	$(abspath components/ir) \
	$(abspath components/wolfssl) \
	$(abspath components/cjson) \
	$(abspath components/homekit)

FONTS_GLCD_5X7 = 1
FONTS_BITOCRA_4X7 = 1
FONTS_BITOCRA_6X11 = 1
FONTS_BITOCRA_7X13 = 1
FONTS_TERMINUS_6X12_ISO8859_1 = 1
FONTS_TERMINUS_8X14_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_8X14_ISO8859_1 = 1
FONTS_TERMINUS_10X18_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_10X18_ISO8859_1 = 1
FONTS_TERMINUS_11X22_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_11X22_ISO8859_1 = 1
FONTS_TERMINUS_12X24_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_12X24_ISO8859_1 = 1
FONTS_TERMINUS_14X28_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_14X28_ISO8859_1 = 1
FONTS_TERMINUS_16X32_ISO8859_1 = 1
FONTS_TERMINUS_BOLD_16X32_ISO8859_1 = 1

FLASH_SIZE ?= 32
ESPBAUD = 921600

LIBS += m
EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS

include $(SDK_PATH)/common.mk

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
