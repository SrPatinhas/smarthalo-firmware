/*
 * SHP_Frames_and_commands.h
 *
 *  Created on: Feb 8, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SHP_FRAMES_AND_COMMANDS_H_
#define SH_FIRMWARE_CODE_SHP_FRAMES_AND_COMMANDS_H_

//==========================================//
//SHP PROTOCOL PARSING STATE MACHINE DEFINITIONS
//	  see latest version of SHP Protocol
//	Specification document for more details
//==========================================//
//used in the protocol to check if this is indeed a true message or just garbage signals from another device
#define START_OF_FRAME_VALUE 				0xDC   //because DC is better than Marvel.
#define END_OF_FRAME_VALUE					0xCD  //OCD for OSEDEA
//miscellaneous
#define STATE_MACHINE_EXIT_CONDITION 		false
#define MAX_PACKET_LENGTH					20 //maximum amount because of the current (02-02-2016) implementation of nordic ble stack s132
#define MIN_PACKET_LENGTH					7 //size of header of SHP protocol
#define MAX_PAYLOAD_LENGTH					MAX_PACKET_LENGTH - MIN_PACKET_LENGTH//command and data bytes are the payload,
#define MAX_LOOP_CYCLES						MAX_PACKET_LENGTH + 2 //used to check for infinite loops in the state machine

//types of errors encountered
#define CHECKSUM_FAILED 					0x01
#define START_OF_FRAME_INCORRECT	 		0x02
#define UNDEFINED_SOURCE_ADDRESS 			0x03
#define UNDEFINED_DESTINATION_ADDRESS 		0x04
#define UNDEFINED_PROTOCOL_VERSION 			0x05
#define UNDEFINED_MESSAGE 					0x06
#define CRC_IS_NOT_INITIALISED 				0x07
#define MEMORY_NOT_FREED 					0x08
#define UNDEFINED_STATE 					0x09
#define BUFFER_MEMORY_NOT_ALLOCATED 		0x0A //10
#define INFINITE_LOOP						0x0B //11
#define MESSAGE_LENGTH_OUT_OF_BOUNDS		0x0C
#define END_OF_FRAME_INCORRECT				0x0D

#define NUMBER_OF_POSSIBLE_ERRORS			13

//state definitions for framing state machine
#define START_OF_FRAME_STATE				0x00
#define SOURCE_ADDRESS_STATE				0x01
#define DESTINATION_ADDRESS_STATE			0x02
#define PROTOCOL_VERSION_AND_LENGTH_STATE	0x03
//#define LENGTH_STATE						0x04
#define MESSAGE_STATE						0x05
#define CHECKSUM_STATE						0x06
#define END_OF_FRAME_STATE					0x07

//definitions for frames in protocol
#define SOF_FRAME 							0x00
#define SRC_FRAME 							0x01
#define DST_FRAME 							0x02
#define VER_LEN_FRAME 						0x03
//#define LEN_FRAME 							0x04
#define MTYP_FRAME							0x04//message type frame
#define MSG_FRAME							0x05
//==========================================//


//==========================================//
//	SHP PROTOCOL COMMAND & DATA DEFINITIONS
//	  see latest version of SHP Protocol
//	Specification document for more details
//==========================================//
//SHP PROTOCOL COMMANDS BYTE (for frame MSG_FRAME_CMD)
		//SHP PROTOCOL DATA for DATA BYTE(s) (for frames MSG_FRAME_DATA)

////GENERAL OPERATIONS////
#define SEND_FIRMWARE_VERSION				0x21
#define SEND_BOOTLOADER_VERSION				0x22
#define SEND_SOFTDEVICE_VERSION				0x23

#define SEND_SERIAL_NUMBER					0x24
		#define SERIAL_LOCK					0x00
		#define SERIAL_KEY					0x01
		#define SERIAL_PCBA					0x02
		#define SERIAL_PRODUCT				0x03


#define SET_NEW_HALO_BRIGHTNESS				0x35

#define ANIMATION_DEMO 						0x36
	#define	PAIRING_ANIMATION_DEMO			0x00
////GENERAL OPERATIONS////


////NAVIGATION////
#define DIRECTION_UPDATE					0x50
		/*data for DIRECTION_UPDATE command*/
			//first byte
		#define CONTINUE					0x00
		#define STRAIGHT					0x01
		#define U_TURN						0x02
		#define TURN_LEFT					0x03
		#define TURN_RIGHT					0x04
		#define SLIGHT_LEFT					0x05
		#define SLIGHT_RIGHT				0x06
		#define HARD_LEFT					0x07
		#define HARD_RIGHT					0x08
		#define DESTINATION					0x09
			//second byte
			#define STEP_ONE				0x00//steps one through five are for all types of turns
			#define STEP_TWO				0x01
			#define STEP_THREE				0x02
			#define STEP_FOUR				0x03
			#define STEP_FIVE				0x04
			#define STEP_SIX				0x05//steps six through eight are only for left and right turns
			#define STEP_SEVEN				0x06
			#define STEP_EIGHT				0x07
			#define STEP_NINE				0x08
			#define STEP_TEN				0x09
//====
#define TURN_COMPLETED						0x51
		/*no data*/
//====
#define RIDE_STATUS_UPDATE					0x52
		/*data for RIDE_STATUS_UPDATE command*/
		#define START_RIDE					0x00
		#define STOP_RIDE					0x01
		#define PAUSE_RIDE					0x02
//====
#define NAVIGATION_MODE						0x53
		/*data for NAVIGATION_MODE command*/
		#define TURN_BY_TURN				0x00
		#define AS_THE_CROW_FLIES			0x01
		#define GOAL_COMPLETION				0x02
//====
#define CURRENT_HEADING						0x54
		/*data for CURRENT_HEADING command*/
	#define NUMBER_OF_LEDS_ON_BOARD		24
		#define NORTH						0x00
		#define EAST						(1 *(NUMBER_OF_LEDS_ON_BOARD/4))
		#define SOUTH						(2 *(NUMBER_OF_LEDS_ON_BOARD/4))
		#define WEST						(3 *(NUMBER_OF_LEDS_ON_BOARD/4))
	/* values range from 0x00 to 0x17 (0 to 23, to within a
	 * precision of the number of LEDs on the device)
	 * Values in between 0x00 and 0x17 will light up an LED
	 * with position proportional to it's value
	 * 0x06 corresponds to EAST (7th led of 24 led board)
	 * 0x0C corresponds to SOUTH (13th led of 24 led board)
	 * 0x12 corresponds to WEST (19th led of 24 led board)
	 * */
//====
#define SET_DESTINATION_HEADING				0x55
		/* data for SET_DESTINATION_HEADING command*/
/// #define NUMBER_OF_LEDS_ON_BOARD		24
///		#define NORTH						0x00
///		#define EAST						(1 *(NUMBER_OF_LEDS_ON_BOARD/4))
///		#define SOUTH						(2 *(NUMBER_OF_LEDS_ON_BOARD/4))
///		#define WEST						(3 *(NUMBER_OF_LEDS_ON_BOARD/4))
	/* values range from 0x00 to 0x17 (0 to 23, to within a
	 * precision of the number of LEDs on the device)
	 * Values in between 0x00 and 0x17 will light up an LED
	 * with position proportional to it's value
	 * 0x06 corresponds to EAST (7th led of 24 led board)
	 * 0x0C corresponds to SOUTH (13th led of 24 led board)
	 * 0x12 corresponds to WEST (19th led of 24 led board)
	 * */
//====
////NAVIGATION////



////TRACKING////
#define GOAL_TRACKING_UPDATE				0x60
		/*data for ERROR_MESSAGE command*/
			//first byte
		#define TIME_UPDATE					0x00
		#define DISTANCE_UPDATE             0x01
		#define SPEED_UPDATE                0x02
		#define ELEVATION_UPDATE            0x03
		#define CALORIES_UPDATE             0x04
			//second byte
				//percentage value from 0 to 99 (0x00 to 0x63)
//====
#define REQUEST_TRACKING_UPDATE				0x61
			/*no data*/
//====
////TRACKING////


////NIGHT LIGHT////
#define NIGHT_LIGHT_SET_MODE				0x70
		/*data for SET_MODE command*/
		#define NIGHT_LIGHT_AUTO			0x00
		#define NIGHT_LIGHT_MANUAL  	    0x01
//====
#define NIGHT_LIGHT_TOGGLE_POWER			0x71
		/*data for TURN_ON command*/
		#define NIGHT_LIGHT_TURN_OFF		0x00
		#define NIGHT_LIGHT_TURN_ON  	    0x01
//====
#define NIGHT_LIGHT_SET_BLINKING_MODE		0x72
		/*data for SET_BLINKING_MODE command*/
		#define BLINKING_OFF				0x00
		#define BLINKING_ON   	            0x01
//====
#define NIGHT_LIGHT_SET_BRIGHTNESS			0x73
		/*data for SET_BRIGHTNESS command*/
			//first data byte is 0 to 255 or 0x00 to 0xFF
////NIGHT LIGHT////


////PERSONAL ASSISTANT////
#define INCOMING_CALL						0x80
		/*no data*/
//====
#define INCOMING_SMS						0x81
		/*data for INCOMING_SMS command*/

//====
#define INCOMING_CUSTOM						0x82
		/*data for INCOMING_CUSTOM command*/
////PERSONAL ASSISTANT////


////ALARM////
#define ALARM_SET_MODE						0x90
		/*data for ALARM_SET_MODE command*/
		#define ALARM_AUTO					0x00
		#define ALARM_MANUAL  		        0x01
//====
#define ALARM_SET_VOLUME					0x91
		/*data for ALARM_SET_MODE command*/
		//data must be one byte between 0x00 and 0x255

#define	ALARM_ACTIVATION					0x92
		/*data for ALARM_SET_MODE command*/
		#define ALARM_OFF	  		        0x00
		#define ALARM_ON					0x01

//====
////ALARM////


/// BATTERY MONITOR ///

#define GET_BATTERY_LEVEL					0xA0


/// BATTERY MONITOR ///



////DFU////
#define ENTER_BOOTLOADER_SERIAL_MODE		0xB0
		/*data for ENTER_BOOTLOADER command*/

//====
////DFU////




////THERMAL ERROR////
#define THERMAL_ERROR						0xF5

//====
////THERMAL ERROR////


////TOUCH CONTROL////
#define SET_NEW_TAPCODE						0xE0

//====
////TOUCH CONTROL////

////RESERVED OPERATIONS////
#define RESET_DEVICE						0xFC

//====
////RESERVED OPERATIONS////




//==========================================//


//==========================================//
//	SHP PROTOCOL ADDRESSES DEFINITIONS
//	  see latest version of SHP Protocol
//	Specification document for more details
//==========================================//

// TBD
// TBD
// TBD

//==========================================//

//this may be subject to change
//#define ERROR_MESSAGE						0x10
		/*data for ERROR_MESSAGE command*/			//Error number referenced at top of this file
///		#define CHECKSUM_FAILED 					1
///		#define START_OF_FRAME_STATE_INCORRECT 		2
///		#define UNDEFINED_SOURCE_ADDRESS 			3
///		#define UNDEFINED_DESTINATION_ADDRESS 		4
///		#define UNDEFINED_PROTOCOL_VERSION 			5
///		#define UNDEFINED_MESSAGE 					6
///		#define CRC_IS_NOT_INITIALISED 				7
///		#define MEMORY_NOT_FREED 					8
///		#define UNDEFINED_STATE 					9
///		#define BUFFER_MEMORY_NOT_ALLOCATED 		10
///		#define INFINITE_LOOP						11
///		#define MESSAGE_LENGTH_OUT_OF_BOUNDS		0x0C
//====



#endif /* SH_FIRMWARE_CODE_SHP_FRAMES_AND_COMMANDS_H_ */
