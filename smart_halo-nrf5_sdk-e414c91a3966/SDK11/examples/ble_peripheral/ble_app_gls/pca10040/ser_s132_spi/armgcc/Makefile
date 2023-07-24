PROJECT_NAME := ble_app_gls_s132_spi_pca10040

export OUTPUT_FILENAME
#MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
MAKEFILE_NAME := $(MAKEFILE_LIST)
MAKEFILE_DIR := $(dir $(MAKEFILE_NAME) ) 

TEMPLATE_PATH = ../../../../../../components/toolchain/gcc
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
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/app_ble_gap_sec_keys.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/app_ble_user_mem.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/middleware/app_mw_ble.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/middleware/app_mw_ble_gap.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/middleware/app_mw_ble_gattc.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/middleware/app_mw_ble_gatts.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/middleware/app_mw_ble_l2cap.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/middleware/app_mw_nrf_soc.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_enable.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_event.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_evt_tx_complete.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_evt_user_mem_release.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_evt_user_mem_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_address_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_address_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_adv_data_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_adv_start.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_adv_stop.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_appearance_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_appearance_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_auth_key_reply.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_authenticate.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_conn_param_update.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_conn_sec_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_connect.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_connect_cancel.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_device_name_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_device_name_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_disconnect.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_encrypt.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_adv_report.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_auth_key_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_auth_status.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_conn_param_update.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_conn_param_update_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_conn_sec_update.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_connected.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_disconnected.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_key_pressed.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_lesc_dhkey_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_passkey_display.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_rssi_changed.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_scan_req_report.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_sec_info_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_sec_params_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_sec_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_evt_timeout.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_keypress_notify.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_lesc_dhkey_reply.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_lesc_oob_data_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_lesc_oob_data_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_ppcp_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_ppcp_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_rssi_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_rssi_start.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_rssi_stop.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_scan_start.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_scan_stop.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_sec_info_reply.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_sec_params_reply.c) \
$(abspath ../../../../../../components/serialization/common/struct_ser/s130/ble_gap_struct_serialization.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gap_tx_power_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_attr_info_discover.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_char_value_by_uuid_read.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_char_values_read.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_characteristics_discover.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_descriptors_discover.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_attr_info_disc_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_char_disc_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_char_val_by_uuid_read_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_char_vals_read_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_desc_disc_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_hvx.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_prim_srvc_disc_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_read_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_rel_disc_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_timeout.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_evt_write_rsp.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_hv_confirm.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_primary_services_discover.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_read.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_relationships_discover.c) \
$(abspath ../../../../../../components/serialization/common/struct_ser/s130/ble_gattc_struct_serialization.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gattc_write.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_attr_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_characteristic_add.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_descriptor_add.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_evt_hvc.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_evt_rw_authorize_request.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_evt_sc_confirm.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_evt_sys_attr_missing.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_evt_timeout.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_evt_write.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_hvx.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_include_add.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_initial_user_handle_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_rw_authorize_reply.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_service_add.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_service_changed.c) \
$(abspath ../../../../../../components/serialization/common/struct_ser/s130/ble_gatts_struct_serialization.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_sys_attr_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_sys_attr_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_value_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_gatts_value_set.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_l2cap_cid_register.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_l2cap_cid_unregister.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_l2cap_evt_rx.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_l2cap_tx.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_opt_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_opt_set.c) \
$(abspath ../../../../../../components/serialization/common/ble_serialization.c) \
$(abspath ../../../../../../components/serialization/common/struct_ser/s130/ble_struct_serialization.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_tx_packet_count_get.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_user_mem_reply.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_uuid_decode.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_uuid_encode.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_uuid_vs_add.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ble_version_get.c) \
$(abspath ../../../../../../components/serialization/common/cond_field_serialization.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/ecb_block_encrypt.c) \
$(abspath ../../../../../../components/serialization/common/struct_ser/s130/nrf_soc_struct_serialization.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/power_system_off.c) \
$(abspath ../../../../../../components/serialization/application/hal/ser_app_hal_nrf51.c) \
$(abspath ../../../../../../components/serialization/application/hal/ser_app_power_system_off.c) \
$(abspath ../../../../../../components/serialization/common/transport/ser_hal_transport.c) \
$(abspath ../../../../../../components/serialization/common/transport/ser_phy/ser_phy_nrf51_nrf_drv_spi.c) \
$(abspath ../../../../../../components/serialization/application/transport/ser_sd_transport.c) \
$(abspath ../../../../../../components/serialization/application/transport/ser_softdevice_handler.c) \
$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers/temp_get.c) \
$(abspath ../../../../../../components/libraries/button/app_button.c) \
$(abspath ../../../../../../components/libraries/util/app_error.c) \
$(abspath ../../../../../../components/libraries/util/app_error_weak.c) \
$(abspath ../../../../../../components/libraries/fifo/app_fifo.c) \
$(abspath ../../../../../../components/libraries/mailbox/app_mailbox.c) \
$(abspath ../../../../../../components/libraries/scheduler/app_scheduler.c) \
$(abspath ../../../../../../components/libraries/timer/app_timer.c) \
$(abspath ../../../../../../components/libraries/util/app_util_platform.c) \
$(abspath ../../../../../../components/libraries/crc16/crc16.c) \
$(abspath ../../../../../../components/libraries/fds/fds.c) \
$(abspath ../../../../../../components/libraries/fstorage/fstorage.c) \
$(abspath ../../../../../../components/libraries/util/nrf_assert.c) \
$(abspath ../../../../../../components/libraries/util/nrf_log.c) \
$(abspath ../../../../../../components/libraries/uart/retarget.c) \
$(abspath ../../../../../../components/libraries/util/sdk_mapped_flags.c) \
$(abspath ../../../../../../components/libraries/sensorsim/sensorsim.c) \
$(abspath ../../../../../../external/segger_rtt/RTT_Syscalls_GCC.c) \
$(abspath ../../../../../../external/segger_rtt/SEGGER_RTT.c) \
$(abspath ../../../../../../external/segger_rtt/SEGGER_RTT_printf.c) \
$(abspath ../../../../../../components/libraries/uart/app_uart_fifo.c) \
$(abspath ../../../../../../components/drivers_nrf/ble_flash/ble_flash.c) \
$(abspath ../../../../../../components/drivers_nrf/delay/nrf_delay.c) \
$(abspath ../../../../../../components/drivers_nrf/common/nrf_drv_common.c) \
$(abspath ../../../../../../components/drivers_nrf/gpiote/nrf_drv_gpiote.c) \
$(abspath ../../../../../../components/drivers_nrf/spi_master/nrf_drv_spi.c) \
$(abspath ../../../../../../components/drivers_nrf/uart/nrf_drv_uart.c) \
$(abspath ../../../../../../components/drivers_nrf/hal/nrf_nvmc.c) \
$(abspath ../../../../../../components/drivers_nrf/nrf_soc_nosd/nrf_soc.c) \
$(abspath ../../../../../../components/drivers_nrf/pstorage/pstorage_nosd.c) \
$(abspath ../../../../../bsp/bsp.c) \
$(abspath ../../../../../bsp/bsp_btn_ble.c) \
$(abspath ../../../main.c) \
$(abspath ../../../../../../components/ble/common/ble_advdata.c) \
$(abspath ../../../../../../components/ble/ble_advertising/ble_advertising.c) \
$(abspath ../../../../../../components/ble/ble_services/ble_bas/ble_bas.c) \
$(abspath ../../../../../../components/ble/common/ble_conn_params.c) \
$(abspath ../../../../../../components/ble/common/ble_conn_state.c) \
$(abspath ../../../../../../components/ble/ble_services/ble_dis/ble_dis.c) \
$(abspath ../../../../../../components/ble/ble_services/ble_gls/ble_gls.c) \
$(abspath ../../../../../../components/ble/ble_services/ble_gls/ble_gls_db.c) \
$(abspath ../../../../../../components/ble/ble_racp/ble_racp.c) \
$(abspath ../../../../../../components/ble/common/ble_srv_common.c) \
$(abspath ../../../../../../components/ble/peer_manager/gatt_cache_manager.c) \
$(abspath ../../../../../../components/ble/peer_manager/gattc_cache_manager.c) \
$(abspath ../../../../../../components/ble/peer_manager/gatts_cache_manager.c) \
$(abspath ../../../../../../components/ble/peer_manager/id_manager.c) \
$(abspath ../../../../../../components/ble/peer_manager/peer_data.c) \
$(abspath ../../../../../../components/ble/peer_manager/peer_data_storage.c) \
$(abspath ../../../../../../components/ble/peer_manager/peer_database.c) \
$(abspath ../../../../../../components/ble/peer_manager/peer_id.c) \
$(abspath ../../../../../../components/ble/peer_manager/peer_manager.c) \
$(abspath ../../../../../../components/ble/peer_manager/pm_buffer.c) \
$(abspath ../../../../../../components/ble/peer_manager/pm_mutex.c) \
$(abspath ../../../../../../components/ble/peer_manager/security_dispatcher.c) \
$(abspath ../../../../../../components/ble/peer_manager/security_manager.c) \
$(abspath ../../../../../../components/toolchain/system_nrf52.c) \
$(abspath ../../../../../../components/softdevice/common/softdevice_handler/softdevice_handler.c) \

#assembly files common to all targets
ASM_SOURCE_FILES  = $(abspath ../../../../../../components/toolchain/gcc/gcc_startup_nrf52.s)

#includes common to all targets
INC_PATHS  = -I$(abspath ../../../config/ble_app_gls_s132_spi_pca10040)
INC_PATHS += -I$(abspath ../../../config)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/scheduler)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/spi_master)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/config)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/timer)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/fifo)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/fstorage/config)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/fds/config)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/delay)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/crc16)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/common)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/mailbox)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/util)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/application/codecs/s130/serializers)
INC_PATHS += -I$(abspath ../../../../../../components/softdevice/s132/headers/nrf52)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/uart)
INC_PATHS += -I$(abspath ../../../../../../components/ble/common)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/sensorsim)
INC_PATHS += -I$(abspath ../../../../../../components/ble/peer_manager)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/pstorage)
INC_PATHS += -I$(abspath ../../../../../../components/ble/ble_services/ble_dis)
INC_PATHS += -I$(abspath ../../../../../../components/device)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/uart)
INC_PATHS += -I$(abspath ../../../../../../components/ble/ble_services/ble_gls)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/button)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/ble_flash)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/fstorage)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/experimental_section_vars)
INC_PATHS += -I$(abspath ../../../../../../components/softdevice/s132/headers)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/common/transport/ser_phy)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/common/transport/ser_phy/config)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/gpiote)
INC_PATHS += -I$(abspath ../../../../../../external/segger_rtt)
INC_PATHS += -I$(abspath ../../../../../bsp)
INC_PATHS += -I$(abspath ../../../../../../components/softdevice/common/softdevice_handler)
INC_PATHS += -I$(abspath ../../../../../../components/toolchain/CMSIS/Include)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/hal)
INC_PATHS += -I$(abspath ../../../../../../components/ble/ble_racp)
INC_PATHS += -I$(abspath ../../../../../../components/toolchain/gcc)
INC_PATHS += -I$(abspath ../../../../../../components/toolchain)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/fds)
INC_PATHS += -I$(abspath ../../../../../../components/drivers_nrf/common)
INC_PATHS += -I$(abspath ../../../../../../components/ble/ble_advertising)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/common/transport)
INC_PATHS += -I$(abspath ../../../../../../components/libraries/trace)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/application/hal)
INC_PATHS += -I$(abspath ../../../../../../components/ble/ble_services/ble_bas)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/application/transport)
INC_PATHS += -I$(abspath ../../../../../../components/serialization/common/struct_ser/s130)

OBJECT_DIRECTORY = _build
LISTING_DIRECTORY = $(OBJECT_DIRECTORY)
OUTPUT_BINARY_DIRECTORY = $(OBJECT_DIRECTORY)

# Sorting removes duplicates
BUILD_DIRECTORIES := $(sort $(OBJECT_DIRECTORY) $(OUTPUT_BINARY_DIRECTORY) $(LISTING_DIRECTORY) )

#flags common to all targets
CFLAGS  = -DNRF52
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
CFLAGS += -DNRF_LOG_USES_UART=1
CFLAGS += -DS132
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DBLE_STACK_SUPPORT_REQD
CFLAGS += -DSWI_DISABLE3
CFLAGS += -DSWI_DISABLE0
CFLAGS += -DNRF52_PAN_20
CFLAGS += -DNRF52_PAN_64
CFLAGS += -DNRF52_PAN_62
CFLAGS += -DNRF52_PAN_63
CFLAGS += -DSVCALL_AS_NORMAL_FUNCTION
CFLAGS += -DSPI_MASTER_0_ENABLE
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
ASMFLAGS += -DNRF52
ASMFLAGS += -DBOARD_PCA10040
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
ASMFLAGS += -DSWI_DISABLE3
ASMFLAGS += -DSWI_DISABLE0
ASMFLAGS += -DNRF52_PAN_20
ASMFLAGS += -DNRF52_PAN_64
ASMFLAGS += -DNRF52_PAN_62
ASMFLAGS += -DNRF52_PAN_63
ASMFLAGS += -DSVCALL_AS_NORMAL_FUNCTION
ASMFLAGS += -DSPI_MASTER_0_ENABLE

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
nrf52832_xxaa: LINKER_SCRIPT=ble_app_gls_gcc_nrf52.ld

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