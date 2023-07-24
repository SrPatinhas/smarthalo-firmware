#include <Event.h>
#include <Timer.h>

#include <Wire.h>
#include <LSM303.h>
#include <HaloFSM.h>

//#include "PinChangeInterrupt.h"

LSM303 compass;
HaloFSM StateMachine;

HaloFSM::PSTATE LastState = HaloFSM::NAP;
HaloFSM::DMODE LastDMode = HaloFSM::GC;

#define INT1_PIN 2
#define INT2_PIN 3

#define CODE_PIN  4
#define BTX_PIN   5

#define DEBOUNCE_INT1 2000
#define DEBOUNCE_INT2 200

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();

  // General settings
  // ODR = 10 Hz, all axis enabled
  compass.writeReg(compass.CTRL_REG1_A,0x27);

////////////////// INT1 ///////////////////////////
/////////// POWER SAVING & ALARM //////////////////

  // Enable HPF for AOI on INT1 and Click function
  compass.writeReg(compass.CTRL_REG2_A,0x05);
  // Route AOI1 to INT1
  compass.writeReg(compass.CTRL_REG3_A,0x40);
  // not-continuous, high resolution mode
  compass.writeReg(compass.CTRL_REG4_A,0x88);
  // Interrupt not latched
  compass.writeReg(compass.CTRL_REG5_A,0x00);
  // Threshold for wakeup (3LSB = 3*15.625mg/LSB = 46.875mg)
  compass.writeReg(compass.INT1_THS_A,0x03); 
  // Duration for wakeup (20LSB * (1/10Hz) = 2s)
  compass.writeReg(compass.INT1_DURATION_A,0x14);
  // Interrupt enables if > ths on any of the axis (OR)
  compass.writeReg(compass.INT1_CFG_A,0x2A);

////////////////// INT2 ///////////////////////////
//////////////// USER INPUT ///////////////////////

  // Enable single-click INT on Z axis only
  compass.writeReg(compass.CLICK_CFG_A,0x10);//0x00);
  // Enable single-click
  compass.writeReg(compass.CLICK_SRC_A,0x10);//0x00);
  // click threshold (6LSB = 6*15.625mg/LSB = 93.75 mg)
  compass.writeReg(compass.CLICK_THS_A,0x06);//0x00);
  // Maximum duration for single-click (2LSB = 2*1/10 = 200ms)
  compass.writeReg(compass.TIME_LIMIT_A,0x02);//0x00);
  // Minimum time to other single-click event (4LSB = 4*1/10 = 400ms)
  compass.writeReg(compass.TIME_LATENCY_A,0x04);//0x00); 
  // For double-click only.
  compass.writeReg(compass.TIME_WINDOW_A,0x00);
  // Route Click on INT2
  compass.writeReg(compass.CTRL_REG6_A,0x80);//0x00);

  // Enable GPIO interrupts on uC
  pinMode(INT1_PIN,INPUT);
  //pinMode(INT2_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(INT1_PIN),INT1_ISR,CHANGE);
  //attachInterrupt(digitalPinToInterrupt(INT2_PIN),INT2_ISR,RISING);    

  // Should be dynamic interrupts signals but lets use static logic signals for now
  pinMode(CODE_PIN,INPUT);
  pinMode(BTX_PIN,INPUT);  
  //attachPCINT(digitalPinToPCINT(CODE_PIN),Code_ISR,CHANGE);
  attachInterrupt(digitalPinToInterrupt(CODE_PIN),Code_ISR,CHANGE);
}

void loop() 
{
  static int code_last = digitalRead(CODE_PIN);
  static int BTX_last = digitalRead(BTX_PIN);
  
  // Check code and BT X or D
  int code = digitalRead(CODE_PIN);
  int BTX = digitalRead(BTX_PIN);

  // Rising-edge
  if(code == HIGH and code_last == LOW)
  {
    StateMachine.codeEntered();    
  }
  code_last = code;
  // Rising-edge
  if(BTX == HIGH and BTX_last == LOW)
  {
    StateMachine.BTConnection();
  }
  // Falling-edge
  else if(BTX == LOW and BTX_last == HIGH)
  {
    StateMachine.BTConnection(false);
  }
  BTX_last = BTX;
  
  StateMachine.updateTimers();
  StateMachine.processPowerState();
  
  if(LastState != StateMachine.getPowerState() || LastDMode != StateMachine.getDisplayMode())
  {
    LastState = StateMachine.getPowerState();
    LastDMode = StateMachine.getDisplayMode();
    Serial.println(""); Serial.print("PowerState: "); Serial.print(StateMachine.getPowerStateStr()); Serial.print("\tDisplayMode: "); Serial.print(StateMachine.getDisplayModeStr());
  }
  delay(300);
}

void Code_ISR()
{
  static int code_last = digitalRead(CODE_PIN);
  
   // Check code and BT X or D
  int code = digitalRead(CODE_PIN);

   // Rising-edge
  if(code == HIGH and code_last == LOW)
  {
    StateMachine.codeEntered();    
  }
  code_last = code;
}

// Maybe debounce the input? Check on oscillo INT1 and INT2. Is it really 500ms square pulse? TRY PULLDOWNS on both INT
void INT1_ISR()
{ 
  static int lastpinstate = LOW;
  static unsigned long last_interrupt_time1 = 0;
  unsigned long interrupt_time1 = millis();
  int pinstate = digitalRead(INT1_PIN);
  // If interrupts come faster than DEBOUNCE_INT1 time (ms), assume it's a bounce and ignore
  if (interrupt_time1 - last_interrupt_time1 > DEBOUNCE_INT1) 
  {
    // RISING_EDGE
    if(lastpinstate == LOW && pinstate == HIGH)
    {
      Serial.println(""); Serial.print("Started moving");
      StateMachine.motionDetected();
      lastpinstate = pinstate;
    }
    else if(lastpinstate == HIGH && pinstate == LOW)
    {
      Serial.println(""); Serial.print("Stopped moving");
      StateMachine.motionDetected(false);
      lastpinstate = pinstate;
    }       
  }
  last_interrupt_time1 = interrupt_time1;
  
}

void INT2_ISR()
{ 
  static unsigned long last_interrupt_time2 = 0;
  unsigned long interrupt_time2 = millis();
  // If interrupts come faster than DEBOUNCE_INT2 time (ms), assume it's a bounce and ignore
  if (interrupt_time2 - last_interrupt_time2 > DEBOUNCE_INT2) 
  {
    Serial.println(""); Serial.print("Tap detected");
    StateMachine.tapDetected();
  }
  last_interrupt_time2 = interrupt_time2;
}

// Helper function

/*
 PrintHex routines for Arduino: to print byte or word data in hex with
 leading zeroes.
 Copyright (C) 2010 Kairama Inc

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex with leading zeroes
{
       Serial.print("0x"); 
       for (int i=0; i<length; i++) { 
         if (data[i]<0x10) {Serial.print("0");} 
         Serial.print(data[i],HEX);          
       }
       Serial.println("");
}

void PrintHex16(uint16_t *data, uint8_t length) // prints 16-bit data in hex with leading zeroes
{
       Serial.print("0x"); 
       for (int i=0; i<length; i++)
       { 
         uint8_t MSB=byte(data[i]>>8);
         uint8_t LSB=byte(data[i]);
         
         if (MSB<0x10) {Serial.print("0");} Serial.print(MSB,HEX); 
         if (LSB<0x10) {Serial.print("0");} Serial.print(LSB,HEX); 
       }
       Serial.println("");
}
