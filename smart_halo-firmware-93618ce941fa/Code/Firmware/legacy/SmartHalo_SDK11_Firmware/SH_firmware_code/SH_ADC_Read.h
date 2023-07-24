#ifndef SH_ADC_Read_H_
#define SH_ADC_Read_H_

//Type SH_BATTERY_LEVELS
//
//Encapsulation of the battery current and voltage levels
//
typedef struct{

	float current;
	float voltage;
	uint8_t SOC;

}SH_Battery_Levels_t;

/*
 * SH_ADC_READ_BATTERY_LEVEL
 *
 * Samples the ADC pin until buffer of current and voltage is full
 */
void SH_adc_read_battery_level(void);

/*
 * Returns the voltage and current of the battery
 */
SH_Battery_Levels_t SH_get_battery_levels();


/*
 * Specify if the current and the voltage were sampled
 */
bool SH_sampling_battery_level_is_done();


/*
 *SAADC_INITIALIZATION
 *
 * Configures two channel of the SAADC to sample on the IBAT pin and on the VBAT pin
 *
 */
void SH_saadc_initialization(void);

#endif /* SH_ADC_Read_H_ */
