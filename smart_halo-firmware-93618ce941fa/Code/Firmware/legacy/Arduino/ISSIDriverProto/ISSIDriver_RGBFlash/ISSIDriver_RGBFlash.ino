// ISSI_Driver
// by Sebastien Gelinas

// Driver to control ISSI LED Test Board
// Communication through I2C
// 

// Created 28 September 2015

// 

#include <Wire.h>

const uint8_t PROGMEM gamma[] = {
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

// Choose the board to be programmed: LRTB_GVTG or LRTB_R98G or LTSN_N213EGBW
#define LTSN_N213EGBW

// I2C 7-bit address must be right-shifted to the LSB (LSB is to indicate read/write but is unused)
#define ISSI1_ADDR         ((0x78 >> 1) & (0x7F))
#define ISSI2_ADDR         ((0x7E >> 1) & (0x7F))

// ISSI driver register addresses
#define LED_EN_REG         0x26
#define GLOBAL_EN_REG      0x4A
#define UPDATE_REG         0x25
#define SHUTDOWN_REG       0x00
#define PWM_REG            0x01

// Chip enable pin
#define SDB_PIN 2
#define C_R 8
#define C_G 9
#define C_B 10

enum LEDMODE {OFF = 0, ALLWHITE, HALFWHITE, LOGO, FLASHRGB, SMILE};
enum POWERMODE {MAX = 0, HALF = 1, THIRD = 2, FOURTH = 3};
LEDMODE ledmode = HALFWHITE;
POWERMODE powermode = HALF;

// Constants
#define NUMPIXELS          24

#if defined(LRTB_GVTG)
////const unsigned char DIODENAMES[NUMPIXELS] = {D43,D42,D41,D40,D39,D38, 
////                                             D37,D36,D35,D34,D33,D32, 
////                                             D31,D30,D29,D28,D27,D26, 
////                                             D25,D48,D47,D46,D45,D44};
//const unsigned char DRIVERADDR[NUMPIXELS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
//                                               ISSI2_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
//const unsigned char DRIVERIDX[NUMPIXELS]       = {19,16,13,10,7,4,
//                                                  1,34,31,28,25,22,
//                                                  19,16,13,10,7,4,
//                                                  1,34,31,28,25,22};
#elif defined(LRTB_R98G)
//const unsigned char DIODENAMES[NUMPIXELS] = {LED19/42,LED20/43,LED21/44,LED22/45,LED23/46,LED24/47 
//                                             LED25/48,LED26/49,LED27/50,LED28/51,LED29/52,LED6/53, 
//                                             LED7/30,LED8/31,LED9/32,LED10/33,LED11/34,LED12/35, 
//                                             LED13/36,LED14/37,LED15/38,LED16/39,LED17/40,LED18/41};
//const unsigned char DRIVERADDR[NUMPIXELS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
//                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
//const unsigned char DRIVERIDX[NUMPIXELS]       = {16,13,10,7,4,1,
//                                                  34,31,28,25,22,19,
//                                                  16,13,10,7,4,1,
//                                                  34,31,28,25,22,19};    
#elif defined(LTSN_N213EGBW)
//const unsigned char DIODENAMES[NUMPIXELS] = {LED19/42,LED20/43,LED21/44,LED22/45,LED23/46,LED24/47 
//                                             LED25/48,LED26/49,LED27/50,LED28/51,LED29/52,LED6/53, 
//                                             LED7/30,LED8/31,LED9/32,LED10/33,LED11/34,LED12/35, 
//                                             LED13/36,LED14/37,LED15/38,LED16/39,LED17/40,LED18/41};
const unsigned char DRIVERADDR[NUMPIXELS]   = {ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
                                               ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,ISSI1_ADDR,
                                               ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR,ISSI2_ADDR};
const unsigned char DRIVERIDX[NUMPIXELS]       = {16,13,10,7,4,1,
                                                  34,31,28,25,22,19,
                                                  16,13,10,7,4,1,
                                                  34,31,28,25,22,19};                                                                                                    
#endif

#define BUFFERLEN 16
int cnt = 0;
char RxBuff[BUFFERLEN];

void enableAll(int disable=0);
void showPixel(uint16_t n, int show=1);
void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void SweepRGBLEDs();

void setup()
{
  Serial.begin(9600);
  Serial.println("Welcome to ISSI LED Driver test program");
  
  // Chip enable
  pinMode(SDB_PIN,OUTPUT);
  digitalWrite(SDB_PIN,HIGH);
  pinMode(C_R,OUTPUT);
  pinMode(C_G,OUTPUT);
  pinMode(C_B,OUTPUT);
  digitalWrite(C_R,LOW);
  digitalWrite(C_G,LOW);
  digitalWrite(C_B,LOW);
  
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.beginTransmission(ISSI1_ADDR);
  Wire.write(0);
  Wire.write(1); // Set driver to normal operation
  Wire.endTransmission();
  Wire.begin(); // join i2c bus (address optional for master)
  Wire.beginTransmission(ISSI2_ADDR);
  Wire.write(0);
  Wire.write(1); // Set driver to normal operation
  Wire.endTransmission();
  enableAll();
  
  ledmode=ALLWHITE;
  powermode=HALF;
  SetAllWhite();
   
  Serial.println("Enter (W)hite, (H)alf white, (L)ogo, (F)lashRGB, (S)mile or (O)ff to toggle mode of operation");  
  Serial.println("Select power mode: (0) Max power, (1) Half power, (2) Third of power, (3) Fourth of power");
}


void loop()
{  
   while( Serial.available() > 0 )
   {
    char c = Serial.read();
    RxBuff[cnt++] = c;

    if ((c == '\n') || (cnt == sizeof(RxBuff)-1))
    {
          switch(toupper(RxBuff[0]))
          {
            case 'W':
              Serial.println("Mode White selected");
              ledmode=ALLWHITE;              
              SetAllWhite();
              break;
            case 'H':
              Serial.println("Half White selected");
              ledmode=HALFWHITE;              
              SetHalfWhite();
              break;
            case 'L':
              Serial.println("Mode Logo selected");
              ledmode=LOGO;              
              ShowLogo();
              break;
            case 'F':
              Serial.println("Mode FlashRGB selected");
              ledmode=FLASHRGB;
              break;
            case 'O':
              Serial.println("Turning Off LEDs");
              ledmode=OFF;
              AllOff();
              break;
            case 'S':
              Serial.println("Smile mode");
              ledmode=SMILE;
              Smile();
              break;
            case '0':
              Serial.println("Max power selected");
              powermode = MAX;
              UpdatePower();
              break;
            case '1':
              Serial.println("Half power selected");
              powermode = HALF;
              UpdatePower();
              break;
            case '2':
              Serial.println("Third of power selected");
              powermode = THIRD;
              UpdatePower();
              break;
            case '3':
              Serial.println("Fourth of power selected");
              powermode = FOURTH;
              UpdatePower();
              break;
            default:
              break;
          }

          cnt = 0;          
    }
   }

   if(ledmode == FLASHRGB)
    SweepRGBLEDs();
    
}

// Test function to perform a sweep, turning on R LEDs only, then G only, then B only
// 500 ms between each change
void SweepRGBLEDs()
{    
  // Enable or disable RGB LEDs one after the other  
  digitalWrite(C_B,LOW);
  digitalWrite(C_R,HIGH);
  
  for(int i=0;i<NUMPIXELS;i++)
  {
    SetPixelColor(i,255,0,0);
    showPixel(i);
  }    
  delay(500);

  digitalWrite(C_R,LOW);
  digitalWrite(C_G,HIGH);
  
  for(int i=0;i<NUMPIXELS;i++)
  {
    SetPixelColor(i,0,255,0);
    showPixel(i);
  }
  delay(500);

  digitalWrite(C_G,LOW);
  digitalWrite(C_B,HIGH);
  
  for(int i=0;i<NUMPIXELS;i++)
  {
    SetPixelColor(i,0,0,255);
    showPixel(i);
  }
  delay(500);
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void showPixel(uint16_t n, int show)
{

  // Powermode
  uint8_t ledreg = ((powermode & 0x03) << 1) | (show & 0x01);
  
  Wire.beginTransmission(DRIVERADDR[n]);    // Enable LED @ Max Current
  Wire.write(LED_EN_REG +DRIVERIDX[n]-1);
  Wire.write(ledreg);
  Wire.write(ledreg);
  Wire.write(ledreg);
  Wire.endTransmission();
  Wire.beginTransmission(DRIVERADDR[n]);
  Wire.write(UPDATE_REG);                    // Update registers
  Wire.write(0);
  Wire.endTransmission();
}

// Default parameter enables LED on both drivers. Pass 1 to disable all LEDs
void enableAll(int disable)
{
  Wire.beginTransmission(ISSI1_ADDR);
  Wire.write(GLOBAL_EN_REG);
  Wire.write(disable);
  Wire.endTransmission();
  Wire.beginTransmission(ISSI2_ADDR);
  Wire.write(GLOBAL_EN_REG);
  Wire.write(disable);
  Wire.endTransmission();
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  Wire.beginTransmission(DRIVERADDR[n]); // transmit to appropriate device
  // n is rgb pixel index [0-11]
  Wire.write(PWM_REG + DRIVERIDX[n]-1); // R LED Address. Autoincrements to G then B on successive writes
  Wire.write(pgm_read_byte(&gamma[r]));
  Wire.write(pgm_read_byte(&gamma[g]));
  Wire.write(pgm_read_byte(&gamma[b]));
  Wire.endTransmission();    // stop transmitting
}

void ShowLogo()
{  
  AllOff();
  SetPixelColor(0, 123, 77, 155);
  showPixel(0);  
  SetPixelColor(1, 87, 97, 171);
  showPixel(1);  
  SetPixelColor(2, 50, 118, 187);  
  showPixel(2);
  SetPixelColor(3, 14, 138, 203);
  showPixel(3);
  SetPixelColor(4, 18, 145, 188);
  showPixel(4);
  SetPixelColor(5, 22, 152, 174);
  showPixel(5);
  SetPixelColor(6, 26, 159, 159);
  showPixel(6);
  SetPixelColor(7, 29, 166, 144);
  showPixel(7);
  SetPixelColor(8, 33, 173, 130);
  showPixel(8);
  SetPixelColor(9, 37, 180, 115);
  showPixel(9);
  SetPixelColor(10, 73, 187, 100);
  showPixel(10);
  SetPixelColor(11, 110, 194, 86);
  showPixel(11);
  SetPixelColor(12, 146, 201, 71);
  showPixel(12);
  SetPixelColor(13, 182, 208, 56);
  showPixel(13);
  SetPixelColor(14, 219, 215, 42);
  showPixel(14);
  SetPixelColor(15, 255, 222, 27);
  showPixel(15);
  SetPixelColor(16, 251, 188, 40);
  showPixel(16);
  SetPixelColor(17, 247, 153, 54);
  showPixel(17);
  SetPixelColor(18, 243, 119, 67);
  showPixel(18);
  SetPixelColor(19, 240, 85, 80);
  showPixel(19);
  SetPixelColor(20, 236, 50, 94);
  showPixel(20);
  SetPixelColor(21, 232, 16, 107);
  showPixel(21);
  SetPixelColor(22, 196, 36, 123);
  showPixel(22);
  SetPixelColor(23, 159, 57, 139);
  showPixel(23);
}

void CentralLedWhite()
{
  digitalWrite(C_R,HIGH);
  digitalWrite(C_G,HIGH);
  digitalWrite(C_B,HIGH);
}

void CentralLedOff()
{
  digitalWrite(C_R,LOW);
  digitalWrite(C_G,LOW);
  digitalWrite(C_B,LOW);
}

void CentralLedR()
{
  digitalWrite(C_R,HIGH);
}

void CentralLedG()
{
  digitalWrite(C_G,HIGH);
}

void CentralLedB()
{
  digitalWrite(C_B,HIGH);
}

void CentralLedRGB(uint8_t r,uint8_t g,uint8_t b)
{  
    digitalWrite(C_R,(r>0)?HIGH:LOW);
    digitalWrite(C_G,(g>0)?HIGH:LOW);
    digitalWrite(C_B,(b>0)?HIGH:LOW);
}

void SetAllWhite()
{  
  AllOff();
  CentralLedWhite();
  
  for(int i=0;i<NUMPIXELS;i++)
  {
    SetPixelColor(i,255,255,255);
    showPixel(i);
  }    
}

void SetHalfWhite()
{  
  AllOff();
  CentralLedWhite();
  
  for(int i=0;i<NUMPIXELS/2;i++)
  {
    SetPixelColor(i,255,255,255);
    showPixel(i);
  }   
}


void AllOff()
{
  CentralLedOff();
  
  for(int i=0;i<NUMPIXELS;i++)
  {
    SetPixelColor(i,0,0,0);
    showPixel(i);
  }    
}

void UpdatePower()
{
  for(int i=0;i<NUMPIXELS;i++)
  {
    showPixel(i);
  }   
}

void Smile()
{
  AllOff();
  
  CentralLedRGB(1,1,0);
  
  for(int i=1;i<NUMPIXELS/2-1;i++)
  {
    SetPixelColor(i,255,0,0);
    showPixel(i);
  }   

  SetPixelColor(16,0,0,255);
  showPixel(15);
  SetPixelColor(19,0,0,255);
  showPixel(20);
}

