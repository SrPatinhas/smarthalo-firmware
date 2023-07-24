/*
 * SH_Animations_colors.h
 *
 *  Created on: 2016-07-13
 *      Author: SmartHalo
 */

#ifndef SH_ANIMATIONS_COLORS_H_
#define SH_ANIMATIONS_COLORS_H_

#define RED_PIXEL_GOAL_COMPLETION 		RED_PIXEL_PINK
#define GREEN_PIXEL_GOAL_COMPLETION 	GREEN_PIXEL_PINK
#define BLUE_PIXEL_GOAL_COMPLETION 		BLUE_PIXEL_PINK

#define RED_PIXEL_ALARM					RED_PIXEL_RED
#define GREEN_PIXEL_ALARM				GREEN_PIXEL_RED
#define BLUE_PIXEL_ALARM				BLUE_PIXEL_RED

#define RED_PIXEL_PAIRING_FADE			RED_PIXEL_YELLOW
#define GREEN_PIXEL_PAIRING_FADE		GREEN_PIXEL_YELLOW
#define BLUE_PIXEL_PAIRING_FADE			BLUE_PIXEL_YELLOW

#define RED_PIXEL_REROUTING				RED_PIXEL_YELLOW
#define GREEN_PIXEL_REROUTING			GREEN_PIXEL_YELLOW
#define BLUE_PIXEL_REROUTING			BLUE_PIXEL_YELLOW

#define RED_PIXEL_DISCONNECTED			RED_PIXEL_ORANGE
#define GREEN_PIXEL_DISCONNECTED		GREEN_PIXEL_ORANGE
#define BLUE_PIXEL_DISCONNECTED			BLUE_PIXEL_ORANGE

#define RED_PIXEL_NOTIFICATION			RED_PIXEL_BLUE
#define GREEN_PIXEL_NOTIFICATION		GREEN_PIXEL_BLUE
#define BLUE_PIXEL_NOTIFICATION			BLUE_PIXEL_BLUE

#define RED_PIXEL_ORANGE				correction_gamma_led_rgb[241]
#define GREEN_PIXEL_ORANGE				correction_gamma_led_rgb[122]
#define BLUE_PIXEL_ORANGE				correction_gamma_led_rgb[64]

#define RED_PIXEL_PINK					correction_gamma_led_rgb[232]
#define GREEN_PIXEL_PINK				correction_gamma_led_rgb[16]
#define BLUE_PIXEL_PINK					correction_gamma_led_rgb[107]

#define RED_PIXEL_YELLOW				correction_gamma_led_rgb[249]
#define GREEN_PIXEL_YELLOW				correction_gamma_led_rgb[227]
#define BLUE_PIXEL_YELLOW				correction_gamma_led_rgb[21]

#define RED_PIXEL_RED					correction_gamma_led_rgb[255]
#define GREEN_PIXEL_RED					correction_gamma_led_rgb[29]
#define BLUE_PIXEL_RED					correction_gamma_led_rgb[37]

#define RED_PIXEL_GREEN					correction_gamma_led_rgb[57]
#define GREEN_PIXEL_GREEN				correction_gamma_led_rgb[181]
#define BLUE_PIXEL_GREEN				correction_gamma_led_rgb[74]

#define RED_PIXEL_BLUE					correction_gamma_led_rgb[14]
#define GREEN_PIXEL_BLUE				correction_gamma_led_rgb[131]
#define BLUE_PIXEL_BLUE					correction_gamma_led_rgb[203]

#define RED_PIXEL_WHITE					correction_gamma_led_rgb[255]
#define GREEN_PIXEL_WHITE				correction_gamma_led_rgb[255]
#define BLUE_PIXEL_WHITE				correction_gamma_led_rgb[255]

#define RED_PIXEL_MID_GREEN				correction_gamma_led_rgb[185]
#define GREEN_PIXEL_MID_GREEN			correction_gamma_led_rgb[212]
#define BLUE_PIXEL_MID_GREEN			correction_gamma_led_rgb[39]

#define BRIGHTNESS_GREEN_MIN_AS_THE_CROW_FLIES	145
#define BRIGHTNESS_RED_MAX_AS_THE_CROW_FLIES 	255
#define BRIGHTNESS_BLUE_AS_THE_CROW_FLIES		0

#define RED_PIXEL_CONNECTION	0.2
#define GREEN_PIXEL_CONNECTION	0.2
#define BLUE_PIXEL_CONNECTION	0.2

#define BRIGHTNESS_MAX					255

static const uint8_t correction_gamma_led_rgb[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

#endif /* COMMON_INCLUDE_SH_ANIMATIONS_COLORS_H_ */
