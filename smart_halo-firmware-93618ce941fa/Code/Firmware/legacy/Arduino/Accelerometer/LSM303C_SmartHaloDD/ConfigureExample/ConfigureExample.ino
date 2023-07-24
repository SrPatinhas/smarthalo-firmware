// I2C interface by default
//
#include "Wire.h"
#include "SparkFunIMU.h"
#include "SparkFunLSM303C.h"
#include "LSM303CTypes.h"

/*
 * define DEBUG 1 in SparkFunLSM303C.cpp turns on debugging statements.
 * Redefine to 0 to turn them off.
 */

/*
 * SPI pins defined in SparkFunLSM303C.h for Pro Mini
 * D10 -> SDI/SDO
 * D11 -> SCLK
 * D12 -> CS_XL
 * D13 -> CS_MAG
 */
 
template <typename T> struct vector
{
  T x, y, z;
};

LSM303C myIMU;

void setup() {
  Serial.begin(115200);
  Serial.println("LSM303CTR Raw values Firmware");

  if (myIMU.begin(
                ///// Interface mode options
                  //MODE_SPI,
                  MODE_I2C,

                ///// Magnetometer output data rate options
                  //MAG_DO_0_625_Hz,
                  //MAG_DO_1_25_Hz,
                  //MAG_DO_2_5_Hz,
                  //MAG_DO_5_Hz,
                  //MAG_DO_10_Hz,
                  //MAG_DO_20_Hz,
                  MAG_DO_40_Hz,
                  //MAG_DO_80_Hz,

                ///// Magnetic field full scale options
                  //MAG_FS_4_Ga,
                  //MAG_FS_8_Ga,
                  //MAG_FS_12_Ga,
                  MAG_FS_16_Ga,
                  
                ///// Magnetometer block data updating options
                  //MAG_BDU_DISABLE,
                  MAG_BDU_ENABLE,

                ///// Magnetometer X/Y axes ouput data rate
                  //MAG_OMXY_LOW_POWER,
                  //MAG_OMXY_MEDIUM_PERFORMANCE,
                  MAG_OMXY_HIGH_PERFORMANCE,
                  //MAG_OMXY_ULTRA_HIGH_PERFORMANCE,

                ///// Magnetometer Z axis ouput data rate
                  //MAG_OMZ_LOW_PW,
                  //MAG_OMZ_MEDIUM_PERFORMANCE,
                  MAG_OMZ_HIGH_PERFORMANCE,
                  //MAG_OMZ_ULTRA_HIGH_PERFORMANCE,

                ///// Magnetometer run mode
                  MAG_MD_CONTINUOUS,
                  //MAG_MD_SINGLE,
                  //MAG_MD_POWER_DOWN_1,
                  //MAG_MD_POWER_DOWN_2,

                ///// Acceleration full scale
                  ACC_FS_2g,
                  //ACC_FS_4g,
                  //ACC_FS_8g,

                ///// Accelerometer block data updating
                  //ACC_BDU_DISABLE,
                  ACC_BDU_ENABLE,

                ///// Enable X, Y, and/or Z axis
                  //ACC_DISABLE_ALL,
                  //ACC_X_ENABLE,
                  //ACC_Y_ENABLE,
                  //ACC_Z_ENABLE,
                  //ACC_X_ENABLE|ACC_Y_ENABLE,
                  //ACC_X_ENABLE|ACC_Z_ENABLE,
                  //ACC_Y_ENABLE|ACC_Z_ENABLE,
                  ACC_X_ENABLE|ACC_Y_ENABLE|ACC_Z_ENABLE,

                ///// Accelerometer output data rate
                  //ACC_ODR_POWER_DOWN
                  //ACC_ODR_10_Hz
                  //ACC_ODR_50_Hz
                  ACC_ODR_100_Hz
                  //ACC_ODR_200_Hz
                  //ACC_ODR_400_Hz
                  //ACC_ODR_800_Hz
                ) != IMU_SUCCESS)
  {
    Serial.println("Failed setup.");
    while(1);
  }
}

void loop()
{
  float value;

  value = myIMU.readAccelX();

  // Assume that if X is not activated then none are (poor assumption, but demo)
  if ( !isnan(value) )
  {
    Serial.print("\nAccelerometer:\n X = ");
    Serial.println(value, 4);
    Serial.print(" Y = ");
    Serial.println(myIMU.readAccelY(), 4);
    Serial.print(" Z = ");
    Serial.println(myIMU.readAccelZ(), 4);
  }

  value = myIMU.readGyroX();
  // Not supported by hardware, so will return NAN
  if ( !isnan(value) )
  {
    Serial.print("\nGyroscope:\n X = ");
    Serial.println(value, 4);
    Serial.print(" Y = ");
    Serial.println(myIMU.readGyroY(), 4);
    Serial.print(" Z = ");
    Serial.println(myIMU.readGyroZ(), 4);
  }

  value = myIMU.readMagX();
  if ( !isnan(value) )
  {
    Serial.print("\nMagnetometer:\n X = ");
    Serial.println(value, 4);
    Serial.print(" Y = ");
    Serial.println(myIMU.readMagY(), 4);
    Serial.print(" Z = ");
    Serial.println(myIMU.readMagZ(), 4);
    Serial.print(" Heading = ");
    Serial.println(heading((vector<int>){1, 0, 0}),4);
  }

  value = myIMU.readTempC();
  if ( !isnan(value) )
  {
    Serial.print("\nThermometer:\n");
    Serial.print(" Degrees C = ");
    Serial.println(value, 4);
//    Serial.print(" Degrees F = ");
//    Serial.println(myIMU.readTempF(), 4);
  } 

  delay(1000);
}

/*
Returns the angular difference in the horizontal plane between the
"from" vector and north, in degrees.

Description of heading algorithm:
Shift and scale the magnetic reading based on calibration data to find
the North vector. Use the acceleration readings to determine the Up
vector (gravity is measured as an upward acceleration). The cross
product of North and Up vectors is East. The vectors East and North
form a basis for the horizontal plane. The From vector is projected
into the horizontal plane and the angle between the projected vector
and horizontal north is returned.
*/

template <typename T> float heading(vector<T> from)
{
    vector<int32_t> temp_m = {myIMU.readMagX(), myIMU.readMagY(), myIMU.readMagZ()};

    // subtract offset (average of min and max) from magnetometer readings
//    temp_m.x -= ((int32_t)myIMU.readMinX() + myIMU.readMaxX()) / 2;
//    temp_m.y -= ((int32_t)myIMU.readMinY() + myIMU.readMaxY()) / 2;
//    temp_m.z -= ((int32_t)myIMU.readMinZ() + myIMU.readMaxZ()) / 2;

    // compute E and N
    vector<float> E;
    vector<float> N;
    vector<int32_t> a;
    a.x = myIMU.readAccelX(); a.y = myIMU.readAccelY(); a.z = myIMU.readAccelZ();
    
    vector_cross(&temp_m, &a, &E);
    vector_normalize(&E);
    vector_cross(&a, &E, &N);
    vector_normalize(&N);

    // compute heading
    float heading = atan2(vector_dot(&E, &from), vector_dot(&N, &from)) * 180 / M_PI;
    if (heading < 0) heading += 360;
    return heading;
}

template <typename Ta, typename Tb, typename To> void vector_cross(const vector<Ta> *a, const vector<Tb> *b, vector<To> *out)
{
  out->x = (a->y * b->z) - (a->z * b->y);
  out->y = (a->z * b->x) - (a->x * b->z);
  out->z = (a->x * b->y) - (a->y * b->x);
}

template <typename Ta, typename Tb> float vector_dot(const vector<Ta> *a, const vector<Tb> *b)
{
  return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

void vector_normalize(vector<float> *a)
{
  float mag = sqrt(vector_dot(a, a));
  a->x /= mag;
  a->y /= mag;
  a->z /= mag;
}
