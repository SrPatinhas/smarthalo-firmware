#######################################
#Used only for usage % info
#Update linker script accordigly
FLASHSIZE = 0xc000
RAMSIZE = 0x10000
HEAPSIZE = 0x0
MINSTACKSIZE = 0x80
#######################################

NAMEVERSION := $(shell ./scripts/version.sh SHBL .)
HEXVERSION := $(shell ./scripts/version.sh SHBL HEX)

PROJECT_NAME     := shbl
OUTPUT_DIRECTORY := _build

#PLATFORM ?= pca10040
#PLATFORM ?= sh_ee
PLATFORM ?= shv1x

PRINTF=
#PRINTF=UART
#PRINTF=RTT

SDK_ROOT := ../../3rd/nrf5_sdk/nRF5_SDK_12.1.0_0d23e2a
PROJ_DIR := src
COMMON_DIR := ../shcommon

PAYLOAD_DIR := ../shapp
PAYLOAD_HEX := _build/shapp.hex
PAYLOAD_NAMEVERSION := $(shell ./scripts/version.sh SHAPP .)
PAYLOAD_VERSION := $(shell ./scripts/version.sh SHAPP HEX)

PRODFILE := $(OUTPUT_DIRECTORY)/prod_s132_nrf52_3.0.0_$(PROJECT_NAME)_$(NAMEVERSION)_shapp_$(PAYLOAD_NAMEVERSION).hex

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).out: \
  LINKER_SCRIPT  := shbootloader_gcc_nrf52.ld
# Source files common to all targets
SRC_FILES += $(PROJ_DIR)/dfu-cc.pb.c
SRC_FILES += $(PROJ_DIR)/dfu_public_key.c
SRC_FILES += $(PROJ_DIR)/dfu_req_handling.c
SRC_FILES += $(PROJ_DIR)/main.c
SRC_FILES += $(PROJ_DIR)/blleds.c
SRC_FILES += $(PROJ_DIR)/bootloader/dfu/nrf_dfu.c
SRC_FILES += $(COMMON_DIR)/nrf_dfu_flash.c
SRC_FILES += $(COMMON_DIR)/nrf_dfu_settings.c
SRC_FILES += $(PROJ_DIR)/bootloader/dfu/nrf_dfu_mbr.c
SRC_FILES += $(PROJ_DIR)/bootloader/dfu/nrf_dfu_transport.c
SRC_FILES += $(PROJ_DIR)/bootloader/dfu/nrf_dfu_utils.c
SRC_FILES += $(PROJ_DIR)/bootloader/ble_dfu/nrf_ble_dfu.c
#SRC_FILES += $(PROJ_DIR)/bootloader/nrf_bootloader.c
SRC_FILES += $(PROJ_DIR)/bootloader/nrf_bootloader_app_start.c
SRC_FILES += $(PROJ_DIR)/bootloader/nrf_bootloader_info.c
SRC_FILES += $(COMMON_DIR)/scheduler.c
SRC_FILES += $(COMMON_DIR)/bootinfo.c
SRC_FILES += $(COMMON_DIR)/keys.c

SRC_FILES += $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c
SRC_FILES += $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c
SRC_FILES += $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c
SRC_FILES += $(SDK_ROOT)/components/libraries/uart/retarget.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/uart/nrf_drv_uart.c
SRC_FILES += $(SDK_ROOT)/components/libraries/pwm/app_pwm.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_error_weak.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fifo/app_fifo.c
SRC_FILES += $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c
SRC_FILES += $(SDK_ROOT)/components/libraries/timer/app_timer.c
SRC_FILES += $(SDK_ROOT)/components/libraries/timer/app_timer_appsh.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/app_util_platform.c
SRC_FILES += $(SDK_ROOT)/components/libraries/crc32/crc32.c
SRC_FILES += $(SDK_ROOT)/components/libraries/ecc/ecc.c
SRC_FILES += $(SDK_ROOT)/components/libraries/fstorage/fstorage.c
SRC_FILES += $(SDK_ROOT)/components/libraries/hci/hci_mem_pool.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/nrf_assert.c
SRC_FILES += $(SDK_ROOT)/components/libraries/crypto/nrf_crypto.c
SRC_FILES += $(SDK_ROOT)/components/libraries/sha256/sha256.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/rng/nrf_drv_rng.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/hal/nrf_nvmc.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/ppi/nrf_drv_ppi.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/pwm/nrf_drv_pwm.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/wdt/nrf_drv_wdt.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_advdata.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_conn_params.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_srv_common.c
SRC_FILES += $(SDK_ROOT)/external/nano-pb/pb_common.c
SRC_FILES += $(SDK_ROOT)/external/nano-pb/pb_decode.c
SRC_FILES += $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S
SRC_FILES += $(SDK_ROOT)/components/toolchain/system_nrf52.c
SRC_FILES += $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c
SRC_FILES += $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler_appsh.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c

# Include folders common to all targets
INC_FOLDERS += $(COMMON_DIR)
INC_FOLDERS += $(COMMON_DIR)/platform/$(PLATFORM)
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/cmsis/include
INC_FOLDERS += $(SDK_ROOT)/examples/bsp
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/rng
INC_FOLDERS += $(SDK_ROOT)/components/toolchain
INC_FOLDERS += $(SDK_ROOT)/components/device
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/hal
INC_FOLDERS += $(SDK_ROOT)/components/libraries/sha256
INC_FOLDERS += $(SDK_ROOT)/components/libraries/crc32
INC_FOLDERS += $(SDK_ROOT)/components/libraries/experimental_section_vars
INC_FOLDERS += $(PROJ_DIR)
INC_FOLDERS += $(PROJ_DIR)/bootloader
INC_FOLDERS += $(PROJ_DIR)/bootloader/dfu
INC_FOLDERS += $(PROJ_DIR)/bootloader/ble_dfu
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fds
INC_FOLDERS += $(SDK_ROOT)/components/libraries/mem_manager
INC_FOLDERS += $(SDK_ROOT)/components/libraries/pwm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/uart
INC_FOLDERS += $(SDK_ROOT)/components/libraries/twi
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fstorage
INC_FOLDERS += $(SDK_ROOT)/components/libraries/util
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/uart
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/twi_master
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/gpiote
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/timer
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/ppi
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pwm
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/wdt
INC_FOLDERS += $(SDK_ROOT)/components
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/common/softdevice_handler
INC_FOLDERS += $(SDK_ROOT)/components/libraries/timer
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/clock
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s132/headers/nrf52
INC_FOLDERS += $(SDK_ROOT)/components/libraries/log/src
#INC_FOLDERS += $(SDK_ROOT)/components/libraries/bootloader/dfu
#INC_FOLDERS += $(SDK_ROOT)/components/libraries/bootloader/ble_dfu
#INC_FOLDERS += $(SDK_ROOT)/components/libraries/bootloader
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/delay
INC_FOLDERS += $(SDK_ROOT)/components/ble/common
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fifo
INC_FOLDERS += $(SDK_ROOT)/components/libraries/svc
INC_FOLDERS += $(SDK_ROOT)/components/libraries/log
INC_FOLDERS += $(SDK_ROOT)/components/libraries/hci
INC_FOLDERS += $(SDK_ROOT)/components/libraries/scheduler
INC_FOLDERS += $(SDK_ROOT)/components/libraries/crypto
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s132/headers
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/common
INC_FOLDERS += $(SDK_ROOT)/external/micro-ecc/micro-ecc
INC_FOLDERS += $(SDK_ROOT)/external/nano-pb
INC_FOLDERS += $(SDK_ROOT)/external/segger_rtt
INC_FOLDERS += $(SDK_ROOT)/components/libraries/ecc
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/gcc

#  $(SDK_ROOT)/components/libraries/bootloader/ble_dfu \

# Libraries common to all targets
LIB_FILES += \
  $(SDK_ROOT)/external/micro-ecc/nrf52_armgcc/armgcc/micro_ecc_lib_nrf52.a \

# C flags common to all targets
CFLAGS += -DNRF52
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=1
CFLAGS += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
#CFLAGS += -DBOARD_PCA10040
CFLAGS += -DNRF52832
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
CFLAGS += -D__HEAP_SIZE=0
CFLAGS += -DS132
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DNRF_SD_BLE_API_VERSION=3
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF52_PAN_62
CFLAGS += -DNRF52_PAN_63
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Os -g3
##CFLAGS += -fomit-frame-pointer
#CFLAGS += -Werror
CFLAGS += -Wformat=0
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums -flto
# generate dependency output file
CFLAGS += -MP -MD
CFLAGS += -DBOOTLOADER

CFLAGS += -DPRINTF_$(PRINTF)
ifeq ($(PRINTF),UART)
CFLAGS += -DRETARGET_ENABLED=1
endif
ifeq ($(PRINTF),RTT)
CFLAGS += -DNRF_LOG_USES_RTT=1
endif


# C++ flags common to all targets
CXXFLAGS += \

# Assembler flags common to all targets
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF_DFU_SETTINGS_VERSION=1
ASMFLAGS += -DSVC_INTERFACE_CALL_AS_NORMAL_FUNCTION
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
ASMFLAGS += -D__HEAP_SIZE=0
ASMFLAGS += -DS132
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DNRF_SD_BLE_API_VERSION=3
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF52_PAN_62
ASMFLAGS += -DNRF52_PAN_63

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys


.PHONY: $(TARGETS) default all clean help flash publish package
# Default target - first one defined
default: $(PROJECT_NAME)
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
	mergehex -m $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex $(PAYLOAD_DIR)/$(PAYLOAD_HEX) -o $(PRODFILE)
	mergehex -m $(PRODFILE) $(OUTPUT_DIRECTORY)/settings.hex -o $(PRODFILE)
#	mergehex -m $(PRODFILE) $(OUTPUT_DIRECTORY)/nodebug.hex -o $(PRODFILE)

flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex
	nrfjprog --program $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex -f nrf52 --sectorerase
	nrfjprog --reset -f nrf52

flash_production: default package
	@echo Flashing: $(PRODFILE)
	nrfjprog --program $(PRODFILE) -f nrf52 --chiperase
	nrfjprog --reset -f nrf52

flash_softdevice:
	@echo Flashing: s132_nrf52_3.0.0_softdevice.hex
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex -f nrf52 --chiperase 
	nrfjprog --reset -f nrf52

exitFactoryTest: 
	nrfjprog -f nrf52 --memwr 0x10001090 --val 0
	nrfjprog --reset -f nrf52

reset:
	nrfjprog --reset -f nrf52

protect: 
	nrfjprog -f nrf52 --memwr 0x10001208 --val 0

recover: 
	nrfjprog -f nrf52 --recover

publish: default package
	./scripts/publish.sh $(PROJECT_NAME) $(NAMEVERSION) $(HEXVERSION)
	gdrive upload -p 0B34CPvDx2JkhaEVENW5MYzg2QVE $(PRODFILE)