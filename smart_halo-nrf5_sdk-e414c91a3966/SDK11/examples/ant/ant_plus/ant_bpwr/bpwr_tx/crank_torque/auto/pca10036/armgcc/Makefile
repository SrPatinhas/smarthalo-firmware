PROJECT_NAME := ant_bpwr_tx_crank_torque_auto_s212_pca10036

export OUTPUT_FILENAME
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = ../../../../../../../../../components/toolchain/gcc
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
$(abspath ../../../../../../../../../components/libraries/button/app_button.c) \
$(abspath ../../../../../../../../../components/libraries/util/app_error.c) \
$(abspath ../../../../../../../../../components/libraries/util/app_error_weak.c) \
$(abspath ../../../../../../../../../components/libraries/fifo/app_fifo.c) \
$(abspath ../../../../../../../../../components/libraries/timer/app_timer.c) \
$(abspath ../../../../../../../../../components/libraries/trace/app_trace.c) \
$(abspath ../../../../../../../../../components/libraries/util/app_util_platform.c) \
$(abspath ../../../../../../../../../components/libraries/util/nrf_assert.c) \
$(abspath ../../../../../../../../../components/libraries/util/nrf_log.c) \
$(abspath ../../../../../../../../../components/libraries/uart/retarget.c) \
$(abspath ../../../../../../../../../components/libraries/sensorsim/sensorsim.c) \
$(abspath ../../../../../../../../../components/libraries/uart/app_uart_fifo.c) \
$(abspath ../../../../../../../../../components/drivers_nrf/delay/nrf_delay.c) \
$(abspath ../../../../../../../../../components/drivers_nrf/common/nrf_drv_common.c) \
$(abspath ../../../../../../../../../components/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath ../../../../../../../../../components/drivers_nrf/uart/nrf_drv_uart.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/ant_bpwr.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/ant_bpwr_common_data.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/ant_bpwr_page_1.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/ant_bpwr_page_16.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/ant_bpwr_page_17.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/ant_bpwr_page_18.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/ant_bpwr_page_torque.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/simulator/ant_bpwr_simulator.c) \
$(abspath ../../../../../../../../../components/ant/ant_channel_config/ant_channel_config.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_common/pages/ant_common_page_80.c) \
$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_common/pages/ant_common_page_81.c) \
$(abspath ../../../../../../../../../components/ant/ant_key_manager/ant_key_manager.c) \
$(abspath ../../../../../../../../../components/ant/ant_stack_config/ant_stack_config.c) \
$(abspath ../../../../../../../../../components/ant/ant_state_indicator/ant_state_indicator.c) \
$(abspath ../../../../../../../../bsp/bsp.c) \
$(abspath ../../../../../../../../bsp/bsp_btn_ant.c) \
$(abspath ../../../../main.c) \
$(abspath ../../../../../../../../../external/segger_rtt/RTT_Syscalls_GCC.c) \
$(abspath ../../../../../../../../../external/segger_rtt/SEGGER_RTT.c) \
$(abspath ../../../../../../../../../external/segger_rtt/SEGGER_RTT_printf.c) \
$(abspath ../../../../../../../../../components/toolchain/system_nrf52.c) \
$(abspath ../../../../../../../../../components/softdevice/common/softdevice_handler/softdevice_handler.c) \

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath ../../../../../../../../../components/toolchain/gcc/gcc_startup_nrf52.s)

#includes common to all targets
INC_PATHS  = -I$(abspath ../../../../config)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_key_manager/config)
INC_PATHS += -I$(abspath ../../../../../../../../../components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_state_indicator)
INC_PATHS += -I$(abspath ../../../../../../../../../components/toolchain)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/uart)
INC_PATHS += -I$(abspath ../../../../../../../../../components/drivers_nrf/common)
INC_PATHS += -I$(abspath ../../../../../../../../../components/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_channel_config)
INC_PATHS += -I$(abspath ../../../../../../../../../components/toolchain/gcc)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/utils)
INC_PATHS += -I$(abspath ../../../../../../../../../external/segger_rtt)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_stack_config)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages/logger)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/simulator)
INC_PATHS += -I$(abspath ../../../../../../../../../components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_common/pages/logger)
INC_PATHS += -I$(abspath ../../../../../../../../../components/drivers_nrf/uart)
INC_PATHS += -I$(abspath ../../../../../../../../../components/device)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/timer)
INC_PATHS += -I$(abspath ../../../../../../../../../components/drivers_nrf/config)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/util)
INC_PATHS += -I$(abspath ../../../../../../../../../components/drivers_nrf/hal)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/button)
INC_PATHS += -I$(abspath ../../../../../../../../bsp)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_common/pages)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/sensorsim)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/fifo)
INC_PATHS += -I$(abspath ../../../../../../../../../components/softdevice/s212/headers/nrf52)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr/pages)
INC_PATHS += -I$(abspath ../../../../../../../../../components/libraries/trace)
INC_PATHS += -I$(abspath ../../../../../../../../../components/softdevice/s212/headers)
INC_PATHS += -I$(abspath ../../../../../../../../../components/drivers_nrf/delay)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_profiles/ant_bpwr)
INC_PATHS += -I$(abspath ../../../../../../../../../components/ant/ant_key_manager)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DNRF52_PAN_16
CFLAGS += -DNRF52_PAN_23
CFLAGS += -DNRF52_PAN_3
CFLAGS += -DNRF52
CFLAGS += -DTRACE_BPWR_PAGE_18_ENABLE
CFLAGS += -DNRF52_PAN_36
CFLAGS += -DNRF52_PAN_4
CFLAGS += -DNRF52_PAN_7
CFLAGS += -DNRF52_PAN_1
CFLAGS += -DMODIFICATION_TYPE=1
CFLAGS += -DNRF52_PAN_2
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF52_PAN_34
CFLAGS += -DNRF52_PAN_9
CFLAGS += -DNRF52_PAN_8
CFLAGS += -DTRACE_BPWR_PAGE_1_ENABLE
CFLAGS += -DNRF52_PAN_11
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DNRF52_PAN_39
CFLAGS += -DNRF52_PAN_38
CFLAGS += -DNRF52_PAN_15
CFLAGS += -DNRF52_PAN_58
CFLAGS += -DNRF52_PAN_17
CFLAGS += -DNRF52_PAN_65
CFLAGS += -DNRF52_PAN_33
CFLAGS += -DTRACE_COMMON_PAGE_80_ENABLE
CFLAGS += -DNRF52_PAN_30
CFLAGS += -DNRF52_PAN_37
CFLAGS += -DTRACE_BPWR_PAGE_17_ENABLE
CFLAGS += -DNRF52_PAN_35
CFLAGS += -DANT_STACK_SUPPORT_REQD
CFLAGS += -DTRACE_BPWR_GENERAL_ENABLE
CFLAGS += -DNRF_LOG_USES_UART=1
CFLAGS += -DS212
CFLAGS += -DNRF52_PAN_10
CFLAGS += -DNRF52_PAN_63
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DNRF52_PAN_12
CFLAGS += -DENABLE_DEBUG_LOG_SUPPORT
CFLAGS += -DTRACE_COMMON_PAGE_81_ENABLE
CFLAGS += -DSENSOR_TYPE=2
CFLAGS += -DNRF52_PAN_28
CFLAGS += -DNRF52_PAN_29
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DTRACE_BPWR_PAGE_16_ENABLE
CFLAGS += -DBOARD_PCA10036
CFLAGS += -DNRF52_PAN_24
CFLAGS += -DNRF52_PAN_25
CFLAGS += -DNRF52_PAN_26
CFLAGS += -DNRF52_PAN_27
CFLAGS += -DNRF52_PAN_42
CFLAGS += -DNRF52_PAN_43
CFLAGS += -DNRF52_PAN_40
CFLAGS += -DNRF52_PAN_41
CFLAGS += -DNRF52_PAN_46
CFLAGS += -DNRF52_PAN_47
CFLAGS += -DNRF52_PAN_44
CFLAGS += -DNRF52_PAN_32
CFLAGS += -DNRF52_PAN_48
CFLAGS += -DNRF52_PAN_49
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror -O3 -g3
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

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF52_PAN_16
ASMFLAGS += -DNRF52_PAN_23
ASMFLAGS += -DNRF52_PAN_3
ASMFLAGS += -DNRF52
ASMFLAGS += -DTRACE_BPWR_PAGE_18_ENABLE
ASMFLAGS += -DNRF52_PAN_36
ASMFLAGS += -DNRF52_PAN_4
ASMFLAGS += -DNRF52_PAN_7
ASMFLAGS += -DNRF52_PAN_1
ASMFLAGS += -DMODIFICATION_TYPE=1
ASMFLAGS += -DNRF52_PAN_2
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF52_PAN_34
ASMFLAGS += -DNRF52_PAN_9
ASMFLAGS += -DNRF52_PAN_8
ASMFLAGS += -DTRACE_BPWR_PAGE_1_ENABLE
ASMFLAGS += -DNRF52_PAN_11
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DNRF52_PAN_39
ASMFLAGS += -DNRF52_PAN_38
ASMFLAGS += -DNRF52_PAN_15
ASMFLAGS += -DNRF52_PAN_58
ASMFLAGS += -DNRF52_PAN_17
ASMFLAGS += -DNRF52_PAN_65
ASMFLAGS += -DNRF52_PAN_33
ASMFLAGS += -DTRACE_COMMON_PAGE_80_ENABLE
ASMFLAGS += -DNRF52_PAN_30
ASMFLAGS += -DNRF52_PAN_37
ASMFLAGS += -DTRACE_BPWR_PAGE_17_ENABLE
ASMFLAGS += -DNRF52_PAN_35
ASMFLAGS += -DANT_STACK_SUPPORT_REQD
ASMFLAGS += -DTRACE_BPWR_GENERAL_ENABLE
ASMFLAGS += -DNRF_LOG_USES_UART=1
ASMFLAGS += -DS212
ASMFLAGS += -DNRF52_PAN_10
ASMFLAGS += -DNRF52_PAN_63
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DNRF52_PAN_12
ASMFLAGS += -DENABLE_DEBUG_LOG_SUPPORT
ASMFLAGS += -DTRACE_COMMON_PAGE_81_ENABLE
ASMFLAGS += -DSENSOR_TYPE=2
ASMFLAGS += -DNRF52_PAN_28
ASMFLAGS += -DNRF52_PAN_29
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DTRACE_BPWR_PAGE_16_ENABLE
ASMFLAGS += -DBOARD_PCA10036
ASMFLAGS += -DNRF52_PAN_24
ASMFLAGS += -DNRF52_PAN_25
ASMFLAGS += -DNRF52_PAN_26
ASMFLAGS += -DNRF52_PAN_27
ASMFLAGS += -DNRF52_PAN_42
ASMFLAGS += -DNRF52_PAN_43
ASMFLAGS += -DNRF52_PAN_40
ASMFLAGS += -DNRF52_PAN_41
ASMFLAGS += -DNRF52_PAN_46
ASMFLAGS += -DNRF52_PAN_47
ASMFLAGS += -DNRF52_PAN_44
ASMFLAGS += -DNRF52_PAN_32
ASMFLAGS += -DNRF52_PAN_48
ASMFLAGS += -DNRF52_PAN_49

#default target - first one defined
default: clean nrf52832_xxaa

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e nrf52832_xxaa

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	nrf52832_xxaa

C_SOURCE_FILE_NAMES = $(notdir $(C_SOURCE_FILES))
C_PATHS = $(call remduplicates, $(dir $(C_SOURCE_FILES) ) )
C_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(C_SOURCE_FILE_NAMES:.c=.o) )

ASM_SOURCE_FILE_NAMES = $(notdir $(ASM_SOURCE_FILES))
ASM_PATHS = $(call remduplicates, $(dir $(ASM_SOURCE_FILES) ))
ASM_OBJECTS = $(addprefix $(OBJECT_DIRECTORY)/, $(ASM_SOURCE_FILE_NAMES:.s=.o) )

vpath %.c $(C_PATHS)
vpath %.s $(ASM_PATHS)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

nrf52832_xxaa: OUTPUT_FILENAME := nrf52832_xxaa
nrf52832_xxaa: LINKER_SCRIPT=ant_bpwr_tx_gcc_nrf52.ld

nrf52832_xxaa: $(BUILD_DIRECTORIES) $(OBJECTS)
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
flash: nrf52832_xxaa
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf52  --chiperase
	nrfjprog --reset -f nrf52

## Flash softdevice