/**
 * Copyright (c) 2017 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */



#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H
// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif
// <h> Board Support 

//==========================================================
// <q> BSP_BTN_BLE_ENABLED  - bsp_btn_ble - Button Control for BLE
 

#ifndef BSP_BTN_BLE_ENABLED
#define BSP_BTN_BLE_ENABLED 1
#endif

// </h> 
//==========================================================

// <h> nRF_BLE 

//==========================================================
// <q> BLE_ADVERTISING_ENABLED  - ble_advertising - Advertising module
 

#ifndef BLE_ADVERTISING_ENABLED
#define BLE_ADVERTISING_ENABLED 1
#endif

// <e> NRF_BLE_CONN_PARAMS_ENABLED - ble_conn_params - Initiating and executing a connection parameters negotiation procedure
//==========================================================
#ifndef NRF_BLE_CONN_PARAMS_ENABLED
#define NRF_BLE_CONN_PARAMS_ENABLED 1
#endif
// <o> NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION - The largest acceptable deviation in slave latency. 
// <i> The largest deviation (+ or -) from the requested slave latency that will not be renegotiated.

#ifndef NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION
#define NRF_BLE_CONN_PARAMS_MAX_SLAVE_LATENCY_DEVIATION 499
#endif

// <o> NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION - The largest acceptable deviation (in 10 ms units) in supervision timeout. 
// <i> The largest deviation (+ or -, in 10 ms units) from the requested supervision timeout that will not be renegotiated.

#ifndef NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION
#define NRF_BLE_CONN_PARAMS_MAX_SUPERVISION_TIMEOUT_DEVIATION 65535
#endif

// </e>

// <q> NRF_BLE_GATT_ENABLED  - nrf_ble_gatt - GATT module
 

#ifndef NRF_BLE_GATT_ENABLED
#define NRF_BLE_GATT_ENABLED 1
#endif

// <e> PEER_MANAGER_ENABLED - peer_manager - Peer Manager
//==========================================================
#ifndef PEER_MANAGER_ENABLED
#define PEER_MANAGER_ENABLED 1
#endif
// <o> PM_MAX_REGISTRANTS - Number of event handlers that can be registered. 
#ifndef PM_MAX_REGISTRANTS
#define PM_MAX_REGISTRANTS 3
#endif

// <o> PM_FLASH_BUFFERS - Number of internal buffers for flash operations. 
// <i> Decrease this value to lower RAM usage.

#ifndef PM_FLASH_BUFFERS
#define PM_FLASH_BUFFERS 2
#endif

// <q> PM_CENTRAL_ENABLED  - Enable/disable central-specific Peer Manager functionality.
 

// <i> Enable/disable central-specific Peer Manager functionality.

#ifndef PM_CENTRAL_ENABLED
#define PM_CENTRAL_ENABLED 0
#endif

// </e>

// </h> 
//==========================================================

// <h> nRF_DFU 

//==========================================================
// <h> ble_dfu - Device Firmware Update

//==========================================================
// <q> BLE_DFU_ENABLED  - Enable DFU Service.
 

#ifndef BLE_DFU_ENABLED
#define BLE_DFU_ENABLED 1
#endif

// <q> NRF_DFU_BLE_BUTTONLESS_SUPPORTS_BONDS  - Buttonless DFU supports bonds.
 

#ifndef NRF_DFU_BLE_BUTTONLESS_SUPPORTS_BONDS
#define NRF_DFU_BLE_BUTTONLESS_SUPPORTS_BONDS 0
#endif

// </h> 
//==========================================================

// </h> 
//==========================================================

// <h> nRF_Drivers 

//==========================================================
// <e> CLOCK_ENABLED - nrf_drv_clock - CLOCK peripheral driver
//==========================================================
#ifndef CLOCK_ENABLED
#define CLOCK_ENABLED 1
#endif
// <o> CLOCK_CONFIG_XTAL_FREQ  - HF XTAL Frequency
 
// <0=> Default (64 MHz) 

#ifndef CLOCK_CONFIG_XTAL_FREQ
#define CLOCK_CONFIG_XTAL_FREQ 0
#endif

// <o> CLOCK_CONFIG_LF_SRC  - LF Clock Source
 
// <0=> RC 
// <1=> XTAL 
// <2=> Synth 

#ifndef CLOCK_CONFIG_LF_SRC
#define CLOCK_CONFIG_LF_SRC 1
#endif

// <o> CLOCK_CONFIG_IRQ_PRIORITY  - Interrupt priority
 

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef CLOCK_CONFIG_IRQ_PRIORITY
#define CLOCK_CONFIG_IRQ_PRIORITY 7
#endif

// </e>

// <e> GPIOTE_ENABLED - nrf_drv_gpiote - GPIOTE peripheral driver
//==========================================================
#ifndef GPIOTE_ENABLED
#define GPIOTE_ENABLED 1
#endif
// <o> GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS - Number of lower power input pins 
#ifndef GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 4
#endif

// <o> GPIOTE_CONFIG_IRQ_PRIORITY  - Interrupt priority
 

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef GPIOTE_CONFIG_IRQ_PRIORITY
#define GPIOTE_CONFIG_IRQ_PRIORITY 7
#endif

// </e>

// <q> PERIPHERAL_RESOURCE_SHARING_ENABLED  - nrf_drv_common - Peripheral drivers common module
 

#ifndef PERIPHERAL_RESOURCE_SHARING_ENABLED
#define PERIPHERAL_RESOURCE_SHARING_ENABLED 0
#endif

// <e> UART_ENABLED - nrf_drv_uart - UART/UARTE peripheral driver
//==========================================================
#ifndef UART_ENABLED
#define UART_ENABLED 1
#endif
// <o> UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control
 
// <0=> Disabled 
// <1=> Enabled 

#ifndef UART_DEFAULT_CONFIG_HWFC
#define UART_DEFAULT_CONFIG_HWFC 0
#endif

// <o> UART_DEFAULT_CONFIG_PARITY  - Parity
 
// <0=> Excluded 
// <14=> Included 

#ifndef UART_DEFAULT_CONFIG_PARITY
#define UART_DEFAULT_CONFIG_PARITY 0
#endif

// <o> UART_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate
 
// <323584=> 1200 baud 
// <643072=> 2400 baud 
// <1290240=> 4800 baud 
// <2576384=> 9600 baud 
// <3862528=> 14400 baud 
// <5152768=> 19200 baud 
// <7716864=> 28800 baud 
// <10289152=> 38400 baud 
// <15400960=> 57600 baud 
// <20615168=> 76800 baud 
// <30801920=> 115200 baud 
// <61865984=> 230400 baud 
// <67108864=> 250000 baud 
// <121634816=> 460800 baud 
// <251658240=> 921600 baud 
// <268435456=> 1000000 baud 

#ifndef UART_DEFAULT_CONFIG_BAUDRATE
#define UART_DEFAULT_CONFIG_BAUDRATE 30801920
#endif

// <o> UART_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority
 

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef UART_DEFAULT_CONFIG_IRQ_PRIORITY
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY 7
#endif

// <q> UART_EASY_DMA_SUPPORT  - Driver supporting EasyDMA
 

#ifndef UART_EASY_DMA_SUPPORT
#define UART_EASY_DMA_SUPPORT 1
#endif

// <q> UART_LEGACY_SUPPORT  - Driver supporting Legacy mode
 

#ifndef UART_LEGACY_SUPPORT
#define UART_LEGACY_SUPPORT 1
#endif

// <e> UART0_ENABLED - Enable UART0 instance
//==========================================================
#ifndef UART0_ENABLED
#define UART0_ENABLED 1
#endif
// <q> UART0_CONFIG_USE_EASY_DMA  - Default setting for using EasyDMA
 

#ifndef UART0_CONFIG_USE_EASY_DMA
#define UART0_CONFIG_USE_EASY_DMA 1
#endif

// </e>

// </e>

// </h> 
//==========================================================

// <h> nRF_Libraries 

//==========================================================
// <e> APP_SCHEDULER_ENABLED - app_scheduler - Events scheduler
//==========================================================
#ifndef APP_SCHEDULER_ENABLED
#define APP_SCHEDULER_ENABLED 1
#endif
// <q> APP_SCHEDULER_WITH_PAUSE  - Enabling pause feature
 

#ifndef APP_SCHEDULER_WITH_PAUSE
#define APP_SCHEDULER_WITH_PAUSE 0
#endif

// <q> APP_SCHEDULER_WITH_PROFILER  - Enabling scheduler profiling
 

#ifndef APP_SCHEDULER_WITH_PROFILER
#define APP_SCHEDULER_WITH_PROFILER 0
#endif

// </e>

// <e> APP_TIMER_ENABLED - app_timer - Application timer functionality
//==========================================================
#ifndef APP_TIMER_ENABLED
#define APP_TIMER_ENABLED 1
#endif
// <o> APP_TIMER_CONFIG_RTC_FREQUENCY  - Configure RTC prescaler.
 
// <0=> 32768 Hz 
// <1=> 16384 Hz 
// <3=> 8192 Hz 
// <7=> 4096 Hz 
// <15=> 2048 Hz 
// <31=> 1024 Hz 

#ifndef APP_TIMER_CONFIG_RTC_FREQUENCY
#define APP_TIMER_CONFIG_RTC_FREQUENCY 0
#endif

// <o> APP_TIMER_CONFIG_IRQ_PRIORITY  - Interrupt priority
 

// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

#ifndef APP_TIMER_CONFIG_IRQ_PRIORITY
#define APP_TIMER_CONFIG_IRQ_PRIORITY 7
#endif

// <o> APP_TIMER_CONFIG_OP_QUEUE_SIZE - Capacity of timer requests queue. 
// <i> Size of the queue depends on how many timers are used
// <i> in the system, how often timers are started and overall
// <i> system latency. If queue size is too small app_timer calls
// <i> will fail.

#ifndef APP_TIMER_CONFIG_OP_QUEUE_SIZE
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10
#endif

// <q> APP_TIMER_CONFIG_USE_SCHEDULER  - Enable scheduling app_timer events to app_scheduler
 

#ifndef APP_TIMER_CONFIG_USE_SCHEDULER
#define APP_TIMER_CONFIG_USE_SCHEDULER 0
#endif

// <q> APP_TIMER_WITH_PROFILER  - Enable app_timer profiling
 

#ifndef APP_TIMER_WITH_PROFILER
#define APP_TIMER_WITH_PROFILER 0
#endif

// <q> APP_TIMER_KEEPS_RTC_ACTIVE  - Enable RTC always on
 

// <i> If option is enabled RTC is kept running even if there is no active timers.
// <i> This option can be used when app_timer is used for timestamping.

#ifndef APP_TIMER_KEEPS_RTC_ACTIVE
#define APP_TIMER_KEEPS_RTC_ACTIVE 0
#endif

// <o> APP_TIMER_CONFIG_SWI_NUMBER  - Configure SWI instance used.
 
// <0=> 0 
// <1=> 1 

#ifndef APP_TIMER_CONFIG_SWI_NUMBER
#define APP_TIMER_CONFIG_SWI_NUMBER 0
#endif

// </e>

// <q> BUTTON_ENABLED  - app_button - buttons handling module
 

#ifndef BUTTON_ENABLED
#define BUTTON_ENABLED 1
#endif

// <q> CRC16_ENABLED  - crc16 - CRC16 calculation routines
 

#ifndef CRC16_ENABLED
#define CRC16_ENABLED 1
#endif

// <e> FDS_ENABLED - fds - Flash data storage module
//==========================================================
#ifndef FDS_ENABLED
#define FDS_ENABLED 1
#endif
// <h> Pages - Virtual page settings

// <i> Configure the number of virtual pages to use and their size.
//==========================================================
// <o> FDS_VIRTUAL_PAGES - Number of virtual flash pages to use. 
// <i> One of the virtual pages is reserved by the system for garbage collection.
// <i> Therefore, the minimum is two virtual pages: one page to store data and one page to be used by the system for garbage collection.
// <i> The total amount of flash memory that is used by FDS amounts to @ref FDS_VIRTUAL_PAGES * @ref FDS_VIRTUAL_PAGE_SIZE * 4 bytes.

#ifndef FDS_VIRTUAL_PAGES
#define FDS_VIRTUAL_PAGES 3
#endif

// <o> FDS_VIRTUAL_PAGE_SIZE  - The size of a virtual flash page.
 

// <i> Expressed in number of 4-byte words.
// <i> By default, a virtual page is the same size as a physical page.
// <i> The size of a virtual page must be a multiple of the size of a physical page.
// <1024=> 1024 
// <2048=> 2048 

#ifndef FDS_VIRTUAL_PAGE_SIZE
#define FDS_VIRTUAL_PAGE_SIZE 1024
#endif

// </h> 
//==========================================================

// <h> Backend - Backend configuration

// <i> Configure which nrf_fstorage backend is used by FDS to write to flash.
//==========================================================
// <o> FDS_BACKEND  - FDS flash backend.
 

// <i> NRF_FSTORAGE_SD uses the nrf_fstorage_sd backend implementation using the SoftDevice API. Use this if you have a SoftDevice present.
// <i> NRF_FSTORAGE_NVMC uses the nrf_fstorage_nvmc implementation. Use this setting if you don't use the SoftDevice.
// <1=> NRF_FSTORAGE_NVMC 
// <2=> NRF_FSTORAGE_SD 

#ifndef FDS_BACKEND
#define FDS_BACKEND 2
#endif

// </h> 
//==========================================================

// <h> Queue - Queue settings

//==========================================================
// <o> FDS_OP_QUEUE_SIZE - Size of the internal queue. 
// <i> Increase this value if you frequently get synchronous FDS_ERR_NO_SPACE_IN_QUEUES errors.

#ifndef FDS_OP_QUEUE_SIZE
#define FDS_OP_QUEUE_SIZE 4
#endif

// </h> 
//==========================================================

// <h> CRC - CRC functionality

//==========================================================
// <e> FDS_CRC_CHECK_ON_READ - Enable CRC checks.

// <i> Save a record's CRC when it is written to flash and check it when the record is opened.
// <i> Records with an incorrect CRC can still be 'seen' by the user using FDS functions, but they cannot be opened.
// <i> Additionally, they will not be garbage collected until they are deleted.
//==========================================================
#ifndef FDS_CRC_CHECK_ON_READ
#define FDS_CRC_CHECK_ON_READ 0
#endif
// <o> FDS_CRC_CHECK_ON_WRITE  - Perform a CRC check on newly written records.
 

// <i> Perform a CRC check on newly written records.
// <i> This setting can be used to make sure that the record data was not altered while being written to flash.
// <1=> Enabled 
// <0=> Disabled 

#ifndef FDS_CRC_CHECK_ON_WRITE
#define FDS_CRC_CHECK_ON_WRITE 0
#endif

// </e>

// </h> 
//==========================================================

// <h> Users - Number of users

//==========================================================
// <o> FDS_MAX_USERS - Maximum number of callbacks that can be registered. 
#ifndef FDS_MAX_USERS
#define FDS_MAX_USERS 4
#endif

// </h> 
//==========================================================

// </e>

// <e> NRF_BALLOC_ENABLED - nrf_balloc - Block allocator module
//==========================================================
#ifndef NRF_BALLOC_ENABLED
#define NRF_BALLOC_ENABLED 1
#endif
// <e> NRF_BALLOC_CONFIG_DEBUG_ENABLED - Enables debug mode in the module.
//==========================================================
#ifndef NRF_BALLOC_CONFIG_DEBUG_ENABLED
#define NRF_BALLOC_CONFIG_DEBUG_ENABLED 0
#endif
// <o> NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS - Number of words used as head guard.  <0-255> 


#ifndef NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS
#define NRF_BALLOC_CONFIG_HEAD_GUARD_WORDS 1
#endif

// <o> NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS - Number of words used as tail guard.  <0-255> 


#ifndef NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS
#define NRF_BALLOC_CONFIG_TAIL_GUARD_WORDS 1
#endif

// <q> NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED  - Enables basic checks in this module.
 

#ifndef NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED
#define NRF_BALLOC_CONFIG_BASIC_CHECKS_ENABLED 0
#endif

// <q> NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED  - Enables double memory free check in this module.
 

#ifndef NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED
#define NRF_BALLOC_CONFIG_DOUBLE_FREE_CHECK_ENABLED 0
#endif

// <q> NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED  - Enables free memory corruption check in this module.
 

#ifndef NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED
#define NRF_BALLOC_CONFIG_DATA_TRASHING_CHECK_ENABLED 0
#endif

// </e>

// </e>

// <q> NRF_FPRINTF_ENABLED  - nrf_fprintf - fprintf function.
 

#ifndef NRF_FPRINTF_ENABLED
#define NRF_FPRINTF_ENABLED 1
#endif

// <e> NRF_FSTORAGE_ENABLED - nrf_fstorage - Flash abstraction library
//==========================================================
#ifndef NRF_FSTORAGE_ENABLED
#define NRF_FSTORAGE_ENABLED 1
#endif
// <h> nrf_fstorage_sd - Implementation using the SoftDevice.

// <i> Configuration options for the fstorage implementation using the SoftDevice.
//==========================================================
// <o> NRF_FSTORAGE_SD_QUEUE_SIZE - Size of the internal queue of operations. 
// <i> Increase this value if API calls frequently return the error @ref NRF_ERROR_NO_MEM.

#ifndef NRF_FSTORAGE_SD_QUEUE_SIZE
#define NRF_FSTORAGE_SD_QUEUE_SIZE 4
#endif

// <o> NRF_FSTORAGE_SD_MAX_RETRIES - Maximum number of attempts at executing an operation when the SoftDevice is busy. 
// <i> Increase this value if events frequently return the @ref NRF_ERROR_TIMEOUT error.
// <i> The SoftDevice might fail to schedule flash access due to high BLE activity.

#ifndef NRF_FSTORAGE_SD_MAX_RETRIES
#define NRF_FSTORAGE_SD_MAX_RETRIES 8
#endif

// <o> NRF_FSTORAGE_SD_MAX_WRITE_SIZE - Maximum number of bytes to be written to flash in a single operation. 
// <i> This value must be a multiple of four.
// <i> Lowering this value can increase the chances of the SoftDevice being able to execute flash operations in between radio activity.
// <i> This value is bound by the maximum number of bytes that can be written to flash in a single call to @ref sd_flash_write.
// <i> That is 1024 bytes for nRF51 ICs and 4096 bytes for nRF52 ICs.

#ifndef NRF_FSTORAGE_SD_MAX_WRITE_SIZE
#define NRF_FSTORAGE_SD_MAX_WRITE_SIZE 4096
#endif

// </h> 
//==========================================================

// </e>

// <q> NRF_MEMOBJ_ENABLED  - nrf_memobj - Linked memory allocator module
 

#ifndef NRF_MEMOBJ_ENABLED
#define NRF_MEMOBJ_ENABLED 1
#endif

// <e> NRF_PWR_MGMT_ENABLED - nrf_pwr_mgmt - Power management module
//==========================================================
#ifndef NRF_PWR_MGMT_ENABLED
#define NRF_PWR_MGMT_ENABLED 1
#endif
// <e> NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED - Enables pin debug in the module.

// <i> Selected pin will be set when CPU is in sleep mode.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED
#define NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_SLEEP_DEBUG_PIN  - Pin number
 
// <0=> 0 (P0.0) 
// <1=> 1 (P0.1) 
// <2=> 2 (P0.2) 
// <3=> 3 (P0.3) 
// <4=> 4 (P0.4) 
// <5=> 5 (P0.5) 
// <6=> 6 (P0.6) 
// <7=> 7 (P0.7) 
// <8=> 8 (P0.8) 
// <9=> 9 (P0.9) 
// <10=> 10 (P0.10) 
// <11=> 11 (P0.11) 
// <12=> 12 (P0.12) 
// <13=> 13 (P0.13) 
// <14=> 14 (P0.14) 
// <15=> 15 (P0.15) 
// <16=> 16 (P0.16) 
// <17=> 17 (P0.17) 
// <18=> 18 (P0.18) 
// <19=> 19 (P0.19) 
// <20=> 20 (P0.20) 
// <21=> 21 (P0.21) 
// <22=> 22 (P0.22) 
// <23=> 23 (P0.23) 
// <24=> 24 (P0.24) 
// <25=> 25 (P0.25) 
// <26=> 26 (P0.26) 
// <27=> 27 (P0.27) 
// <28=> 28 (P0.28) 
// <29=> 29 (P0.29) 
// <30=> 30 (P0.30) 
// <31=> 31 (P0.31) 
// <4294967295=> Not connected 

#ifndef NRF_PWR_MGMT_SLEEP_DEBUG_PIN
#define NRF_PWR_MGMT_SLEEP_DEBUG_PIN 31
#endif

// </e>

// <q> NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED  - Enables CPU usage monitor.
 

// <i> Module will trace percentage of CPU usage in one second intervals.

#ifndef NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED
#define NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED 0
#endif

// <e> NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED - Enable standby timeout.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S - Standby timeout (in seconds). 
// <i> Shutdown procedure will begin no earlier than after this number of seconds.

#ifndef NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S 3
#endif

// </e>

// <q> NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED  - Enables FPU event cleaning.
 

#ifndef NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED
#define NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED 1
#endif

// <q> NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY  - Blocked shutdown procedure will be retried every second.
 

#ifndef NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY
#define NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY 1
#endif

// <q> NRF_PWR_MGMT_CONFIG_USE_SCHEDULER  - Module will use @ref app_scheduler.
 

#ifndef NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
#define NRF_PWR_MGMT_CONFIG_USE_SCHEDULER 0
#endif

// <o> NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT - The number of priorities for module handlers. 
// <i> The number of stages of the shutdown process.

#ifndef NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT
#define NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT 3
#endif

// </e>

// <q> NRF_SECTION_ITER_ENABLED  - nrf_section_iter - Section iterator
 

#ifndef NRF_SECTION_ITER_ENABLED
#define NRF_SECTION_ITER_ENABLED 1
#endif

// <q> NRF_STRERROR_ENABLED  - nrf_strerror - Library for converting error code to string.
 

#ifndef NRF_STRERROR_ENABLED
#define NRF_STRERROR_ENABLED 1
#endif

// </h> 
//==========================================================

// <h> nRF_Log 

//==========================================================
// <e> NRF_LOG_BACKEND_RTT_ENABLED - nrf_log_backend_rtt - Log RTT backend
//==========================================================
#ifndef NRF_LOG_BACKEND_RTT_ENABLED
#define NRF_LOG_BACKEND_RTT_ENABLED 0
#endif
// <o> NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE - Size of buffer for partially processed strings. 
// <i> Size of the buffer is a trade-off between RAM usage and processing.
// <i> if buffer is smaller then strings will often be fragmented.
// <i> It is recommended to use size which will fit typical log and only the
// <i> longer one will be fragmented.

#ifndef NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE
#define NRF_LOG_BACKEND_RTT_TEMP_BUFFER_SIZE 64
#endif

// </e>

// <e> NRF_LOG_BACKEND_UART_ENABLED - nrf_log_backend_uart - Log UART backend
//==========================================================
#ifndef NRF_LOG_BACKEND_UART_ENABLED
#define NRF_LOG_BACKEND_UART_ENABLED 1
#endif
// <o> NRF_LOG_BACKEND_UART_TX_PIN - UART TX pin 
#ifndef NRF_LOG_BACKEND_UART_TX_PIN
#define NRF_LOG_BACKEND_UART_TX_PIN 6
#endif

// <o> NRF_LOG_BACKEND_UART_BAUDRATE  - Default Baudrate
 
// <323584=> 1200 baud 
// <643072=> 2400 baud 
// <1290240=> 4800 baud 
// <2576384=> 9600 baud 
// <3862528=> 14400 baud 
// <5152768=> 19200 baud 
// <7716864=> 28800 baud 
// <10289152=> 38400 baud 
// <15400960=> 57600 baud 
// <20615168=> 76800 baud 
// <30801920=> 115200 baud 
// <61865984=> 230400 baud 
// <67108864=> 250000 baud 
// <121634816=> 460800 baud 
// <251658240=> 921600 baud 
// <268435456=> 1000000 baud 

#ifndef NRF_LOG_BACKEND_UART_BAUDRATE
#define NRF_LOG_BACKEND_UART_BAUDRATE 30801920
#endif

// <o> NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE - Size of buffer for partially processed strings. 
// <i> Size of the buffer is a trade-off between RAM usage and processing.
// <i> if buffer is smaller then strings will often be fragmented.
// <i> It is recommended to use size which will fit typical log and only the
// <i> longer one will be fragmented.

#ifndef NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE
#define NRF_LOG_BACKEND_UART_TEMP_BUFFER_SIZE 64
#endif

// </e>

// <h> nrf_log - Logger

//==========================================================
// <e> NRF_LOG_ENABLED - Logging module for nRF5 SDK
//==========================================================
#ifndef NRF_LOG_ENABLED
#define NRF_LOG_ENABLED 1
#endif
// <e> NRF_LOG_USES_COLORS - If enabled then ANSI escape code for colors is prefixed to every string
//==========================================================
#ifndef NRF_LOG_USES_COLORS
#define NRF_LOG_USES_COLORS 0
#endif
// <o> NRF_LOG_COLOR_DEFAULT  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_LOG_COLOR_DEFAULT
#define NRF_LOG_COLOR_DEFAULT 0
#endif

// <o> NRF_LOG_ERROR_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_LOG_ERROR_COLOR
#define NRF_LOG_ERROR_COLOR 2
#endif

// <o> NRF_LOG_WARNING_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_LOG_WARNING_COLOR
#define NRF_LOG_WARNING_COLOR 4
#endif

// </e>

// <o> NRF_LOG_DEFAULT_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_LOG_DEFAULT_LEVEL
#define NRF_LOG_DEFAULT_LEVEL 3
#endif

// <q> NRF_LOG_DEFERRED  - Enable deffered logger.
 

// <i> Log data is buffered and can be processed in idle.

#ifndef NRF_LOG_DEFERRED
#define NRF_LOG_DEFERRED 1
#endif

// <o> NRF_LOG_BUFSIZE  - Size of the buffer for storing logs (in bytes).
 

// <i> Must be power of 2 and multiple of 4.
// <i> If NRF_LOG_DEFERRED = 0 then buffer size can be reduced to minimum.
// <128=> 128 
// <256=> 256 
// <512=> 512 
// <1024=> 1024 
// <2048=> 2048 
// <4096=> 4096 
// <8192=> 8192 
// <16384=> 16384 

#ifndef NRF_LOG_BUFSIZE
#define NRF_LOG_BUFSIZE 1024
#endif

// <q> NRF_LOG_ALLOW_OVERFLOW  - Configures behavior when circular buffer is full.
 

// <i> If set then oldest logs are overwritten. Otherwise a 
// <i> marker is injected informing about overflow.

#ifndef NRF_LOG_ALLOW_OVERFLOW
#define NRF_LOG_ALLOW_OVERFLOW 1
#endif

// <q> NRF_LOG_USES_TIMESTAMP  - Enable timestamping
 

// <i> Function for getting the timestamp is provided by the user

#ifndef NRF_LOG_USES_TIMESTAMP
#define NRF_LOG_USES_TIMESTAMP 0
#endif

// <q> NRF_LOG_FILTERS_ENABLED  - Enable dynamic filtering of logs.
 

#ifndef NRF_LOG_FILTERS_ENABLED
#define NRF_LOG_FILTERS_ENABLED 0
#endif

// <q> NRF_LOG_CLI_CMDS  - Enable CLI commands for the module.
 

#ifndef NRF_LOG_CLI_CMDS
#define NRF_LOG_CLI_CMDS 0
#endif

// <h> Log message pool - Configuration of log message pool

//==========================================================
// <o> NRF_LOG_MSGPOOL_ELEMENT_SIZE - Size of a single element in the pool of memory objects. 
// <i> If a small value is set, then performance of logs processing
// <i> is degraded because data is fragmented. Bigger value impacts
// <i> RAM memory utilization. The size is set to fit a message with
// <i> a timestamp and up to 2 arguments in a single memory object.

#ifndef NRF_LOG_MSGPOOL_ELEMENT_SIZE
#define NRF_LOG_MSGPOOL_ELEMENT_SIZE 20
#endif

// <o> NRF_LOG_MSGPOOL_ELEMENT_COUNT - Number of elements in the pool of memory objects 
// <i> If a small value is set, then it may lead to a deadlock
// <i> in certain cases if backend has high latency and holds
// <i> multiple messages for long time. Bigger value impacts
// <i> RAM memory usage.

#ifndef NRF_LOG_MSGPOOL_ELEMENT_COUNT
#define NRF_LOG_MSGPOOL_ELEMENT_COUNT 8
#endif

// </h> 
//==========================================================

// </e>

// <h> nrf_log module configuration 

//==========================================================
// <h> nrf_log in nRF_Core 

//==========================================================
// <e> NRF_MPU_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_MPU_CONFIG_LOG_ENABLED
#define NRF_MPU_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_MPU_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_MPU_CONFIG_LOG_LEVEL
#define NRF_MPU_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_MPU_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_MPU_CONFIG_INFO_COLOR
#define NRF_MPU_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_MPU_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_MPU_CONFIG_DEBUG_COLOR
#define NRF_MPU_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_STACK_GUARD_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_STACK_GUARD_CONFIG_LOG_ENABLED
#define NRF_STACK_GUARD_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_STACK_GUARD_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_STACK_GUARD_CONFIG_LOG_LEVEL
#define NRF_STACK_GUARD_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_STACK_GUARD_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_STACK_GUARD_CONFIG_INFO_COLOR
#define NRF_STACK_GUARD_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_STACK_GUARD_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_STACK_GUARD_CONFIG_DEBUG_COLOR
#define NRF_STACK_GUARD_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TASK_MANAGER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TASK_MANAGER_CONFIG_LOG_ENABLED
#define TASK_MANAGER_CONFIG_LOG_ENABLED 0
#endif
// <o> TASK_MANAGER_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef TASK_MANAGER_CONFIG_LOG_LEVEL
#define TASK_MANAGER_CONFIG_LOG_LEVEL 3
#endif

// <o> TASK_MANAGER_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TASK_MANAGER_CONFIG_INFO_COLOR
#define TASK_MANAGER_CONFIG_INFO_COLOR 0
#endif

// <o> TASK_MANAGER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TASK_MANAGER_CONFIG_DEBUG_COLOR
#define TASK_MANAGER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </h> 
//==========================================================

// <h> nrf_log in nRF_Drivers 

//==========================================================
// <e> CLOCK_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef CLOCK_CONFIG_LOG_ENABLED
#define CLOCK_CONFIG_LOG_ENABLED 0
#endif
// <o> CLOCK_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef CLOCK_CONFIG_LOG_LEVEL
#define CLOCK_CONFIG_LOG_LEVEL 3
#endif

// <o> CLOCK_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef CLOCK_CONFIG_INFO_COLOR
#define CLOCK_CONFIG_INFO_COLOR 0
#endif

// <o> CLOCK_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef CLOCK_CONFIG_DEBUG_COLOR
#define CLOCK_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> COMMON_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef COMMON_CONFIG_LOG_ENABLED
#define COMMON_CONFIG_LOG_ENABLED 0
#endif
// <o> COMMON_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef COMMON_CONFIG_LOG_LEVEL
#define COMMON_CONFIG_LOG_LEVEL 3
#endif

// <o> COMMON_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef COMMON_CONFIG_INFO_COLOR
#define COMMON_CONFIG_INFO_COLOR 0
#endif

// <o> COMMON_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef COMMON_CONFIG_DEBUG_COLOR
#define COMMON_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> COMP_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef COMP_CONFIG_LOG_ENABLED
#define COMP_CONFIG_LOG_ENABLED 0
#endif
// <o> COMP_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef COMP_CONFIG_LOG_LEVEL
#define COMP_CONFIG_LOG_LEVEL 3
#endif

// <o> COMP_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef COMP_CONFIG_INFO_COLOR
#define COMP_CONFIG_INFO_COLOR 0
#endif

// <o> COMP_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef COMP_CONFIG_DEBUG_COLOR
#define COMP_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> GPIOTE_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef GPIOTE_CONFIG_LOG_ENABLED
#define GPIOTE_CONFIG_LOG_ENABLED 0
#endif
// <o> GPIOTE_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef GPIOTE_CONFIG_LOG_LEVEL
#define GPIOTE_CONFIG_LOG_LEVEL 3
#endif

// <o> GPIOTE_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef GPIOTE_CONFIG_INFO_COLOR
#define GPIOTE_CONFIG_INFO_COLOR 0
#endif

// <o> GPIOTE_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef GPIOTE_CONFIG_DEBUG_COLOR
#define GPIOTE_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> I2S_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef I2S_CONFIG_LOG_ENABLED
#define I2S_CONFIG_LOG_ENABLED 0
#endif
// <o> I2S_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef I2S_CONFIG_LOG_LEVEL
#define I2S_CONFIG_LOG_LEVEL 3
#endif

// <o> I2S_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef I2S_CONFIG_INFO_COLOR
#define I2S_CONFIG_INFO_COLOR 0
#endif

// <o> I2S_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef I2S_CONFIG_DEBUG_COLOR
#define I2S_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> LPCOMP_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef LPCOMP_CONFIG_LOG_ENABLED
#define LPCOMP_CONFIG_LOG_ENABLED 0
#endif
// <o> LPCOMP_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef LPCOMP_CONFIG_LOG_LEVEL
#define LPCOMP_CONFIG_LOG_LEVEL 3
#endif

// <o> LPCOMP_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef LPCOMP_CONFIG_INFO_COLOR
#define LPCOMP_CONFIG_INFO_COLOR 0
#endif

// <o> LPCOMP_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef LPCOMP_CONFIG_DEBUG_COLOR
#define LPCOMP_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PDM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef PDM_CONFIG_LOG_ENABLED
#define PDM_CONFIG_LOG_ENABLED 0
#endif
// <o> PDM_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef PDM_CONFIG_LOG_LEVEL
#define PDM_CONFIG_LOG_LEVEL 3
#endif

// <o> PDM_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef PDM_CONFIG_INFO_COLOR
#define PDM_CONFIG_INFO_COLOR 0
#endif

// <o> PDM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef PDM_CONFIG_DEBUG_COLOR
#define PDM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PPI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef PPI_CONFIG_LOG_ENABLED
#define PPI_CONFIG_LOG_ENABLED 0
#endif
// <o> PPI_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef PPI_CONFIG_LOG_LEVEL
#define PPI_CONFIG_LOG_LEVEL 3
#endif

// <o> PPI_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef PPI_CONFIG_INFO_COLOR
#define PPI_CONFIG_INFO_COLOR 0
#endif

// <o> PPI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef PPI_CONFIG_DEBUG_COLOR
#define PPI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> PWM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef PWM_CONFIG_LOG_ENABLED
#define PWM_CONFIG_LOG_ENABLED 0
#endif
// <o> PWM_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef PWM_CONFIG_LOG_LEVEL
#define PWM_CONFIG_LOG_LEVEL 3
#endif

// <o> PWM_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef PWM_CONFIG_INFO_COLOR
#define PWM_CONFIG_INFO_COLOR 0
#endif

// <o> PWM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef PWM_CONFIG_DEBUG_COLOR
#define PWM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> QDEC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef QDEC_CONFIG_LOG_ENABLED
#define QDEC_CONFIG_LOG_ENABLED 0
#endif
// <o> QDEC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef QDEC_CONFIG_LOG_LEVEL
#define QDEC_CONFIG_LOG_LEVEL 3
#endif

// <o> QDEC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef QDEC_CONFIG_INFO_COLOR
#define QDEC_CONFIG_INFO_COLOR 0
#endif

// <o> QDEC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef QDEC_CONFIG_DEBUG_COLOR
#define QDEC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> RNG_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef RNG_CONFIG_LOG_ENABLED
#define RNG_CONFIG_LOG_ENABLED 0
#endif
// <o> RNG_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef RNG_CONFIG_LOG_LEVEL
#define RNG_CONFIG_LOG_LEVEL 3
#endif

// <o> RNG_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef RNG_CONFIG_INFO_COLOR
#define RNG_CONFIG_INFO_COLOR 0
#endif

// <o> RNG_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef RNG_CONFIG_DEBUG_COLOR
#define RNG_CONFIG_DEBUG_COLOR 0
#endif

// <q> RNG_CONFIG_RANDOM_NUMBER_LOG_ENABLED  - Enables logging of random numbers.
 

#ifndef RNG_CONFIG_RANDOM_NUMBER_LOG_ENABLED
#define RNG_CONFIG_RANDOM_NUMBER_LOG_ENABLED 0
#endif

// </e>

// <e> RTC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef RTC_CONFIG_LOG_ENABLED
#define RTC_CONFIG_LOG_ENABLED 0
#endif
// <o> RTC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef RTC_CONFIG_LOG_LEVEL
#define RTC_CONFIG_LOG_LEVEL 3
#endif

// <o> RTC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef RTC_CONFIG_INFO_COLOR
#define RTC_CONFIG_INFO_COLOR 0
#endif

// <o> RTC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef RTC_CONFIG_DEBUG_COLOR
#define RTC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> SAADC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SAADC_CONFIG_LOG_ENABLED
#define SAADC_CONFIG_LOG_ENABLED 0
#endif
// <o> SAADC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef SAADC_CONFIG_LOG_LEVEL
#define SAADC_CONFIG_LOG_LEVEL 3
#endif

// <o> SAADC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SAADC_CONFIG_INFO_COLOR
#define SAADC_CONFIG_INFO_COLOR 0
#endif

// <o> SAADC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SAADC_CONFIG_DEBUG_COLOR
#define SAADC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> SPIS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SPIS_CONFIG_LOG_ENABLED
#define SPIS_CONFIG_LOG_ENABLED 0
#endif
// <o> SPIS_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef SPIS_CONFIG_LOG_LEVEL
#define SPIS_CONFIG_LOG_LEVEL 3
#endif

// <o> SPIS_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SPIS_CONFIG_INFO_COLOR
#define SPIS_CONFIG_INFO_COLOR 0
#endif

// <o> SPIS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SPIS_CONFIG_DEBUG_COLOR
#define SPIS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> SPI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SPI_CONFIG_LOG_ENABLED
#define SPI_CONFIG_LOG_ENABLED 0
#endif
// <o> SPI_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef SPI_CONFIG_LOG_LEVEL
#define SPI_CONFIG_LOG_LEVEL 3
#endif

// <o> SPI_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SPI_CONFIG_INFO_COLOR
#define SPI_CONFIG_INFO_COLOR 0
#endif

// <o> SPI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SPI_CONFIG_DEBUG_COLOR
#define SPI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> SWI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef SWI_CONFIG_LOG_ENABLED
#define SWI_CONFIG_LOG_ENABLED 0
#endif
// <o> SWI_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef SWI_CONFIG_LOG_LEVEL
#define SWI_CONFIG_LOG_LEVEL 3
#endif

// <o> SWI_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SWI_CONFIG_INFO_COLOR
#define SWI_CONFIG_INFO_COLOR 0
#endif

// <o> SWI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef SWI_CONFIG_DEBUG_COLOR
#define SWI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TIMER_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TIMER_CONFIG_LOG_ENABLED
#define TIMER_CONFIG_LOG_ENABLED 0
#endif
// <o> TIMER_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef TIMER_CONFIG_LOG_LEVEL
#define TIMER_CONFIG_LOG_LEVEL 3
#endif

// <o> TIMER_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TIMER_CONFIG_INFO_COLOR
#define TIMER_CONFIG_INFO_COLOR 0
#endif

// <o> TIMER_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TIMER_CONFIG_DEBUG_COLOR
#define TIMER_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TWIS_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TWIS_CONFIG_LOG_ENABLED
#define TWIS_CONFIG_LOG_ENABLED 0
#endif
// <o> TWIS_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef TWIS_CONFIG_LOG_LEVEL
#define TWIS_CONFIG_LOG_LEVEL 3
#endif

// <o> TWIS_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TWIS_CONFIG_INFO_COLOR
#define TWIS_CONFIG_INFO_COLOR 0
#endif

// <o> TWIS_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TWIS_CONFIG_DEBUG_COLOR
#define TWIS_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> TWI_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef TWI_CONFIG_LOG_ENABLED
#define TWI_CONFIG_LOG_ENABLED 0
#endif
// <o> TWI_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef TWI_CONFIG_LOG_LEVEL
#define TWI_CONFIG_LOG_LEVEL 3
#endif

// <o> TWI_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TWI_CONFIG_INFO_COLOR
#define TWI_CONFIG_INFO_COLOR 0
#endif

// <o> TWI_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef TWI_CONFIG_DEBUG_COLOR
#define TWI_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef UART_CONFIG_LOG_ENABLED
#define UART_CONFIG_LOG_ENABLED 0
#endif
// <o> UART_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef UART_CONFIG_LOG_LEVEL
#define UART_CONFIG_LOG_LEVEL 3
#endif

// <o> UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef UART_CONFIG_INFO_COLOR
#define UART_CONFIG_INFO_COLOR 0
#endif

// <o> UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef UART_CONFIG_DEBUG_COLOR
#define UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> USBD_CONFIG_LOG_ENABLED - Enable logging in the module
//==========================================================
#ifndef USBD_CONFIG_LOG_ENABLED
#define USBD_CONFIG_LOG_ENABLED 0
#endif
// <o> USBD_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef USBD_CONFIG_LOG_LEVEL
#define USBD_CONFIG_LOG_LEVEL 3
#endif

// <o> USBD_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef USBD_CONFIG_INFO_COLOR
#define USBD_CONFIG_INFO_COLOR 0
#endif

// <o> USBD_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef USBD_CONFIG_DEBUG_COLOR
#define USBD_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> WDT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef WDT_CONFIG_LOG_ENABLED
#define WDT_CONFIG_LOG_ENABLED 0
#endif
// <o> WDT_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef WDT_CONFIG_LOG_LEVEL
#define WDT_CONFIG_LOG_LEVEL 3
#endif

// <o> WDT_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef WDT_CONFIG_INFO_COLOR
#define WDT_CONFIG_INFO_COLOR 0
#endif

// <o> WDT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef WDT_CONFIG_DEBUG_COLOR
#define WDT_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </h> 
//==========================================================

// <h> nrf_log in nRF_Libraries 

//==========================================================
// <e> APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED
#define APP_USBD_CDC_ACM_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_CDC_ACM_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef APP_USBD_CDC_ACM_CONFIG_LOG_LEVEL
#define APP_USBD_CDC_ACM_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_CDC_ACM_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef APP_USBD_CDC_ACM_CONFIG_INFO_COLOR
#define APP_USBD_CDC_ACM_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_CDC_ACM_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef APP_USBD_CDC_ACM_CONFIG_DEBUG_COLOR
#define APP_USBD_CDC_ACM_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> APP_USBD_MSC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef APP_USBD_MSC_CONFIG_LOG_ENABLED
#define APP_USBD_MSC_CONFIG_LOG_ENABLED 0
#endif
// <o> APP_USBD_MSC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef APP_USBD_MSC_CONFIG_LOG_LEVEL
#define APP_USBD_MSC_CONFIG_LOG_LEVEL 3
#endif

// <o> APP_USBD_MSC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef APP_USBD_MSC_CONFIG_INFO_COLOR
#define APP_USBD_MSC_CONFIG_INFO_COLOR 0
#endif

// <o> APP_USBD_MSC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef APP_USBD_MSC_CONFIG_DEBUG_COLOR
#define APP_USBD_MSC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_BALLOC_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_BALLOC_CONFIG_LOG_ENABLED
#define NRF_BALLOC_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_BALLOC_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_BALLOC_CONFIG_LOG_LEVEL
#define NRF_BALLOC_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_BALLOC_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_BALLOC_CONFIG_INFO_COLOR
#define NRF_BALLOC_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_BALLOC_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_BALLOC_CONFIG_DEBUG_COLOR
#define NRF_BALLOC_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_CLI_BLE_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_CLI_BLE_UART_CONFIG_LOG_ENABLED
#define NRF_CLI_BLE_UART_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_CLI_BLE_UART_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_CLI_BLE_UART_CONFIG_LOG_LEVEL
#define NRF_CLI_BLE_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_CLI_BLE_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_CLI_BLE_UART_CONFIG_INFO_COLOR
#define NRF_CLI_BLE_UART_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_CLI_BLE_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_CLI_BLE_UART_CONFIG_DEBUG_COLOR
#define NRF_CLI_BLE_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_CLI_UART_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_CLI_UART_CONFIG_LOG_ENABLED
#define NRF_CLI_UART_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_CLI_UART_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_CLI_UART_CONFIG_LOG_LEVEL
#define NRF_CLI_UART_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_CLI_UART_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_CLI_UART_CONFIG_INFO_COLOR
#define NRF_CLI_UART_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_CLI_UART_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_CLI_UART_CONFIG_DEBUG_COLOR
#define NRF_CLI_UART_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_MEMOBJ_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_MEMOBJ_CONFIG_LOG_ENABLED
#define NRF_MEMOBJ_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_MEMOBJ_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_MEMOBJ_CONFIG_LOG_LEVEL
#define NRF_MEMOBJ_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_MEMOBJ_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_MEMOBJ_CONFIG_INFO_COLOR
#define NRF_MEMOBJ_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_MEMOBJ_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_MEMOBJ_CONFIG_DEBUG_COLOR
#define NRF_MEMOBJ_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_PWR_MGMT_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_LOG_ENABLED
#define NRF_PWR_MGMT_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_PWR_MGMT_CONFIG_LOG_LEVEL
#define NRF_PWR_MGMT_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_PWR_MGMT_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_PWR_MGMT_CONFIG_INFO_COLOR
#define NRF_PWR_MGMT_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_PWR_MGMT_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_PWR_MGMT_CONFIG_DEBUG_COLOR
#define NRF_PWR_MGMT_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_ANT_LOG_ENABLED - Enable logging in SoftDevice handler (ANT) module.
//==========================================================
#ifndef NRF_SDH_ANT_LOG_ENABLED
#define NRF_SDH_ANT_LOG_ENABLED 0
#endif
// <o> NRF_SDH_ANT_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_SDH_ANT_LOG_LEVEL
#define NRF_SDH_ANT_LOG_LEVEL 3
#endif

// <o> NRF_SDH_ANT_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_ANT_INFO_COLOR
#define NRF_SDH_ANT_INFO_COLOR 0
#endif

// <o> NRF_SDH_ANT_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_ANT_DEBUG_COLOR
#define NRF_SDH_ANT_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_BLE_LOG_ENABLED - Enable logging in SoftDevice handler (BLE) module.
//==========================================================
#ifndef NRF_SDH_BLE_LOG_ENABLED
#define NRF_SDH_BLE_LOG_ENABLED 1
#endif
// <o> NRF_SDH_BLE_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_SDH_BLE_LOG_LEVEL
#define NRF_SDH_BLE_LOG_LEVEL 3
#endif

// <o> NRF_SDH_BLE_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_BLE_INFO_COLOR
#define NRF_SDH_BLE_INFO_COLOR 0
#endif

// <o> NRF_SDH_BLE_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_BLE_DEBUG_COLOR
#define NRF_SDH_BLE_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_LOG_ENABLED - Enable logging in SoftDevice handler module.
//==========================================================
#ifndef NRF_SDH_LOG_ENABLED
#define NRF_SDH_LOG_ENABLED 1
#endif
// <o> NRF_SDH_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_SDH_LOG_LEVEL
#define NRF_SDH_LOG_LEVEL 3
#endif

// <o> NRF_SDH_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_INFO_COLOR
#define NRF_SDH_INFO_COLOR 0
#endif

// <o> NRF_SDH_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_DEBUG_COLOR
#define NRF_SDH_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SDH_SOC_LOG_ENABLED - Enable logging in SoftDevice handler (SoC) module.
//==========================================================
#ifndef NRF_SDH_SOC_LOG_ENABLED
#define NRF_SDH_SOC_LOG_ENABLED 1
#endif
// <o> NRF_SDH_SOC_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_SDH_SOC_LOG_LEVEL
#define NRF_SDH_SOC_LOG_LEVEL 3
#endif

// <o> NRF_SDH_SOC_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_SOC_INFO_COLOR
#define NRF_SDH_SOC_INFO_COLOR 0
#endif

// <o> NRF_SDH_SOC_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SDH_SOC_DEBUG_COLOR
#define NRF_SDH_SOC_DEBUG_COLOR 0
#endif

// </e>

// <e> NRF_SORTLIST_CONFIG_LOG_ENABLED - Enables logging in the module.
//==========================================================
#ifndef NRF_SORTLIST_CONFIG_LOG_ENABLED
#define NRF_SORTLIST_CONFIG_LOG_ENABLED 0
#endif
// <o> NRF_SORTLIST_CONFIG_LOG_LEVEL  - Default Severity level
 
// <0=> Off 
// <1=> Error 
// <2=> Warning 
// <3=> Info 
// <4=> Debug 

#ifndef NRF_SORTLIST_CONFIG_LOG_LEVEL
#define NRF_SORTLIST_CONFIG_LOG_LEVEL 3
#endif

// <o> NRF_SORTLIST_CONFIG_INFO_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SORTLIST_CONFIG_INFO_COLOR
#define NRF_SORTLIST_CONFIG_INFO_COLOR 0
#endif

// <o> NRF_SORTLIST_CONFIG_DEBUG_COLOR  - ANSI escape code prefix.
 
// <0=> Default 
// <1=> Black 
// <2=> Red 
// <3=> Green 
// <4=> Yellow 
// <5=> Blue 
// <6=> Magenta 
// <7=> Cyan 
// <8=> White 

#ifndef NRF_SORTLIST_CONFIG_DEBUG_COLOR
#define NRF_SORTLIST_CONFIG_DEBUG_COLOR 0
#endif

// </e>

// </h> 
//==========================================================

// </h> 
//==========================================================

// </h> 
//==========================================================

// </h> 
//==========================================================

// <h> nRF_Segger_RTT 

//==========================================================
// <h> segger_rtt - SEGGER RTT

//==========================================================
// <o> SEGGER_RTT_CONFIG_BUFFER_SIZE_UP - Size of upstream buffer. 
// <i> Note that either @ref NRF_LOG_BACKEND_RTT_OUTPUT_BUFFER_SIZE
// <i> or this value is actually used. It depends on which one is bigger.

#ifndef SEGGER_RTT_CONFIG_BUFFER_SIZE_UP
#define SEGGER_RTT_CONFIG_BUFFER_SIZE_UP 512
#endif

// <o> SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS - Size of upstream buffer. 
#ifndef SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS
#define SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS 2
#endif

// <o> SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN - Size of upstream buffer. 
#ifndef SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN
#define SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN 16
#endif

// <o> SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS - Size of upstream buffer. 
#ifndef SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS
#define SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS 2
#endif

// <o> SEGGER_RTT_CONFIG_DEFAULT_MODE  - RTT behavior if the buffer is full.
 

// <i> The following modes are supported:
// <i> - SKIP  - Do not block, output nothing.
// <i> - TRIM  - Do not block, output as much as fits.
// <i> - BLOCK - Wait until there is space in the buffer.
// <0=> SKIP 
// <1=> TRIM 
// <2=> BLOCK_IF_FIFO_FULL 

#ifndef SEGGER_RTT_CONFIG_DEFAULT_MODE
#define SEGGER_RTT_CONFIG_DEFAULT_MODE 0
#endif

// </h> 
//==========================================================

// </h> 
//==========================================================

// <h> nRF_SoftDevice 

//==========================================================
// <e> NRF_SDH_BLE_ENABLED - nrf_sdh_ble - SoftDevice BLE event handler
//==========================================================
#ifndef NRF_SDH_BLE_ENABLED
#define NRF_SDH_BLE_ENABLED 1
#endif
// <h> BLE Stack configuration - Stack configuration parameters

// <i> These values are not used directly by the SoftDevice handler but the application or other libraries might depend on them.
// <i> Keep them up-to-date with the desired configuration.
//==========================================================
// <o> NRF_SDH_BLE_PERIPHERAL_LINK_COUNT - Maximum number of peripheral links. 
#ifndef NRF_SDH_BLE_PERIPHERAL_LINK_COUNT
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 1
#endif

// <o> NRF_SDH_BLE_CENTRAL_LINK_COUNT - Maximum number of central links. 
#ifndef NRF_SDH_BLE_CENTRAL_LINK_COUNT
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 0
#endif

// <o> NRF_SDH_BLE_TOTAL_LINK_COUNT - Maximum number of total concurrent connections using the default configuration. 
#ifndef NRF_SDH_BLE_TOTAL_LINK_COUNT
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1
#endif

// <o> NRF_SDH_BLE_GAP_EVENT_LENGTH - The time set aside for this connection on every connection interval in 1.25 ms units. 
#ifndef NRF_SDH_BLE_GAP_EVENT_LENGTH
#define NRF_SDH_BLE_GAP_EVENT_LENGTH 3
#endif

// <o> NRF_SDH_BLE_GATT_MAX_MTU_SIZE - Static maximum MTU size. 
#ifndef NRF_SDH_BLE_GATT_MAX_MTU_SIZE
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 23
#endif

// <o> NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE - Attribute Table size in bytes. The size must be a multiple of 4. 
#ifndef NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE
#define NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE 1408
#endif

// <o> NRF_SDH_BLE_VS_UUID_COUNT - The number of vendor-specific UUIDs. 
#ifndef NRF_SDH_BLE_VS_UUID_COUNT
#define NRF_SDH_BLE_VS_UUID_COUNT 1
#endif

// <q> NRF_SDH_BLE_SERVICE_CHANGED  - Include the Service Changed characteristic in the Attribute Table.
 

#ifndef NRF_SDH_BLE_SERVICE_CHANGED
#define NRF_SDH_BLE_SERVICE_CHANGED 1
#endif

// </h> 
//==========================================================

// <h> BLE Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_BLE_OBSERVER_PRIO_LEVELS - Total number of priority levels for BLE observers. 
// <i> This setting configures the number of priority levels available for BLE event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_BLE_OBSERVER_PRIO_LEVELS
#define NRF_SDH_BLE_OBSERVER_PRIO_LEVELS 4
#endif

// <h> BLE Observers priorities - Invididual priorities

//==========================================================
// <o> BLE_ADV_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Advertising module.

#ifndef BLE_ADV_BLE_OBSERVER_PRIO
#define BLE_ADV_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_ANCS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Apple Notification Service Client.

#ifndef BLE_ANCS_C_BLE_OBSERVER_PRIO
#define BLE_ANCS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_ANS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Alert Notification Service Client.

#ifndef BLE_ANS_C_BLE_OBSERVER_PRIO
#define BLE_ANS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BAS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Battery Service.

#ifndef BLE_BAS_BLE_OBSERVER_PRIO
#define BLE_BAS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BAS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Battery Service Client.

#ifndef BLE_BAS_C_BLE_OBSERVER_PRIO
#define BLE_BAS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BPS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Blood Pressure Service.

#ifndef BLE_BPS_BLE_OBSERVER_PRIO
#define BLE_BPS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CONN_PARAMS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Connection parameters module.

#ifndef BLE_CONN_PARAMS_BLE_OBSERVER_PRIO
#define BLE_CONN_PARAMS_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_CONN_STATE_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Connection State module.

#ifndef BLE_CONN_STATE_BLE_OBSERVER_PRIO
#define BLE_CONN_STATE_BLE_OBSERVER_PRIO 0
#endif

// <o> BLE_CSCS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Cycling Speed and Cadence Service.

#ifndef BLE_CSCS_BLE_OBSERVER_PRIO
#define BLE_CSCS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CTS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Current Time Service Client.

#ifndef BLE_CTS_C_BLE_OBSERVER_PRIO
#define BLE_CTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_DB_DISC_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Database Discovery module.

#ifndef BLE_DB_DISC_BLE_OBSERVER_PRIO
#define BLE_DB_DISC_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_DFU_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the DFU Service.

#ifndef BLE_DFU_BLE_OBSERVER_PRIO
#define BLE_DFU_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_GLS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Glucose Service.

#ifndef BLE_GLS_BLE_OBSERVER_PRIO
#define BLE_GLS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HIDS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Human Interface Device Service.

#ifndef BLE_HIDS_BLE_OBSERVER_PRIO
#define BLE_HIDS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HRS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Heart Rate Service.

#ifndef BLE_HRS_BLE_OBSERVER_PRIO
#define BLE_HRS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HRS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Heart Rate Service Client.

#ifndef BLE_HRS_C_BLE_OBSERVER_PRIO
#define BLE_HRS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HTS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Health Thermometer Service.

#ifndef BLE_HTS_BLE_OBSERVER_PRIO
#define BLE_HTS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_IAS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Immediate Alert Service.

#ifndef BLE_IAS_BLE_OBSERVER_PRIO
#define BLE_IAS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_IAS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Immediate Alert Service Client.

#ifndef BLE_IAS_C_BLE_OBSERVER_PRIO
#define BLE_IAS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LBS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the LED Button Service.

#ifndef BLE_LBS_BLE_OBSERVER_PRIO
#define BLE_LBS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LBS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the LED Button Service Client.

#ifndef BLE_LBS_C_BLE_OBSERVER_PRIO
#define BLE_LBS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LLS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Link Loss Service.

#ifndef BLE_LLS_BLE_OBSERVER_PRIO
#define BLE_LLS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LNS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Location Navigation Service.

#ifndef BLE_LNS_BLE_OBSERVER_PRIO
#define BLE_LNS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_NUS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the UART Service.

#ifndef BLE_NUS_BLE_OBSERVER_PRIO
#define BLE_NUS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_NUS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the UART Central Service.

#ifndef BLE_NUS_C_BLE_OBSERVER_PRIO
#define BLE_NUS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_OTS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Object transfer service.

#ifndef BLE_OTS_BLE_OBSERVER_PRIO
#define BLE_OTS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_OTS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Object transfer service client.

#ifndef BLE_OTS_C_BLE_OBSERVER_PRIO
#define BLE_OTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_RSCS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Running Speed and Cadence Service.

#ifndef BLE_RSCS_BLE_OBSERVER_PRIO
#define BLE_RSCS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_RSCS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Running Speed and Cadence Client.

#ifndef BLE_RSCS_C_BLE_OBSERVER_PRIO
#define BLE_RSCS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_TPS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the TX Power Service.

#ifndef BLE_TPS_BLE_OBSERVER_PRIO
#define BLE_TPS_BLE_OBSERVER_PRIO 2
#endif

// <o> BSP_BTN_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Button Control module.

#ifndef BSP_BTN_BLE_OBSERVER_PRIO
#define BSP_BTN_BLE_OBSERVER_PRIO 1
#endif

// <o> NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the NFC pairing library.

#ifndef NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
#define NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO 1
#endif

// <o> NRF_BLE_BMS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Bond Management Service.

#ifndef NRF_BLE_BMS_BLE_OBSERVER_PRIO
#define NRF_BLE_BMS_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_CGMS_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Contiuon Glucose Monitoring Service.

#ifndef NRF_BLE_CGMS_BLE_OBSERVER_PRIO
#define NRF_BLE_CGMS_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_ES_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Eddystone module.

#ifndef NRF_BLE_ES_BLE_OBSERVER_PRIO
#define NRF_BLE_ES_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the GATT Service Client.

#ifndef NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO
#define NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_GATT_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the GATT module.

#ifndef NRF_BLE_GATT_BLE_OBSERVER_PRIO
#define NRF_BLE_GATT_BLE_OBSERVER_PRIO 1
#endif

// <o> NRF_BLE_QWR_BLE_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the Queued writes module.

#ifndef NRF_BLE_QWR_BLE_OBSERVER_PRIO
#define NRF_BLE_QWR_BLE_OBSERVER_PRIO 2
#endif

// <o> PM_BLE_OBSERVER_PRIO - Priority with which BLE events are dispatched to the Peer Manager module. 
#ifndef PM_BLE_OBSERVER_PRIO
#define PM_BLE_OBSERVER_PRIO 1
#endif

// </h> 
//==========================================================

// </h> 
//==========================================================


// </e>

// <e> NRF_SDH_ENABLED - nrf_sdh - SoftDevice handler
//==========================================================
#ifndef NRF_SDH_ENABLED
#define NRF_SDH_ENABLED 1
#endif
// <h> Dispatch model 

// <i> This setting configures how Stack events are dispatched to the application.
//==========================================================
// <o> NRF_SDH_DISPATCH_MODEL
 

// <i> NRF_SDH_DISPATCH_MODEL_INTERRUPT: SoftDevice events are passed to the application from the interrupt context.
// <i> NRF_SDH_DISPATCH_MODEL_APPSH: SoftDevice events are scheduled using @ref app_scheduler.
// <i> NRF_SDH_DISPATCH_MODEL_POLLING: SoftDevice events are to be fetched manually.
// <0=> NRF_SDH_DISPATCH_MODEL_INTERRUPT 
// <1=> NRF_SDH_DISPATCH_MODEL_APPSH 
// <2=> NRF_SDH_DISPATCH_MODEL_POLLING 

#ifndef NRF_SDH_DISPATCH_MODEL
#define NRF_SDH_DISPATCH_MODEL 0
#endif

// </h> 
//==========================================================

// <h> Clock - SoftDevice clock configuration

//==========================================================
// <o> NRF_SDH_CLOCK_LF_SRC  - SoftDevice clock source.
 
// <0=> NRF_CLOCK_LF_SRC_RC 
// <1=> NRF_CLOCK_LF_SRC_XTAL 
// <2=> NRF_CLOCK_LF_SRC_SYNTH 

#ifndef NRF_SDH_CLOCK_LF_SRC
#define NRF_SDH_CLOCK_LF_SRC 1
#endif

// <o> NRF_SDH_CLOCK_LF_RC_CTIV - SoftDevice calibration timer interval. 
#ifndef NRF_SDH_CLOCK_LF_RC_CTIV
#define NRF_SDH_CLOCK_LF_RC_CTIV 0
#endif

// <o> NRF_SDH_CLOCK_LF_RC_TEMP_CTIV - SoftDevice calibration timer interval under constant temperature. 
// <i> How often (in number of calibration intervals) the RC oscillator shall be calibrated
// <i>  if the temperature has not changed.

#ifndef NRF_SDH_CLOCK_LF_RC_TEMP_CTIV
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV 0
#endif

// <o> NRF_SDH_CLOCK_LF_XTAL_ACCURACY  - External crystal clock accuracy used in the LL to compute timing windows.
 
// <0=> NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM 
// <1=> NRF_CLOCK_LF_XTAL_ACCURACY_500_PPM 
// <2=> NRF_CLOCK_LF_XTAL_ACCURACY_150_PPM 
// <3=> NRF_CLOCK_LF_XTAL_ACCURACY_100_PPM 
// <4=> NRF_CLOCK_LF_XTAL_ACCURACY_75_PPM 
// <5=> NRF_CLOCK_LF_XTAL_ACCURACY_50_PPM 
// <6=> NRF_CLOCK_LF_XTAL_ACCURACY_30_PPM 
// <7=> NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM 

#ifndef NRF_SDH_CLOCK_LF_XTAL_ACCURACY
#define NRF_SDH_CLOCK_LF_XTAL_ACCURACY 7
#endif

// </h> 
//==========================================================

// <h> SDH Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_REQ_OBSERVER_PRIO_LEVELS - Total number of priority levels for request observers. 
// <i> This setting configures the number of priority levels available for the SoftDevice request event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_REQ_OBSERVER_PRIO_LEVELS
#define NRF_SDH_REQ_OBSERVER_PRIO_LEVELS 2
#endif

// <o> NRF_SDH_STATE_OBSERVER_PRIO_LEVELS - Total number of priority levels for state observers. 
// <i> This setting configures the number of priority levels available for the SoftDevice state event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_STATE_OBSERVER_PRIO_LEVELS
#define NRF_SDH_STATE_OBSERVER_PRIO_LEVELS 2
#endif

// <o> NRF_SDH_STACK_OBSERVER_PRIO_LEVELS - Total number of priority levels for stack event observers. 
// <i> This setting configures the number of priority levels available for the SoftDevice stack event handlers (ANT, BLE, SoC).
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_STACK_OBSERVER_PRIO_LEVELS
#define NRF_SDH_STACK_OBSERVER_PRIO_LEVELS 2
#endif


// <h> State Observers priorities - Invididual priorities

//==========================================================
// <o> CLOCK_CONFIG_STATE_OBSERVER_PRIO  
// <i> Priority with which state events are dispatched to the Clock driver.

#ifndef CLOCK_CONFIG_STATE_OBSERVER_PRIO
#define CLOCK_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// <o> POWER_CONFIG_STATE_OBSERVER_PRIO  
// <i> Priority with which state events are dispatched to the Power driver.

#ifndef POWER_CONFIG_STATE_OBSERVER_PRIO
#define POWER_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// <o> RNG_CONFIG_STATE_OBSERVER_PRIO  
// <i> Priority with which state events are dispatched to this module.

#ifndef RNG_CONFIG_STATE_OBSERVER_PRIO
#define RNG_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// </h> 
//==========================================================

// <h> Stack Event Observers priorities - Invididual priorities

//==========================================================
// <o> NRF_SDH_ANT_STACK_OBSERVER_PRIO  
// <i> This setting configures the priority with which ANT events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have ANT events dispatched before or after other stack events, such as BLE or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_ANT_STACK_OBSERVER_PRIO
#define NRF_SDH_ANT_STACK_OBSERVER_PRIO 0
#endif

// <o> NRF_SDH_BLE_STACK_OBSERVER_PRIO  
// <i> This setting configures the priority with which BLE events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have BLE events dispatched before or after other stack events, such as ANT or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_BLE_STACK_OBSERVER_PRIO
#define NRF_SDH_BLE_STACK_OBSERVER_PRIO 0
#endif

// <o> NRF_SDH_SOC_STACK_OBSERVER_PRIO  
// <i> This setting configures the priority with which SoC events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have SoC events dispatched before or after other stack events, such as ANT or BLE.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_SOC_STACK_OBSERVER_PRIO
#define NRF_SDH_SOC_STACK_OBSERVER_PRIO 0
#endif

// </h> 
//==========================================================

// </h> 
//==========================================================


// </e>

// <e> NRF_SDH_SOC_ENABLED - nrf_sdh_soc - SoftDevice SoC event handler
//==========================================================
#ifndef NRF_SDH_SOC_ENABLED
#define NRF_SDH_SOC_ENABLED 1
#endif
// <h> SoC Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_SOC_OBSERVER_PRIO_LEVELS - Total number of priority levels for SoC observers. 
// <i> This setting configures the number of priority levels available for the SoC event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_SOC_OBSERVER_PRIO_LEVELS
#define NRF_SDH_SOC_OBSERVER_PRIO_LEVELS 2
#endif

// <h> SoC Observers priorities - Invididual priorities

//==========================================================
// <o> BLE_ADV_SOC_OBSERVER_PRIO  
// <i> Priority with which SoC events are dispatched to the Advertising module.

#ifndef BLE_ADV_SOC_OBSERVER_PRIO
#define BLE_ADV_SOC_OBSERVER_PRIO 1
#endif

// <o> BLE_DFU_SOC_OBSERVER_PRIO  
// <i> Priority with which BLE events are dispatched to the DFU Service.

#ifndef BLE_DFU_SOC_OBSERVER_PRIO
#define BLE_DFU_SOC_OBSERVER_PRIO 1
#endif

// <o> CLOCK_CONFIG_SOC_OBSERVER_PRIO  
// <i> Priority with which SoC events are dispatched to the Clock driver.

#ifndef CLOCK_CONFIG_SOC_OBSERVER_PRIO
#define CLOCK_CONFIG_SOC_OBSERVER_PRIO 0
#endif

// <o> POWER_CONFIG_SOC_OBSERVER_PRIO  
// <i> Priority with which SoC events are dispatched to the Power driver.

#ifndef POWER_CONFIG_SOC_OBSERVER_PRIO
#define POWER_CONFIG_SOC_OBSERVER_PRIO 0
#endif

// </h> 
//==========================================================

// </h> 
//==========================================================


// </e>

// </h> 
//==========================================================

// <<< end of configuration section >>>
#endif //SDK_CONFIG_H

