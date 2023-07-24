/*
SmartHalo FirmWare Beta 1.2
 Material: 
 Adafruit BlueFruit NRF8001 Bluetooth Transceiver
 Adafruit NeoPixel Ring 12
 ATMEGA328P-AU Pkg, Arduino chip
 Method:
 Serial debugging messages, no interrupts, serialEvents
 
 Floriane Ennaji (May 2015)
*/

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Adafruit_BLE_UART.h>

#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2     // This should be an interrupt pin, on Uno thats #2 or #3
#define ADAFRUITBLE_RST 9

//Macro pour lire l'état d'un bit
#define BIT_STATE(variable,position) ((variable)&(1<<(position)))

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

#define PIN            6  // NEOPIXEL    
#define R	       5  // RED   PIXEL
#define G              7  // GREEN PIXEL
#define B              9  // BLUE  PIXEL
#define NUMPIXELS      24
#define EIGHTH         3
#define QUARTER        6
#define HALF           12
#define LQUARTER       18
//#define BRIGHTNESS     250
#define CATCHUP        2

//  if (NUMPIXELS%8 == 0){
//    eighth = NUMPIXELS/8
//    quarter = NUMPIXELS/4
//    half = NUMPIXELS/2
//  } else {
//    eighth = NUMPIXELS/8 + 1
//    quarter = NUMPIXELS/4 +1
//    half = NUMPIXELS/2 +1
//  }
//  lQuarter = half + half - quarter

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel centerPix = Adafruit_NeoPixel(1, 7, NEO_GRB + NEO_KHZ800);
uint8_t typeOfInstruction; //Will be used to know if we're in the middle of an instruction
uint8_t navDetails[3] = {(uint8_t) 0,(uint8_t) 0,(uint8_t) 0}; //Direction, progression and step of animation
int brightness;
int frontLedBright = 125;
int on = 0;

void setup() {
  Serial.begin(9600);
  while(!Serial); //on attend la fin de l'initialisation
  pixels.begin(); //on initialise la bibliotheque Neopixel
  centerPix.begin();
  brightness = 230;
  pixels.setBrightness(brightness);
  centerPix.setBrightness(brightness);
  BTLEserial.begin();
  typeOfInstruction = (uint8_t) 9; //Will be used to know if we're in the middle of an instruction
  pinMode(3, OUTPUT);
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;

void loop() {
  BTLEserial.pollACI();
  manageCommunication();
}

//turn off all the lights
void turnOff(){
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0)); 
    pixels.show(); 
  }
}

void turnCenterOff(){
  centerPix.setPixelColor(0, pixels.Color(0,0,0)); 
  centerPix.show(); 
}

//read the first byte to know the type of the instruction, then call adapted function with correct parameters
void manageCommunication(){
   // Tell the nRF8001 to do whatever it should be working on.
  
  // Ask what is our current status
  aci_evt_opcode_t status = BTLEserial.getState();
  // If the status changed....
  if (status != laststatus) {
    // print it out!
    if (status == ACI_EVT_DEVICE_STARTED) {
      Serial.println(F("* Advertising started"));
      returnToOriginal();
    }
    if (status == ACI_EVT_CONNECTED) {
      Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
      Serial.println(F("* Disconnected or advertising timed out"));
    }
    // OK set the last status change to this one
    laststatus = status;
    returnToOriginal();
  }

  if (status == ACI_EVT_CONNECTED) {
    // Lets see if there's any data for us!
    if (BTLEserial.available()) {
      Serial.print("* "); 
      Serial.print(BTLEserial.available()); 
      Serial.println(F(" bytes available from BTLE"));
    }
    while (BTLEserial.available()) {
      uint8_t data = BTLEserial.read();
      delay(10);
      
      Serial.print("Data recieved :");
      Serial.println(data , DEC);
      if (data < 10 && data != typeOfInstruction){    //TODO : Don't forget to reset typeOfInstruction to uint8_t(10)
        switch(data){
          case (uint8_t) 0 : // End Byte
            Serial.println("-------------------------End byte");
            //Setting everything to how they were at the setup step
              navDetails[0] = (uint8_t) 0;
              navDetails[1] = (uint8_t) 0;
              typeOfInstruction = (uint8_t) 9;
            break;
          case (uint8_t) 1 : //Navigation
            Serial.println("-------------------------Nav");
            typeOfInstruction = data;
            break;
          case (uint8_t) 2 : //Front LED
            if (on == 1) {
              turnFrontLedOff();
              on = 0;
            } else {
              turnFrontLedOn();
              on = 1;
            }
            break;
          case (uint8_t) 3 : //Center pix notification
            typeOfInstruction = data;
            break;
          case (uint8_t) 4 : //TODO :Sound
            break;
          case (uint8_t) 5 : //Brightness
            typeOfInstruction = data;
            break;
          case (uint8_t) 6 : //Front LED brighness
            typeOfInstruction = data;
            break; 
          case (uint8_t) 7 : //Goal completion
            typeOfInstruction = data;
            break;
          case (uint8_t) 8 : //TODO : Initial pairing
            pairingSequence();
            break;
        }
      } else {
        Serial.println("-------------------------In an instruction");
        switch (typeOfInstruction){
          
          case (uint8_t) 1 :    //Navigation
          
            if (navDetails[0] == (uint8_t) 0){
              navDetails[0] = data; //direction
            } else {
              if (navDetails[1] == (uint8_t) 0){
                navDetails[1] = data; //progression
              }
            }
            
            if (navDetails[1] != (uint8_t) 0){ //Everything ready to start turning on LEDs
              Serial.println("-------------------------Everything ready to start turning on LEDs");
              uint8_t dir = navDetails[0];
              uint8_t prog = navDetails[1];
              uint8_t stepOfAnim = navDetails[2];
              
              if (prog == uint8_t(10)){
                returnToOriginal();
              } else {
                prog = uint8_t(prog - 10);
               switch (dir) {
                  case (uint8_t) 10 :    //Straight -- 8 steps + turn off
                    Serial.println("Straight");
                    straight();
                    break;
                  case (uint8_t) 11 :    //Slight Right -- 5 steps + turn off
                    Serial.println("Slight Right");
                    if (stepOfAnim != prog) {
                      Serial.println("In the if !!");
                      if (prog == stepOfAnim+1) {
                        slightRight(prog);
                        navDetails[2] = prog;
                      } else {
                        do {
                          slightRight(stepOfAnim);
                          stepOfAnim = stepOfAnim+1;
                          delay(CATCHUP);
                        } while (stepOfAnim != prog);
                        slightRight(prog);
                        navDetails[2] = prog;
                      }
                    }
                    break;
                  case (uint8_t) 12 :    //Right -- 8 steps + turn off
                    Serial.println("Right");
                    if (stepOfAnim != prog) {
                      if (prog == stepOfAnim+1) {
                        right(prog);
                        navDetails[2] = prog;
                      } else {
                        do {
                          right(stepOfAnim);
                          stepOfAnim = stepOfAnim+1;
                          delay(CATCHUP);
                        } while (stepOfAnim != prog);
                        right(prog);
                        navDetails[2] = prog;
                      }
                    }
                    break;
                  case (uint8_t) 13 :    //Hard Right -- 5 steps + turn off
                    Serial.println("Hard Right");
                    if (stepOfAnim != prog) {
                      if (prog == stepOfAnim+1) {
                        hardRight(prog);
                        navDetails[2] = prog;
                      } else {
                        do {
                          hardRight(stepOfAnim);
                          stepOfAnim = stepOfAnim+1;
                          delay(CATCHUP);
                        } while (stepOfAnim != prog);
                        hardRight(prog);
                        navDetails[2] = prog;
                      }
                    }
                    break;
                  case (uint8_t) 14 :    //U Turn -- 1 step + turn off
                    Serial.println("uTurn");
                    uTurn();
                    break;
                  case (uint8_t) 15 :    //Hard Left -- 5 steps + turn off
                    Serial.println("Hard Left");
                    if (stepOfAnim != prog) {
                      Serial.println("In the if !!");
                      if (prog == stepOfAnim+1) {
                        hardLeft(prog);
                        navDetails[2] = prog;
                      } else {
                        do {
                          hardLeft(stepOfAnim);
                          stepOfAnim = stepOfAnim+1;
                          delay(CATCHUP);
                        } while (stepOfAnim != prog);
                        hardLeft(prog);
                        navDetails[2] = prog;
                      }
                    }
                    break;
                  case (uint8_t) 16 :    //Left -- 8 steps + turn off
                    Serial.println("Left");

                    if (stepOfAnim != prog) {
                      Serial.println("In the if !!");
                      if (prog == stepOfAnim+1) {
                        left(prog);
                        navDetails[2] = prog;
                      } else {
                        do {
                          left(stepOfAnim);
                          stepOfAnim = stepOfAnim+1;
                          delay(CATCHUP);
                        } while (stepOfAnim != prog);
                        left(prog);
                        navDetails[2] = prog;
                      }
                    }
                    break; 
                  case (uint8_t) 17 :    //Slight Left -- 5 steps + turn off
                    Serial.println("Slight left");
                    if (stepOfAnim != prog) {
                      Serial.println("In the if !!");
                      if (prog == stepOfAnim+1) {
                        slightLeft(prog);
                        navDetails[2] = prog;
                      } else {
                        do {
                          slightLeft(stepOfAnim);
                          stepOfAnim = stepOfAnim+1;
                          delay(CATCHUP);
                        } while (stepOfAnim != prog);
                        slightLeft(prog);
                        navDetails[2] = prog;
                      }
                    }
                    break;
                 case (uint8_t) 18 :    //Destination
                    destination();
                    break;
                    
                }
              }
            }
            break;
            case (uint8_t) 3 : //Center pix notification
              if (data == (uint8_t) 10) {
                callAnim();
              } else if (data == (uint8_t) 11) {
                SMSAnim(); 
              } else if (data == (uint8_t) 12) {
                alarm();
              }
            break;
            case (uint8_t) 5 : //Brightness
              if (data == (uint8_t) 10) {
                turnOff();
              } else {
                brightness = (int) data;
              }
            break;
            case (uint8_t) 6 : //Front LED Brightness
              frontLedBright = (int) data;
            break;
            case (uint8_t) 7 : //Goal completion
              goalCompletion(data);
            break;
//          case (uint8_t) 4 :  //TODO : sound
//            break;            
          }
        }
    }
  }
}

void returnToOriginal(){
  turnOff();
  typeOfInstruction = (uint8_t) 9;
  navDetails[0] = (uint8_t) 0;
  navDetails[1] = (uint8_t) 0;
  navDetails[2] = (uint8_t) 0;
}

void straight(){
  pixels.setBrightness(brightness);

  Serial.println("ooooooo In the straight func !!");
  
  for (int i=0;i<16;i++){
    for (int j=LQUARTER;j<NUMPIXELS;j++){
      pixels.setPixelColor(j, pixels.Color(0,16*i,0));
      pixels.show();
      delay(2);
      pixels.setPixelColor(NUMPIXELS-j, pixels.Color(0,16*i,0));
      pixels.show();
      delay(2);
    }
    pixels.setPixelColor(0, pixels.Color(0,16*i,0));
    pixels.show();
    delay(2);
    delay(6);
  }
}

void slightRight(uint8_t fifth){
  Serial.println("Slight right inside");  

  pixels.setBrightness(brightness);
  
  switch (fifth) {  //Which step we are
  
    case (uint8_t) 1 :  // Everything white
      for (int i=0;i<16;i++){
        for (int j=0;j<QUARTER+1;j++){
          pixels.setPixelColor(j, pixels.Color(16*i,16*i,16*i));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
    break;
    
    case (uint8_t) 2 :  //First and last LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(0, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(QUARTER, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break; 
    
    case (uint8_t) 3 :  //Second set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(QUARTER-1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 4 :  //Third set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(QUARTER-2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 5 :  //Fading in and out to blink
      //How will we stop blinking ??
      for (int k=0;k<6;k++){
        for (int i=15;i>0;i--){
          for (int j=0;j<QUARTER+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(2);
          }
          delay(10);
        }
        for (int i=0;i<16;i++){
          for (int j=0;j<QUARTER+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(2);
          }
          delay(10);
        }
      }
      for (int i=15;i>0;i--){
        for (int j=0;j<QUARTER+1;j++){
          pixels.setPixelColor(j, pixels.Color(0,16*i,0));
          pixels.show();
          delay(2);
        }
        delay(10);
      }
      turnOff();
    break;
  }
}

void right(uint8_t eighth){
    pixels.setBrightness(brightness);
  
  switch (eighth) {  //Which step we are
  
    case (uint8_t) 1 :  // Everything white
      for (int i=0;i<16;i++){
        for (int j=0;j<HALF+1;j++){
          pixels.setPixelColor(j, pixels.Color(16*i,16*i,16*i));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
    break;
    
    case (uint8_t) 2 :  //First and last LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(0, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break; 
    
    case (uint8_t) 3 :  //Second set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 4 :  //Third set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 5 :  //Fourth set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 6 :  //Fifth set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(4, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-4, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 7 :  //Sixth set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(5, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-6, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-5, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 8 :  //Fading in and out to blink
      //How will we stop blinking ??
      for (int k=0;k<6;k++){
        for (int i=15;i>0;i--){
          for (int j=0;j<HALF+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          delay(6);
        }
        for (int i=0;i<16;i++){
          for (int j=0;j<HALF+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          delay(6);
        }
      }
      for (int i=15;i>0;i--){
        for (int j=0;j<HALF+1;j++){
          pixels.setPixelColor(j, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
      turnOff();
    break;
  }
}

void hardRight(uint8_t fifth){
  pixels.setBrightness(brightness);
  
  switch (fifth) {  //Which step we are
  
    case (uint8_t) 1 :  // Everything white
      for (int i=0;i<16;i++){
        for (int j=QUARTER;j<HALF+1;j++){
          pixels.setPixelColor(j, pixels.Color(16*i,16*i,16*i));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
    break;
    
    case (uint8_t) 2 :  //First and last LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(QUARTER, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break; 
    
    case (uint8_t) 3 :  //Second set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(QUARTER+1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 4 :  //Third set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(QUARTER+2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(HALF-2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 5 :  //Fading in and out to blink
      //How will we stop blinking ??
      for (int k=0;k<6;k++){
        for (int i=15;i>0;i--){
          for (int j=QUARTER;j<HALF+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          delay(6);
        }
        for (int i=0;i<16;i++){
          for (int j=QUARTER;j<HALF+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          delay(6);
        }
      }
      for (int i=15;i>0;i--){
        for (int j=QUARTER;j<HALF+1;j++){
          pixels.setPixelColor(j, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
      turnOff();
    break;
  }

}

void uTurn(){
  //Fading in and out to blink
  //How will we stop blinking ??
  pixels.setBrightness(brightness);
  for (int k=0;k<8;k++){
    for (int i=0;i<16;i++){
      for (int j=QUARTER;j<LQUARTER+1;j++){
         pixels.setPixelColor(j, pixels.Color(16*i,0,0));
         pixels.show();
         delay(1);
      }
     delay(6);
    }
    for (int i=15;i>0;i--){
      for (int j=QUARTER;j<LQUARTER+1;j++){
         pixels.setPixelColor(j, pixels.Color(16*i,0,0));
         pixels.show();
         delay(1);
      }
     delay(6);
    }
  }
  turnOff();
}

void hardLeft(uint8_t fifth){
  pixels.setBrightness(brightness);
  
  switch (fifth) {  //Which step we are
  
    case (uint8_t) 1 :  // Everything white
      for (int i=0;i<16;i++){
        for (int j=HALF;j<LQUARTER+1;j++){
          pixels.setPixelColor(j, pixels.Color(16*i,16*i,16*i));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
    break;
    
    case (uint8_t) 2 :  //First and last LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(LQUARTER, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break; 
    
    case (uint8_t) 3 :  //Second set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(LQUARTER-1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 4 :  //Third set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(LQUARTER-3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(LQUARTER-2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 5 :  //Fading in and out to blink
      //How will we stop blinking ??
      for (int k=0;k<6;k++){
        for (int i=15;i>0;i--){
          for (int j=HALF;j<LQUARTER+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          delay(6);
        }
        for (int i=0;i<16;i++){
          for (int j=HALF;j<LQUARTER+1;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          delay(6);
        }
      }
      for (int i=15;i>0;i--){
        for (int j=HALF;j<LQUARTER+1;j++){
          pixels.setPixelColor(j, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
        }
        delay(6);
      }
      turnOff();
    break;
  }
}

void left(uint8_t eighth){
    pixels.setBrightness(brightness);
  
  switch (eighth) {  //Which step we are
  
    case (uint8_t) 1 :  // Everything white
      for (int i=0;i<16;i++){
        for (int j=HALF;j<NUMPIXELS;j++){
          pixels.setPixelColor(j, pixels.Color(16*i,16*i,16*i));
          pixels.show();
          delay(1);
        }
        pixels.setPixelColor(0, pixels.Color(16*i,16*i,16*i));
        pixels.show();
        delay(1);
        delay(6);
      }
    break;
    
    case (uint8_t) 2 :  //First and last LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(0, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break; 
    
    case (uint8_t) 3 :  //Second set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 4 :  //Third set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 5 :  //Fourth set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 6 :  //Fifth set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+4, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-4, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 7 :  //Sixth set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(HALF+5, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-6, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-5, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 8 :  //Fading in and out to blink
      //How will we stop blinking ??
      for (int k=0;k<6;k++){
        for (int i=15;i>0;i--){
          for (int j=HALF;j<NUMPIXELS;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          pixels.setPixelColor(0, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
          delay(6);
        }
        for (int i=0;i<16;i++){
          for (int j=HALF;j<NUMPIXELS;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          pixels.setPixelColor(0, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
          delay(6);
        }
      }
      for (int i=15;i>0;i--){
        for (int j=HALF;j<NUMPIXELS;j++){
          pixels.setPixelColor(j, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
        }
        pixels.setPixelColor(0, pixels.Color(0,16*i,0));
        pixels.show();
        delay(1);
        delay(6);
      }
      turnOff();
    break;
  }
}
void slightLeft(uint8_t fifth){
  pixels.setBrightness(brightness);
  
  switch (fifth) {  //Which step we are
  
    case (uint8_t) 1 :  // Everything white
      for (int i=0;i<16;i++){
        for (int j=LQUARTER;j<NUMPIXELS;j++){
          pixels.setPixelColor(j, pixels.Color(16*i,16*i,16*i));
          pixels.show();
          delay(1);
        }
        pixels.setPixelColor(0, pixels.Color(16*i,16*i,16*i));
        pixels.show();
        delay(1);
        delay(6);
      }
    break;
    
    case (uint8_t) 2 :  //First and last LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(LQUARTER, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(0, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break; 
    
    case (uint8_t) 3 :  //Second set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(LQUARTER+1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-1, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;

    case (uint8_t) 4 :  //Third set of LED green
      for (int i=0;i<24;i++){
        pixels.setPixelColor(LQUARTER+2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-3, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
        pixels.setPixelColor(NUMPIXELS-2, pixels.Color(0,64+8*i,0));
        pixels.show();
        delay(10);
      }
    break;
    
    case (uint8_t) 5 :  //Fading in and out to blink
      //How will we stop blinking ??
      for (int k=0;k<6;k++){
         for (int i=15;i>0;i--){
          for (int j=LQUARTER;j<NUMPIXELS;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          pixels.setPixelColor(0, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
          delay(6);
        }
        for (int i=0;i<16;i++){
          for (int j=LQUARTER;j<NUMPIXELS;j++){
            pixels.setPixelColor(j, pixels.Color(0,16*i,0));
            pixels.show();
            delay(1);
          }
          pixels.setPixelColor(0, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
          delay(6);
        }
      }
      for (int i=15;i>0;i--){
        for (int j=LQUARTER;j<NUMPIXELS;j++){
          pixels.setPixelColor(j, pixels.Color(0,16*i,0));
          pixels.show();
          delay(1);
        }
        pixels.setPixelColor(0, pixels.Color(0,16*i,0));
        pixels.show();
        delay(1);
        delay(6);
      }
      turnOff();
      delay(2);
    break;
  }
}

void destination(){
  pixels.setBrightness(brightness);
  
  for (int k=0;k<4;k++){
    for (int i=0;i<16;i++){
      for (int j=0;j<NUMPIXELS;j++){
         pixels.setPixelColor(j, pixels.Color(0,16*i,0));
         pixels.show();
      }
     delay(8);
    }
    for (int i=15;i>0;i--){
      for (int j=0;j<NUMPIXELS;j++){
         pixels.setPixelColor(j, pixels.Color(0,16*i,0));
         pixels.show();
      }
     delay(8);
    }
  }
  turnOff();
}

void pairingSequence() {
  pixels.setBrightness(brightness);
  pairing(0,NUMPIXELS-1);
// For more circles : 
//  pairingTransition();
//  pairing(5,NUMPIXELS);
//  pairingTransition();
//  pairing(5,NUMPIXELS-1);
  pairingFade();
  success();
}

void pairing(int imin, int imax) {
  for(int i=imin;i<imax;i++) {
    pixels.setPixelColor(i, pixels.Color(100,100,100)); 
    pixels.setPixelColor(i-1, pixels.Color(75,75,75)); 
    pixels.setPixelColor(i-2, pixels.Color(50,50,50)); 
    pixels.setPixelColor(i-3, pixels.Color(25,25,25)); 
    pixels.setPixelColor(i-4, pixels.Color(10,10,10)); 
    pixels.setPixelColor(i-5, pixels.Color(5,5,5));
    pixels.show(); 
    delay(30); 
    pixels.clear();
  }
}

void pairingTransition() {
  pixels.setPixelColor(0, pixels.Color(100,100,100));
  pixels.setPixelColor(NUMPIXELS-1, pixels.Color(75,75,75)); 
  pixels.setPixelColor(NUMPIXELS-2, pixels.Color(50,50,50));
  pixels.setPixelColor(NUMPIXELS-3, pixels.Color(25,25,25)); 
  pixels.setPixelColor(NUMPIXELS-4, pixels.Color(10,10,10)); 
  pixels.setPixelColor(NUMPIXELS-5, pixels.Color(5,5,5));
  pixels.show(); 
  delay(30); 
  pixels.clear();
  
  pixels.setPixelColor(1, pixels.Color(100,100,100));
  pixels.setPixelColor(0, pixels.Color(75,75,75)); 
  pixels.setPixelColor(NUMPIXELS-1, pixels.Color(50,50,50));
  pixels.setPixelColor(NUMPIXELS-2, pixels.Color(25,25,25)); 
  pixels.setPixelColor(NUMPIXELS-3, pixels.Color(10,10,10)); 
  pixels.setPixelColor(NUMPIXELS-4, pixels.Color(5,5,5));
  pixels.show(); 
  delay(30); 
  pixels.clear();
 
  pixels.setPixelColor(2, pixels.Color(100,100,100));
  pixels.setPixelColor(1, pixels.Color(75,75,75)); 
  pixels.setPixelColor(0, pixels.Color(50,50,50));
  pixels.setPixelColor(NUMPIXELS-1, pixels.Color(25,25,25)); 
  pixels.setPixelColor(NUMPIXELS-2, pixels.Color(10,10,10)); 
  pixels.setPixelColor(NUMPIXELS-3, pixels.Color(5,5,5));
  pixels.show(); 
  delay(30); 
  pixels.clear();
  
  pixels.setPixelColor(3, pixels.Color(100,100,100));
  pixels.setPixelColor(2, pixels.Color(75,75,75)); 
  pixels.setPixelColor(1, pixels.Color(50,50,50));
  pixels.setPixelColor(0, pixels.Color(25,25,25)); 
  pixels.setPixelColor(NUMPIXELS-1, pixels.Color(10,10,10)); 
  pixels.setPixelColor(NUMPIXELS-2, pixels.Color(5,5,5));
  pixels.show(); 
  delay(30); 
  pixels.clear();
  
  pixels.setPixelColor(4, pixels.Color(100,100,100));
  pixels.setPixelColor(3, pixels.Color(75,75,75)); 
  pixels.setPixelColor(2, pixels.Color(50,50,50));
  pixels.setPixelColor(1, pixels.Color(25,25,25)); 
  pixels.setPixelColor(0, pixels.Color(10,10,10)); 
  pixels.setPixelColor(NUMPIXELS-1, pixels.Color(5,5,5));
  pixels.show(); 
  delay(30); 
  pixels.clear();
}

void pairingFade() {
  pixels.setPixelColor(NUMPIXELS-1, pixels.Color(100,100,100)); 
  pixels.setPixelColor(NUMPIXELS-2, pixels.Color(75,75,75)); 
  pixels.setPixelColor(NUMPIXELS-3, pixels.Color(50,50,50)); 
  pixels.setPixelColor(NUMPIXELS-4, pixels.Color(25,25,25)); 
  pixels.setPixelColor(NUMPIXELS-5, pixels.Color(10,10,10)); 
  pixels.setPixelColor(NUMPIXELS-6, pixels.Color(5,5,5));
  pixels.show(); 
  delay(30); 
  
  for (int i = 0 ; i < NUMPIXELS ; i++) {
    pixels.setPixelColor(i, pixels.Color(100, 100, 100));
    pixels.show(); 
    delay(30); 
  }
  delay(200);
  for (int i = 255 ; i > 150 ; i--) {
    delay(2);
    pixels.setBrightness(i);
    pixels.show(); 
  }

}

void success() {
  // fade in
  for (int i=0; i<255; i++) {
    setColors();
    pixels.setBrightness(i);
    pixels.show();
    delay(4);
  }

  delay(100);

  // fade out
  for (int i=255; i>0; i--) {
    setColors();
    pixels.setBrightness(i);
    pixels.show();
    delay(4);
  }
  turnOff();
}

void callAnim() {
  centerPix.setBrightness(brightness);
  for (int i = 0; i<8; i++) {
    blueCenterBlink(4,4);
  }
  blueCenterBlink(4,80);
}

void SMSAnim() {
  centerPix.setBrightness(brightness);
  blueCenterBlink(4,4);
  blueCenterBlink(4,40);
}

void blueCenterBlink(int durationOn, int durationOff) {
  for (int i = 0; i<16; i++) {
    centerPix.setPixelColor(0, pixels.Color(0, 0, 16*i));
    centerPix.show();
    delay(durationOn);
  }
  for (int i = 15; i>0; i--) {
    centerPix.setPixelColor(0, pixels.Color(0, 0, 16*i));
    centerPix.show();
    delay(durationOff);
  }
  turnCenterOff();
}

void alarm() {
  pixels.setBrightness(brightness);
  
  for (int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(255,0,0));
    pixels.show();
    delay(300);
  }
  turnOff();
  for (int j=0;j<40;j++){
    delay(100);
    for (int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(255,0,0));
      pixels.show();
    }
    delay(100);
    turnOff();
  }
  
}

void goalCompletion(uint8_t completion) {
  pixels.setBrightness(brightness);
  for (int i = 0; i<completion;i++){
    pixels.setPixelColor(i, pixels.Color(232, 16, 107));
    pixels.show();
    delay(50);
  }
  delay(1000);
  for (int j=15;j>0;j--) {
    for (int i=0;i<completion;i++) {
      pixels.setPixelColor(i, pixels.Color(15*j, j, 7*j));
      pixels.show();
      delay(4);
    }
    delay(10);
  }
  turnOff();
}

void turnFrontLedOff(){
  for (int i=frontLedBright;i>0;i--) {
    analogWrite(3,i);
    delay(8);
  }
  analogWrite(3,0); 
}

void turnFrontLedOn(){
  for (int i = 0;i<frontLedBright;i++) {
    analogWrite(3,i);
    delay(8);
  }
//  analogWrite(3,frontLedBright);
}

void setColors() {
//if (B>R) { 
//    color=cool
  pixels.setPixelColor(0, pixels.Color(123, 77, 155));
  //(87, 97, 171)
  pixels.setPixelColor(1, pixels.Color(87, 97, 171));
  //(50, 118, 187)
  pixels.setPixelColor(2, pixels.Color(50, 118, 187));
  //(14, 138, 203) ****BLEU
  pixels.setPixelColor(3, pixels.Color(14, 138, 203));
  //(18, 145, 188)
  pixels.setPixelColor(4, pixels.Color(18, 145, 188));
  //(22, 152, 174)
  pixels.setPixelColor(5, pixels.Color(22, 152, 174));
  //(26, 159, 159)
  pixels.setPixelColor(6, pixels.Color(26, 159, 159));
  //(29, 166, 144)
  pixels.setPixelColor(7, pixels.Color(29, 166, 144));
  //(33, 173, 130)
  pixels.setPixelColor(8, pixels.Color(33, 173, 130));
  //(37, 180, 115) ****VERT
  pixels.setPixelColor(9, pixels.Color(37, 180, 115));
  //(73, 187, 100)
  pixels.setPixelColor(10, pixels.Color(73, 187, 100));
  //(110, 194, 86)
  pixels.setPixelColor(11, pixels.Color(110, 194, 86));
  //(146, 201, 71)
  pixels.setPixelColor(12, pixels.Color(146, 201, 71));
  //(182, 208, 56)
  pixels.setPixelColor(13, pixels.Color(182, 208, 56));
  //(219, 215, 42)
  pixels.setPixelColor(14, pixels.Color(219, 215, 42));
  //(255, 222, 27) **** JAUNE
  pixels.setPixelColor(15, pixels.Color(255, 222, 27));
  //(251, 188, 40)
  pixels.setPixelColor(16, pixels.Color(251, 188, 40));
  //(247, 153, 54)
  pixels.setPixelColor(17, pixels.Color(247, 153, 54));
  //(243, 119, 67)
  pixels.setPixelColor(18, pixels.Color(243, 119, 67));
  //(240, 85, 80)
  pixels.setPixelColor(19, pixels.Color(240, 85, 80));
  //(236, 50, 94)
  pixels.setPixelColor(20, pixels.Color(236, 50, 94));
  //(232, 16, 107) ***ROSEROUGE
  pixels.setPixelColor(21, pixels.Color(232, 16, 107));
  //(196, 36, 123)
  pixels.setPixelColor(22, pixels.Color(196, 36, 123));
  //(159, 57, 139)
  pixels.setPixelColor(23, pixels.Color(159, 57, 139));
  //(123, 77, 155)
}
