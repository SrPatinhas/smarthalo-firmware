PROJECT_NAME := TestFirmware

export OUTPUT_FILENAME
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = ../../3rd/nrf5_sdk/SDK11/components/toolchain/gcc
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

#source common to all targets
C_SOURCE_FILES += \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/button/app_button.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/util/app_error.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/util/app_error_weak.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/fifo/app_fifo.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/timer/app_timer.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/util/app_util_platform.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/fstorage/fstorage.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/util/nrf_assert.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/util/nrf_log.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/uart/retarget.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/external/segger_rtt/RTT_Syscalls_GCC.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/external/segger_rtt/SEGGER_RTT.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/external/segger_rtt/SEGGER_RTT_printf.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/uart/app_uart_fifo.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/delay/nrf_delay.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/common/nrf_drv_common.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/uart/nrf_drv_uart.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/pstorage/pstorage.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/examples/bsp/bsp.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/examples/bsp/bsp_btn_ble.c) \
$(abspath ./src/main.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/common/ble_advdata.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/ble_advertising/ble_advertising.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/common/ble_conn_params.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/ble_services/ble_nus/ble_nus.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/common/ble_srv_common.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/toolchain/system_nrf52.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/softdevice/common/softdevice_handler/softdevice_handler.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/clock/nrf_drv_clock.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/twi_master/nrf_drv_twi.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/pwm/app_pwm.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/timer/nrf_drv_timer.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/ppi/nrf_drv_ppi.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/rtc/nrf_drv_rtc.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/hal/nrf_saadc.c) \
$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/saadc/nrf_drv_saadc.c) \
$(abspath ./src/UART.c) \
$(abspath ./src/I2C.c) \
$(abspath ./src/Bluetooth.c) \
$(abspath ./src/CommandLineInterface.c) \
$(abspath ./src/Batmon.c) \
$(abspath ./src/LEDTest.c) \
$(abspath ./src/TouchTest.c) \
$(abspath ./src/Tone.c) \
$(abspath ./src/EasyScale.c) \
$(abspath ./src/SoundTest.c) \
$(abspath ./src/AcceleroTest.c) \
$(abspath ./src/MagnetoTest.c) \
$(abspath ./src/PowerSupply.c) \
$(abspath ./src/LC709203F.c) \
$(abspath ./src/STC3115.c) \
$(abspath ./src/SAADC.c) \

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath ../../3rd/nrf5_sdk/SDK11/components/toolchain/gcc/gcc_startup_nrf52.s)

#includes common to all targets
INC_PATHS = -I$(abspath ./config)
INC_PATHS += -I$(abspath ../common/include)
INC_PATHS += -I$(abspath ./src)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/config)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/timer)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/fifo)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/fstorage/config)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/delay)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/softdevice/s132/headers/nrf52)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/util)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/uart)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/common)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/pstorage)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/uart)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/device)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/button)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/fstorage)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/experimental_section_vars)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/softdevice/s132/headers)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/external/segger_rtt)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/examples/bsp)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/ble_services/ble_nus)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/hal)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/toolchain/gcc)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/toolchain)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/common)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/ble/ble_advertising)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/clock)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/twi_master)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/libraries/pwm)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/timer)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/ppi)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/rtc)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/SDK11/components/drivers_nrf/saadc)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DNRF52
CFLAGS += -DSOFTDEVICE_PRESENT
#CFLAGS += -DBOARD_PCA10040		#SEB
#CFLAGS += -DTESTFIRMWARE		#SEB
CFLAGS += -DBOARD_CUSTOM		#SEB
#CFLAGS += -DSMARTHALO_EE		#SEB
CFLAGS += -DSMARTHALO_FF		#SEB
#CFLAGS += -DSHMP_DEVBOARD		#SEB
CFLAGS += -DSMARTHALO_UUID_COUNT=1 #Sean
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
#CFLAGS += -DNRF_LOG_USES_UART=1
CFLAGS += -DS132
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DNRF52_PAN_62
CFLAGS += -DNRF52_PAN_63
#SEB
#CFLAGS += -DDEBUG
CFLAGS += -DRADIOTEST
CFLAGS += -DLEDTEST
CFLAGS += -DSOUNDTEST
CFLAGS += -DTOUCHTEST
CFLAGS += -DBATTEST
CFLAGS += -DACCTEST
CFLAGS += -DMAGTEST
#SEB
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror -O0 -g3
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums 
# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys
#SEB
LDFLAGS += -u _printf_float
LDFLAGS += -u _scanf_float


# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF52
ASMFLAGS += -DSOFTDEVICE_PRESENT
#ASMFLAGS += -DBOARD_PCA10040		#SEB
#ASMFLAGS += -DTESTFIRMWARE			#SEB
ASMFLAGS += -DBOARD_CUSTOM			#SEB
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
#ASMFLAGS += -DNRF_LOG_USES_UART=1
ASMFLAGS += -DS132
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DNRF52_PAN_62
ASMFLAGS += -DNRF52_PAN_63

#default target - first one defined
default: clean nrf52832_xxaa_s132

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf52832_xxaa_s132

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf52832_xxaa_s132
	@echo 	flash_softdevice

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

nrf52832_xxaa_s132: OUTPUT_FILENAME := nrf52832_xxaa_s132
nrf52832_xxaa_s132: LINKER_SCRIPT=TestFirmware.ld

nrf52832_xxaa_s132: $(BUILD_DIRECTORIES) $(OBJECTS)
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
# Linkmd
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
flash: nrf52832_xxaa_s132
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf52  --sectorerase
	nrfjprog --reset -f nrf52

## Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_2.0.0_softdevice.hex
	nrfjprog --program ../../3rd/nrf5_sdk/SDK11/components/softdevice/s132/hex/s132_nrf52_2.0.0_softdevice.hex -f nrf52 --chiperase
	nrfjprog --reset -f nrf52