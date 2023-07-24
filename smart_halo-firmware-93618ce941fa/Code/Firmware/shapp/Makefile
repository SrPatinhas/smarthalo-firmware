#######################################
#Used only for usage % info
#Update linker script accordigly
FLASHSIZE = 0x80000
RAMSIZE = 0x10000
HEAPSIZE = 0x0
MINSTACKSIZE = 0x80
#######################################

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

SDK_ROOT := ../../3rd/nrf5_sdk/nRF5_SDK_12.1.0_0d23e2a
PROJ_DIR := .
COMMON_DIR := ../shcommon

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out: \
  LINKER_SCRIPT  := shapp_gcc_nrf52.ld

SRC_FILES += $(PROJ_DIR)/src/main.c
SRC_FILES += $(PROJ_DIR)/src/battery.c
SRC_FILES += $(PROJ_DIR)/src/bleapp.c
SRC_FILES += $(PROJ_DIR)/src/bslink.c
SRC_FILES += $(PROJ_DIR)/src/dispatch.c
SRC_FILES += $(PROJ_DIR)/src/alarm.c
SRC_FILES += $(PROJ_DIR)/src/auth.c
SRC_FILES += $(PROJ_DIR)/src/ui.c
SRC_FILES += $(PROJ_DIR)/src/device.c
SRC_FILES += $(PROJ_DIR)/src/leds.c
SRC_FILES += $(PROJ_DIR)/src/meters.c
SRC_FILES += $(PROJ_DIR)/src/aes.c
SRC_FILES += $(PROJ_DIR)/src/twi.c
SRC_FILES += $(PROJ_DIR)/src/matrix.c
SRC_FILES += $(PROJ_DIR)/src/calibration.c
SRC_FILES += $(PROJ_DIR)/src/sound.c
SRC_FILES += $(PROJ_DIR)/src/power.c
SRC_FILES += $(PROJ_DIR)/src/touch.c
SRC_FILES += $(PROJ_DIR)/src/test.c
SRC_FILES += $(PROJ_DIR)/src/exp.c
SRC_FILES += $(PROJ_DIR)/src/animscript.c
SRC_FILES += $(COMMON_DIR)/scheduler.c
SRC_FILES += $(COMMON_DIR)/record.c
SRC_FILES += $(COMMON_DIR)/bootinfo.c
SRC_FILES += $(COMMON_DIR)/keys.c
#SRC_FILES += $(COMMON_DIR)/nrf_dfu_settings.c
#SRC_FILES += $(COMMON_DIR)/nrf_dfu_flash.c
SRC_FILES += $(SDK_ROOT)/components/libraries/sha256/sha256.c
SRC_FILES += $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c
SRC_FILES += $(SDK_ROOT)/components/libraries/timer/app_timer_appsh.c
SRC_FILES += $(SDK_ROOT)/components/libraries/timer/app_timer.c
SRC_FILES += $(SDK_ROOT)/components/libraries/button/app_button.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_error.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_error_weak.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_util_platform.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fstorage/fstorage.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/nrf_assert.c
SRC_FILES += $(SDK_ROOT)/components/libraries/sensorsim/sensorsim.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fifo/app_fifo.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fds/fds.c
SRC_FILES += $(SDK_ROOT)/components/libraries/twi/app_twi.c
SRC_FILES += $(SDK_ROOT)/components/libraries/mem_manager/mem_manager.c
SRC_FILES += $(SDK_ROOT)/components/libraries/pwm/app_pwm.c
SRC_FILES += $(SDK_ROOT)/components/libraries/crc16/crc16.c
SRC_FILES += $(SDK_ROOT)/components/libraries/crc32/crc32.c
SRC_FILES += $(SDK_ROOT)/components/libraries/hardfault/hardfault_implementation.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/sdk_mapped_flags.c
SRC_FILES += $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c
SRC_FILES += $(SDK_ROOT)/components/libraries/ecc/ecc.c
SRC_FILES += $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c
SRC_FILES += $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/hal/nrf_nvmc.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/rng/nrf_drv_rng.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/twi_master/nrf_drv_twi.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/uart/nrf_drv_uart.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/ppi/nrf_drv_ppi.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/wdt/nrf_drv_wdt.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/clock/nrf_drv_clock.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/pwm/nrf_drv_pwm.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_advdata.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_conn_params.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_srv_common.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_conn_state.c
SRC_FILES += $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c
SRC_FILES += $(SDK_ROOT)/components/ble/ble_services/ble_nus/ble_nus.c #factory tests
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/gatt_cache_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/gatts_cache_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/id_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_data.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_data_storage.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_database.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_id.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/peer_manager.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/pm_buffer.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/pm_mutex.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/security_dispatcher.c
SRC_FILES += $(SDK_ROOT)/components/ble/peer_manager/security_manager.c
SRC_FILES += $(SDK_ROOT)/components/toolchain/system_nrf52.c
SRC_FILES += $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c
SRC_FILES += $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c
SRC_FILES += $(SDK_ROOT)/components/libraries/uart/retarget.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c

SRC_FILES += $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S

INC_FOLDERS += $(PROJ_DIR)/src
INC_FOLDERS += $(COMMON_DIR)
INC_FOLDERS += $(COMMON_DIR)/platform/$(PLATFORM)
INC_FOLDERS += $(SDK_ROOT)/examples/bsp
INC_FOLDERS += $(SDK_ROOT)/components/libraries/sha256
INC_FOLDERS += $(SDK_ROOT)/components/libraries/ecc
INC_FOLDERS += $(SDK_ROOT)/components/libraries/pwm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/log
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fstorage
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fifo
INC_FOLDERS += $(SDK_ROOT)/components/libraries/crc16
INC_FOLDERS += $(SDK_ROOT)/components/libraries/mailbox
INC_FOLDERS += $(SDK_ROOT)/components/libraries/experimental_section_vars
INC_FOLDERS += $(SDK_ROOT)/components/libraries/slip
INC_FOLDERS += $(SDK_ROOT)/components/libraries/mem_manager
INC_FOLDERS += $(SDK_ROOT)/components/libraries/scheduler
INC_FOLDERS += $(SDK_ROOT)/components/libraries/util
INC_FOLDERS += $(SDK_ROOT)/components/libraries/csense_drv
INC_FOLDERS += $(SDK_ROOT)/components/libraries/csense
INC_FOLDERS += $(SDK_ROOT)/components/libraries/low_power_pwm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/hardfault
INC_FOLDERS += $(SDK_ROOT)/components/libraries/uart
INC_FOLDERS += $(SDK_ROOT)/components/libraries/hci
INC_FOLDERS += $(SDK_ROOT)/components/libraries/timer
INC_FOLDERS += $(SDK_ROOT)/components/libraries/led_softblink
INC_FOLDERS += $(SDK_ROOT)/components/libraries/crc32
INC_FOLDERS += $(SDK_ROOT)/components/libraries/sensorsim
INC_FOLDERS += $(SDK_ROOT)/components/libraries/button
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fds
INC_FOLDERS += $(SDK_ROOT)/components/libraries/twi
INC_FOLDERS += $(SDK_ROOT)/components/libraries/log/src
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/rng
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/comp
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/twi_master
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/i2s
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/gpiote
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/common
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/adc
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/uart
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/wdt
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/hal
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/rtc
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/ppi
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/twis_slave
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/delay
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/timer
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pwm
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/spi_slave
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/lpcomp
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/qdec
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/spi_master
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/saadc
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pdm
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/swi
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/clock
INC_FOLDERS += $(SDK_ROOT)/components
INC_FOLDERS += $(SDK_ROOT)/components/ble/common
INC_FOLDERS += $(SDK_ROOT)/components/ble/peer_manager
INC_FOLDERS += $(SDK_ROOT)/components/ble/nrf_ble_qwr
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_racp
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_dtm
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_advertising
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_services/ble_nus #factory tests
INC_FOLDERS += $(SDK_ROOT)/components/toolchain
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/gcc
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/cmsis/include
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s132/headers/nrf52
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s132/headers
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/common/softdevice_handler
INC_FOLDERS += $(SDK_ROOT)/external/micro-ecc/micro-ecc
INC_FOLDERS += $(SDK_ROOT)/external/segger_rtt
INC_FOLDERS += $(SDK_ROOT)/components/device

# Libraries common to all targets
LIB_FILES += $(SDK_ROOT)/external/micro-ecc/nrf52_armgcc/armgcc/micro_ecc_lib_nrf52.a


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
CFLAGS += -DNRF_SD_BLE_API_VERSION=3
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=1
CFLAGS += -DFWVERSION=$(DEFVERSION)

CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Os -g3
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
ASMFLAGS += -x assembler-with-cpp
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
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=3
ASMFLAGS += -DNRF_DFU_SETTINGS_VERSION=1

##ASMFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1
#ASMFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
##ASMFLAGS += -DuECC_SUPPORTS_secp256r1=1
#ASMFLAGS += -DuECC_SQUARE_FUNC=1
#ASMFLAGS += -DuECC_ENABLE_VLI_API
##ASMFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
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
#	@size --format=SysV -x $@
	@size -x $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out
	@echo " "
	@SZTEXT=`size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$1 }'`; SZDATA=`size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$2 }'`; SZBSS=`size $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out | awk 'NR <= 1 { next } { print $$3 }'`; TOTFLASH=`printf 0x%x $$(( $$SZTEXT + $$SZDATA ))`; PERC=$$(( $$TOTFLASH * 100 / $(FLASHSIZE) )); echo "  Flash: $$TOTFLASH / $(FLASHSIZE)    $$PERC %"; TOTRAM=`printf 0x%x $$(( $$SZBSS + $(HEAPSIZE) + $(MINSTACKSIZE) ))`; PERC=$$(( $$TOTRAM * 100 / $(RAMSIZE) )); echo "    RAM: $$TOTRAM / $(RAMSIZE)    $$PERC %"
	@echo "\n===================================================================="

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
	nrfjprog --program $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52

exitFactoryTest: 
	nrfjprog -f nrf52 --memwr 0x10001090 --val 0
	nrfjprog --reset -f nrf52

protect: 
	nrfjprog -f nrf52 --memwr 0x10001208 --val 0

recover: 
	nrfjprog -f nrf52 --recover

# Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_3.0.0_softdevice.hex
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex -f nrf52 --chiperase
	nrfjprog --reset -f nrf52

reset:
	nrfjprog --reset -f nrf52

publish: default
	./scripts/publish.sh $(PROJECT_NAME) $(NAMEVERSION) $(HEXVERSION)

