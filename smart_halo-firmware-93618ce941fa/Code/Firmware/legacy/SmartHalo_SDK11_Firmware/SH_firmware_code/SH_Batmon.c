/*
 * SH_Batmon.c
 *
 *  Created on: 2016-07-23
 *      Author: SmartHalo
 */


#include "SH_Includes.h"
#include "SH_Batmon.h"

#define TEMPMIN       	-19.95
#define TEMPMAX       	60.05
#define APAMIN			0x0000
#define APAMAX			0x00FF
#define APTMIN			0x0000
#define APTMAX			0xFFFF
#define BATPROFILEMIN	0x0000
#define BATPROFILEMAX	0x0001
#define ALARMRSOCMIN	0x0000
#define ALARMRSOCMAX	0x0064
#define ALARMCELLMIN	0.0
#define ALARMCELLMAX	65.535
#define POWERMODEMIN	0x0001
#define POWERMODEMAX	0x0002
#define STATUSBITMIN	0x0000
#define STATUSBITMAX	0x0001

#define ALARMSOC		0x0035 //(53%)
#define INITIAL_RSOC_VALUE 0xAA55

#define THERMISTOR_CONSTANT 0x0000

#define TEMP_INITIALIZATION 	25.0f
#define APA_INITIALIZATION		0x2D //2000 mAh

#define CELL_VOLTAGE_DIVISION	1000.0f
#define ITE_DIVISION			10.0f
#define TEMPERATURE_DIVISION 	10.0f
#define TEMPERATIRE_INITIAL_VALUE	273.15f

#ifdef SMARTHALO_EE
#define STATUS_BIT_FOR_INIT STATUS_BIT_I2C
#else
#define STATUS_BIT_FOR_INIT STATUS_BIT_THERMISTOR
#endif

static void Batmon_writeReg(size_t batmon_address, BatMonRegisters reg, unsigned int data);
static uint8_t crc8_ccitt_update (uint8_t inCrc, uint8_t inData);
static uint8_t computeCRC(uint8_t *msg, int length);

static void SH_Batmon_execute_rsoc_initialization();
static uint8_t Batmon_getICVersion();
static void Batmon_setCellTemperature(float cellT);
static void Batmon_setConstantofThermistor(unsigned int thermistor_constant);
static uint8_t Batmon_getAlarmLowRSOC();
static float Batmon_getAlarmLowCellVoltage();
static uint8_t Batmon_getAPA();
static void Batmon_setAPA(unsigned int APA);
static void Batmon_setPowerMode(unsigned int powermode);
static uint8_t Batmon_getPowerMode();
static uint8_t Batmon_getStatusBit();
static void Batmon_setStatusBit(unsigned int statusbit);
static uint8_t Batmon_getBatteryProfile();
static void Batmon_setBatteryProfile(unsigned int batteryprofile);

static uint8_t crc8_ccitt_update (uint8_t inCrc, uint8_t inData)
{
    uint8_t   i;
    uint8_t   data;

    data = inCrc ^ inData;

    for ( i = 0; i < 8; i++ )
    {
        if (( data & 0x80 ) != 0 )
        {
            data <<= 1;
            data ^= 0x07;
        }
        else
        {
            data <<= 1;
        }
    }
    return data;
}

static uint8_t computeCRC(uint8_t *msg, int length)
{
	uint8_t crc = 0, i;

	for (i = 0; i < length; i++)
		crc = crc8_ccitt_update(crc, msg[i]);

	return crc;
}

static void Batmon_writeReg(size_t batmon_address, BatMonRegisters reg, unsigned int data)
{
	uint8_t byteH = ((data >> 8) & 0xFF);
	uint8_t byteL = (data & 0xFF);
	uint8_t devaddress = BATMON_ADDRESS<<1;

	// Cannot use the standard 16 bit write function...needs to write a CRC
	uint8_t CRCdata[4] = {devaddress,reg,byteL,byteH};
	uint8_t CRC8 = computeCRC(CRCdata, 4);

	uint8_t regdata[3] = {byteL,byteH,CRC8};

	set_three_registers(BATMON_ADDRESS, reg, regdata);

}

void SH_Batmon_init()
{
	int8_t battery_temperature = read_battery_temperature();

	// Wake up from sleep mode
	Batmon_setPowerMode(OPERATIONAL_MODE);

	// Set fuel gage to normal operation mode
	Batmon_setPowerMode(OPERATIONAL_MODE);

	// Set battery capacity to 2000 mAh
	Batmon_setAPA(APA_INITIALIZATION);

	// Set battery profile to 3.7V/4.2V
	Batmon_setBatteryProfile(BATTERY_PROFILE1);

	SH_Batmon_execute_rsoc_initialization();

	// Set thermistor mode provided by host over I2C
	Batmon_setStatusBit(STATUS_BIT_FOR_INIT);

	if (STATUS_BIT_FOR_INIT == STATUS_BIT_I2C){
		// Set cell temperature to temperature of the accelerometer
		Batmon_setCellTemperature(battery_temperature);
	}
	else{
		Batmon_setConstantofThermistor(THERMISTOR_CONSTANT);
	}

	// Set up alarm pin
	SH_Batmon_setAlarmLowRSOC(ALARMSOC);
	SH_Gas_Gauge_INT_initialisation();

}

void SH_Batmon_disable_alarmlowrsoc(){
	SH_Batmon_setAlarmLowRSOC(ALARMRSOCMIN);
}

void SH_Batmon_test(){

	float cell_voltage;
	uint8_t rsco;
	float ite;
	uint8_t icversion;
	uint8_t alarm_low_rsoc;
	float alarm_voltage;
	uint8_t apa;
	uint8_t power_mode;
	uint8_t status_bit;
	uint8_t battery_profile;

	cell_voltage = SH_Batmon_getCellVoltage();
	rsco = SH_Batmon_getRSOC();
	ite = SH_Batmon_getITE();
	icversion = Batmon_getICVersion();
	alarm_low_rsoc = Batmon_getAlarmLowRSOC();
	alarm_voltage = Batmon_getAlarmLowCellVoltage();
	apa = Batmon_getAPA();
	power_mode = Batmon_getPowerMode();
	status_bit = Batmon_getStatusBit();
	battery_profile = Batmon_getBatteryProfile();

	 cell_voltage++;
	 rsco++;
	 ite++;
	 icversion++;
	 alarm_low_rsoc++;
	 alarm_voltage++;
	 apa++;
	 power_mode++;
	 status_bit++;
	 battery_profile++;

}

static void SH_Batmon_execute_rsoc_initialization(){
	Batmon_writeReg(BATMON_ADDRESS, INITIAL_RSOC, INITIAL_RSOC_VALUE);
}
float SH_Batmon_getCellVoltage()
{
	return (float) check_register(BATMON_ADDRESS, CELL_VOLT_REG)/CELL_VOLTAGE_DIVISION;
}

uint8_t SH_Batmon_getRSOC()
{
	return check_register(BATMON_ADDRESS, RSOC_REG);
}

float SH_Batmon_getITE()
{
	return (float) check_register(BATMON_ADDRESS, ITE_REG)/ITE_DIVISION;
}

static uint8_t Batmon_getICVersion()
{
	return check_register(BATMON_ADDRESS, IC_VERSION_REG);
}

float SH_Batmon_getCellTemperature()
{
	return ((float) check_register(BATMON_ADDRESS, CELL_TEMP_REG)/TEMPERATURE_DIVISION)-TEMPERATIRE_INITIAL_VALUE;
}

static void Batmon_setCellTemperature(float cellT)
{
	if (cellT >= TEMPMIN && cellT<= TEMPMAX){
		unsigned int cellT_int = (TEMPERATURE_DIVISION*(cellT + TEMPERATIRE_INITIAL_VALUE)); // 0x0BA6 = 25.0C
		Batmon_writeReg(BATMON_ADDRESS, CELL_TEMP_REG, cellT_int);
	}
}

static void Batmon_setConstantofThermistor(unsigned int thermistor_constant)
{

	Batmon_writeReg(BATMON_ADDRESS, THERMISTORB_REG, thermistor_constant);
}

static uint8_t Batmon_getAlarmLowRSOC()
{
	return check_register(BATMON_ADDRESS, ALARMLOWRSOC_REG);
}

void SH_Batmon_setAlarmLowRSOC(unsigned int alarmLowRSOC)
{
	if (alarmLowRSOC <= ALARMRSOCMAX && alarmLowRSOC >= ALARMRSOCMIN){
		Batmon_writeReg(BATMON_ADDRESS, ALARMLOWRSOC_REG, alarmLowRSOC);
	}
}

static float Batmon_getAlarmLowCellVoltage()
{
	return (float) (check_register(BATMON_ADDRESS, ALARMLOWCELL_REG)/CELL_VOLTAGE_DIVISION);
}

void SH_Batmon_setAlarmLowCellVoltage(float alarmLowCellVoltage)
{
	if (alarmLowCellVoltage >= ALARMCELLMIN && alarmLowCellVoltage <= ALARMCELLMAX){
		unsigned int low_cell_voltage = alarmLowCellVoltage * CELL_VOLTAGE_DIVISION;
		Batmon_writeReg(BATMON_ADDRESS, ALARMLOWCELL_REG, low_cell_voltage);
	}
}

static uint8_t Batmon_getAPA()
{
	return check_register(BATMON_ADDRESS, APA_REG);
}

static void Batmon_setAPA(unsigned int APA)
{
	if (APA <= APAMAX && APA >= APAMIN){
		Batmon_writeReg(BATMON_ADDRESS, APA_REG, APA);
	}
}

/*static unsigned int Batmon_getAPT()
{
	return check_register(BATMON_ADDRESS, APT_REG);
}

static void Batmon_setAPT(unsigned int APT)
{
	Batmon_writeReg(BATMON_ADDRESS, APT_REG, APT);
}*/

static uint8_t Batmon_getPowerMode()
{
	return check_register(BATMON_ADDRESS, ICPOWERMODE_REG);
}

static void Batmon_setPowerMode(unsigned int powermode)
{
	if (powermode <= POWERMODEMAX && powermode >= POWERMODEMIN)
	Batmon_writeReg(BATMON_ADDRESS, ICPOWERMODE_REG, powermode);
}

static uint8_t Batmon_getStatusBit()
{
	return check_register(BATMON_ADDRESS,STATUSBIT_REG);
}

static void Batmon_setStatusBit(unsigned int statusbit)
{
	if (statusbit <= STATUSBITMAX &&& statusbit >= STATUSBITMIN){
		Batmon_writeReg(BATMON_ADDRESS, STATUSBIT_REG, statusbit);
	}
}

static uint8_t Batmon_getBatteryProfile()
{
	return check_register(BATMON_ADDRESS, BAT_PROFILE_R_REG);
}

static void Batmon_setBatteryProfile(unsigned int batteryprofile)
{
	if (batteryprofile <= BATPROFILEMAX && batteryprofile >= BATPROFILEMIN){
		Batmon_writeReg(BATMON_ADDRESS, BAT_PROFILE_RW_REG, batteryprofile);
	}
}

