# This Makefile is called within the Docker environment
# It should be used for building the firmware binaries

# Configure as required here
CHIP=nrf52840_xxaa
FAMILY=NRF52840 # Note that this must be NRF52840 if using this chip
BOARD=pca10056
SD_TYPE=s140
SD_VERSION=7.2.0
SD_FWID=0x0100 # Look in $(SDK_ROOT)/components/softdevice/$(SD_TYPE)/doc/
SDK_ROOT=/nrf5/nRF5_SDK_17.0.2

# Directories used to store files
TEMP_DIR = tmp
ARTEFACTS_DIR = artefacts
APP_MK_DIR = app/src/$(BOARD)/$(SD_TYPE)/armgcc
DTM_MK_DIR = dtm/src/$(BOARD)/blank/armgcc
BOOT_MK_DIR = boot/secure_bootloader/$(BOARD)_$(SD_TYPE)_ble/armgcc

# Files generated as part of the build process
APPLICATION = $(APP_MK_DIR)/_build_$(COMBINED_PRODUCT_HARDWARE_VERSION)/$(CHIP).hex
BOOTLOADER = $(BOOT_MK_DIR)/_build_$(COMBINED_PRODUCT_HARDWARE_VERSION)/$(CHIP)_$(SD_TYPE).hex
SOFTDEVICE = $(SDK_ROOT)/components/softdevice/$(SD_TYPE)/hex/$(SD_TYPE)_nrf52_$(SD_VERSION)_softdevice.hex
BL_SETTINGS = $(TEMP_DIR)/bootloadersettings.hex

# Human-readable identifier for artefacts
VERSION_IDENTIFIER = $(PRODUCT_ID).$(PCBVER)

# Combined version is a single number which combines both the Product ID and PCB revision. Useful for bootloader hardware version number...
A = $(PRODUCT_ID)
B = $(PCBVER)
COMBINED_PRODUCT_HARDWARE_VERSION=$(shell echo $$(($A*256+$B)))

# Identify the number of processors present on the build machine. Use this to set the number of jobs for best performance.
NUMPROC := $(shell grep -c "processor" /proc/cpuinfo)
ifeq ($(NUMPROC),0)
        NUMPROC = 1
endif
NUMJOBS := $(shell echo $$(($(NUMPROC)*2)))


.PHONY: all clean
all:
	mkdir -p $(TEMP_DIR)
	@echo "project.mk is building for $(PRODUCT_NAME) with PRODUCT_ID=$(PRODUCT_ID), PCBVER=$(PCBVER)"
	
	# Build the sub-projects
	make -j$(NUMJOBS) PASS_LINKER_INPUT_VIA_FILE=0 SDK_ROOT=$(SDK_ROOT) OUTPUT_DIRECTORY=_build_$(COMBINED_PRODUCT_HARDWARE_VERSION) -C $(APP_MK_DIR)
	make -j$(NUMJOBS) PASS_LINKER_INPUT_VIA_FILE=0 SDK_ROOT=$(SDK_ROOT) OUTPUT_DIRECTORY=_build_$(COMBINED_PRODUCT_HARDWARE_VERSION) -C $(DTM_MK_DIR)
	make -j$(NUMJOBS) PASS_LINKER_INPUT_VIA_FILE=0 SDK_ROOT=$(SDK_ROOT) OUTPUT_DIRECTORY=_build_$(COMBINED_PRODUCT_HARDWARE_VERSION) COMBINED_PRODUCT_HARDWARE_VERSION=$(COMBINED_PRODUCT_HARDWARE_VERSION) -C $(BOOT_MK_DIR)
	
	# Make the artefacts directory if it does not exist
	mkdir -p $(ARTEFACTS_DIR)
	
	# Build the DFU package from the app
	nrfutil pkg generate --hw-version $(COMBINED_PRODUCT_HARDWARE_VERSION) --application-version 1 --application $(APPLICATION) --sd-req $(SD_FWID) --key-file private.pem $(ARTEFACTS_DIR)/dfu-$(VERSION_IDENTIFIER).zip
	
	# Merge hex files to form a complete package
	# In order to boot into the app immediately after flashing, the bootloader's settings page needs to be written
	nrfutil settings generate --family $(FAMILY) --application $(APPLICATION) --application-version 0 --bootloader-version 0 --bl-settings-version 1 $(BL_SETTINGS)
	srec_cat $(SOFTDEVICE) -intel $(APPLICATION) -intel $(BOOTLOADER) -intel $(BL_SETTINGS) -intel -o $(ARTEFACTS_DIR)/img-$(VERSION_IDENTIFIER).hex -intel -address-length=4
	
	# For convenience, store this hex file as our latest successful build
	cp $(ARTEFACTS_DIR)/img-$(VERSION_IDENTIFIER).hex latest.hex

clean:
	@echo "project.mk is cleaning for $(PRODUCT_NAME) with PRODUCT_ID=$(PRODUCT_ID), PCBVER=$(PCBVER)"
	make SDK_ROOT=$(SDK_ROOT) OUTPUT_DIRECTORY=_build_$(COMBINED_PRODUCT_HARDWARE_VERSION) -C $(APP_MK_DIR) clean
	make SDK_ROOT=$(SDK_ROOT) OUTPUT_DIRECTORY=_build_$(COMBINED_PRODUCT_HARDWARE_VERSION) -C $(DTM_MK_DIR) clean
	make SDK_ROOT=$(SDK_ROOT) OUTPUT_DIRECTORY=_build_$(COMBINED_PRODUCT_HARDWARE_VERSION) -C $(BOOT_MK_DIR) clean
	rm -f $(ARTEFACTS_DIR)/img-$(VERSION_IDENTIFIER).hex
	rm -f $(ARTEFACTS_DIR)/dfu-$(VERSION_IDENTIFIER).zip
