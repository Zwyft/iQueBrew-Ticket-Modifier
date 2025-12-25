BUILD_DIR = build
ROM_NAME = ique_menu

# Source files
SRCS = src/main.c

# Libdragon paths (set N64_INST environment variable)
N64_INST ?= /opt/libdragon

include $(N64_INST)/include/n64.mk

all: $(ROM_NAME).z64

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(ROM_NAME).elf: $(SRCS) | $(BUILD_DIR)
	$(N64_CC) $(N64_CFLAGS) -o $@ $^ $(N64_LDFLAGS)

$(ROM_NAME).z64: $(BUILD_DIR)/$(ROM_NAME).elf
	$(N64_OBJCOPY) -O binary $< $(BUILD_DIR)/$(ROM_NAME).bin
	$(N64_TOOL) -l 2M -h $(N64_INST)/mips64-elf/lib/header -o $@ $(BUILD_DIR)/$(ROM_NAME).bin

clean:
	rm -rf $(BUILD_DIR) $(ROM_NAME).z64

.PHONY: all clean
