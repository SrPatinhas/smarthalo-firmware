
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "nrf_dfu_settings.h"
#include "scheduler.h"
#include "bleapp.h"
#include "bslink.h"
#include "dispatch.h"
#include "leds.h"
#include "record.h"
#include "sound.h"
#include "bootinfo.h"
#include "leds.h"
#include "meters.h"
#include "battery.h"
#include "device.h"
#include "ui.h"

#include "alarm.h"

#ifdef DEBUG
#define ALARM_VOLUME 1
#else
#define ALARM_VOLUME 100
#endif


sound_bit_t alarm_trk_siren[] = {
{2800,100 | SOUND_SWEEP},
{3600,100 | SOUND_SWEEP},
{0,0}
};
#define SIREN_SECONDS(x) (((x)*1000)/200)

sound_bit_t alarm_trk_warn[] = {
{90,500 | SOUND_SWEEP},
{75,200},
{0,0}
};

typedef struct {
    uint8_t code;
    uint8_t codeLen;
    uint8_t mode;
    uint8_t severity;
} alarm_settings_t;

alarm_settings_t alarm_settings __attribute__ ((aligned (4)));

uint8_t alarm_seed[4];
bool alarm_seed_valid = false;

bool alarm_code_valid;
uint32_t alarm_code = 0;
uint32_t alarm_codeLen = 0;

uint8_t alarm_mode = 0;
uint8_t alarm_severity = 0;

bool alarm_enabled = false;
bool alarm_active = false;

bool alarm_isMovement = false;

bool alarm_authentificated = false;

bool alarm_tap_snooze = false;

uint32_t alarm_trigger_cnt = 0;

uint32_t alarm_unarm_tap_tries = 0;

#define ALARM_GRACE_TIME_LENIENT 14
#define ALARM_GRACE_TIME_AGRESSIVE 8
#define ALARM_QUIET_TIME_LENIENT 4
#define ALARM_QUIET_TIME_AGRESSIVE 2

uint32_t alarm_grace_time = ALARM_GRACE_TIME_LENIENT;
uint32_t alarm_quiet_time = ALARM_QUIET_TIME_LENIENT;

#define ALARM_TAP_MAXRETRIES 4

uint32_t alarm_trigger_span;

bool alarm_isCodeValid() {
    return alarm_code_valid;
}

bool alarm_isEnabled() {
    return alarm_enabled;
}

void alarm_trigger_on() {
    if(!alarm_active) {
        alarm_active = true;
        alarm_trigger_cnt++;
    }
    leds_alarm_trigger(true, 0);
    sound_play(ALARM_VOLUME, SIREN_SECONDS(15), alarm_trk_siren);
}

void alarm_trigger_tick(void *ctx) {
    if(alarm_trigger_span < alarm_grace_time) {
        alarm_active = false;
        alarm_trigger_span++;
        if(alarm_trigger_span>alarm_grace_time) {
            alarm_trigger_span = alarm_grace_time;
        }
        //
        float progress = (float)(alarm_trigger_span)/(float)alarm_grace_time;
        leds_alarm_trigger(false, progress);
        //
        if(alarm_trigger_span > alarm_quiet_time) {
            sound_play(100 * progress, 0, alarm_trk_warn);
        }        
        //
        if(alarm_enabled) {
            sch_unique_oneshot(alarm_trigger_tick, 1000);
        }
    } else {
        alarm_trigger_on();
    }
}

void alarm_reset_tick(void *ctx) {
    if(alarm_trigger_span > 0) {
        alarm_trigger_span--;
        //
        sch_unique_oneshot(alarm_reset_tick, 10000);
    }
}

void alarm_movement_on() {
    alarm_isMovement = true;
    if(alarm_enabled) {
        if(!sch_unique_isScheduled(alarm_trigger_tick)) {
            sch_unique_oneshot(alarm_trigger_tick, 0);
        }
        if(sch_unique_isScheduled(alarm_reset_tick)) {
            sch_unique_cancel(alarm_reset_tick);
        }
    }
}

void alarm_movement_off() {
    alarm_isMovement = false;
    if(alarm_enabled) {
        if(!sch_unique_isScheduled(alarm_reset_tick)) {
            sch_unique_oneshot(alarm_reset_tick, (alarm_active) ? 60000 : 10000);
        }
        if(sch_unique_isScheduled(alarm_trigger_tick)) {
            sch_unique_cancel(alarm_trigger_tick);
        }
    }
}

bool alarm_flash_state_pending = false;
bool alarm_flash_state_update = false;
bool alarm_flash_state_armed = false;

void alarm_flash_sch(void *ctx);

void alarm_flash_callback() {
    printf("alarm_flash_callback\r\n");
    alarm_flash_state_pending = false;
    if(alarm_flash_state_update) {
        alarm_flash_state_update = false;
        sch_unique_oneshot(alarm_flash_sch, 0);
    }
}

void alarm_flash_sch(void *ctx) {
    uint32_t binfo = binfo_getWord();

    if(alarm_flash_state_armed == BINFO_IS_ALARM_ON(binfo)) {
        alarm_flash_state_pending = false;
        alarm_flash_state_update = false;
        return;
    }

    binfo = BINFO_ALARM(binfo, alarm_flash_state_armed);

    binfo_setWord(binfo, alarm_flash_callback);

    printf("alarm_flash_sch\r\n");

}

void alarm_flash_set(bool armed) {
    alarm_flash_state_armed = armed;
    if(!alarm_flash_state_pending) {
        alarm_flash_state_pending = true;
        sch_unique_oneshot(alarm_flash_sch, 0);
    } else {
        alarm_flash_state_update = true;
    }
}

void alarm_arm() {
    alarm_flash_set(true);
    alarm_enabled = true;
    alarm_trigger_span = 0;
    alarm_trigger_cnt = 0;
    alarm_unarm_tap_tries = ALARM_TAP_MAXRETRIES;
    alarm_tap_snooze = false;
}

void alarm_disarm() {
    alarm_flash_set(false);
    alarm_enabled = false;
    sch_unique_cancel(alarm_trigger_tick);
    sch_unique_cancel(alarm_reset_tick);
    sound_stop();
    leds_anim_off(ANIM_ALARM);
}

//============================================================================
// Mode 2 RSSI

bool alarm_monitor_rssi_trigger = false;

void alarm_monitor_rssi_arm(void *ctx) {
    alarm_monitor_rssi_trigger = false;
    leds_alarm_arm();
    alarm_arm();
}
void alarm_monitor_rssi_disarm(void *ctx) {
    alarm_monitor_rssi_trigger = false;
    leds_alarm_disarm();
    alarm_disarm();
    ui_onAlarm_disarm();
}

void alarm_monitor_rssi(void *ctx) {
    if (alarm_mode != 2) {
        return;
    }
    double rssi = bleapp_getRSSI();
    if (alarm_authentificated) {
        if (alarm_enabled) {
            if (rssi >= -70.) {
                if(!alarm_monitor_rssi_trigger) {
                    alarm_monitor_rssi_trigger = true;
                    sch_unique_oneshot(alarm_monitor_rssi_disarm, 2000);
                }
            } else if (alarm_monitor_rssi_trigger) {
                alarm_monitor_rssi_trigger = false;
                sch_unique_cancel(alarm_monitor_rssi_disarm);
            }
        } else {
            if (rssi <= -80. && !alarm_isMovement) {
                if(!alarm_monitor_rssi_trigger) {
                    alarm_monitor_rssi_trigger = true;
                    sch_unique_oneshot(alarm_monitor_rssi_arm, 2000);
                }
            } else if (alarm_monitor_rssi_trigger) {
                alarm_monitor_rssi_trigger = false;
                sch_unique_cancel(alarm_monitor_rssi_arm);
            }
        }
    }
    sch_unique_oneshot(alarm_monitor_rssi, 500);
}


//============================================================================
// Touch code

void alarm_tap_unsnooze(void *ctx) {
    alarm_tap_snooze = false;
    alarm_unarm_tap_tries = ALARM_TAP_MAXRETRIES;
}

bool alarm_codeHandle(uint32_t code, uint32_t len) {
    if(len != 5 || (!alarm_isEnabled() && bat_isUSBPlugged()) || !alarm_isCodeValid()) {
        return false;
    }

    if(
        alarm_code_valid &&
        (alarm_codeLen == len) && 
        (alarm_code == code) &&
        (!alarm_tap_snooze)
    ) {
        if(alarm_enabled) {
            leds_alarm_disarm();
            alarm_disarm();
            leds_lowBat_notify();
        } else {
            if(!bleapp_isConnected()) {
                leds_alarm_arm();
                alarm_arm();
                leds_frontlight_set(false);
                leds_front_turnOff();
                leds_lowBat_notify();
            }
        }

        //Event
        uint8_t buf[2];
        uint32_t ptr = 0;
        buf[ptr++] = NOTIFY_ALARM;
        buf[ptr++] = (alarm_enabled) ? 1 : 0;
        bslink_up_write(buf, ptr);

        return true;
    }

    if(alarm_enabled) {
        if(!alarm_unarm_tap_tries) {
            alarm_trigger_span = alarm_grace_time;
            alarm_trigger_on();
            alarm_tap_snooze = true;
        } else {
            alarm_unarm_tap_tries--;
        }
        sch_unique_oneshot(alarm_tap_unsnooze, 15*1000);
        return true;
    }

    return false;
}

//============================================================================
// Comm

void alarm_report(uint8_t *buf, uint32_t len) {
    uint8_t reply[5];
    uint32_t ptr = 0;

    printf("alarm_report\r\n");
    reply[ptr++] = RET_OK;
    reply[ptr++] = (alarm_enabled) ? 1 : 0;
    reply[ptr++] = (alarm_trigger_cnt > 255) ? 255 : alarm_trigger_cnt;
    reply[ptr++] = alarm_severity;
    reply[ptr++] = alarm_mode;

    bslink_write(reply, ptr);
}

void alarm_getSeed(uint8_t *buf, uint32_t len) {
    uint8_t reply[1+4];
    uint32_t ptr = 0;

    printf("alarm_getSeed\r\n");
    nrf_drv_rng_block_rand(alarm_seed, 4);
    alarm_seed_valid = true;

    reply[ptr++] = RET_OK;

    memcpy(reply+ptr, alarm_seed, 4);
    ptr += 4;

    bslink_write(reply, ptr);
}

void alarm_comm_arm(uint8_t *buf, uint32_t len) {
    bool arm = (buf[4] != 0);

    if(
        len != 5 || 
        !alarm_seed_valid ||
        memcmp((const char*)buf, (const char*)alarm_seed, 4) != 0 
        ) {
        disp_respond(RET_FAIL);
        return;
    }
    alarm_seed_valid = false;

    if(arm) {
        leds_alarm_arm();
        alarm_arm();
        leds_frontlight_set(false);
        leds_front_turnOff();
    } else {
        leds_alarm_disarm();
        alarm_disarm();
    }

    disp_respond(RET_OK);
}

void alarm_setConfig(uint8_t *buf, uint32_t len) {
    if(
        (len != 7 && len != 8)  || 
        !(buf[5] == 0 || (buf[5] >= 5 && buf[5] <= 32)) || //codeLen
        !alarm_seed_valid ||
        memcmp((const char*)buf, (const char*)alarm_seed, 4) != 0 
        ) {
        disp_respond(RET_FAIL);
        return;
    }
    alarm_seed_valid = false;

    if(buf[5] < 5) {
        buf[4] = 0;
        buf[5] = 0;
    }

    alarm_code = buf[4];
    alarm_codeLen = buf[5];
    alarm_code_valid = (buf[5] > 0);
    alarm_mode = buf[6];
    alarm_severity = (len == 8) ? buf[7] : 0;

    if(alarm_mode == 2) {
        sch_unique_oneshot(alarm_monitor_rssi, 500);
    }

    if(alarm_severity == 0) {
        alarm_grace_time = ALARM_GRACE_TIME_LENIENT;
        alarm_quiet_time = ALARM_QUIET_TIME_LENIENT;
    } else {
        alarm_grace_time = ALARM_GRACE_TIME_AGRESSIVE;
        alarm_quiet_time = ALARM_QUIET_TIME_AGRESSIVE;
    }

    memset(&alarm_settings, 0, 4);
    memcpy(&alarm_settings, buf+4, (len == 8) ? 4 : 3);
    rec_write(KEY_ALARMSETTINGS, &alarm_settings, sizeof(alarm_settings_t), NULL);

    disp_respond(RET_OK);
}

disp_cmdfn_t alarm_cmdfn[] = {
    alarm_report,
    alarm_getSeed,
    alarm_comm_arm,
    alarm_setConfig,
};
#define ALARM_CMDFN_CNT (sizeof(alarm_cmdfn)/sizeof(disp_cmdfn_t))

void alarm_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= ALARM_CMDFN_CNT) {
        disp_respond(RET_UNIMPLEMENTED);
    } else {
        alarm_cmdfn[buf[0]](buf+1, len-1);
    }
}

//============================================================================

void alarm_auto_arm(void *ctx) {
    alarm_arm();
}

void alarm_onAuth() {
    if(alarm_enabled) {
        alarm_disarm();
    }
    sch_unique_cancel(alarm_auto_arm);
    alarm_authentificated = true;
}

void alarm_evt_cb( ble_evt_t * p_ble_evt ) {
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GAP_EVT_CONNECTED:
            if(alarm_mode == 2) {
                sch_unique_oneshot(alarm_monitor_rssi, 500);
            }
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            if(alarm_authentificated && (alarm_mode > 0) && !alarm_enabled && !bat_isUSBPlugged() && !dev_isTravelMode()) {
                sch_unique_oneshot(alarm_auto_arm, 5000);
            }
            alarm_authentificated = false;
            sch_unique_cancel(alarm_monitor_rssi);
            break;
        default:
            break;
    }

}

//============================================================================
void configure_alarm(){
    alarm_code = alarm_settings.code;
    alarm_codeLen = alarm_settings.codeLen;
    alarm_code_valid = (alarm_codeLen >= 5);
    alarm_mode = alarm_settings.mode;
    alarm_severity = alarm_settings.severity;    
}

void alarm_onPasswordClear() {
    memset(&alarm_settings, 0, 4);
    rec_delete(KEY_ALARMSETTINGS, NULL);
    configure_alarm();
    if(alarm_severity == 0) {
        alarm_grace_time = ALARM_GRACE_TIME_LENIENT;
        alarm_quiet_time = ALARM_QUIET_TIME_LENIENT;
    } else {
        alarm_grace_time = ALARM_GRACE_TIME_AGRESSIVE;
        alarm_quiet_time = ALARM_QUIET_TIME_AGRESSIVE;
    }

}

void alarm_init() {
    memset(&alarm_settings, 0, 4);
    rec_read(KEY_ALARMSETTINGS, &alarm_settings, 4, NULL);
    configure_alarm();
    if(alarm_severity == 0) {
        alarm_grace_time = ALARM_GRACE_TIME_LENIENT;
        alarm_quiet_time = ALARM_QUIET_TIME_LENIENT;
    } else {
        alarm_grace_time = ALARM_GRACE_TIME_AGRESSIVE;
        alarm_quiet_time = ALARM_QUIET_TIME_AGRESSIVE;
    }

    printf("alarm_init: %s, %d, severity:%d\r\n", (alarm_code_valid) ? "code valid" : "no code", alarm_mode, alarm_severity);

//Comm
    disp_register(3, 1, alarm_dispatch);
    bleapp_evt_register(alarm_evt_cb);

    uint32_t binfo = binfo_getWord();

    if(BINFO_IS_ALARM_ON(binfo)) {
        printf("alarm trigger from reset!\r\n");
        alarm_arm();
        alarm_trigger_span = alarm_grace_time;
        alarm_trigger_on();
    }

}
