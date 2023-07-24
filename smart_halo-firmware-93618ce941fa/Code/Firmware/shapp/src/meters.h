#ifndef _METERS_H
#define _METERS_H

typedef struct {
	bool pending;
	bool acc_success;
	bool mag_success;
	bool int_success;
	bool agr;
	int16_t res_acc[3];
	int16_t res_mag[3];
} met_selftest_state_t;

#if !defined(PLATFORM_pca10040) && !defined(PLATFORM_shmp)

bool met_isMoving();

void met_compass_calibrate(void);

float met_getCompass(void);
int16_t met_getTemperature(void);

void met_selftest_do(void);
met_selftest_state_t *met_selftest_getState(void);

void met_shutdown(void);

uint8_t met_getHW(void);

void met_init(void);

#else

#define met_selftest_getState() (met_selftest_state_t*)0
#define met_selftest_do() (void)0
#define met_isMoving() false
#define met_compass_calibrate() (void)0
#define met_getCompass() 0
#define met_getTemperature() 0
#define met_shutdown() (void)0
#define met_getHW() (void)0
#define met_init() (void)0

#endif

#endif