# This is the root Makefile.

# If called by the CI build script, the required targets are:
#   * default (no target)
#   * clean

# Additionally, a local developer may want to use:
#   * flash - flash the most recent build
#   * erase - wipe the connected device

# Makefile defaults to building the first product
PRODUCT_ID?=1

# Makefile defaults to building for the latest version of the PCB
ifeq ($(PRODUCT_ID),1)
PCBVER?=1
endif

# Get the configuration for this combination, and check that the given PRODUCT_ID and PCBVER are valid.
include products.mk

# Always run the commands to build these targets
TARGETS = clean
.PHONY: $(TARGETS)


all:
	@echo Building default target for $(PRODUCT_NAME), PCB $(PCBVER)
	@docker run --rm -t -v "$(CURDIR)":/build -w /build -e "PRODUCT_ID=$(PRODUCT_ID)" -e "PCBVER=$(PCBVER)" -e "PRODUCT_NAME=$(PRODUCT_NAME)" -e "PRODUCT_BOOTLOADER_NAME=$(PRODUCT_BOOTLOADER_NAME)" "$(BUILD_ENVIRONMENT)" make -f $(BUILD_FILE)

$(TARGETS):
	@echo Building target $@ for $(PRODUCT_NAME), PCB $(PCBVER)
	@docker run --rm -t -v "$(CURDIR)":/build -w /build -e "PRODUCT_ID=$(PRODUCT_ID)" -e "PCBVER=$(PCBVER)" -e "PRODUCT_NAME=$(PRODUCT_NAME)" -e "PRODUCT_BOOTLOADER_NAME=$(PRODUCT_BOOTLOADER_NAME)" "$(BUILD_ENVIRONMENT)" make -f $(BUILD_FILE) $@


