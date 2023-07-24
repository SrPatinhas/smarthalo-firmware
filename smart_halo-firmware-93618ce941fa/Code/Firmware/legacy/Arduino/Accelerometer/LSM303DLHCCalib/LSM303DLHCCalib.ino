#include <Wire.h>
#include <LSM303.h>
LSM303 compass;

const float alpha = 0.15;
float fXa = 0;
float fYa = 0;
float fZa = 0;
float fXm = 0;
float fYm = 0;
float fZm = 0;

void setup()
{
Serial.begin(9600);
Wire.begin();
compass.init();
compass.enableDefault();

//byte regcontent = compass.readReg(compass.CRB_REG_M);
//Serial.print("CRB_REG_M: 0x"); Serial.println(regcontent, HEX);
compass.writeReg(compass.CTRL_REG4_A,0x08);
byte regcontent = compass.readReg(compass.CTRL_REG4_A);
Serial.print("CTRL_REG4_A: 0x"); Serial.println(regcontent, HEX);

//Serial.println("Magnetometer Calibrated (Units in Nanotesla)"); 
//Serial.println("Accelerometer Uncalibrated (Units in mGal)"); 
}

void loop()
{
compass.read();
float pitch, pitch_print, roll, roll_print, Heading, fXm_comp, fYm_comp;
float Xm_off, Ym_off, Zm_off, Xm_cal, Ym_cal, Zm_cal;
float Xa_off, Ya_off, Za_off, Xa_cal, Ya_cal, Za_cal;

// Accelerometer calibration
Xa_off = compass.a.x/16.0 - 18.617144; //X-axis combined bias (Non calibrated data - bias)
Ya_off = compass.a.y/16.0 + 1.050613; //Y-axis combined bias (Default: substracting bias)
Za_off = compass.a.z/16.0 - 44.642943; //Z-axis combined bias

Xa_cal =  0.981229*Xa_off - 0.010158*Ya_off + 0.005605*Za_off; //X-axis correction for combined scale factors (Default: positive factors)
Ya_cal =  -0.010158*Xa_off + 1.006086*Ya_off - 0.010793*Za_off; //Y-axis correction for combined scale factors
Za_cal =  0.005605*Xa_off - 0.010793*Ya_off + 0.996479*Za_off; //Z-axis correction for combined scale factors

//Serial.print(Xa_cal); Serial.print(" "); Serial.print(Ya_cal); Serial.print(" "); Serial.println(Za_cal);

// Magnetometer calibration
Xm_off = compass.m.x*(100000.0/1100.0) - 77116.852009; //X-axis combined bias (Non calibrated data - bias)
Ym_off = compass.m.y*(100000.0/1100.0) + 119065.335304; //Y-axis combined bias (Default: substracting bias)
Zm_off = compass.m.z*(100000.0/980.0 ) + 260888.205432; //Z-axis combined bias

Xm_cal =  0.182754*Xm_off + -0.005954*Ym_off + 0.012667*Zm_off; //X-axis correction for combined scale factors (Default: positive factors)
Ym_cal =  -0.005954*Xm_off + 0.174532*Ym_off + -0.002439*Zm_off; //Y-axis correction for combined scale factors
Zm_cal =  0.012667*Xm_off + -0.002439*Ym_off + 0.173523*Zm_off; //Z-axis correction for combined scale factors

//Serial.print(Xm_cal, 10); Serial.print(" "); Serial.print(Ym_cal, 10); Serial.print(" "); Serial.println(Zm_cal, 10);

// Low-Pass filter accelerometer
fXa = Xa_cal * alpha + (fXa * (1.0 - alpha));
fYa = Ya_cal * alpha + (fYa * (1.0 - alpha));
fZa = Za_cal * alpha + (fZa * (1.0 - alpha));

// Low-Pass filter magnetometer
fXm = Xm_cal * alpha + (fXm * (1.0 - alpha));
fYm = Ym_cal * alpha + (fYm * (1.0 - alpha));
fZm = Zm_cal * alpha + (fZm * (1.0 - alpha));

// Pitch and roll
roll  = atan2(fYa, sqrt(fXa*fXa + fZa*fZa));
pitch = atan2(fXa, sqrt(fYa*fYa + fZa*fZa));
roll_print = roll*180.0/M_PI;
pitch_print = pitch*180.0/M_PI;

// Tilt compensated magnetic sensor measurements
fXm_comp = fXm*cos(pitch)+fZm*sin(pitch);
fYm_comp = fXm*sin(roll)*sin(pitch)+fYm*cos(roll)-fZm*sin(roll)*cos(pitch);

// Arctangent of y/x
Heading = (atan2(fYm_comp,fXm_comp)*180.0)/M_PI;
if (Heading < 0)
Heading += 360;

Serial.print("Pitch (X): "); Serial.print(pitch_print); Serial.print("  ");
Serial.print("Roll (Y): "); Serial.print(roll_print); Serial.print("  ");
Serial.print("Heading: "); Serial.println(Heading);
delay(250);
}
