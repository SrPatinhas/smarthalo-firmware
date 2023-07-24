/*
 * vector.h
 *
 *  Created on: 2016-04-25
 *      Author: SmartHalo
 */

#ifndef SH_FIRMWARE_CODE_VECTOR_H_
#define SH_FIRMWARE_CODE_VECTOR_H_

typedef struct vector
{
  float x, y, z;
} vector;

extern void vector_cross(const vector *a, const vector *b, vector *out);
extern float vector_dot(const vector *a,const vector *b);
extern void vector_normalize(vector *a);


#endif /* SH_FIRMWARE_CODE_VECTOR_H_ */
