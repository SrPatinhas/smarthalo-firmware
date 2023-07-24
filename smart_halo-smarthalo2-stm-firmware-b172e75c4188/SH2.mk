#
# @file		SH2.mk
# @brief	The actual makefile for SH2 firmware
# @details	The contents here are modified from a Makefile that is
#			created by CubeMX.
#

# Boilerplate for quieter builds
ifeq ($(V),1)
Q=
NQ= :
else
Q=@
NQ=echo
endif

TARGET ?= SH2_STM
export TARGET

NAMEVERSION ?= $(shell ./scripts/version.sh SH2STM .)
DEFVERSION ?= $(shell ./scripts/version.sh SH2STM ,)
HEXVERSION ?= $(shell ./scripts/version.sh SH2STM HEX)

# Target specific overrides
golden: export NAMEVERSION := $(shell ./scripts/version.sh SH2STM . golden)
golden: export DEFVERSION := $(shell ./scripts/version.sh SH2STM , golden)
golden: export HEXVERSION := $(shell ./scripts/version.sh SH2STM HEX golden)
golden: export TARGET := SH2_STM_golden
golden: export BUILD_DIR := build_golden
golden: export SH2_CFLAGS := -DGOLDEN=1

megabug: export TARGET := SH2_STM_megabug
megabug: export BUILD_DIR := build_megabug
megabug: export SH2_CFLAGS := -DMEGABUG_HUNT

# Do timestamp once and export (can't have it changing as the build runs)
TIMESTAMP ?= $(shell date '+%y%m%d-%H%M%S')
export TIMESTAMP

# debug build?
DEBUG = 1
# optimization
OPT = -Os

BUILD_DIR ?= build
export BUILD_DIR

C_SOURCES =  \
Src/assets.c \
Src/qspiflash.c \
Src/adc.c \
Src/crc.c \
Src/dma.c \
Src/freertos.c \
Src/gpio.c \
Src/i2c.c \
Src/iwdg.c \
Src/main.c \
Src/quadspi.c \
Src/rtc.c \
Src/sh2_tsl.c \
Src/spi.c \
Src/stm32l4xx_hal_msp.c \
Src/stm32l4xx_hal_timebase_tim.c \
Src/stm32l4xx_it.c \
Src/stmCriticalSection.c \
Src/syscalls.c \
Src/system_stm32l4xx.c \
Src/tim.c \
Src/touchsensing.c \
Src/tsc.c \
Src/tsl_user.c \
Src/usart.c \
Src/wwdg.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_iwdg.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_wwdg.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_qspi.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tsc.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c \
Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
Middlewares/Third_Party/FreeRTOS/Source/list.c \
Middlewares/Third_Party/FreeRTOS/Source/queue.c \
Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
Middlewares/Third_Party/FreeRTOS/Source/timers.c \
Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c \
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_acq_tsc.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_globals.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_touchkey.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_acq.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_dxs.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_linrot.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_ecs.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_object.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_time.c \
Middlewares/ST/STM32_TouchSensing_Library/src/tsl_filter.c \
Src/Applications/AlarmTask.c \
Src/Applications/NightLightTask.c \
Src/Applications/PersonalityTask.c \
Src/Applications/RideTask.c \
Src/OSS/miniz.c \
Src/Peripherals/BoardRev.c \
Src/Peripherals/DebugConsole.c \
Src/Peripherals/ECompassDriver.c \
Src/Peripherals/HaloLedsDriver.c \
Src/Peripherals/OLEDDriver.c \
Src/Peripherals/PhotoSensor.c \
Src/Peripherals/PiezoDriver.c \
Src/Peripherals/Power.c \
Src/Peripherals/rtcUtils.c \
Src/Peripherals/Shell.c \
Src/Peripherals/ShellComPort.c \
Src/Peripherals/lsm303agr_reg.c \
Src/Peripherals/BLEDriver.c \
Src/Peripherals/FrontLEDDriver.c \
Src/SmartHaloOS/CommunicationTask.c \
Src/SmartHaloOS/FirmwareUpdate.c \
Src/SmartHaloOS/FSUtil.c \
Src/SmartHaloOS/GraphicsTask.c \
Src/SmartHaloOS/SensorsTask.c \
Src/SmartHaloOS/SoundTask.c \
Src/SmartHaloOS/SHftp.c \
Src/SmartHaloOS/SHTaskUtils.c \
Src/SmartHaloOS/SHTimers.c \
Src/SmartHaloOS/SystemUtilitiesTask.c \
Src/SmartHaloOS/WatchdogTask.c \
Src/SmartHaloOS/device_telemetry.c \
Src/SmartHaloOS/reboot.c \
Src/SmartHaloOS/spiffs_cache.c \
Src/SmartHaloOS/spiffs_check.c \
Src/SmartHaloOS/spiffs_gc.c \
Src/SmartHaloOS/spiffs_hydrogen.c \
Src/SmartHaloOS/spiffs_nucleus.c \
Src/SmartHaloOS/AnimationsLibrary.c \
Src/SmartHaloOS/HardwareTests.c \
Src/SmartHaloOS/OLEDLibrary.c \
Src/SmartHaloOS/SubscriptionHelper.c \
Src/SmartHaloOS/gzutil.c \
$(BUILD_DIR)/fwversion.c

# ASM sources
ASM_SOURCES =  \
startup/startup_stm32l451xx.s

PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
# The gap-fill and pad-to options are to create fixed size bin files
# for easy flashing onto SH2 devices
BIN = $(CP) -O binary --gap-fill=0xff --pad-to=0x8040000 -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L451xx \
'-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))'

# AS includes
AS_INCLUDES =  \
-I\Inc

# C includes
C_INCLUDES =  \
-Iassets \
-IInc \
-IInc/Applications \
-IInc/OSS \
-IInc/Peripherals \
-IInc/SmartHaloOS \
-IDrivers/STM32L4xx_HAL_Driver/Inc \
-IDrivers/STM32L4xx_HAL_Driver/Inc/Legacy \
-IMiddlewares/Third_Party/FreeRTOS/Source/include \
-IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
-IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
-IMiddlewares/ST/STM32_TouchSensing_Library/inc \
-IDrivers/CMSIS/Device/ST/STM32L4xx/Include \
-IDrivers/CMSIS/Include

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fstack-usage

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -Werror -fdata-sections -ffunction-sections -fstack-usage

ifeq ($(DEBUG), 1)
CFLAGS += -g3
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -std=gnu11

# link script
LDSCRIPT = STM32L451RC_FLASH.ld

# libraries
LIBS = -Wl,--start-group -lc -lm -lnosys -Wl,--end-group
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

TARGET_BIN = $(BUILD_DIR)/$(TARGET).bin
TARGET_ELF = $(BUILD_DIR)/$(TARGET).elf
TARGET_HEX = $(BUILD_DIR)/$(TARGET).hex

PUBLISHED_BASENAME := $(BUILD_DIR)/$(TARGET)_$(NAMEVERSION)_$(TIMESTAMP)
PUBLISHED_GZIP     := $(PUBLISHED_BASENAME).gz
PUBLISHED_ELF      := $(PUBLISHED_BASENAME).elf

FWVERSION_C = $(BUILD_DIR)/fwversion.c

# Recursive make required to make BUILD_DIR/TARGET overrides work
all golden megabug:
	@$(MAKE) firmware

firmware: $(TARGET_ELF) $(TARGET_BIN) | $(FWVERSION_C)
	@echo "\n==================================================================="
	@echo " "
	@echo "$(TARGET) version: $(NAMEVERSION)"
	@echo " "
	@$(RM) $(BUILD_DIR)/$(TARGET)_$(NAMEVERSION)_*.gz
	@gzip -c $(TARGET_BIN) > $(PUBLISHED_GZIP)
	@echo "===================================================================\n"

publish:
	@$(if $(value FIRMWARE_DESCRIPTION),,$(error FIRMWARE_DESCRIPTION is required))
	@$(MAKE) clean
	@$(MAKE) all
	@$(NQ) UPLOAD $(PUBLISHED_GZIP)
	$(Q)cp $(TARGET_ELF) $(PUBLISHED_ELF)
	$(Q)./scripts/publish.sh $(NAMEVERSION) $(PUBLISHED_GZIP) "$(FIRMWARE_DESCRIPTION)"


clean:
	-rm -fR $(BUILD_DIR)

distclean veryclean:
	-rm -rf build build_golden build_megabug

# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

.PHONY: all golden megabug firmware publis clean distclean veryclean FORCE

$(FWVERSION_C): FORCE
	@$(NQ) GENERATE $(@)
	$(Q)scripts/mkversion.sh $@ $(DEFVERSION)

# suffix rules
$(BUILD_DIR)/%.o: %.c SH2Makefile | $(BUILD_DIR)
	@$(NQ) CC $(notdir $@)
	$(Q)$(CC) -c $(CFLAGS) $(SH2_CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s SH2Makefile | $(BUILD_DIR)
	@$(NQ) AS $(notdir $@)
	$(Q)$(AS) -c $(CFLAGS) $(SH2_CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) SH2Makefile
	@$(NQ) LINK $(notdir $@)
	$(Q)$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

-include $(wildcard $(BUILD_DIR)/*.d)
