/*
 * battery.c
 *
 *  Created on: Nov 8, 2016
 *      Author: Sean
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"
#include "scheduler.h"
#include "bslink.h"
#include "dispatch.h"
#include "twi.h"
#include "leds.h"
#include "bleapp.h"
#include "meters.h"
#include "ui.h"
#include "alarm.h"
#include "sound.h"

#include "battery.h"

#if defined(PLATFORM_shv1x)

uint32_t bat_powergood_pin;

#define REGISTER_AUTO_INCREMENT    			(1 << 7)

#define TWI_STC3115_ADDR   					((0xE0 >> 1) & (0x7F))//fuel gauge address

//registers
#define REG_MODE							0
#define REG_CTRL							1
#define REG_SOC_L 							2
#define REG_SOC_H							3
#define REG_COUNTER_L						4
#define REG_COUNTER_H						5
#define REG_CURRENT_L						6
#define REG_CURRENT_H						7
#define REG_VOLTAGE_L						8
#define REG_VOLTAGE_H						9
#define REG_TEMPERATURE						10
#define REG_CC_ADJ_HIGH						11
#define REG_VM_ADJ_HIGH						12
#define REG_OCV_L							13
#define REG_OCV_H							14
#define REG_CC_CNF_L						15
#define REG_CC_CNF_H						16
#define REG_VM_CNF_L						17
#define REG_VM_CNF_H						18
#define REG_ALARM_SOC						19
#define REG_ALARM_VOLTAGE					20
#define REG_CURRENT_THRES					21
#define REG_RELAX_COUNT						22
#define REG_RELAX_MAX						23
#define REG_ID								24
#define REG_CC_ADJ_LOW						25
#define REG_VM_ADJ_LOW						26
#define ACC_CC_ADJ_L						27
#define ACC_CC_ADJ_H						28
#define ACC_VM_ADJ_L						29
#define ACC_VM_ADJ_H						30

//initialisation values
#define ALARMRSOCMIN						0x0000
#define ALARMRSOCMAX						0x0064
#define ALARMCELLMIN						0.0f
#define ALARMCELLMAX						4.33f
#define POWERMODEMIN						0x0000
#define POWERMODEMAX						0x0001
#define STATUSBITMIN						0x0000
#define STATUSBITMAX						0x0001
#define VM_CNF_DEF          				379U //approximations based on ideal battery
#define VM_CNF_DEF_L 						VM_CNF_DEF & 0x00FF
#define VM_CNF_DEF_H 						(VM_CNF_DEF >> 8) & 0x00FF
#define CC_CNF_DEF          				393U //approximations based on ideal battery
#define CC_CNF_DEF_L						CC_CNF_DEF & 0x00FF
#define CC_CNF_DEF_H						(CC_CNF_DEF >> 8) & 0x00FF
#define ALARM_SOC_DEF       				2    // 1%
#define ALARM_V_DEF         				170   // 2992 mV
#define CURRENT_THRES_DEF   				10   // 47.04 mA
				
#define CONV_SOC  							0.001953125
#define CONV_I    							5.88
#define CONV_V    							0.0022
#define CONV_T    							1
#define RSENSE    							0.01

static int convert8Bto16BSigned(unsigned char lowB, unsigned char highB);

bool bat_state_pg = false;
bool bat_state_chg = false;

//============================================================================
//

/*
//operations must be executed in this order
volatile uint8_t bat_data_init[] =
{	
	REG_OCV_L, 0,	//initial open circuit voltage must be read in order to have a point of reference for the state of charge
	REG_OCV_H, 0,	//bat_data_init[1] and bat_data_init[3] are placeholders for initial ocv values 
	REG_MODE, 0x00,  // disable run mode
	REG_VM_CNF_L, VM_CNF_DEF_L,
	REG_VM_CNF_H, VM_CNF_DEF_H,
	REG_CC_CNF_L, CC_CNF_DEF_L,
	REG_CC_CNF_H, CC_CNF_DEF_H,
	REG_ALARM_SOC, ALARM_SOC_DEF,
	REG_ALARM_VOLTAGE, ALARM_V_DEF,
	//REG_CTRL, 0x01, //clear interrupts for alarm (conserved even if chip is power cycled)
	REG_CURRENT_THRES, CURRENT_THRES_DEF,
	REG_OCV_L, 0, // initial open circuit voltage is written here 
	REG_OCV_H, 0,
	REG_MODE, 0x11, //enable run mode, disable alarm, power saving voltage mode
};

static app_twi_transfer_t const bat_trans_init[] =
{
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init  , 1, APP_TWI_NO_STOP),
	APP_TWI_READ(TWI_STC3115_ADDR, bat_data_init+21, 1, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+2, 1, APP_TWI_NO_STOP),
	APP_TWI_READ(TWI_STC3115_ADDR, bat_data_init+23, 1, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+4, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+6, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+8, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+10, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+12, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+14, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+16, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+18, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+20, 2, 0), //initial ocv value
	//APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+1, 1, 0), 
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+22, 2, 0), //initial ocv value
	//APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+3, 1, 0), 
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+24, 2, 0),
};
*/

volatile uint8_t bat_data_init[] =
{	
	REG_MODE, 0x00,  // disable run mode
	REG_VM_CNF_L, VM_CNF_DEF_L,
	REG_VM_CNF_H, VM_CNF_DEF_H,
	REG_CC_CNF_L, CC_CNF_DEF_L,
	REG_CC_CNF_H, CC_CNF_DEF_H,
	REG_CURRENT_THRES, CURRENT_THRES_DEF,
	REG_MODE, 0x10, //enable run mode, disable alarm, mixed mode
};


static app_twi_transfer_t const bat_trans_init[] =
{
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+0, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+2, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+4, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+6, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+8, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+10, 2, 0),
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+12, 2, 0),
};

volatile uint8_t bat_data_standby[] =
{	
	REG_MODE, 0x00,  // disable run mode
};

static app_twi_transfer_t const bat_trans_standby[] =
{
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_data_init+0, 2, 0),
};

//============================================================================
//

volatile uint8_t bat_SOC_read[] = {REG_SOC_L,REG_SOC_H};
uint8_t bat_SOC_buffer[2];

volatile uint8_t bat_voltage_read[] = {REG_VOLTAGE_L, REG_VOLTAGE_H};
uint8_t bat_voltage_buffer[2];

volatile uint8_t bat_current_read[] = {REG_CURRENT_L, REG_CURRENT_H};
uint8_t bat_current_buffer[2];

volatile uint8_t bat_temperature_read[] = {REG_TEMPERATURE};
uint8_t bat_temperature_buffer[1];

volatile uint8_t bat_alarm_read[] = {REG_CTRL};
uint8_t bat_alarm_buffer[1];

volatile uint8_t bat_alarm_clear[] = {REG_CTRL, 0x01};

volatile uint8_t bat_mode_ctrl_reg_read[] = {REG_MODE,REG_CTRL};
uint8_t bat_settings_buffer[2];


static app_twi_transfer_t const bat_trans_read[] =
{    //tansactions are seperate and do not use auto increment to store in seperate buffers
    APP_TWI_WRITE(TWI_STC3115_ADDR, bat_SOC_read, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_STC3115_ADDR, bat_SOC_buffer, 2, 0),
    APP_TWI_WRITE(TWI_STC3115_ADDR, bat_voltage_read, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_STC3115_ADDR, bat_voltage_buffer, 2, 0),
    APP_TWI_WRITE(TWI_STC3115_ADDR, bat_current_read, 1, APP_TWI_NO_STOP),
    APP_TWI_READ (TWI_STC3115_ADDR, bat_current_buffer, 2, 0),
//    APP_TWI_WRITE(TWI_STC3115_ADDR, bat_temperature_read, 1, APP_TWI_NO_STOP),
//    APP_TWI_READ (TWI_STC3115_ADDR, bat_temperature_buffer, 2, 0),
//	APP_TWI_WRITE(TWI_STC3115_ADDR, bat_alarm_read, 1, APP_TWI_NO_STOP),//read alarm
//	APP_TWI_READ (TWI_STC3115_ADDR, bat_alarm_buffer, 2, 0),
//	APP_TWI_WRITE(TWI_STC3115_ADDR, bat_mode_ctrl_reg_read, 1, APP_TWI_NO_STOP),//read alarm
//	APP_TWI_READ (TWI_STC3115_ADDR, bat_settings_buffer, 2, 0),

};

static app_twi_transfer_t const bat_trans_clearAlarm[] =
{
	APP_TWI_WRITE(TWI_STC3115_ADDR,bat_alarm_clear, 2, 0), //clears alarm registers (not the hw interrupt)
};

//============================================================================
//

#define BAT_ISET2_FLOAT() \
	do { \
		NRF_GPIO->OUTSET = (1UL << V10_PATCH_ISET2); \
		NRF_GPIO->PIN_CNF[V10_PATCH_ISET2] = ((NRF_GPIO->PIN_CNF[V10_PATCH_ISET2]) & 0xfffffffe); \
	} while(0)

#define BAT_ISET2_HIGH() \
	do { \
		NRF_GPIO->OUTSET = (1UL << V10_PATCH_ISET2); \
		NRF_GPIO->PIN_CNF[V10_PATCH_ISET2] = ((NRF_GPIO->PIN_CNF[V10_PATCH_ISET2]) | 0x00000001); \
	} while(0)

#define BAT_ISET2_LOW() \
	do { \
		NRF_GPIO->OUTCLR = (1UL << V10_PATCH_ISET2); \
		NRF_GPIO->PIN_CNF[V10_PATCH_ISET2] = ((NRF_GPIO->PIN_CNF[V10_PATCH_ISET2]) | 0x00000001); \
	} while(0)

//============================================================================
//

int32_t bat_soc;

uint32_t bat_getSOC()
{
	return bat_soc;
}

bool is_lowBat(){
    if (bat_getSOC() < 15 && !bat_isUSBPlugged()){
       return true;
    }
    return false;
}


void bat_notify() {
		uint8_t buf[4];
		uint32_t ptr = 0;
		buf[ptr++] = NOTIFY_DEVICE;
		buf[ptr++] = (uint8_t)bat_soc;
		buf[ptr++] = met_getTemperature();
		buf[ptr++] = (bat_isUSBPlugged() ? 1 : 0);
		bslink_up_write(buf, ptr);
}

void bat_on_plug(void *ctx) {
	bool startup = (bool)ctx;
	bool plugged = (nrf_gpio_pin_read(bat_powergood_pin) == 0);
	bool bcd0 = (nrf_gpio_pin_read(V10_PATCH_BCD0) == 1);
	bool bcd1 = (nrf_gpio_pin_read(V10_PATCH_BCD1) == 1);

	if(startup) {
		if(plugged) {
		    leds_battery(bat_getSOC());
		}
	}

	if(platform_getHW() == HW_V11) {
		printf("USB %s BCD: %s%s\r\n", (plugged) ? "PLUGGED" : "UNPLUGGED", (bcd1) ? "1" : "0", (bcd0) ? "1" : "0");
		if(plugged) {
			if(bcd0 && !bcd1) {
				printf("CHG 960mA\r\n");
				BAT_ISET2_LOW();
			} else {
				printf("CHG 500mA\r\n");
				BAT_ISET2_HIGH();
			}
		} else {
			printf("CHG 100mA\r\n");
			BAT_ISET2_FLOAT();
		}
	} else {
		printf("USB %s \r\n", (plugged) ? "PLUGGED" : "UNPLUGGED");
	}

	if(plugged) {
		if(bat_soc > 95){
			sch_unique_oneshot(leds_charged, 1000);
		}else{
			sch_unique_oneshot(leds_charging,1000);
		}
	} else {
		sch_unique_cancel(leds_charging);
		sch_unique_cancel(leds_charged);
		sch_unique_oneshot(leds_charging_off,0);
	}

	bat_notify();

}


void bat_int_onplug(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	sch_unique_oneshot_ctx(bat_on_plug, 1000, (void*)false);
}

void bat_twi_init_done(ret_code_t result, void *ctx)
{
	//must wait for at least two samples of 500ms after init to read SOC

	sch_unique_oneshot(bat_readData, 1000);
}

void bat_init()
{
	ret_code_t err;

    twi_transaction_do(bat_trans_init, sizeof(bat_trans_init)/sizeof(app_twi_transfer_t), bat_twi_init_done, NULL);

    if(!nrf_drv_gpiote_is_init())
    {
        err = nrf_drv_gpiote_init();
        ERR_CHECK("nrf_drv_gpiote_init", err);
    }

	if(platform_getHW() == HW_V12) {
		bat_powergood_pin = V12_USB_POWERGOOD_N_PIN;
	} else {
		bat_powergood_pin = V10_USB_POWERGOOD_N_PIN;
	}

	NRF_GPIO->PIN_CNF[bat_powergood_pin] = (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
	config.pull = NRF_GPIO_PIN_PULLUP;
	err = nrf_drv_gpiote_in_init(bat_powergood_pin, &config, bat_int_onplug);
	ERR_CHECK("nrf_drv_gpiote_in_init", err);
	nrf_drv_gpiote_in_event_enable(bat_powergood_pin, true);

	//USB_CHARGING_N_PIN pull-up
	NRF_GPIO->PIN_CNF[USB_CHARGING_N_PIN] = (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos); //| (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos);

	if(platform_getHW() == HW_V11) {
		NRF_GPIO->PIN_CNF[V10_PATCH_BCD0] = 0;
		NRF_GPIO->PIN_CNF[V10_PATCH_BCD1] = 0;
		NRF_GPIO->PIN_CNF[V10_PATCH_ISET2] = (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos);
		BAT_ISET2_FLOAT();
	}

	sch_unique_oneshot_ctx(bat_on_plug, 1500, (void*)true);

}

#define BATMIN 20

void bat_twi_readData_done(ret_code_t result, void *ctx)
{
	if(result == NRF_SUCCESS) {

		bat_soc = (((uint32_t)bat_SOC_buffer[1]) << 8) + ((uint32_t)bat_SOC_buffer[0]);
		bat_soc -= BATMIN*512;
		bat_soc *= 100;
		bat_soc /= 100-BATMIN;
		if(bat_soc < 0) {
			bat_soc = 0;
		}
		bat_soc /= 512;
		if(bat_soc >= 95) {
			bat_soc = 100;
		}
		printf("bat_getSOC: %d\r\n",bat_soc);

		//if the battery is low, turn off the alarm regardless of whether it's plugged in.
		// the idea here is that the alarm should not be enabled on a dying on dead SmartHalo in any case
		if(bat_soc <= 0) {
			printf("SHUTDOWN\r\n");
			alarm_disarm();
			if (nrf_gpio_pin_read(bat_powergood_pin) != 0){
				bleapp_shutdown();
			}
		}

		bat_notify();

		if(bat_isUSBPlugged() && bat_isCharging()){
			if(bat_soc > 95){
				sch_unique_oneshot(leds_charged,1000);
			}else{
				sch_unique_oneshot(leds_charging,1000);	
			}			
		}

		//printf("bat_getCurrent_uA = %f\r\n",bat_getCurrent_uA());
		//printf("bat: %02X%02X\r\n", bat_current_buffer[1], bat_current_buffer[0]);
		/*
		printf("bat_getSOC = %f\r\n",bat_getSOC());
		printf("bat_getVoltage = %f\r\n",bat_getVoltage());
		
		printf("bat_getTemperature = %f\r\n",bat_getTemperature());
		printf("bat_get_alarm_status = %d\r\n\n",bat_get_alarm_status());
		//printf("bat_current_buffer[0] = %x, bat_current_buffer[1] = %x\r\n", bat_current_buffer[0], bat_current_buffer[1]);
		//printf("converted raw current:%x\r\n",convert8Bto16BSigned(bat_current_buffer[0], bat_current_buffer[1]));
		//printf("Mode Reg:%x\r\nCTRL Reg: %x\r\n",bat_settings_buffer[0],bat_settings_buffer[1]);
		*/
	}

	sch_unique_oneshot(bat_readData, 60*1000);

}

void bat_readData()
{
	twi_transaction_do(bat_trans_read, sizeof(bat_trans_read)/sizeof(app_twi_transfer_t), bat_twi_readData_done, NULL);
}

bool bat_isUSBPlugged() {
	return (nrf_gpio_pin_read(bat_powergood_pin) == 0);
}

bool bat_isCharging() {
	return (nrf_gpio_pin_read(USB_CHARGING_N_PIN) == 0);
}

double bat_getVoltage()
{
	return CONV_V * convert8Bto16BSigned(bat_voltage_buffer[0], bat_voltage_buffer[1]);
}

//only call in mixed mode or else it will return garbage
double bat_getCurrent_uA()
{
	//return ((CONV_I * convert8Bto16BSigned(bat_current_buffer[0], bat_current_buffer[1])) )/ RSENSE;
	int16_t val = (int16_t) ((((uint32_t)bat_current_buffer[1]) << 8) | ((uint32_t)bat_current_buffer[0]));
	return (CONV_I * (double)val ) / RSENSE;
}

double bat_getCurrent_mA()
{
	return bat_getCurrent_uA()/1000;
}

int32_t bat_getTemperature()
{
	return CONV_T * bat_temperature_buffer[0];
}

//returns alarm status : b00 alarms are not trigerred, b01 SOC low alarm, b10 voltage low alarm, b11 both alarms
uint8_t bat_get_alarm_status()
{
	return (bat_alarm_buffer[0] >> 5) & 0x03; //keep only bits 5 and 6
}

/*unsigned int convert8Bto16BUnsigned(unsigned char lowB, unsigned char highB)
{
  return (unsigned int)(((unsigned int)highB << 8) | (unsigned int)lowB);
}*/

int convert8Bto16BSigned(unsigned char lowB, unsigned char highB)
{
	return (int16_t)((((uint16_t)highB << 8) &0xFF00) | (((uint16_t)lowB)&0x00FF));
}

#endif
