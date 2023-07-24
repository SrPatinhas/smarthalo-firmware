
#include "stdafx.h"
#include "SH_Primary_state_machine.h"
#include "SH_Piezo_sounds.h"
#include "SH_typedefs.h"
#include "app_error.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "SH_Accelerometer_Magnetometer.h"
#include	<math.h>
#include "SH_TWI.h"
#include "SH_pinMap.h"
#include "app_pwm.h"




//Event handler for the timer of the time that the piezo is HIGH
static void piezo_ON_timer_handler(void* p_context);

//Event handler for the timer of the time that the piezo is LOW
static void piezo_OFF_timer_handler(void* p_context);

//Event handler for the timer of the time that the alarm is ON
static void  piezo_stop_alarm_timer_handler (void* p_context);

//Event handler for the timer of the time that the alarm is ON
static void  piezo_chirp_timer_handler (void* p_context);

//Sends the start bit to the easyscale
static void EasyScale_start_bit();

//Sends the data to the easyscale
static void EasyScale_send_piezo_data();

//Sends the easyscale address
static void EasyScale_send_piezo_address();

//High bit for the easyscale
static void EasyScale_send_high_bit();

//Low bit for the easyscale
static void EasyScale_send_low_bit();

//Sends the stop bit to the easy scale
static void EasyScale_stop_bit();

//Stop the piezo
static void stop_piezo();

//plays the sms tone
static void SMS_sound();

//plays the warning sound
static void warning_sound();


//plays the turn notification sound
static void turn_notification_sound();

static void turn_succesful_sound();

static void phonecall_sound();

static void alarm_sound();

//update the alarm
//static void piezo_update();

/*
 * Plays the alarm depending on the defined soundmode
 */

//Chirp log and lin
static void Chirp(int f_min, int f_max, int numpts, int duration_ms, bool logmode);
//Dualtone
static void DualTone(int f_min, int f_max, int duration_ms);
//Single tone
static void Tone(int f);

static uint8_t volume_of_piezo = 0b11111;
static bool high_frequency_for_dual_tone_on = false;
static uint8_t sound_type;
//static bool tone_is_done = true;
static uint16_t number_of_tone_lin_called = 0;
static uint16_t number_of_tone_log_called = 0;
static bool begun = false;
static uint16_t time_on;
static uint16_t time_off;
//static uint16_t duration_time;
static bool tone_duration_for_chirp = false;
static bool tone_for_chirp_is_done = true;

bool first_call = true;

//used to count how many tones to play
uint8_t timer_loop_counter = 0;

int tone_f           = F_DEF;
int chirp_min_f      = FMIN_DEF;
int chirp_max_f      = FMAX_DEF;
int num_pts          = N_DEF;
int duration_ms      = DURATION_DEF;

void* p_context;

APP_TIMER_DEF(piezo_ON_timer_id);
APP_TIMER_DEF(piezo_OFF_timer_id);
APP_TIMER_DEF(piezo_alarm_duration_timer_id);
APP_TIMER_DEF(piezo_chirp_duration_timer_id);


static volatile bool ready_flag;            // A flag indicating PWM status.
//app_pwm_config_t pwm_piezo_cfg;
APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

static void piezo_pwm_ready_callback(uint32_t pwm_id)   // PWM callback function
{
	//low_power_pwm_t * pwm_instance = (low_power_pwm_t*)p_context;
	ready_flag = true;
}
//Initialization of the piezo
void piezo_init()
{
	if (!begun){
		uint8_t err_code = 0;

		nrf_gpio_cfg_output(PIEZO_DRIVE_PIN);
		nrf_gpio_cfg_output(PIEZO_VOLUME_PIN);

		//ret_code_t err_code;

		/* 1-channel PWM, 2800Hz*/
		app_pwm_config_t pwm_piezo_cfg = APP_PWM_DEFAULT_CONFIG_1CH(357L, PIEZO_DRIVE_PIN);

	   // pwm_piezo_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;

    	/* Initialize and enable PWM. */
    	err_code = app_pwm_init(&PWM1,&pwm_piezo_cfg, piezo_pwm_ready_callback);
    	APP_ERROR_CHECK(err_code);
    	app_pwm_enable(&PWM1);
    	app_pwm_channel_duty_set(&PWM1, 0, 0);


		err_code = app_timer_create(&piezo_ON_timer_id,APP_TIMER_MODE_SINGLE_SHOT,piezo_ON_timer_handler);
		APP_ERROR_CHECK(err_code);

		err_code = app_timer_create(&piezo_OFF_timer_id,APP_TIMER_MODE_SINGLE_SHOT,piezo_OFF_timer_handler);
		APP_ERROR_CHECK(err_code);

		err_code = app_timer_create(&piezo_alarm_duration_timer_id,APP_TIMER_MODE_SINGLE_SHOT,piezo_stop_alarm_timer_handler);
		APP_ERROR_CHECK(err_code);

		err_code = app_timer_create(&piezo_chirp_duration_timer_id,APP_TIMER_MODE_SINGLE_SHOT, piezo_chirp_timer_handler);
		APP_ERROR_CHECK(err_code);

		piezo_modify_sound(TONE);

		begun = true;
	}

}

void pwm_change_period(uint16_t period_in_us)
{

app_pwm_disable(&PWM1);
app_pwm_uninit(&PWM1);
uint8_t err_code = 0;

 app_pwm_config_t pwm_piezo_cfg = APP_PWM_DEFAULT_CONFIG_1CH(period_in_us, PIEZO_DRIVE_PIN);

 /* Switch the polarity of the second channel. */
 //pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;

 /* Initialize and enable PWM. */
 err_code = app_pwm_init(&PWM1,&pwm_piezo_cfg,piezo_pwm_ready_callback);
 APP_ERROR_CHECK(err_code);
 app_pwm_enable(&PWM1);

 }


void play_sound(SOUNDMODE new_sound)
{
	piezo_modify_sound(new_sound);
	switch(sound_type)
	{
		case CHIRPLIN :
			Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, false);
		break;

		case DUALTONE :
			DualTone(chirp_min_f, chirp_max_f, duration_ms);
		break;

		case CHIRPLOG :
			Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, true);
		break;

		case TONE :
			Tone(tone_f);
		break;

		case WARNING_SOUND:
			warning_sound();
		break;

		case SMS_SOUND :
			SMS_sound();
		break;

		case TURN_NOTIFICATION_SOUND :
			turn_notification_sound();
		break;

		case TURN_SUCCESFUL_SOUND :
			turn_succesful_sound();
		break;

		case PHONECALL_SOUND :
			phonecall_sound();
		break;

		case ALARM_SOUND:
			alarm_sound();
		break;

		case SOUND_OFF :
			piezo_alarm_stop();
			number_of_tone_lin_called = 0;
			number_of_tone_log_called = 0;
			high_frequency_for_dual_tone_on = false;
			first_call = true;
		break;

		default:
		break;
	}
}

bool piezo_alarm_start()
{
	play_sound(ALARM_SOUND);
	return true;

}

void piezo_alarm_stop()
{
	CRITICAL_REGION_ENTER();//regions so taht interrupt cannot restart the sounds in between function calls
	app_timer_stop(piezo_ON_timer_id);
	app_timer_stop(piezo_OFF_timer_id);
	CRITICAL_REGION_EXIT();
	app_pwm_channel_duty_set(&PWM1, 0, 0);
}

void piezo_modify_sound (SOUNDMODE new_sound){
	sound_type = new_sound;
	first_call = true;
}

SOUNDMODE piezo_get_current_sound (){
	return sound_type;
}

void piezo_modify_volume (uint8_t new_volume){
	volume_of_piezo =  new_volume;
}

uint8_t piezo_get_current_volume(){
	return volume_of_piezo;
}

//static void piezo_update(){
//
//	//this function will call an update for the current piezo alarm sound
//	if (sound_type == DUALTONE){
//		high_frequency_for_dual_tone_on = !high_frequency_for_dual_tone_on;
//	}
//
//	else if (sound_type == CHIRPLIN){
//		if (!tone_duration_for_chirp){
//			number_of_tone_lin_called++;
//		}
//		else if (tone_duration_for_chirp && tone_for_chirp_is_done){
//			number_of_tone_lin_called++;
//		}
//	}
//
//	else if (sound_type == CHIRPLOG){
//		if (!tone_duration_for_chirp){
//			number_of_tone_log_called++;
//		}
//		else if (tone_duration_for_chirp && tone_for_chirp_is_done){
//			number_of_tone_log_called++;
//		}
//	}
//
//	piezo_alarm_start();
//}

void piezo_set_volume()
{
	//configure the easyScale protocol to output given volume
	//Start bit
	EasyScale_start_bit();

	//Address
	EasyScale_send_piezo_address();

	//Stop bit
	EasyScale_stop_bit();

	//Start bit
	EasyScale_start_bit();

	//RFA bit (If high, acknowledge is applied by device)
	EasyScale_send_low_bit();

	//Data adress
	EasyScale_send_low_bit();
	EasyScale_send_low_bit();

	//Data for volume
	EasyScale_send_piezo_data();

	//Stop bit
	EasyScale_stop_bit();

	//Static high
	nrf_gpio_pin_set(PIEZO_VOLUME_PIN);
}

static void stop_piezo(){

	app_timer_stop(piezo_ON_timer_id);
	app_timer_stop(piezo_OFF_timer_id);
	nrf_gpio_pin_clear(PIEZO_DRIVE_PIN);
}

static void piezo_stop_alarm_timer_handler(void* p_context){
	piezo_modify_sound(SOUND_OFF);
	stop_piezo();
}

static void piezo_ON_timer_handler(void* p_context){
	switch(sound_type)
	{
		case CHIRPLIN :
			//Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, false);
		break;

		case DUALTONE :
			//DualTone(chirp_min_f, chirp_max_f, duration_ms);
		break;

		case CHIRPLOG :
			//Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, true);
		break;

		case TONE :
			//Tone(tone_f);
		break;

		case WARNING_SOUND:
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(DO2_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;


		case SMS_SOUND :
			if(timer_loop_counter > 1)// stops the sounds if the sound is done
			{
				timer_loop_counter =0;
				app_pwm_channel_duty_set(&PWM1, 0, 0);
				break;
			}
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(SOL3_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;


		case TURN_NOTIFICATION_SOUND :
			if(timer_loop_counter > 1)// stops the sounds if the sound is done
			{
				timer_loop_counter =0;
				app_pwm_channel_duty_set(&PWM1, 0, 0);
				break;
			}
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(LA3_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;

		case TURN_SUCCESFUL_SOUND :
			if(timer_loop_counter > 1)// stops the sounds if the sound is done
			{
				timer_loop_counter =0;
				app_pwm_channel_duty_set(&PWM1, 0, 0);
				break;
			}
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(DO4_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;

		case PHONECALL_SOUND :
			if(timer_loop_counter > 10)// stops the sounds if the sound is done
			{
				timer_loop_counter =0;
				app_pwm_channel_duty_set(&PWM1, 0, 0);
				break;
			}
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(LA4_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;

		case ALARM_SOUND:
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(DO2_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;

		case SOUND_OFF :
			piezo_alarm_stop();
			number_of_tone_lin_called = 0;
			number_of_tone_log_called = 0;
			high_frequency_for_dual_tone_on = false;
			first_call = true;
		break;

		default:
		break;
	}
	timer_loop_counter++;
}

static void piezo_OFF_timer_handler(void* p_context){
	switch(sound_type)
	{
		case CHIRPLIN :
			//Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, false);
		break;

		case DUALTONE :
			//DualTone(chirp_min_f, chirp_max_f, duration_ms);
		break;

		case CHIRPLOG :
			//Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, true);
		break;

		case TONE :
			//Tone(tone_f);
		break;


		case WARNING_SOUND:
			timer_loop_counter =0;
			app_pwm_channel_duty_set(&PWM1, 0, 0);
		break;


		case SMS_SOUND :
			app_timer_start(piezo_ON_timer_id, time_on,p_context);
			pwm_change_period(DO4_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;


		case TURN_NOTIFICATION_SOUND :
			timer_loop_counter =0;
			app_pwm_channel_duty_set(&PWM1, 0, 0);
		break;

		case TURN_SUCCESFUL_SOUND :
			timer_loop_counter =0;
			app_pwm_channel_duty_set(&PWM1, 0, 0);
		break;

		case PHONECALL_SOUND :
			app_timer_start(piezo_ON_timer_id, time_on,p_context);
			pwm_change_period(DO4_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;

		case ALARM_SOUND:
			app_timer_start(piezo_OFF_timer_id,time_off,p_context);
			pwm_change_period(DO5_PERIOD);
			app_pwm_channel_duty_set(&PWM1, 0, 50);
		break;

		case SOUND_OFF :
			piezo_alarm_stop();
			number_of_tone_lin_called = 0;
			number_of_tone_log_called = 0;
			high_frequency_for_dual_tone_on = false;
			first_call = true;
		break;

		default:
		break;
	}
	timer_loop_counter++;
}

static void piezo_chirp_timer_handler (void* p_context){
	//tone_for_chirp_is_done = true;
	//piezo_update();
}

//Chirp
static void Chirp(int f_min, int f_max, int numpts, int duration_ms, bool logmode)
{
	double chirp_min_f  = 0;//logmode?log10((double)f_min):f_min;
	double chirp_max_f  = 0;//logmode?log10((double)f_max):f_max;
	double chirp_range  = chirp_max_f - chirp_min_f;
	double chirp_f_inc  = chirp_range/num_pts;
	uint8_t toneduration_ms =0;// round((double)duration_ms/(double)num_pts);

	static double f_log = 0;
	static double f_lin = 0;

  	  if (first_call && duration_ms>0){
  		first_call = false;
  		app_timer_start(piezo_alarm_duration_timer_id, APP_TIMER_TICKS(duration_ms, APP_TIMER_PRESCALER) ,p_context);
  	  }

  	  if (toneduration_ms > 0) {
  		  tone_duration_for_chirp = true;
  		  	  if (tone_for_chirp_is_done){

  		  		  f_log += chirp_f_inc;

  		  		  if ((number_of_tone_lin_called || number_of_tone_log_called) > 0){
  		  			  f_lin = 0;//logmode?pow(10,f_log):(f_lin+chirp_f_inc);
  		  		  }

  		  		  else {
  		  			  f_lin = f_log;
  		  		  }
  		  		  tone_for_chirp_is_done = false;
  		  		  app_timer_start(piezo_chirp_duration_timer_id, APP_TIMER_TICKS(toneduration_ms, APP_TIMER_PRESCALER) ,p_context);
  		  	  }
  	  }

  	  else {
  		  tone_duration_for_chirp = false;
  		  f_log += chirp_f_inc;

  		  if ((number_of_tone_lin_called || number_of_tone_log_called) > 0){
  			  f_lin = 0;//logmode?pow(10,f_log):(f_lin+chirp_f_inc);
  		  }

  		  else {
  			  f_lin = f_log;
  		  }
  	  }

  	  time_on = HIGH_TIME_IN_MS_WITH_FREQUENCY(f_lin);
  	  time_off = HIGH_TIME_IN_MS_WITH_FREQUENCY(f_lin);

  	  app_timer_start(piezo_ON_timer_id,APP_TIMER_TICKS(time_on, APP_TIMER_PRESCALER),p_context);
  	  nrf_gpio_pin_set(PIEZO_DRIVE_PIN);
}

//Dual Tone
static void DualTone(int f_min, int f_max, int duration_ms)
{

	if (!high_frequency_for_dual_tone_on){
		time_on = HIGH_TIME_IN_MS_WITH_FREQUENCY(f_min);
		time_off = HIGH_TIME_IN_MS_WITH_FREQUENCY(f_min);
	}
	else {
		time_on = HIGH_TIME_IN_MS_WITH_FREQUENCY(f_max);
		time_off = HIGH_TIME_IN_MS_WITH_FREQUENCY(f_max);
	}
	 if (first_call && duration_ms>0){
		first_call = false;
		app_timer_start(piezo_alarm_duration_timer_id, APP_TIMER_TICKS(duration_ms, APP_TIMER_PRESCALER),p_context);
	 }

	app_timer_start(piezo_ON_timer_id, APP_TIMER_TICKS(time_on, APP_TIMER_PRESCALER), p_context);
	nrf_gpio_pin_set(PIEZO_DRIVE_PIN);
}

//Tone
static void Tone(int f)
{

	//time_on = ;
	//time_off = ;

	//low_power_pwm_piezo_config.period = UINT8_MAX;

	time_on = 3200; //APP_TIMER_TICKS(HIGH_TIME_IN_MS_WITH_FREQUENCY(f), APP_TIMER_PRESCALER);
	time_off = 3200; //APP_TIMER_TICKS(HIGH_TIME_IN_MS_WITH_FREQUENCY(f), APP_TIMER_PRESCALER);

	app_timer_start(piezo_ON_timer_id, time_on, p_context);

	app_pwm_channel_duty_set(&PWM1, 0, 50);

	//nrf_gpio_pin_set(PIEZO_DRIVE_PIN);
}



static void SMS_sound() //do sol do(+1 octave)
{
	piezo_modify_sound(SMS_SOUND);
	//@@@ modify volume here
	timer_loop_counter =0;
	time_on = 5000;
	time_off = 5000;

	pwm_change_period(DO3_PERIOD);
	app_pwm_channel_duty_set(&PWM1, 0, 50);
	app_timer_start(piezo_ON_timer_id, time_on, p_context);


}


static void warning_sound() // sol do
{
	piezo_modify_sound(WARNING_SOUND);
	//@@@ modify volume here
	timer_loop_counter =0;
	time_on = 5000;
	time_off = 5000;

	pwm_change_period(SOL2_PERIOD);
	app_pwm_channel_duty_set(&PWM1, 0, 50);
	app_timer_start(piezo_ON_timer_id, time_on, p_context);

}

static void turn_notification_sound() //la si
{
	piezo_modify_sound(TURN_NOTIFICATION_SOUND);
	//@@@ modify volume here
	timer_loop_counter =0;
	time_on = 5000;
	time_off = 5000;

	pwm_change_period(SI3_PERIOD);
	app_pwm_channel_duty_set(&PWM1, 0, 50);
	app_timer_start(piezo_ON_timer_id, time_on, p_context);
}


static void turn_succesful_sound() //do, do(+1 octave)
{
	piezo_modify_sound(TURN_SUCCESFUL_SOUND);
	//@@@ modify volume here
	timer_loop_counter =0;
	time_on = 5000;
	time_off = 5000;

	pwm_change_period(DO3_PERIOD);
	app_pwm_channel_duty_set(&PWM1, 0, 50);
	app_timer_start(piezo_ON_timer_id, time_on, p_context);
}


static void phonecall_sound() //do, la, do, la, do, la
{
	piezo_modify_sound(PHONECALL_SOUND);
	//@@@ modify volume here
	timer_loop_counter =0;
	time_on = 2000;
	time_off = 2000;

	pwm_change_period(DO4_PERIOD);
	app_pwm_channel_duty_set(&PWM1, 0, 50);
	app_timer_start(piezo_ON_timer_id, time_on, p_context);
}

static void alarm_sound()
{
	piezo_modify_sound(ALARM_SOUND);
	//@@@ modify volume here
	timer_loop_counter =0;
	time_on = 3000;
	time_off = 3000;

	pwm_change_period(DO4_PERIOD);
	app_pwm_channel_duty_set(&PWM1, 0, 50);
	app_timer_start(piezo_ON_timer_id, time_on, p_context);
}



//Easy scale process
static void EasyScale_start_bit(){
	nrf_gpio_pin_set(PIEZO_VOLUME_PIN);
	nrf_delay_us(TIME_FOR_START_BIT);
}

static void EasyScale_stop_bit(){
	nrf_gpio_pin_clear(PIEZO_VOLUME_PIN);
	nrf_delay_us(TIME_FOR_END_BIT);
}

static void EasyScale_send_low_bit(){
	nrf_gpio_pin_clear(PIEZO_VOLUME_PIN);
	nrf_delay_us(TIME_FOR_LOW_FOR_LOW_BIT);
	nrf_gpio_pin_set(PIEZO_VOLUME_PIN);
	nrf_delay_us(TIME_FOR_HIGH_FOR_LOW_BIT);
}

static void EasyScale_send_high_bit(){
	nrf_gpio_pin_clear(PIEZO_VOLUME_PIN);
	nrf_delay_us(TIME_FOR_LOW_FOR_HIGH_BIT);
	nrf_gpio_pin_set(PIEZO_VOLUME_PIN);
	nrf_delay_us(TIME_FOR_HIGH_FOR_HIGH_BIT);
}

static void EasyScale_send_piezo_address(){

	for (uint8_t i=0; i<NUMBER_OF_BITS_ADDRESS; i++){
		if (CHECK_BIT(VOLUME_CHIP_ADDRESS, i)){
			EasyScale_send_high_bit();
		}
		else{
			EasyScale_send_low_bit();
		}
	}
}

static void EasyScale_send_piezo_data(){

	for (uint8_t i=0; i <NUMBER_OF_BITS_DATA_FOR_VOLUME; i++){
		if (CHECK_BIT(volume_of_piezo, i)){
			EasyScale_send_high_bit();
		}
		else {
			EasyScale_send_low_bit();
		}
	}
}























