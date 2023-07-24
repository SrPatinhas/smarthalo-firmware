#ifndef _CALIBRATION_H
#define _CALIBRATION_H

void cal_do(double *x, double *y, double *z, uint32_t ptr, double *center, double *radii);

bool cal_circle_fitting(double *x, double *y, uint32_t ptr, double *center, double *radius);

#endif