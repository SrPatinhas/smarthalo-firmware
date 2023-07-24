



#include "SH_Includes.h"
#include "Batmon.h"


#define ONE_HOUR_COUNT 							3600
#define USB_CONNECTED_PIN 						USB_POWERGOOD_N_PIN  //this pin has inverted logic!

extern bool uart_get_ready_flag;

#define STR_BUF_SIZE 50
typedef struct factory_test_message{
	uint8_t command[STR_BUF_SIZE];
	uint8_t parameter1[STR_BUF_SIZE];
	uint8_t parameter2[STR_BUF_SIZE];
	uint8_t parameter3[STR_BUF_SIZE];
	uint8_t parameter4[STR_BUF_SIZE];
	uint8_t parameter5[STR_BUF_SIZE];
	uint8_t parameter6[STR_BUF_SIZE];
}factory_test_message_t;
int center_red_intensity ;
int center_green_intensity ;
int center_blue_intensity ;

enum factory_test_message_parse_enum{FT_command=0,FT_parameter1,FT_parameter2,FT_parameter3,FT_parameter4,FT_parameter5,FT_parameter6};

static int8_t rssi_DBm =-100;
static TXPOWER 							txPowerdBm = POWER_0dBm;

static void factory_handler(factory_test_message_t parsed_message);

static factory_test_message_t factory_calls_parser();


void factory_test_mode()
{
	factory_test_message_t current_message;

	while(1)//a message must be sent to exit factory mode by system reset
	{
		current_message = factory_calls_parser();
		factory_handler(current_message);
		NRF_WDT->RR[0] = WDT_RR_RR_Reload;
	}

}


static factory_test_message_t factory_calls_parser()
{
/*Command syntax
The command string is contained within quotes (â€œâ€�) and should not exceed 255 characters:

â€œCommandName <parameter1> <parameter2> [optional parameter3] ...\r\nâ€�

Carriage return, line feed (\r\n) delimits the commands. Special characters are escaped with \.
Special characters are used for control purposes (i.e. in loopback test to start or stop the test).
Mandatory parameters or return values are delimited by <>, while optional ones are delimited by [].*/

	uint8_t str_to_parse[255];
	uint8_t index_counter = 0;
	factory_test_message_t parsed_message;

	//reallocate values to 0 because of memory conservation of the buffers
	memset(parsed_message.command,0,STR_BUF_SIZE);
	memset(parsed_message.parameter1,0,STR_BUF_SIZE);
	memset(parsed_message.parameter2,0,STR_BUF_SIZE);
	memset(parsed_message.parameter3,0,STR_BUF_SIZE);
	memset(parsed_message.parameter4,0,STR_BUF_SIZE);
	memset(parsed_message.parameter5,0,STR_BUF_SIZE);
	memset(parsed_message.parameter6,0,STR_BUF_SIZE);
	uint8_t parameter_counter= 0;
			//parameter_counter++;
			//parsed_message.command = &(parameter_counter); //garbage code
	bool exit_condition = false;
	while(!exit_condition)//str_to_parse[index_counter - 1] != 10) //while != "\n" in ascii
	{
		//sd_app_evt_wait();
		NRF_WDT->RR[0] = WDT_RR_RR_Reload;
		if(index_counter>=255)
		{
			break;
		}

		if(uart_get_ready_flag | (app_uart_get_nb_rx_fifo_chars() != 0))
		{//these two if() statements could have been combined, but the code is executed quicker if they are not
			if(app_uart_get(&str_to_parse[index_counter])==NRF_SUCCESS)
			{
				uart_get_ready_flag = false;
				if(str_to_parse[index_counter] == 10)
				{
					exit_condition = true; //if \n
				}
				//((str_to_parse[index_counter] == " ")||(str_to_parse[index_counter] == "\r")||(str_to_parse[index_counter] != "\n"))
				if((str_to_parse[index_counter] == 32)||(str_to_parse[index_counter] == 13))//||(str_to_parse[index_counter] == 10))
				{
					if(str_to_parse != NULL)
					{
						if(index_counter >= STR_BUF_SIZE)//prevents memcpy overflow
						{
							break;
						}
						switch (parameter_counter)
						{
							case FT_command:
								memcpy(parsed_message.command, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							case FT_parameter1:
								memcpy(parsed_message.parameter1, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							case FT_parameter2:
								memcpy(parsed_message.parameter2, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							case FT_parameter3:
								memcpy(parsed_message.parameter3, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							case FT_parameter4:
								memcpy(parsed_message.parameter4, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							case FT_parameter5:
								memcpy(parsed_message.parameter5, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							case FT_parameter6:
								memcpy(parsed_message.parameter6, str_to_parse, index_counter);
								index_counter = 0;
								++parameter_counter;
								break;
							default:
								break;
						}
					}
				}
				else
				{
					++index_counter;
				}
			}
		}
	}
	app_uart_flush();
	return parsed_message;
}




static void factory_handler(factory_test_message_t parsed_message)
{

	/*Command syntax
	The command string is contained within quotes (â€œâ€�) and should not exceed 255 characters:

	â€œCommandName <parameter1> <parameter2> [optional parameter3] ...\r\nâ€�

	Carriage return, line feed (\r\n) delimits the commands. Special characters are escaped with \.
	Special characters are used for control purposes (i.e. in loopback test to start or stop the test).
	Mandatory parameters or return values are delimited by <>, while optional ones are delimited by [].*/

	//these hold the pointers to the strings
	char* factory_commands_strings[NUMBER_OF_COMMANDS];
	char* factory_parameters1_strings[NUMBER_OF_PARAMETERS];

//this will load all the messages into ram, very memory hungry
////	/* UART/General device commands */
	//â€�GetDeviceSerialNumber <component>\r\nâ€�

	char  GEN_GetSerialNumber[]							= "GetSerialNumber";
	factory_commands_strings[GetSerialNumber] 			= GEN_GetSerialNumber;
			char component_lock[]						= "lock";
			factory_parameters1_strings[lock] 			= component_lock;
			char component_key[]						= "key";
			factory_parameters1_strings[key] 			= component_key;
			char component_PCBA[]						= "PCBA";
			factory_parameters1_strings[PCBA] 			= component_PCBA;
			char component_product[]					= "product";
			factory_parameters1_strings[product] 		= component_product;

	char  GEN_SetSerialNumber[] 						= "SetSerialNumber";
	factory_commands_strings[SetSerialNumber] 			= GEN_SetSerialNumber;
			//uint8_t component_lock[]					= "lock ";
			//uint8_t component_key[]					= "key "; //same component strings
			//uint8_t component_PCBA[]					= "PCBA ";
			//uint8_t component_product[]				= "product ";

	char  GEN_GetBootloaderVersion[] 					= "GetBootloaderVersion";
	factory_commands_strings[GetBootloaderVersion]		= GEN_GetBootloaderVersion;
	char  GEN_GetFirmwareVersion[] 						= "GetFirmwareVersion";
	factory_commands_strings[GetFirmwareVersion] 		= GEN_GetFirmwareVersion;
	char  GEN_GetSoftdeviceVersion[] 					= "GetSoftdeviceVersion";
	factory_commands_strings[GetSoftdeviceVersion] 		= GEN_GetSoftdeviceVersion;
	char  GEN_LoopbackTest[] 							= "LoopbackTest";
	factory_commands_strings[LoopbackTest] 				= GEN_LoopbackTest;
			char interface_USB[]						= "USB";
			factory_parameters1_strings[USB_interface] 	= interface_USB;
			char interface_BLE[]						= "BLE";
			factory_parameters1_strings[BLE_interface] 	= interface_BLE;
			char interface_ANT[]						= "ANT ";//currently unimplemented
			factory_parameters1_strings[ANT_interface]  = interface_ANT;

//	Bluetooth radio
	char  BT_SetTxPower[] 								= "SetTxPower";
	factory_commands_strings[SetTxPower] 				= BT_SetTxPower;
	char  BT_GetTxPower[] 								= "GetTxPower";
	factory_commands_strings[GetTxPower] 				= BT_GetTxPower;
	char  BT_GetRSSI[] 									= "GetRSSI";
	factory_commands_strings[GetRSSI] 					= BT_GetRSSI;
	char  BT_PairingTest[] 								= "PairingTest";
	factory_commands_strings[PairingTest] 				= BT_PairingTest;

//	Halo LED and Central LED
	char  HaloLed_SetLedColor[] 						= "SetLedColor";
	factory_commands_strings[SetLedColor] 				= HaloLed_SetLedColor;
	char  HaloLed_TurnLedOn[] 							= "TurnLedOn";
	factory_commands_strings[TurnLedOn] 				= HaloLed_TurnLedOn;

//	Front LED
	char  FrLed_SetFrontLedIntensity[] 					= "SetFrontLedIntensity";
	factory_commands_strings[SetFrontLedIntensity] 		= FrLed_SetFrontLedIntensity;

//	Touch
	char  Touch_HeartbeatTest[] 						= "HeartbeatTest";
	factory_commands_strings[HeartbeatTest] 			= Touch_HeartbeatTest;
	char  Touch_SetTouchTarget[] 						= "SetTouchTarget";
	factory_commands_strings[SetTouchTarget] 			= Touch_SetTouchTarget;
			char location_front[]						= "front";
			factory_parameters1_strings[front]		 	= location_front;
			char location_back[]						= "back";
			factory_parameters1_strings[back] 			= location_back;
			char location_left[]						= "left";
			factory_parameters1_strings[left] 			= location_left;
			char location_right[]						= "right";
			factory_parameters1_strings[right] 			= location_right;

//	Accelerometer/Magnetometer
	char  A_M_Selftest[] 								= "Selftest";
	factory_commands_strings[Selftest] 					= A_M_Selftest;
			char device_acc[]							= "acc";
			factory_parameters1_strings[acc] 			= device_acc;
			char device_mag[]							= "mag";
			factory_parameters1_strings[mag] 			= device_mag;

	char  A_M_GetAccelerationXYZ[] 						= "GetAccelerationXYZ";
	factory_commands_strings[GetAccelerationXYZ] 		= A_M_GetAccelerationXYZ;
	char  A_M_FreefallTest[] 							= "FreefallTest";
	factory_commands_strings[FreefallTest] 				= A_M_FreefallTest;
	char  A_M_ActivityTriggerTest[] 					= "ActivityTriggerTest";
	factory_commands_strings[ActivityTriggerTest] 		= A_M_ActivityTriggerTest;
	char  A_M_GetMagnetometerXYZ[] 						= "GetMagnetometerXYZ";
	factory_commands_strings[GetMagnetometerXYZ] 		= A_M_GetMagnetometerXYZ;
	char  A_M_GetMaxAccelerationXYZ[] 					= "GetMaxAccelerationXYZ";
	factory_commands_strings[GetMaxAccelerationXYZ]		= A_M_GetMaxAccelerationXYZ;
	char  A_M_GetMaxMagnetometerXYZ[] 					= "GetMaxMagnetometerXYZ";
	factory_commands_strings[GetMaxMagnetometerXYZ] 	= A_M_GetMaxMagnetometerXYZ;
	char  A_M_SetMagnetometerCalibrationOffset[] 		= "SetMagnetometerCalibrationOffset";
	factory_commands_strings[SetMagnetometerCalibrationOffset] = A_M_SetMagnetometerCalibrationOffset;
	char  A_M_GetCompassHeading[] 						= "GetCompassHeading";
	factory_commands_strings[GetCompassHeading] 		= A_M_GetCompassHeading;

//	Battery and power supply
	char  PS_GetBatteryVoltage[] 						= "GetBatteryVoltage";
	factory_commands_strings[GetBatteryVoltage] 		= PS_GetBatteryVoltage;
	char  PS_GetBatterySOC[] 							= "GetBatterySOC";
	factory_commands_strings[GetBatterySOC] 			= PS_GetBatterySOC;
	char  PS_GetChargeCurrent[] 						= "GetChargeCurrent";
	factory_commands_strings[GetChargeCurrent] 			= PS_GetChargeCurrent;
	char  PS_GetRailVoltage[] 							= "GetRailVoltage";
	factory_commands_strings[GetRailVoltage] 			= PS_GetRailVoltage;
				char VLED_rail_p[]						= "VLED_rail";
				factory_parameters1_strings[VLED_rail] 	= VLED_rail_p;
				char TOUCH_rail_p[]						= "TOUCH_rail";
				factory_parameters1_strings[TOUCH_rail] = TOUCH_rail_p;
				char VPIEZO_rail_p[]					= "VPIEZO_rail";
				factory_parameters1_strings[VPIEZO_rail]= VPIEZO_rail_p;
				char VBAT_rail_p[]						= "VBAT_rail";
				factory_parameters1_strings[VBAT_rail] 	= VBAT_rail_p;

//	Piezo buzzer
	char  piezoPlay[] 									= "Play";
	factory_commands_strings[Play] 						= piezoPlay;
				char piezoChirp[]						= "Chirp";
				factory_parameters1_strings[p_Chirp]	= piezoChirp;
				char piezoOff[]							= "off";
				factory_parameters1_strings[off]	 	= piezoOff;

//	Exit Conditions
	char  EC_ExitFactoryMode[]							= "ExitFactoryMode";
	factory_commands_strings[ExitFactoryMode] 			= EC_ExitFactoryMode;
	char  EC_ExitIntoShippingMode[]						= "ExitIntoShippingMode";
	factory_commands_strings[ExitIntoShippingMode] 		= EC_ExitIntoShippingMode;

//// miscelaneous
	uint8_t endLine[]									= "\r\n";
//
//
//// errors
	uint8_t unknownCommand[]							= "Unknown Command, check syntax \r\n";
	uint8_t unknownParameter[]							= "Unknown Parameter, check syntax \r\n";
//
////answers
	uint8_t answer_OK[] 								= "OK \r\n";
	uint8_t answer_ERROR[] 								= "ERROR \r\n";
	uint8_t answer_DONE[] 								= "DONE \r\n";


//	make structs for commands and for parameters

	uint8_t command_comparison = 255; // 255 value that is never used in the below switch case
	uint8_t parameter1_comparison = 255;

	NRF_WDT->RR[0] = WDT_RR_RR_Reload;

	char* casted_string = (char* )parsed_message.command;
	for(uint8_t iter =0 ; iter < NUMBER_OF_COMMANDS; ++iter)
	{
		if((factory_commands_strings[iter] != NULL) && (casted_string != NULL))
		{
			if(strcmp(casted_string,factory_commands_strings[iter]) == 0) //if equal
			{
				command_comparison = iter;
				break;
			}
		}
	}

	if(parsed_message.parameter1 != NULL)
	{
		casted_string = (char* )parsed_message.parameter1;
		for(uint8_t iter =0 ; iter < NUMBER_OF_PARAMETERS; ++iter)
		{
			if((factory_commands_strings[iter] != NULL) && (casted_string != NULL))
			{
				if(strcmp(casted_string,factory_parameters1_strings[iter]) == 0)//if equal
				{
					parameter1_comparison = iter;
					break;
				}
			}
		}
	}

	NRF_WDT->RR[0] = WDT_RR_RR_Reload;
	//char uart_answer[STR_BUF_SIZE] = {0};

	switch (command_comparison)
	{
		case(GetSerialNumber):
		{

			switch (parameter1_comparison)
			{

				case(lock):
				{
					int temp = read_to_UICR(UICR_LOCK_SERIAL_NUMBER_ADDRESS);
					printf("%d",temp);
					printf("%s",endLine);
					break;
				}
				case(key):
				{
					int temp = read_to_UICR(UICR_KEY_SERIAL_NUMBER_ADDRESS);
					printf("%d",temp);
					printf("%s",endLine);
					break;
				}
				case(PCBA):
				{
					int temp = read_to_UICR(UICR_PCBA_SERIAL_NUMBER_ADDRESS);
					printf("%d",temp);
					printf("%s",endLine);
					break;
				}
				case(product):
				{
					int temp = read_to_UICR(UICR_PRODUCT_SERIAL_NUMBER_ADDRESS);
					printf("%d",temp);
					printf("%s",endLine);
					break;
				}
				default:
					printf("%s",unknownParameter);
					break;
			}
			break;
		}
		case(SetSerialNumber):
		{
			switch (parameter1_comparison)
			{
				case(lock):
				{
					casted_string = (char*)parsed_message.parameter2;
					if(write_to_UICR(UICR_LOCK_SERIAL_NUMBER_ADDRESS,atol(casted_string)) == NRF_SUCCESS)
					{
						printf("%s",answer_OK);
					}
					else
					{
						printf("%s",answer_ERROR);
					}
					break;
				}
				case(key):
				{
					casted_string = (char*)parsed_message.parameter2;
					if(write_to_UICR(UICR_KEY_SERIAL_NUMBER_ADDRESS,atol(casted_string)) == NRF_SUCCESS)
					{
						printf("%s",answer_OK);
					}
					else
					{
						printf("%s",answer_ERROR);
					}
					break;
				}
				case(PCBA):
				{
					casted_string = (char*)parsed_message.parameter2;
					if(write_to_UICR(UICR_PCBA_SERIAL_NUMBER_ADDRESS,atol(casted_string)) == NRF_SUCCESS)
					{
						printf("%s",answer_OK);
					}
					else
					{
						printf("%s",answer_ERROR);
					}
					break;
				}
				case(product):
				{
					casted_string = (char*)parsed_message.parameter2;
					if(write_to_UICR(UICR_PRODUCT_SERIAL_NUMBER_ADDRESS,atol(casted_string)) == NRF_SUCCESS)
					{
						printf("%s",answer_OK);
					}
					else
					{
						printf("%s",answer_ERROR);
					}
					break;
				}
				default:
					printf("%s",unknownParameter);
					break;
			}
			break;
		}
		case(GetBootloaderVersion):
		{
			printf("%d.%d",BOOTLOADER_REV_MAJOR,BOOTLOADER_REV_MINOR);
			printf("%s",endLine);
			break;
		}
		case(GetFirmwareVersion):
		{
			printf("%d.%d",APPLICATION_REVISION_MAJOR,APPLICATION_REVISION_MINOR);
			printf("%s",endLine);
			break;
		}
		case(GetSoftdeviceVersion):
		{
			ble_version_t p_version;
			sd_ble_version_get(&p_version);
			printf("%d",p_version.subversion_number);
			printf("%s",endLine);
			break;
		}
		case(LoopbackTest):
		{
			disable_led_drivers();
			switch (parameter1_comparison)
			{
				case(USB_interface):
				{
					printf("%s",answer_OK);
					uint8_t c;
					int cnt = 0;
					//UART_startStopLoopbackTest(true);
					char RxBuff[64];
					bool UART_LoopbackTest_is_Active =true;
					bool possible_stop_condition = false;
					while(UART_LoopbackTest_is_Active)//UART_isLoopbackTestActive())
					{
						NRF_WDT->RR[0] = WDT_RR_RR_Reload;
						if(uart_get_ready_flag | (app_uart_get_nb_rx_fifo_chars() != 0))
						{
							if((cnt < sizeof(RxBuff)-1))
							{
								while(UART_get_character(&c) == NRF_SUCCESS && (cnt < sizeof(RxBuff)-1))
								{
									RxBuff[cnt++] = (char)c;
								}
							}
							if((strncmp(RxBuff,"\e",1)!=0) && !possible_stop_condition)
							{
								int i = 0;
								while(i<cnt)
									UART_put_character(RxBuff[i++]);
								cnt = 0;
							}
							else
							{
								possible_stop_condition = true;
							}
							if(possible_stop_condition) //(/*(c == '\n') || */(cnt == sizeof(RxBuff)-1))
							{
								// Terminate with null character
								//RxBuff[cnt] = '\0';

								if((cnt==7) && strncmp(RxBuff,"\eSTOP\r\n",7)==0)
								{
									int i = 0;
									while(i<cnt)
										UART_put_character(RxBuff[i++]);
									app_uart_flush();
									UART_LoopbackTest_is_Active = false;
									printf("%s",answer_DONE);
									break;
								}
								else if((cnt>=7)&& strncmp(RxBuff,"\eSTOP\r\n",7)!=0)
								{
									possible_stop_condition = false;
									int i = 0;
									while(i<cnt)
										UART_put_character(RxBuff[i++]);
									cnt = 0;
								}
							}
						}
					}
				//	UART_startStopLoopbackTest(false);

					break;
				}
				case(BLE_interface):
				{
					uint8_t c;
					int cnt = 0;
					char RxBuff[64];
					bool BLE_loopback_test_is_active = true;
					if(Bluetooth_startStopLoopbackTest(true))
					{
						printf("%s",answer_OK);
					}
					else
					{
						printf("%s",answer_ERROR);
						break;
					}
					while(BLE_loopback_test_is_active)
					{
						NRF_WDT->RR[0] = WDT_RR_RR_Reload;
						if(uart_get_ready_flag | (app_uart_get_nb_rx_fifo_chars() != 0))
						{
							while( UART_get_character(&c) == NRF_SUCCESS )
							{
								RxBuff[cnt++] = (char)c;
							}

							if ((c == '\n') || (cnt == sizeof(RxBuff)-1))
							{
								// Terminate with null character
								RxBuff[cnt] = '\0';

								if((cnt==7) && strncmp(RxBuff,"\eSTOP\r\n",7)==0)
								{
									app_uart_flush();
									BLE_loopback_test_is_active = false;
									break;
								}
								cnt = 0;
							}
							else if((strncmp(RxBuff,"\e",1)!=0))
							{
								cnt = 0;
							}
						}
					}
					printf("%s",answer_DONE);
					BLE_loopback_test_is_active = false;
					Bluetooth_startStopLoopbackTest(false);
					break;
				}
				case(ANT_interface):
				{

					break;
				}
				default:
					printf("%s",unknownParameter);
					break;
			}
			break;
		}
		case(SetTxPower):
		{
			casted_string = (char*)parsed_message.parameter1;
			int powerdBm = atoi(casted_string);

			if(Bluetooth_isValidPower(powerdBm))
			{
				txPowerdBm = powerdBm;
				Bluetooth_setTXPowerdBm(powerdBm);
				printf("%s",answer_OK);

			}
			else
			{
				printf("%s",unknownParameter);
			}

			break;
		}
		case(GetTxPower):
		{
			printf("%d dBm", Bluetooth_getTXPowerdBm());
			printf("%s",endLine);
			break;
		}
		case(GetRSSI):
		{
			printf("%d dBm",Bluetooth_getRSSIdBm());
			printf("%s",endLine);
			break;
		}
		case(PairingTest) :
		{
			if(Bluetooth_isConnected()) printf("%s",answer_OK);
			else printf("%s",answer_ERROR);
			break;
		}
		case(SetLedColor):
		{
			enable_led_drivers();
			casted_string = (char*)parsed_message.parameter2;

			if(check_led_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}

			if(casted_string[0] == 45) //"-"
			{
				casted_string = (char*)parsed_message.parameter1;
				int led_start = atoi(casted_string)-1;
				casted_string = (char*)parsed_message.parameter3;
				int led_end = atoi(casted_string)-1;
				//cast as char* to use atoi()
				int red_intensity = atoi((char*)parsed_message.parameter4);
				int green_intensity = atoi((char*)parsed_message.parameter5);
				int blue_intensity = atoi((char*)parsed_message.parameter6);

				if(led_start<0 || led_start>NUMBER_OF_LEDS+1)
				{
					printf("%s",unknownParameter);
					break;
				}
				if(led_end<0 || led_end>NUMBER_OF_LEDS+1 || led_end < led_start)
				{
					printf("%s",unknownParameter);
					break;
				}
				if(!((red_intensity<0)||(red_intensity>255)||(green_intensity<0)||(green_intensity>255)||(blue_intensity<0)||(blue_intensity>255)))
				{
					for(int led = led_start; led <= led_end; led++)
					{
						if(led != 24) LEDTest_setPixelColor((uint16_t) led, (uint8_t)red_intensity, (uint8_t)green_intensity, (uint8_t)blue_intensity);
						else
						{
							center_red_intensity =(uint8_t)red_intensity;
							center_green_intensity = (uint8_t)green_intensity;
							center_blue_intensity =(uint8_t)blue_intensity;

						}
					}
					printf("%s",answer_OK);
				}
				else
				{
					printf("%s",unknownParameter);
				}
			}
			else
			{
				casted_string = (char*)parsed_message.parameter1;
				int led = atoi(casted_string) - 1;

				//cast as char* to use atoi()
				 int red_intensity = atoi((char*)parsed_message.parameter2);
				 int green_intensity = atoi((char*)parsed_message.parameter3);
				 int blue_intensity = atoi((char*)parsed_message.parameter4);

				if(led<0 || led>NUMBER_OF_LEDS+1)
				{
					printf("%s",unknownParameter);
					break;
				}
				if(!((red_intensity<0)||(red_intensity>255)||(green_intensity<0)||(green_intensity>255)||(blue_intensity<0)||(blue_intensity>255)))
				{
					if(led != 24) LEDTest_setPixelColor((uint16_t) led, (uint8_t)red_intensity, (uint8_t)green_intensity, (uint8_t)blue_intensity);
					else
					{
						center_red_intensity = (uint8_t)red_intensity;
						center_green_intensity = (uint8_t)green_intensity;
						center_blue_intensity = (uint8_t)blue_intensity;

					}
					printf("%s",answer_OK);
				}
				else
				{
					printf("%s",unknownParameter);
				}
			}
			break;
		}

		case(TurnLedOn):
		{
			enable_led_drivers();
			if(check_led_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}

			casted_string = (char*)parsed_message.parameter2;

			if(casted_string[0] == 45) // "-"
			{
				casted_string = (char*)parsed_message.parameter1;
				int led_start = atoi(casted_string) - 1;
				casted_string = (char*)parsed_message.parameter3;
				int led_end = atoi(casted_string) - 1;

				casted_string = (char*)parsed_message.parameter4;

				char ledOn[] = "on";
				char ledOff[] = "off";

				if(led_start<0 || led_start>NUMBER_OF_LEDS+1)
				{
					printf("%s",unknownParameter);
					break;
				}
				if(led_end<0 || led_end>NUMBER_OF_LEDS+1 || led_end < led_start)
				{
					printf("%s",unknownParameter);
					break;
				}
				if(!strcmp(ledOn,casted_string))
				{
					for(int led = led_start; led <= led_end; led++)
					{
						if(led == 24) LEDTest_setPixelColor((uint16_t) led, (uint8_t)center_red_intensity, (uint8_t)center_green_intensity, (uint8_t)center_blue_intensity);
						else LEDTest_showPixel((uint16_t) led, 1);
					}
					printf("%s",answer_OK);
				}
				else if(!strcmp(ledOff,casted_string))
				{
					for(int led = led_start; led <= led_end; led++)
					{
						if(led == 24) LEDTest_setPixelColor((uint16_t) led, 0,0,0);
						else LEDTest_showPixel((uint16_t) led, 0);
					}
					printf("%s",answer_OK);
				}
				else
				{
					printf("%s",unknownParameter);
				}
			}
			else
			{
				casted_string = (char*)parsed_message.parameter1;

				int led = atoi(casted_string)-1;

				casted_string = (char*)parsed_message.parameter2;

				char ledOn[] = "on";
				char ledOff[] = "off";

				if(led<0 || led>NUMBER_OF_LEDS +1)
				{
					printf("%s",unknownParameter);
					break;
				}
				if(!strcmp(ledOn,casted_string))
				{
					if(led == 24) LEDTest_setPixelColor((uint16_t) led, (uint8_t)center_red_intensity, (uint8_t)center_green_intensity, (uint8_t)center_blue_intensity);
					else LEDTest_showPixel((uint16_t) led, 1);
					printf("%s",answer_OK);
				}
				else if(!strcmp(ledOff,casted_string))
				{
					if(led == 24) LEDTest_setPixelColor((uint16_t) led, 0,0,0);
					else LEDTest_showPixel((uint16_t) led, 0);
					printf("%s",answer_OK);
				}
				else
				{
					printf("%s",unknownParameter);
				}
			}
			break;
		}
		case(SetFrontLedIntensity):
		{
			nrf_gpio_pin_write(EN_VLED, 1); //turn on led power supply
			casted_string = (char*)parsed_message.parameter1;
			int intensity = atoi(casted_string);
			if(intensity == 0)
			{
				LEDTest_setFrontLedBrightness(intensity);
				printf("%s",answer_OK);
			}
			else if(intensity > 255 || intensity < 0 )
			{
				printf("%s",unknownParameter);
				break;
			}
			else
			{
				LEDTest_setFrontLedBrightness(intensity);
				printf("%s",answer_OK);
			}
			break;
		}
		case(HeartbeatTest):
		{
			//TouchTest_setup(TOUCH_OUT_PIN, TOUCH_MODE_PIN);
			casted_string = (char*)parsed_message.parameter1;
			int duration = atoi(casted_string);
			if(duration == 0)
			{
				printf("%s",unknownParameter);
			}
			else
			{
				printf("%s",answer_OK);
				TouchTest_startHearbeatTest(duration);
			}
			break;
		}
		case(SetTouchTarget):
		{
			//TouchTest_setup(TOUCH_OUT_PIN, TOUCH_MODE_PIN);
			nrf_gpio_pin_write(EN_VLED, 1);
			casted_string = (char*)parsed_message.parameter1;
			int duration = atoi(casted_string);
			if(check_led_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}
			if(duration == 0)
			{
				printf("%s",unknownParameter);
			}
			else
			{
				printf("%s",answer_OK);
				TouchTest_startTouchLocationTest(duration);
			}
			break;
		}
		case(Selftest):
		{
			switch (parameter1_comparison)
			{
				case(acc):
				{
					if(check_mag_acc_drivers())
					{
						printf("%s",answer_ERROR);
						break;
					}
					else printf("%s",answer_OK);
					AcceleroTest_setup();
					AcceleroTest_selfTest();
					break;
				}
				case(mag):
				{
					if(check_mag_acc_drivers())
					{
						printf("%s",answer_ERROR);
						break;
					}
					else printf("%s",answer_OK);
					MagnetoTest_setup();
					MagnetoTest_selfTest();
					break;
				}
				default:
					printf("%s",unknownParameter);
					break;
			}
			break;
		}
		case(GetAccelerationXYZ):
		{
			if(check_mag_acc_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}
			else printf("%s",answer_OK);
			AcceleroTest_setup();
			float aX = 0;
			float aY = 0;
			float aZ = 0;
			AcceleroTest_getAccXYZ(&aX, &aY, &aZ);
			printf("%5.2f mg, %5.2f mg, %5.2f mg\r\n", (double)aX, (double)aY, (double)aZ);
			printf("%s",endLine);
			break;
		}
//		case(FreefallTest):
//		{
//
//			break;
//		}
//		case(ActivityTriggerTest):
//		{
//
//			break;
//		}
		case(GetMagnetometerXYZ):
		{
			if(check_mag_acc_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}
			else printf("%s",answer_OK);
			MagnetoTest_setup();
			float mX = 0;
			float mY = 0;
			float mZ = 0;
			MagnetoTest_getMagXYZ(&mX, &mY, &mZ);
			printf("%2.3f mG, %2.3f mG, %2.3f mG\r\n", (double)mX, (double)mY, (double)mZ);
			printf("%s",endLine);
			break;
		}
		case(GetMaxAccelerationXYZ):
		{

			break;
		}
		case(GetMaxMagnetometerXYZ):
		{
			if(check_mag_acc_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}
			MagnetoTest_setup();
			MagnetoTest_getMagMinMaxXYZ(10);
			break;
		}
//#ifdef FACTORY_PROGRAMMED_FIRMWARE
		case(SetMagnetometerCalibrationOffset):
		{
			if(check_mag_acc_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}
			else printf("%s",answer_OK);
			MagnetoTest_setup();
			float mOffsetX = 0;
			float mOffsetY = 0;
			float mOffsetZ = 0;
			mOffsetX = atof((char*)parsed_message.parameter1);
			mOffsetY = atof((char*)parsed_message.parameter2);
			mOffsetZ = atof((char*)parsed_message.parameter3);
			if(mOffsetX>=50.0f || mOffsetX<=-50.0f || mOffsetY>=50.0f || mOffsetY<=-50.0f || mOffsetZ>=50.0f || mOffsetZ<=-50.0f )
			{
				printf("%s",unknownParameter);
				break;
			}
			MagnetoTest_setMagCalibOffset( mOffsetX,  mOffsetY,  mOffsetZ);
			mOffsetX = 0;
			mOffsetY = 0;
			mOffsetZ = 0;
			MagnetoTest_getMagCalibOffset(&mOffsetX,&mOffsetY,&mOffsetZ);
			printf("Xoffset = %f, Yoffset =  %f, Zoffset =  %f \r\n", (double)mOffsetX, (double)mOffsetY, (double)mOffsetZ);
			printf("%s",endLine);
			break;
		}
//#endif
		case(GetCompassHeading):
		{
			if(check_mag_acc_drivers())
			{
				printf("%s",answer_ERROR);
				break;
			}
			AcceleroTest_setup();
			MagnetoTest_setup();
			float result = 0.0f;
			MagnetoTest_getHeading();
			printf("%f degree",result);
			printf("%s",endLine);
			break;
		}
		case(GetBatteryVoltage):
		{
			if(!Batmon_checkPresent())//this will send an error message
			{
				break;
			}
			float cell_voltage = 0;
			Batmon_setup(BATMON_ALARM_PIN);
			cell_voltage = Batmon_getCellVoltage();
			printf("Battery voltage: %f \r\n",cell_voltage);
			break;
		}
		case(GetBatterySOC):
		{
			if(!Batmon_checkPresent())//this will send an error message
			{
				break;
			}
			int state_of_charge = 0;
			Batmon_setup(BATMON_ALARM_PIN);
			state_of_charge = Batmon_getRSOC();
			printf("Battery state of charge: %d \r\n",state_of_charge);
			break;
		}
		case(GetChargeCurrent):
		{
			int charge_current = 0;

			printf("%d mA\r\n",charge_current);
			break;
		}
		case(GetRailVoltage):
		{
			int rail_voltage = 0;
			switch (parameter1_comparison)
			{
				case(VLED_rail):
				{
					//read rail voltage here and make error message
					printf("VLED_rail :");
					printf(" %d \r\n",rail_voltage);
					break;
				}
				case(TOUCH_rail):
				{
					//read rail voltage here
					printf("TOUCH_rail :");
					printf(" %d \r\n",rail_voltage);
					break;
				}
				case(VPIEZO_rail):
				{
					//read rail voltage here
					printf("VPIEZO_rail :");
					printf(" %d \r\n",rail_voltage);
					break;
				}
				case(VBAT_rail):
				{
					//read rail voltage here
					printf("VBAT_rail :");
					printf(" %d \r\n",rail_voltage);
					break;
				}
				default :
					printf("%s",unknownParameter);
					break;
			}

			break;
		}
		case(Play):
		{
			//TO DO: enable VPIEZO here
			switch(parameter1_comparison)
			{
				case(p_Chirp):
				{
					casted_string = (char*)parsed_message.parameter2;
					int duration = atoi(casted_string);
					if((duration>10)|(duration<0))
					{
						printf("%s",unknownParameter);
						//printf("%s", answer_ERROR);
						break;
					}
					//make the piezo play the sound here for duration
					printf("%s", answer_OK);
					break;
				}
				case(off):
				{
					//completely turn off the piezo
					printf("%s", answer_OK);
					break;
				}
				default :
					printf("%s",unknownParameter);
					break;
			}

			printf("%s",answer_OK);
			break;
		}
		case(ExitFactoryMode):
		{
			NVIC_SystemReset();
			break;
		}
		case(ExitIntoShippingMode):
		{
			uint32_t factory_check = 0;
			printf("%s", answer_OK);
			softdevice_handler_sd_disable();

			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
			*(uint32_t *)UICR_FACTORY_MODE_BOOL_ADDRESS = 0;
			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

			memcpy(&factory_check,(uint32_t*)UICR_FACTORY_MODE_BOOL_ADDRESS, 4);
			NRF_WDT->RR[0] = WDT_RR_RR_Reload;

			char* functionnal_tests_string = "\r\nUICR factory bool reset: ,TESTS SUCCESSFUL";
			printf("%s",functionnal_tests_string);

			nrf_delay_ms(2000);
			NVIC_SystemReset();
			break;
		}

		default:
			printf("%s",unknownCommand);
			break;

	}
}


//giote event handler if woken from shipping mode
void usb_wake_up_handler1(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

}


void shipping_mode()
{
//note that in shipping mode, there is a factory mode action (wait till technician disconnects usb connector)
// refer to FACTORY MODE ARTIFACT


	////SHIPPING MODE////
	/// do nothing until plugged into usb
	/// write to uicr for shipping mode finished
	uint32_t shipping_check = 0;
	memcpy(&shipping_check,(uint32_t*)UICR_SHIPPING_MODE_BOOL_ADDRESS, 4);
	uint32_t err_code = 0 ;
	if(shipping_check) //enter shipping mode
	{

		// Initialize only enough to send into a deep sleep that is recoverable only from a usb connection
		NRF_WDT->RR[0] = WDT_RR_RR_Reload;


#ifdef BOARD_PCA10040
		nrf_gpio_cfg_input(USB_CONNECTED_PIN,NRF_GPIO_PIN_PULLDOWN);
#elif defined(SMARTHALO_EE)
		nrf_gpio_cfg_input(USB_CONNECTED_PIN,NRF_GPIO_PIN_NOPULL);
#else
#error "no target"
#endif
		////FACTORY MODE ARTIFACT////
		// wait one hour till shipping mode engages so that there can be no accidental
		// starting of standard operating mode
		// if WDT reload interval is changed, then the delay period must be revised
		uint32_t one_hour_factory_pause_check = 0;
		memcpy(&one_hour_factory_pause_check,(uint32_t*)UICR_ONE_HOUR_PAUSE_BOOL_ADDRESS, 4);
		//this is so the technician does not accidentally engage standard
		//operating mode and this only happens once

		if(one_hour_factory_pause_check)
		{
			//this will drain +- 4mAh of the battery, offsetting the charge the device will receive from the functionnale tests
			//writing to UICR causes a system failure so this waits till technician unplugs the factory usb connector
			while(nrf_gpio_pin_read(USB_CONNECTED_PIN)) // stay in an infinite loop until tests done and usb unplugged
			{
				nrf_delay_ms(100);

				NRF_WDT->RR[0] = WDT_RR_RR_Reload;
			}
			uint16_t primitive_counter = 0;
			while(primitive_counter < ONE_HOUR_COUNT)
			{
				NRF_WDT->RR[0] = WDT_RR_RR_Reload;
				nrf_delay_ms(1000);
				primitive_counter++;
			}
			//set factory pause bool check to 0
			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
			*(uint32_t *)UICR_ONE_HOUR_PAUSE_BOOL_ADDRESS = 0; //set shipping mode flag to 0
			NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
			while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
		}
		////FACTORY MODE ARTIFACT////


		// @@@ to do: set the power settings registers to ultra low current consumption deep sleep mode
		// Configure the RAM retention parameters
		NRF_POWER->RAMON = POWER_RAMON_ONRAM0_RAM0On   << POWER_RAMON_ONRAM0_Pos
						 | POWER_RAMON_ONRAM1_RAM1On   << POWER_RAMON_ONRAM1_Pos
						 | POWER_RAMON_OFFRAM0_RAM0Off << POWER_RAMON_OFFRAM0_Pos
						 | POWER_RAMON_OFFRAM1_RAM1Off << POWER_RAMON_OFFRAM1_Pos;

		// Set the GPIOTE PORT event as interrupt source, and enable interrupts for GPIOTE
		//setup interrupt for usb detection
		err_code = nrf_drv_ppi_init();
		APP_ERROR_CHECK(err_code);
		err_code = nrf_drv_gpiote_init();
		APP_ERROR_CHECK(err_code);
		nrf_drv_gpiote_in_config_t config1 = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
#ifdef BOARD_PCA10040
		config1.pull = NRF_GPIO_PIN_PULLDOWN;
#elif defined(SMARTHALO_EE)
		config1.pull = NRF_GPIO_PIN_NOPULL;
#else
#error "no target"
#endif
		err_code = nrf_drv_gpiote_in_init(USB_CONNECTED_PIN, &config1, usb_wake_up_handler1);
		APP_ERROR_CHECK(err_code);
		nrf_drv_gpiote_in_event_enable(USB_CONNECTED_PIN, true);

        while(nrf_gpio_pin_read(USB_CONNECTED_PIN)) // stay in an infinite loop until product is plugged into usb port
		{
        	nrf_delay_ms(10);
        	// the below functions only work with a softdevice present
			// power_manage();
			// sd_app_evt_wait();

        	// SET TO SLEEP!
    		// Enter System ON sleep mode
    		__WFE();
    		// Make sure any pending events are cleared
    		__SEV();
    		__WFE();

			NRF_WDT->RR[0] = WDT_RR_RR_Reload;
		}
		//set shipping mode bool check to 0
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
		*(uint32_t *)UICR_SHIPPING_MODE_BOOL_ADDRESS = 0; //set shipping mode flag to 0
		NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
		while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

		NVIC_SystemReset();//reset system here, without passing by the softdevice
	}
	////SHIPPING MODE////
}


void set_rssidBm(int8_t new_rssi)
{
	rssi_DBm = new_rssi;
}







