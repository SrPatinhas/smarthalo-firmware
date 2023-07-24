PROJECT_NAME := test
OUTPUT_DIRECTORY := _build

#PLATFORM ?= pca10040
PLATFORM ?= sh_ee

#SDK_ROOT := ../../3rd/nrf5_sdk
SDK_ROOT := ../../../../../nRF5_SDK_11.0.0_89a8197
PROJ_DIR := ..

export OUTPUT_FILENAME
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = $(SDK_ROOT)/components/toolchain/gcc
ifeq ($(OS),Windows_NT)
include $(TEMPLATE_PATH)/Makefile.windows
else
include $(TEMPLATE_PATH)/Makefile.posix
endif

MK := mkdir
RM := rm -rf

#echo suspend
ifeq ("$(VERBOSE)","1")
NO_ECHO := 
else
NO_ECHO := @
endif

# Toolchain commands
CC              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc'
AS              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as'
AR              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar' -r
LD              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld'
NM              := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm'
OBJDUMP         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump'

OBJCOPY         := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy'
SIZE            := '$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size'

#function for removing duplicates in a list
remduplicates = $(strip $(if $1,$(firstword $1) $(call remduplicates,$(filter-out $(firstword $1),$1))))

SRC_FILES += $(PROJ_DIR)/src/main.c
SRC_FILES += $(PROJ_DIR)/src/scheduler.c
SRC_FILES += $(PROJ_DIR)/src/record.c
SRC_FILES += $(PROJ_DIR)/src/bleapp_11.c
SRC_FILES += $(PROJ_DIR)/src/bslink.c
SRC_FILES += $(PROJ_DIR)/src/dispatch.c
SRC_FILES += $(PROJ_DIR)/src/auth.c
SRC_FILES += $(PROJ_DIR)/src/twi.c
SRC_FILES += $(PROJ_DIR)/src/ui.c
SRC_FILES += $(PROJ_DIR)/src/leds.c
SRC_FILES += $(PROJ_DIR)/src/meters.c
SRC_FILES += $(PROJ_DIR)/src/crypto/uECC.c
SRC_FILES += $(PROJ_DIR)/src/crypto/aes.c
SRC_FILES += $(SDK_ROOT)/examples/bsp/bsp.c
SRC_FILES += $(SDK_ROOT)/examples/bsp/bsp_btn_ble.c
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
SRC_FILES += $(SDK_ROOT)/components/libraries/trace/app_trace.c
SRC_FILES += $(SDK_ROOT)/components/libraries/util/nrf_log.c
SRC_FILES += $(SDK_ROOT)/components/libraries/uart/retarget.c
SRC_FILES += $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/delay/nrf_delay.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/common/nrf_drv_common.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/uart/nrf_drv_uart.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/ppi/nrf_drv_ppi.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/timer/nrf_drv_timer.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/twi_master/nrf_drv_twi.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/pstorage/pstorage.c
SRC_FILES += $(SDK_ROOT)/components/drivers_nrf/rng/nrf_drv_rng.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_advdata.c
SRC_FILES += $(SDK_ROOT)/components/ble/ble_advertising/ble_advertising.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_conn_params.c
SRC_FILES += $(SDK_ROOT)/components/ble/common/ble_srv_common.c
SRC_FILES += $(SDK_ROOT)/components/ble/device_manager/device_manager_peripheral.c
SRC_FILES += $(SDK_ROOT)/components/toolchain/system_nrf52.c
SRC_FILES += $(SDK_ROOT)/components/softdevice/common/softdevice_handler/softdevice_handler.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/RTT_Syscalls_GCC.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c
SRC_FILES += $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c

ASM_SOURCE_FILES  = $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.s


INC_FOLDERS += $(PROJ_DIR)/src
INC_FOLDERS += $(PROJ_DIR)/src/crypto
INC_FOLDERS += ./config
INC_FOLDERS += $(PROJ_DIR)/platform/$(PLATFORM)
INC_FOLDERS += $(SDK_ROOT)/examples/bsp
INC_FOLDERS += $(SDK_ROOT)/components/libraries/scheduler
INC_FOLDERS += $(SDK_ROOT)/components/libraries/timer
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fstorage/config
INC_FOLDERS += $(SDK_ROOT)/components/libraries/sensorsim
INC_FOLDERS += $(SDK_ROOT)/components/libraries/uart
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fifo
INC_FOLDERS += $(SDK_ROOT)/components/libraries/util
INC_FOLDERS += $(SDK_ROOT)/components/libraries/button
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fstorage
INC_FOLDERS += $(SDK_ROOT)/components/libraries/experimental_section_vars
INC_FOLDERS += $(SDK_ROOT)/components/libraries/trace
INC_FOLDERS += $(SDK_ROOT)/components/libraries/twi
INC_FOLDERS += $(SDK_ROOT)/components/libraries/mem_manager
INC_FOLDERS += $(SDK_ROOT)/components/libraries/pwm
INC_FOLDERS += $(SDK_ROOT)/components/libraries/fds
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/config
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/delay
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/uart
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pstorage
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/gpiote
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/hal
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/common
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/rng
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/twi_master
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/pwm
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/timer
INC_FOLDERS += $(SDK_ROOT)/components/drivers_nrf/ppi
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s132/headers/nrf52
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/s132/headers
INC_FOLDERS += $(SDK_ROOT)/components/softdevice/common/softdevice_handler
INC_FOLDERS += $(SDK_ROOT)/components/ble/ble_advertising
INC_FOLDERS += $(SDK_ROOT)/components/ble/device_manager
INC_FOLDERS += $(SDK_ROOT)/components/ble/common
INC_FOLDERS += $(SDK_ROOT)/components/device
INC_FOLDERS += $(SDK_ROOT)/external/segger_rtt
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/CMSIS/Include
INC_FOLDERS += $(SDK_ROOT)/components/toolchain/gcc
INC_FOLDERS += $(SDK_ROOT)/components/toolchain


OBJECT_DIRECTORY = $(OUTPUT_DIRECTORY)
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS += -DNRF52
CFLAGS += -DNRF52832
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DBOARD_PCA10040
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
CFLAGS += -DNRF_LOG_USES_UART=1

CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -O0 -g3
CFLAGS += -fomit-frame-pointer
#CFLAGS += -Werror
CFLAGS += -Wformat=0
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
CFLAGS += --std=gnu99

#CFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1
CFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
#CFLAGS += -DuECC_SUPPORTS_secp256r1=1
#CFLAGS += -DuECC_SUPPORTS_secp256k1=1
CFLAGS += -DuECC_SQUARE_FUNC=1
CFLAGS += -DuECC_ENABLE_VLI_API
#CFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0
CFLAGS += -DNRF_LOG_USES_RTT=1
CFLAGS += -DDEBUG
CFLAGS += -DSMARTHALO_UUID_COUNT=1

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
ASMFLAGS += -DNRF_LOG_USES_UART=1

#ASMFLAGS += -DuECC_VLI_NATIVE_LITTLE_ENDIAN=1
ASMFLAGS += -DuECC_OPTIMIZATION_LEVEL=3
#ASMFLAGS += -DuECC_SUPPORTS_secp256r1=1
ASMFLAGS += -DuECC_SQUARE_FUNC=1
ASMFLAGS += -DuECC_ENABLE_VLI_API
#ASMFLAGS += -DuECC_SUPPORT_COMPRESSED_POINT=0

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -u _printf_float
# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map


# $1 type of item
# $2 path to check
define ensure_exists
$(if $(wildcard $(2)),, $(warning Cannot find $(1): $(2)))
endef
$(eval INC_PATHS := \
  $(foreach folder, $(INC_FOLDERS) $(INC_FOLDERS_$(strip $(1))), \
    $(call ensure_exists,include folder, $(folder)) \
    -I"$(folder)"))


#default target - first one defined
default: clean $(PROJECT_NAME)

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e $(PROJECT_NAME)

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	$(PROJECT_NAME)
	@echo 	flash_softdevice

C_SOURCE_FILE_NAMES = $(notdir $(SRC_FILES))
C_PATHS = $(call remduplicates, $(dir $(SRC_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

$(PROJECT_NAME): OUTPUT_FILENAME := $(PROJECT_NAME)
$(PROJECT_NAME): LINKER_SCRIPT=ble_app_template_gcc_nrf52.ld

$(PROJECT_NAME): $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@

# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Assembly file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<
# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -lm -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
## Create binary .bin file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex: $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex

finalize: genbin genhex echosize

genbin:
	@echo Preparing: $(OUTPUT_FILENAME).bin
	$(NO_ECHO)$(OBJCOPY) -O binary $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).bin

## Create binary .hex file from the .out file
genhex: 
	@echo Preparing: $(OUTPUT_FILENAME).hex
	$(NO_ECHO)$(OBJCOPY) -O ihex $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).hex
echosize:
	-@echo ''
	$(NO_ECHO)$(SIZE) $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	-@echo ''

clean:
	$(RM) $(BUILD_DIRECTORIES)

cleanobj:
	$(RM) $(BUILD_DIRECTORIES)/*.o
flash: $(PROJECT_NAME)
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf52  --sectorerase
	nrfjprog --reset -f nrf52

## Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_2.0.0_softdevice.hex
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_2.0.0_softdevice.hex -f nrf52 --chiperase
	nrfjprog --reset -f nrf52