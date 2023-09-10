#######################################
#Used only for usage % info
#Update linker script accordigly
FLASHSIZE = 0xc000
RAMSIZE = 0x10000
HEAPSIZE = 0x0
MINSTACKSIZE = 0x80
#######################################

# SERIAL_OPT = $(shell ../shapp/scripts/nrf-select.sh)

# NAMEVERSION := $(shell ./scripts/version.sh SHBL .)
# HEXVERSION := $(shell ./scripts/version.sh SHBL HEX)

PROJECT_NAME     := shbl
OUTPUT_DIRECTORY := _build

PLATFORM ?= shv1x

SDK_ROOT := ../../3rd/nrf5_sdk/nRF5_SDK_16.0.0_98a08e2
PROJ_DIR := src
COMMON_DIR := ../shcommon

PAYLOAD_DIR := ../shapp
PAYLOAD_HEX := _build/shapp.hex
PAYLOAD_NAMEVERSION := $(shell ./scripts/version.sh SHAPP .)
PAYLOAD_VERSION := $(shell ./scripts/version.sh SHAPP HEX)

PRODFILE := $(OUTPUT_DIRECTORY)/prod_s132_nrf52_7.0.1_$(PROJECT_NAME)_$(NAMEVERSION)_shapp_$(PAYLOAD_NAMEVERSION).hex

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out: \
  LINKER_SCRIPT  := secure_bootloader_gcc_nrf52.ld

#   LINKER_SCRIPT  := shbootloader_gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/crc32/crc32.c \
  $(SDK_ROOT)/components/libraries/mem_manager/mem_manager.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_nvmc.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/sha256/sha256.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/micro_ecc/micro_ecc_backend_ecc.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/micro_ecc/micro_ecc_backend_ecdh.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/micro_ecc/micro_ecc_backend_ecdsa.c \
  $(SDK_ROOT)/components/boards/boards.c \
  $(SDK_ROOT)/modules/nrfx/hal/nrf_nvmc.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_ecc.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_ecdsa.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_hash.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_init.c \
  $(SDK_ROOT)/components/libraries/crypto/nrf_crypto_shared.c \
  $(PROJ_DIR)/dfu_public_key.c \
  $(PROJ_DIR)/main.c \
  $(PROJ_DIR)/dfu-cc.pb.c \
  $(PROJ_DIR)/dfu_public_key.c \
  $(PROJ_DIR)/main.c \
  $(PROJ_DIR)/bootloader/dfu/nrf_dfu.c \
  $(COMMON_DIR)/nrf_dfu_flash.c \
  $(COMMON_DIR)/bootinfo.c \
  $(PROJ_DIR)/bootloader/dfu/nrf_dfu_mbr.c \
  $(PROJ_DIR)/bootloader/dfu/nrf_dfu_transport.c \
  $(PROJ_DIR)/bootloader/dfu/nrf_dfu_utils.c \
  $(COMMON_DIR)/scheduler.c \
  $(COMMON_DIR)/keys.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader_app_start.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader_app_start_final.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader_dfu_timers.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader_fw_activation.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader_info.c \
  $(SDK_ROOT)/components/libraries/bootloader/nrf_bootloader_wdt.c \
  $(SDK_ROOT)/external/nano-pb/pb_common.c \
  $(SDK_ROOT)/external/nano-pb/pb_decode.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_sw/nrf_sw_backend_hash.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/dfu-cc.pb.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu.c \
  $(SDK_ROOT)/components/libraries/bootloader/ble_dfu/nrf_dfu_ble.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_flash.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_handling_error.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_mbr.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_req_handler.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_settings.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_settings_svci.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_transport.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_utils.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_validation.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_ver_validation.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_svci.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_svci_handler.c \
  $(SDK_ROOT)/components/libraries/svc/nrf_svc_handler.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/libraries/ecc/ecc.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_rng.c \
  $(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rng.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_chacha_poly_aead.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_ecc.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_ecdh.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_ecdsa.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_eddsa.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_hash.c \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon/oberon_backend_hmac.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(COMMON_DIR) \
  $(COMMON_DIR)/micro-ecc/micro-ecc \
  $(COMMON_DIR)/platform/$(PLATFORM) \
  $(SDK_ROOT)/components/libraries/crypto/backend/micro_ecc \
  $(SDK_ROOT)/components/softdevice/s132/headers \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/sha256 \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/mem_manager \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/external/nrf_oberon/include \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon \
  $(SDK_ROOT)/components/libraries/crypto/backend/cifra \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310_bl \
  $(SDK_ROOT)/components/softdevice/s132/headers/nrf52 \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/libraries/bootloader/dfu \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/svc \
  $(SDK_ROOT)/components/libraries/stack_info \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_hw \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/external/nrf_oberon \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/crypto/backend/mbedtls \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310 \
  $(SDK_ROOT)/components/libraries/bootloader \
  $(SDK_ROOT)/external/fprintf \
  $(SDK_ROOT)/components/libraries/crypto \
  $(SDK_ROOT)/components/libraries/crypto/backend/optiga \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(PROJ_DIR) \
  $(PROJ_DIR)/bootloader/dfu \
  $(PROJ_DIR)/bootloader/ble_dfu \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_sw \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/components/libraries/bootloader/ble_dfu \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/external/nano-pb \
  $(SDK_ROOT)/components/libraries/queue \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/uart \
  $(SDK_ROOT)/components/libraries/ecc \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/modules/nrfx/drivers/src \

# Libraries common to all targets
LIB_FILES += \
  $(SDK_ROOT)/external/nrf_oberon/lib/cortex-m4/hard-float/liboberon_3.0.1.a \
  $(SDK_ROOT)/external/micro-ecc/nrf52hf_armgcc/armgcc/micro_ecc_lib_nrf52.a \

# Optimization flags
OPT = -Os -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DBOARD_PCA10040
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=2
CFLAGS += -DNRF_DFU_SVCI_ENABLED
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -DS132
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
CFLAGS += -DuECC_ENABLE_VLI_API=0
CFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
CFLAGS += -DuECC_SQUARE_FUNC=0
CFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0
CFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
# CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# C++ flags common to all targets
CXXFLAGS += $(OPT)
# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74
ASMFLAGS += -DNRF_DFU_SETTINGS_VERSION=2
ASMFLAGS += -DNRF_DFU_SVCI_ENABLED
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=7
ASMFLAGS += -DS132
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
ASMFLAGS += -DuECC_ENABLE_VLI_API=0
ASMFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
ASMFLAGS += -DuECC_SQUARE_FUNC=0
ASMFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0
ASMFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52832_xxaa_s132: CFLAGS += -D__HEAP_SIZE=0
nrf52832_xxaa_s132: ASMFLAGS += -D__HEAP_SIZE=0

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: $(TARGETS) default all clean help flash publish package
# Default target - first one defined
default: $(PROJECT_NAME)
# 	@echo "\n===================================================================="
# 	@echo " "
# 	@echo "$(PROJECT_NAME) version: $(NAMEVERSION)"
# 	@echo " "
# 	@echo "Memory Usage Report:"
# #	@arm-none-eabi-size --format=SysV -x $@
# 	@arm-none-eabi-size -x $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out
# 	@echo " "
# 	@SZTEXT=`arm-none-eabi-size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$1 }'`; SZDATA=`arm-none-eabi-size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$2 }'`; SZBSS=`arm-none-eabi-size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$3 }'`; TOTFLASH=`printf 0x%x $$(( $$SZTEXT + $$SZDATA ))`; PERC=$$(( $$TOTFLASH * 100 / $(FLASHSIZE) )); echo "  Flash: $$TOTFLASH / $(FLASHSIZE)    $$PERC %"; TOTRAM=`printf 0x%x $$(( $$SZBSS + $(HEAPSIZE) + $(MINSTACKSIZE) ))`; PERC=$$(( $$TOTRAM * 100 / $(RAMSIZE) )); echo "    RAM: $$TOTRAM / $(RAMSIZE)    $$PERC %"
# 	@echo "\n===================================================================="
# 	@../shapp/scripts/nrf-select.sh > /dev/null  # test for jlinks

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	$(PROJECT_NAME)

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common
#$(foreach target, $(TARGETS), $(call define_target, $(target)))
#-include $(foreach target, $(TARGETS), $($(target)_dependencies))
$(call define_target, $(PROJECT_NAME))
-include $($(PROJECT_NAME)_dependencies)

$(OUTPUT_DIRECTORY)/nodebug.hex:
	@echo ":020000041000EA" > $(OUTPUT_DIRECTORY)/nodebug.hex
	@echo ":0412080000FFFFFFE5" >> $(OUTPUT_DIRECTORY)/nodebug.hex
	@echo ":00000001FF" >> $(OUTPUT_DIRECTORY)/nodebug.hex

package: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex $(OUTPUT_DIRECTORY)/nodebug.hex
	@echo " BL: $(PROJECT_NAME) version: $(NAMEVERSION)"
	@echo "APP: $(PAYLOAD_HEX) version: $(PAYLOAD_NAMEVERSION)"
	nrfutil settings generate --family NRF52 --bootloader-version $(HEXVERSION) --bl-settings-version 1 --application $(PAYLOAD_DIR)/$(PAYLOAD_HEX) --application-version $(PAYLOAD_VERSION) $(OUTPUT_DIRECTORY)/settings.hex
	mergehex -m $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_7.0.1_softdevice.hex $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex $(PAYLOAD_DIR)/$(PAYLOAD_HEX) -o $(PRODFILE)
	mergehex -m $(PRODFILE) $(OUTPUT_DIRECTORY)/settings.hex -o $(PRODFILE)
#	mergehex -m $(PRODFILE) $(OUTPUT_DIRECTORY)/nodebug.hex -o $(PRODFILE)

flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex
	nrfjprog $(SERIAL_OPT) --program $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex -f nrf52 --sectorerase
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

flash_production: default package
	@echo Flashing: $(PRODFILE)
	nrfjprog $(SERIAL_OPT) --program $(PRODFILE) -f nrf52 --chiperase
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

flash_softdevice:
	@echo Flashing: s132_nrf52_7.0.1_softdevice.hex
	nrfjprog $(SERIAL_OPT) --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_7.0.1_softdevice.hex -f nrf52 --chiperase
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

exitFactoryTest: 
	nrfjprog $(SERIAL_OPT) -f nrf52 --memwr 0x10001090 --val 0
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

reset:
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

protect: 
	nrfjprog $(SERIAL_OPT) -f nrf52 --memwr 0x10001208 --val 0

recover: 
	nrfjprog $(SERIAL_OPT) -f nrf52 --recover

publish: default package
	./scripts/publish.sh $(PROJECT_NAME) $(NAMEVERSION) $(HEXVERSION)
