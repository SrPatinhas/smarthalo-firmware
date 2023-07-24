PROJECT_NAME := SmartHalo_SDK11_Firmware

export OUTPUT_FILENAME
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = ../../3rd/nrf5_sdk/components/toolchain/gcc
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
$(abspath ../../3rd/nrf5_sdk/components/libraries/button/app_button.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/util/app_error.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/fifo/app_fifo.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/timer/app_timer.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/trace/app_trace.c) \
$(abspath ../SmartHalo_BLE_Bootloader/src/dfu_transport_ble.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/bootloader_util.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/dfu_app_handler.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/util/nrf_assert.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/gpiote/app_gpiote.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/twi/app_twi.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/uart/retarget.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/sensorsim/sensorsim.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/uart/app_uart_fifo.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/delay/nrf_delay.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/twi_master/nrf_drv_twi.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/common/nrf_drv_common.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/uart/nrf_drv_uart.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/util/app_util_platform.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/pstorage/pstorage.c) \
$(abspath ../../3rd/nrf5_sdk/examples/bsp/bsp.c) \
$(abspath ../../3rd/nrf5_sdk/examples/bsp/bsp_btn_ble.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/fstorage/fstorage.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/low_power_pwm/low_power_pwm.c) \
$(abspath /main.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/common/ble_advdata.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/ble_advertising/ble_advertising.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_bas/ble_bas.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/common/ble_conn_params.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_dfu/ble_dfu.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_dis/ble_dis.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_hrs/ble_hrs.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/common/ble_srv_common.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/device_manager/device_manager_peripheral.c) \
$(abspath ../../3rd/nrf5_sdk/components/toolchain/system_nrf52.c) \
$(abspath ../../3rd/nrf5_sdk/components/softdevice/common/softdevice_handler/softdevice_handler.c) \
$(abspath SH_firmware_code/SH_BLE.c) \
$(abspath SH_firmware_code/SH_timer_for_state_machine.c) \
$(abspath SH_firmware_code/SH_Task_handler.c) \
$(abspath SH_firmware_code/SH_Primary_state_machine.c) \
$(abspath SH_firmware_code/SH_Priority_queue.c) \
$(abspath SH_firmware_code/SH_Mouvement_Detection.c) \
$(abspath SH_firmware_code/SH_Factory_tests.c) \
$(abspath SH_firmware_code/SH_UICR_Addresses.c) \
$(abspath SH_firmware_code/SH_Gas_Gauge.c) \
$(abspath SH_firmware_code/SH_GasGauge_INT.c) \
$(abspath SH_firmware_code/vector.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/clock/nrf_drv_clock.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/ppi/nrf_drv_ppi.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/bootloader.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/bootloader_settings.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/crc16/crc16.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/dfu_dual_bank.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/dfu_init_template.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/hci/hci_mem_pool.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/scheduler/app_scheduler.c) \
$(abspath SH_firmware_code/dfu_ble_svc.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/wdt/nrf_drv_wdt.c) \
$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_nus/ble_nus.c) \
$(abspath SH_firmware_code/SH_TWI.c) \
$(abspath SH_firmware_code/SH_Batmon.c) \
$(abspath SH_firmware_code/SH_app_fifo_accelerometer.c) \
$(abspath SH_firmware_code/SH_queue_twi.c) \
$(abspath SH_firmware_code/SH_TapCode.c) \
$(abspath SH_firmware_code/SH_ADC_Read.c) \
$(abspath SH_firmware_code/crc.c) \
$(abspath SH_firmware_code/SH_Accelerometer_Magnetometer.c) \
$(abspath SH_firmware_code/SH_PWM_Center_LED.c) \
$(abspath SH_firmware_code/SH_PWM_FrontLight.c) \
$(abspath SH_firmware_code/SH_CenterLed_Animations.c) \
$(abspath SH_firmware_code/SH_FrontLed_Animations.c) \
$(abspath SH_firmware_code/SH_HaloPixelnRF.c) \
$(abspath SH_firmware_code/SH_Animations.c) \
$(abspath SH_firmware_code/SH_Accelerometer_INT1.c) \
$(abspath SH_firmware_code/SH_Accelerometer_INT2.c) \
$(abspath SH_firmware_code/SH_Tap_Qtouch.c) \
$(abspath SH_firmware_code/SH_Piezo_sounds.c) \
$(abspath SH_firmware_code/SH_Magnetic_field_detection.c) \
$(abspath SH_firmware_code/SH_Temperature_monitoring.c) \
$(abspath SH_firmware_code/SH_Current_manager.c) \
$(abspath SH_firmware_code/SH_Softdevice_and_communication_init.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/saadc/nrf_drv_saadc.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/hal/nrf_saadc.c) \
$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/timer/nrf_drv_timer.c) \
$(abspath ../TestFirmware/src/UART.c) \
$(abspath ../TestFirmware/src/Bluetooth.c) \
$(abspath ../TestFirmware/src/AcceleroTest.c) \
$(abspath ../TestFirmware/src/MagnetoTest.c) \
$(abspath ../TestFirmware/src/I2C.c) \
$(abspath ../TestFirmware/src/CommandLineInterface.c) \
$(abspath ../TestFirmware/src/Batmon.c) \
$(abspath ../TestFirmware/src/LEDTest.c) \
$(abspath ../TestFirmware/src/TouchTest.c) \
$(abspath ../TestFirmware/src/Tone.c) \
$(abspath ../TestFirmware/src/SoundTest.c) \
$(abspath ../TestFirmware/src/PowerSupply.c) \
$(abspath ../../3rd/nrf5_sdk/components/libraries/pwm/app_pwm.c) \
$(abspath ../../3rd/nrf5_sdk/external/segger_rtt/SEGGER_RTT_printf.c) \
$(abspath ../../3rd/nrf5_sdk/external/segger_rtt/SEGGER_RTT.c) \
$(abspath ../../3rd/nrf5_sdk/external/segger_rtt/RTT_Syscalls_GCC.c) \

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath ../../3rd/nrf5_sdk/components/toolchain/gcc/gcc_startup_nrf52.s)

#includes common to all targets
INC_PATHS = -I$(abspath config)
INC_PATHS += -I$(abspath ../TestFirmware/src)
INC_PATHS += -I$(abspath ../SmartHalo_BLE_Bootloader/src)
INC_PATHS += -I$(abspath ../common/include)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/examples/bsp)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/fifo)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/delay)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/softdevice/s132/headers/nrf52)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/util)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/device_manager)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/uart)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/common)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/sensorsim)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/pstorage)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_dis)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/device)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/uart)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/button)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/timer)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/gpiote)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/softdevice/s132/headers)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/hal)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/toolchain/gcc)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/toolchain)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/common)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_advertising)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/trace)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_bas)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_dfu)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_hrs)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/fstorage/config)
INC_PATHS += -I$(abspath SH_firmware_code)
INC_PATHS += -I$(abspath src)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/clock)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/ppi)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/scheduler)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/bootloader_dfu/ble_transport)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/hci)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_dfu)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/crc16)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/twi)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/low_power_pwm)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/pwm)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/common)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/examples/dfu/bootloader) 
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/wdt)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/saadc)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/ble/ble_services/ble_nus)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/common)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/pwm)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/timer)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/simple_timer)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/twi_master)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/fstorage)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/experimental_section_vars)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/libraries/low_power_pwm)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/components/drivers_nrf/rtc)
INC_PATHS += -I$(abspath ../../3rd/nrf5_sdk/external/segger_rtt)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DNRF52
#CFLAGS += -DBOARD_PCA10040
CFLAGS += -DBOARD_CUSTOM
CFLAGS += -DSMARTHALO_EE
#CFLAGS += -DFACTORY_PROGRAMMED_FIRMWARE #enable this flag to include functionnal tests in compilation and remove alarm
#CFLAGS += -DTEST_HALO_BOX #enable this flag to compile for the halo box
#CFLAGS += -DDEBUG_UART_MESSAGES #enable this to send debug messages only when uart comm lines connected to computer
CFLAGS += -DLOW_TEMP_BATTERY
#CFLAGS += -DHIGH_TEMP_BATTERY
#CFLAGS += -DFACTORY_MODE_READY
#CFLAGS += -DDFU_UPDATE_READY_MODE
#CFLAGS += -DDEBUG_MODE
#CFLAGS += -DBACKDOOR_OPEN
CFLAGS += -DSMARTHALO_5_LEDS_QUARTER_HALO_11_LEDS_HALF_HALO #enable this flag to light 2 leds less on the halo
CFLAGS += -DSMARTHALO_UUID_COUNT=2
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
CFLAGS += -DNRF_LOG_USES_UART=1
CFLAGS += -DS132
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DSOFTDEVICE_PRESENT
CFLAGS += -DNRF52_PAN_62
CFLAGS += -DNRF52_PAN_63
CFLAGS += -DBLE_DFU_APP_SUPPORT
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs --std=gnu99
CFLAGS += -Wall -Werror -O0 -g3 # normalement O0 pas Os
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in separate section. This will allow linker to dump unused functions
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin --short-enums
CFLAGS += -fomit-frame-pointer

# keep every function in separate section. This will allow linker to dump unused functions
LDFLAGS += -Xlinker -Map=$(LISTING_DIRECTORY)/$(OUTPUT_FILENAME).map
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -u _printf_float
LIBS = -lm #added to have math functionnalities

# Assembler flags
ASMFLAGS += -x assembler-with-cpp
ASMFLAGS += -DNRF52
#ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DBOARD_CUSTOM
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
ASMFLAGS += -DNRF_LOG_USES_UART=1
ASMFLAGS += -DS132
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DBLE_STACK_SUPPORT_REQD
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DSOFTDEVICE_PRESENT
ASMFLAGS += -DNRF52_PAN_62
ASMFLAGS += -DNRF52_PAN_63
ASMFLAGS += -DBLE_DFU_APP_SUPPORT
#default target - first one defined
default: clean s132_pca10040

#building all targets
all: clean
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e cleanobj
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e s132_pca10040

#target for printing all targets
help:
	@echo following targets are available:
	@echo 	s132_pca10040
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

s132_pca10040: OUTPUT_FILENAME := s132_pca10040
s132_pca10040: CFLAGS += -DBLE_DFU_APP_SUPPORT
s132_pca10040: ASMFLAGS += -DBLE_DFU_APP_SUPPORT
s132_pca10040: LINKER_SCRIPT=ble_app_hrs_gcc_nrf52.ld
s132_pca10040: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out
	$(NO_ECHO)$(MAKE) -f $(MAKEFILE_NAME) -C $(MAKEFILE_DIR) -e finalize

## Create build directories
$(BUILD_DIRECTORIES):
	echo $(MAKEFILE_NAME)
	$(MK) $@
# Create objects from C SRC files
src/stdafx.h.gch: src/stdafx.h
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<
	
# Create objects from C SRC files
$(OBJECT_DIRECTORY)/%.o: %.c
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(CFLAGS) $(INC_PATHS) -c -o $@ $<

# Assemble files
$(OBJECT_DIRECTORY)/%.o: %.s
	@echo Compiling file: $(notdir $<)
	$(NO_ECHO)$(CC) $(ASMFLAGS) $(INC_PATHS) -c -o $@ $<


# Link
$(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out: $(BUILD_DIRECTORIES) $(OBJECTS)
	@echo Linking target: $(OUTPUT_FILENAME).out
	$(NO_ECHO)$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(OUTPUT_BINARY_DIRECTORY)/$(OUTPUT_FILENAME).out


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

flash: s132_pca10040
	@echo Flashing: $(OUTPUT_BINARY_DIRECTORY)/$<.hex
	nrfjprog --program $(OUTPUT_BINARY_DIRECTORY)/$<.hex -f nrf52  --sectorerase
	nrfjprog --reset -f nrf52

## Flash softdevice
flash_softdevice:
	@echo Flashing: s132_nrf52_2.0.0-7.alpha_softdevice.hex
	nrfjprog --program ../../3rd/nrf5_sdk/components/softdevice/s132/hex/s132_nrf52_2.0.0-7.alpha_softdevice.hex -f nrf52 --chiperase
	nrfjprog --reset -f nrf52
