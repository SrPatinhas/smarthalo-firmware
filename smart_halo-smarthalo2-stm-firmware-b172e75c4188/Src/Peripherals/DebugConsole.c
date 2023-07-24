///
/// \file 		DebugConsole.c
/// \brief 		[Source file]
///				
/// \author 	NOVO
///
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <CommunicationTask.h>
#include "DebugConsole.h"
#include "usart.h"
#include "SoundTask.h"
#include "SensorsTask.h"
#include <GraphicsTask.h>
#include <SystemUtilitiesTask.h>
#include "tsl_user.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define bit0Pos 0
#define bit1Pos 1
#define bit2Pos 2
#define bit3Pos 3
#define bit4Pos 4

#define bit0Mask (1 << bit0Pos)
#define bit1Mask (1 << bit1Pos)
#define bit2Mask (1 << bit2Pos)
#define bit3Mask (1 << bit3Pos)
#define bit4Mask (1 << bit4Pos)


////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Private variables
////////////////////////////////////////////////////////////////////////////////
//oDebugConsole_t oDebugConsole;

bool bIsTestMode = false;

/**
 * @brief Set the console to functional test mode
 * @details All console prints will send only bytes, not human readable
 */
void setTestMode_DebugConsole(bool isTestMode){
	bIsTestMode = isTestMode;
}

/**
 * @brief Play an animation
 * @details The animation played and it's payload vary depending on the input data
 */
void animation_DebugConsole (int argc, char **argv)
{
	uint8_t u8Type = atoi(argv[1]);
	uint8_t values[argc-2];
	for(int i=0;i<argc-2;i++){
	    values[i] = atoi(argv[i+2]);
	}

	startAnimation_GraphicsTask((HaloAnimation_function_e)u8Type, values);
}

/**
 * @brief Turn off Animations
 */
void animationOff_DebugConsole(int argc, char **argv){
    animOff_GraphicsTask();
}


/**
 * @brief Read the photo sensor channels and print them
 */
void readPhotoSensor_DebugConsole (int argc, char **argv)
{
	uint16_t u16Ch1 = 0;
	uint16_t u16Ch2 = 0;
	bool bResp = false;

	getPhotoData_SensorsTask(&u16Ch1, &u16Ch2, &bResp);
	if(bIsTestMode){
		printf("%c%c%c%c",(char)((u16Ch1>>8)&0xff),(char)(u16Ch1&0xff),(char)((u16Ch2>>8)&0xff),(char)(u16Ch2&0xff));
	}else{
		printf("LSen ch1 %d, ch2 %d", u16Ch1, u16Ch2);
	}
}

/**
 * @brief Read the Acc sensor axis and print them
 */
void readAccSensor_DebugConsole (int argc, char **argv)
{
	float x = 0;
	float y = 0;
	float z = 0;
	bool bResp = false;

	getAccData_SensorsTask(&x, &y, &z, &bResp);
	if(bIsTestMode){
		if(bResp) printf("%c%c%c%c%c%c",(char)(((int)x>>8)&0xff),(char)((int)x&0xff),(char)(((int)y>>8)&0xff),(char)((int)y&0xff),(char)(((int)z>>8)&0xff),(char)((int)z&0xff));
	}else{
		if(bResp) printf("Acc X %d, Y %d, Z %d", (int)x, (int)y, (int)z);
	}
}

/**
 * @brief Read the magnetic sensor axis and print them
 */
void readMagSensor_DebugConsole (int argc, char **argv)
{
	float x = 0;
	float y = 0;
	float z = 0;
	bool bResp = false;

	getMagData_SensorsTask(&x, &y, &z, &bResp);
	if(bIsTestMode){
		if(bResp) printf("%c%c%c%c%c%c",(char)(((int)x>>8)&0xff),(char)((int)x&0xff),(char)(((int)y>>8)&0xff),(char)((int)y&0xff),(char)(((int)z>>8)&0xff),(char)((int)z&0xff));
	}else{
		if(bResp) printf("Mag X %d, Y %d, Z %d", (int)x, (int)y, (int)z);
	}
}

/**
 * @brief Read the temp sensor and print it
 */
void readTempSensor_DebugConsole (int argc, char **argv)
{
	float fTemp = 0;
	bool bResp = false;

	getTempData_SensorsTask(&fTemp, &bResp);
	if(bIsTestMode){
		if(bResp) printf("%c%c",(char)(((int)fTemp>>8)&0xff),(char)((int)fTemp&0xff));
	}else{
		if(bResp) printf("Temperature C %d", (int)fTemp);
	}
}

/**
 * @brief Read the state of charge and print it
 */
void readStateOfCharge_DebugConsole(int argc, char **argv)
{
    uint8_t stateOfCharge = getStateOfCharge_SystemUtilities();

    if (bIsTestMode) {
        printf("%c", stateOfCharge);
    } else {
        printf("State of Charge: %d", stateOfCharge);
    }
}

/**
 * @brief State the touch tests
 * @details Call will initiate the touch tests if they've started it will progress them
 */
void startTouchTest_DebugConsole( int argc, char **argv){
	send_touch_test_update_SensorsTask();
}

/**
 * @brief State the touch tests
 * @details Call will initiate the touch tests if they've started it will progress them
 */
void calibrateTouch_DebugConsole( int argc, char **argv){
    uint8_t calibration[argc-1];
    for(int i=0;i<argc-1;i++){
        calibration[i] = atoi(argv[i+1]);
    }

    tsl_user_ConfigureThresholds(calibration);
}

/**
 * @brief Instigate a swipe on the touch sensor
 * @details Sending a 0 will swipe left and a 1 will swipe right
 */
void swipeTest_DebugConsole (int argc, char **argv){
    uint8_t isRight = atoi(argv[1]);
    sendSwipe_SensorsTask(isRight);
}

void tapTest_DebugConsole (int argc, char **argv){
    uint8_t code = atoi(argv[1]);
    uint8_t length = atoi(argv[2]);
    sendTaps_SensorsTask(code, length);
}

void releaseTest_DebugConsole(int argc, char **argv){
    sendRelease_SensorsTask(true);
}

/**
 * @brief Play the destination sound for testing
 */
void soundTest_DebugConsole (int argc, char **argv){
	sound_cmd_t testTrack;
	testTrack.volume = 100;
	testTrack.repeat = 0;
	testTrack.nbr_seq = 4;
	testTrack.freq[0] = 261;
	testTrack.duration[0] = 81;
	testTrack.sweep[0] = 0;
	testTrack.freq[1] = 329;
	testTrack.duration[1] = 107;
	testTrack.sweep[1] = 0;
	testTrack.freq[2] = 391;
	testTrack.duration[2] = 105;
	testTrack.sweep[2] = 0;
	testTrack.freq[3] = 523;
	testTrack.duration[3] = 195;
	testTrack.sweep[3] = 0;

	setSound_SoundTask(&testTrack);
}

/**
 * @brief Display the halo LEDs tests
 * @details All red, all blue, all green or rainbow modes. Can control brightness and turn LEDs off for testing
 */
void ledsTest_DebugConsole (int argc, char **argv){
	uint8_t u8Value = atoi(argv[1]);
	uint8_t brightness = 255;
	if(argc > 2){
		brightness = atoi(argv[2]);
	}
	//disabled
	uint8_t offLED = 255;
	if(argc > 3){
		offLED = atoi(argv[3]);
	}
	startLEDTest_GraphicsTask(u8Value, brightness, offLED);
}

/**
 * @brief Turn the front light on
 * @details Can send the brightness as a parameter
 */
void frontTest_DebugConsole (int argc, char **argv){
	uint8_t percentage = atoi(argv[1]);
	uint8_t mode = 0;
	if(argc > 2){
	    mode = atoi(argv[2]);
	}
	testNightLight_GraphicsTask(percentage, mode);
}

/**
 * @brief Turn on the OLED for testing
 * @details No parameters will turn the OLED on all white. Sending a 0 will turn it off. Send a number between 0-1023 and a byte will change an 8 bit column
 */
void oledTest_DebugConsole (int argc, char **argv){
	if(argc == 1){
		testOledDisplay_CommunicationTask();
	}

	if(argc == 2){
		uint8_t u8Value = atoi(argv[1]);
		if(u8Value == 0) testOLEDByTurningOff_GraphicsTask();
	}

	if(argc == 3){
		uint16_t u16Position = atoi(argv[1]);
		uint8_t u8Value = atoi(argv[2]);
		editDebugImage_GraphicsTask(u16Position, u8Value);
	}
}

/**
 * @brief Read the device ID
 * @details The device ID is held and generated on the Nordic Chip, this will trigger a request to receive it.
 */
void readDeviceID_DebugConsole (int argc, char **argv){
	uint8_t length = 1;
	uint8_t ble_msg[length];
	ble_msg[0] = BLE_GET_ID;

	sendData_CommunicationTask(BLE_TYPE_MSG, BLE_COMMAND_SYNC, length, ble_msg);
}

/**
 * @brief Print the device ID
 * @details The device ID is held and generated on the Nordic Chip
 */
void printDeviceID_DebugConsole(uint8_t * ID, uint8_t length){
	for(int i=0; i<length; i++){
		if(bIsTestMode){
			printf("%c",(char)ID[i]);
		}else{
			printf("%02x", ID[i]);
		}
	}
}

/**
 * @brief Run the hardware tests and print the results
 * @details This will run 5 tests and return a single byte result with a bit map of any failed tests. Compass, photo, oled, halo and flash
 */
void hardwareTest_DebugConsole (int argc, char **argv){
	if(bIsTestMode){
		printf("%c",(char)hardwareTests_SystemUtilities());
	}else{
		printf("Test results: %d", hardwareTests_SystemUtilities());
	}
}

void pinTest_DebugConsole (int argc, char **argv){
    char port = atoi(argv[1]);
    uint8_t pin = atoi(argv[2]);
    uint16_t period = atoi(argv[3]);
    if(setConductivityTest_SystemUtilities(port, pin, period) && !bIsTestMode){
        printf("Testing port %c on pin %d at a period of %d", port, pin, period);
    }
}
