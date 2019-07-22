PROGRAM = esp-homekit-cooler-fan
PROGRAM_SRC_DIR = src

EXTRA_COMPONENTS = \
	extras/http-parser \
	extras/dht \
	$(abspath components/ir) \
	$(abspath components/wolfssl) \
	$(abspath components/cjson) \
	$(abspath components/homekit)

FLASH_SIZE ?= 32
ESPBAUD = 921600

EXTRA_CFLAGS += -I../.. -DHOMEKIT_SHORT_APPLE_UUIDS

include $(SDK_PATH)/common.mk

monitor:
	$(FILTEROUTPUT) --port $(ESPPORT) --baud 115200 --elf $(PROGRAM_OUT)
