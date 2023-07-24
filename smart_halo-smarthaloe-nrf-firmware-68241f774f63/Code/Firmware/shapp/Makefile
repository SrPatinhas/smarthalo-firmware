#######################################
#Used only for usage % info
#Update linker script accordigly
FLASHSIZE = 0x80000
RAMSIZE = 0x10000
HEAPSIZE = 0x0
MINSTACKSIZE = 0x80
#######################################

SERIAL_OPT = $(shell scripts/nrf-select.sh)

NAMEVERSION := $(shell ./scripts/version.sh SHAPP .)
DEFVERSION := $(shell ./scripts/version.sh SHAPP ,)
HEXVERSION := $(shell ./scripts/version.sh SHAPP HEX)

#NAMEVERSION := 0.0.0.0
#DEFVERSION := 0,0,0,0
#HEXVERSION := 0x00000000

PROJECT_NAME := shapp
OUTPUT_DIRECTORY := _build

VERSIONFILE=$(OUTPUT_DIRECTORY)/version.txt

#PLATFORM ?= pca10040
#PLATFORM ?= shmp
#PLATFORM ?= sh_ee
PLATFORM ?= shv1x

#DEBUG=ON
#DEBUG_GETPASSWORD=ON

#PRINTF=
#PRINTF=UART
PRINTF=RTT

SDK_ROOT := ../../3rd/nrf5_sdk/nRF5_SDK_16.0.0_98a08e2
PROJ_DIR := .
COMMON_DIR := ../shcommon

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out: \
  LINKER_SCRIPT  := shapp_gcc_nrf52.ld

SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52.S \
  $(PROJ_DIR)/src/main.c \
  $(PROJ_DIR)/src/bleapp.c \
  $(PROJ_DIR)/src/bslink.c \
  $(PROJ_DIR)/src/dispatch.c \
  $(PROJ_DIR)/src/auth.c \
  $(PROJ_DIR)/src/device.c \
  $(PROJ_DIR)/src/aes.c \
  $(PROJ_DIR)/src/test.c \
  $(PROJ_DIR)/src/uart.c \
  $(PROJ_DIR)/src/shimano.c \
  $(COMMON_DIR)/scheduler.c \
  $(COMMON_DIR)/record.c \
  $(COMMON_DIR)/bootinfo.c \
  $(COMMON_DIR)/keys.c \
  $(SDK_ROOT)/components/ble/nrf_ble_scan/nrf_ble_scan.c \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas_c/ble_bas_c.c \
  $(SDK_ROOT)/components/libraries/sha256/sha256.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/components/libraries/button/app_button.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/sensorsim/sensorsim.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/fifo/app_fifo.c \
  $(SDK_ROOT)/components/libraries/fds/fds.c \
  $(SDK_ROOT)/components/libraries/atomic_flags/nrf_atflags.c \
  $(SDK_ROOT)/components/libraries/mem_manager/mem_manager.c \
  $(SDK_ROOT)/components/libraries/pwm/app_pwm.c \
  $(SDK_ROOT)/components/libraries/crc16/crc16.c \
  $(SDK_ROOT)/components/libraries/crc32/crc32.c \
  $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c \
  $(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c \
  $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rng.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
  $(SDK_ROOT)/components/libraries/ecc/ecc.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/modules/nrfx/hal/nrf_nvmc.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_rng.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_twi.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/components/libraries/gpiote/app_gpiote.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_ppi.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_wdt.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_pwm.c \
  $(SDK_ROOT)/components/ble/common/ble_advdata.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_params.c \
  $(SDK_ROOT)/components/ble/common/ble_srv_common.c \
  $(SDK_ROOT)/components/ble/common/ble_conn_state.c \
  $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/id_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_database.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_id.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_manager.c \
  $(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c \
  $(SDK_ROOT)/components/ble/peer_manager/security_manager.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
  $(SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
  $(SDK_ROOT)/components/ble/ble_db_discovery/ble_db_discovery.c \
  $(SDK_ROOT)/components/ble/nrf_ble_gq/nrf_ble_gq.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr/nrf_ble_qwr.c \
  $(SDK_ROOT)/components/ble/ble_services/experimental_gatts_c/nrf_ble_gatts_c.c \
  $(SDK_ROOT)/components/ble/peer_manager/peer_manager_handler.c \

INC_FOLDERS +=  \
  $(PROJ_DIR)/src \
  $(COMMON_DIR) \
  $(COMMON_DIR)/micro-ecc/micro-ecc \
  $(COMMON_DIR)/platform/$(PLATFORM) \
  $(SDK_ROOT)/components/ble/nrf_ble_scan \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas_c \
  $(SDK_ROOT)/components/ble/ble_services/ble_bas  \
  $(SDK_ROOT)/components/libraries/sha256 \
  $(SDK_ROOT)/components/libraries/ecc \
  $(SDK_ROOT)/components/libraries/pwm \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/libraries/fifo \
  $(SDK_ROOT)/components/libraries/crc16 \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/components/libraries/slip \
  $(SDK_ROOT)/components/libraries/mem_manager \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/libraries/csense_drv \
  $(SDK_ROOT)/components/libraries/csense \
  $(SDK_ROOT)/components/libraries/low_power_pwm \
  $(SDK_ROOT)/components/libraries/hardfault \
  $(SDK_ROOT)/components/libraries/uart \
  $(SDK_ROOT)/components/libraries/hci \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/components/libraries/led_softblink \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/libraries/sensorsim \
  $(SDK_ROOT)/components/libraries/button \
  $(SDK_ROOT)/components/libraries/fds \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/components/drivers_nrf/twi_master \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/ble/common \
  $(SDK_ROOT)/components/ble/peer_manager \
  $(SDK_ROOT)/components/ble/nrf_ble_qwr \
  $(SDK_ROOT)/components/ble/ble_racp \
  $(SDK_ROOT)/components/ble/ble_dtm \
  $(SDK_ROOT)/components/ble/ble_advertising \
  $(SDK_ROOT)/components/toolchain \
  $(SDK_ROOT)/components/toolchain/gcc \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/softdevice/s132/headers/nrf52 \
  $(SDK_ROOT)/components/softdevice/common \
  $(SDK_ROOT)/components/softdevice/s132/headers \
  $(SDK_ROOT)/external/segger_rtt \
  $(SDK_ROOT)/modules/nrfx/hal/ \
  $(SDK_ROOT)/modules/nrfx/drivers/src \
  $(SDK_ROOT)/modules/nrfx/ \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/integration/nrfx/legacy/ \
  $(SDK_ROOT)/integration/nrfx/ \
  $(SDK_ROOT)/components/libraries/ \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  $(SDK_ROOT)/components/libraries/ecc \
  $(SDK_ROOT)/components/libraries/queue \
  $(SDK_ROOT)/components/libraries/atomic_flags \
  $(SDK_ROOT)/components/ble/ble_link_ctx_manager \
  $(SDK_ROOT)/components/ble/nrf_ble_gatt \
  $(SDK_ROOT)/components/libraries/mutex \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/ \
  $(SDK_ROOT)/components/ble/ble_db_discovery/ \
  $(SDK_ROOT)/components/ble/nrf_ble_gq/ \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/components/libraries/bootloader \
  $(SDK_ROOT)/components/ble/ble_services/experimental_gatts_c \
  $(SDK_ROOT)/external/fprintf/ \

# Libraries common to all targets
LIB_FILES += $(COMMON_DIR)/micro-ecc/nrf52_armgcc/armgcc/micro_ecc_lib_nrf52.a


# C flags common to all targets
CFLAGS += -DNRF52
CFLAGS += -DNRF52832
CFLAGS += -DSOFTDEVICE_PRESENT
#CFLAGS += -DBOARD_PCA10040
CFLAGS += -DPLATFORM_$(PLATFORM)
CFLAGS += -DNRF52_PAN_12
CFLAGS += -DNRF52_PAN_15
CFLAGS += -DNRF52_PAN_58
CFLAGS += -DNRF52_PAN_55
CFLAGS += -DNRF52_PAN_54
CFLAGS += -DNRF52_PAN_31
CFLAGS += -DNRF52_PAN_30
CFLAGS += -DNRF52_PAN_51
CFLAGS += -DNRF52_PAN_36
CFLAGS += -DNRF52_PAN_53
CFLAGS += -DS132
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DNRF52_PAN_62
CFLAGS += -DNRF52_PAN_63
CFLAGS += -D__HEAP_SIZE=0
CFLAGS += -DNRF_SD_BLE_API_VERSION=7
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=1
CFLAGS += -DFWVERSION=$(DEFVERSION)
#CFLAGS += -DDEBUG

CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -O0 -g
CFLAGS += -fomit-frame-pointer
#CFLAGS += -Werror
CFLAGS += -Wformat=0
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
# generate dependency output file
CFLAGS += -MP -MD
#CFLAGS += -fstack-usage

ifeq ($(DEBUG),ON)
CFLAGS += -DDEBUG
	ifeq ($(DEBUG_GETPASSWORD),ON)
	CFLAGS += -DDEBUG_GETPASSWORD
	endif
endif

ifeq ($(LEGACYTEST),ON)
CFLAGS += -DLEGACYTEST
endif

CFLAGS += -DPRINTF_$(PRINTF)

ifeq ($(PRINTF),UART)
CFLAGS += -DRETARGET_ENABLED=1
endif
ifeq ($(PRINTF),RTT)
CFLAGS += -DNRF_LOG_USES_RTT=1
endif

#CFLAGS += -DBSLINK_INDICATE
#CFLAGS += -DDBG_MOTION_VISUAL
#CFLAGS += -DOVERRIDE_GAP_PARAM

# C++ flags common to all targets
CXXFLAGS += \

# Assembler flags
ASMFLAGS += -DNRF52
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DNRF52832
ASMFLAGS += -DNRF52_PAN_12
ASMFLAGS += -DNRF52_PAN_15
ASMFLAGS += -DNRF52_PAN_58
ASMFLAGS += -DNRF52_PAN_55
ASMFLAGS += -DNRF52_PAN_54
ASMFLAGS += -DNRF52_PAN_31
ASMFLAGS += -DNRF52_PAN_30
ASMFLAGS += -DNRF52_PAN_51
ASMFLAGS += -DNRF52_PAN_36
ASMFLAGS += -DNRF52_PAN_53
ASMFLAGS += -DS132
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DNRF52_PAN_62
ASMFLAGS += -DNRF52_PAN_63
ASMFLAGS += -D__HEAP_SIZE=0
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=7
ASMFLAGS += -DNRF_DFU_SETTINGS_VERSION=1

##ASMFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1
#ASMFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
##ASMFLAGS += -DuECC_SUPPORTS_secp256r1=1
#ASMFLAGS += -DuECC_SQUARE_FUNC=1
#ASMFLAGS += -DuECC_ENABLE_VLI_API
##ASMFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0

COMMON_LINKER_PATH := $(SDK_ROOT)/modules/nrfx/mdk/

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(COMMON_LINKER_PATH) -T$(LINKER_SCRIPT)  
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -u _printf_float


.PHONY: $(PROJECT_NAME) default all clean help flash flash_softdevice reset publish

# Default target - first one defined
default: $(VERSIONFILE) $(PROJECT_NAME) $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex
	./scripts/devpkg.sh $(PROJECT_NAME) $(NAMEVERSION) $(HEXVERSION)
	@echo "\n===================================================================="
	@echo " "
	@echo "$(PROJECT_NAME) version: $(NAMEVERSION)"
	@echo " "
	@echo "Memory Usage Report:"
#	@arm-none-eabi-size --format=SysV -x $@
	@arm-none-eabi-size -x $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out
	@echo " "
	@SZTEXT=`arm-none-eabi-size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$1 }'`; SZDATA=`arm-none-eabi-size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$2 }'`; SZBSS=`arm-none-eabi-size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$3 }'`; TOTFLASH=`printf 0x%x $$(( $$SZTEXT + $$SZDATA ))`; PERC=$$(( $$TOTFLASH * 100 / $(FLASHSIZE) )); echo "  Flash: $$TOTFLASH / $(FLASHSIZE)    $$PERC %"; TOTRAM=`printf 0x%x $$(( $$SZBSS + $(HEAPSIZE) + $(MINSTACKSIZE) ))`; PERC=$$(( $$TOTRAM * 100 / $(RAMSIZE) )); echo "    RAM: $$TOTRAM / $(RAMSIZE)    $$PERC %"
	@echo "\n===================================================================="
	@./scripts/nrf-select.sh > /dev/null  # test for jlinks

# $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	$(PROJECT_NAME)
	@echo 	flash_softdevice

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

#$(OUTPUT_DIRECTORY)/compiler_flags

include $(TEMPLATE_PATH)/Makefile.common
#$(foreach target, $(PROJECT_NAME), $(call define_target, $(target)))
##-include $(foreach target, $(PROJECT_NAME), $($(target)_dependencies))
$(call define_target, $(PROJECT_NAME))
-include $($(PROJECT_NAME)_dependencies)

.PHONY: force
$(VERSIONFILE): $(OUTPUT_DIRECTORY) force
	@echo '$(NAMEVERSION)' | cmp -s - $@ || ($(RM) $(OUTPUT_DIRECTORY)/* && echo '$(NAMEVERSION)' > $@)

# Flash the program
flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex
	nrfjprog $(SERIAL_OPT) --program $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex -f nrf52 --sectorerase
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

exitFactoryTest: 
	nrfjprog $(SERIAL_OPT) -f nrf52 --memwr 0x10001090 --val 0
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

protect: 
	nrfjprog $(SERIAL_OPT) -f nrf52 --memwr 0x10001208 --val 0

recover: 
	nrfjprog $(SERIAL_OPT) -f nrf52 --recover

# Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_7.0.1_softdevice.hex
	nrfjprog $(SERIAL_OPT) --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_7.0.1_softdevice.hex -f nrf52 --chiperase
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

reset:
	nrfjprog $(SERIAL_OPT) --reset -f nrf52

publish: default
	./scripts/publish.sh $(PROJECT_NAME) $(NAMEVERSION) $(HEXVERSION)
