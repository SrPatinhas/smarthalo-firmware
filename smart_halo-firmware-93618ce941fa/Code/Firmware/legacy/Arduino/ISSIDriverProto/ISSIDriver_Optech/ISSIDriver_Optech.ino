// ISSI_Driver
// by Sebastien Gelinas

// Driver to control ISSI LED Test Board
// Communication through I2C
// 

// Created 09 February 2016

// Special control function through UART for Optech

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
#define C_R 9
#define C_G 10
#define C_B 11
#define FLED 6

enum POWERMODE {MAX = 0, HALF = 1, THIRD = 2, FOURTH = 3};
POWERMODE powermode = MAX;

// Constants
#define NUMPIXELS          24

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


#define BUFFERLEN 32
int cnt = 0;
char RxBuff[BUFFERLEN];

void enableAll(int disable=0);
void showPixel(uint16_t n, int show=1);
void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void SweepRGBLEDs();

void setup()
{
  Serial.begin(115200);
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
    
  powermode=MAX;
   
  PrintHelp();
}


void loop()
{       
   while( Serial.available() > 0 )
   {    
    char c = Serial.read();
    RxBuff[cnt++] = c;
    bool invalid = false;
    char RxBuff2[BUFFERLEN];
    int ledIndex1 = -2, ledIndex2 = -2, Rvalue = -1, Gvalue = -1, Bvalue = -1;
    char * pch = NULL;
        
    if ((c == '\n') || (cnt == sizeof(RxBuff)-1))
    {
          RxBuff[cnt] = '\0';               
          for(int i=0; i<cnt; i++) RxBuff[i] = toupper(RxBuff[i]);
          
          if(cnt=2 && (RxBuff[0]=='H'))
          {
            PrintHelp();
          } 
          else if(sscanf(RxBuff, "FLED%d", &Rvalue)==1)
          {
            if(Rvalue < 0 || Rvalue > 255)
              Serial.println("Invalid brightness value: must be between 0 to 255"), invalid = true;

            if(!invalid)
            {
              SetFrontLedBrightness(Rvalue);
            }
          }
          else if(sscanf(RxBuff, "LED%dR%dG%dB%d", &ledIndex1, &Rvalue, &Gvalue, &Bvalue)==4)
          {  
            if(ledIndex1 < 0 || ledIndex1 > 25)             
              Serial.println("Invalid LED: must be between 0 and 25. 0 will control all the leds at once"), invalid = true;
            if(Rvalue < 0 || Rvalue > 255)
              Serial.println("Invalid R value: must be between 0 to 255"), invalid = true;
            if(Gvalue < 0 || Gvalue > 255)
              Serial.println("Invalid G value: must be between 0 to 255"), invalid = true;
            if(Bvalue < 0 || Bvalue > 255)
              Serial.println("Invalid B value: must be between 0 to 255"), invalid = true;

            if(!invalid)
             {
                if(ledIndex1 == 0)
                {
                  Serial.print("Setting all LED to:");
                  ShowAllPixelColor(Rvalue, Gvalue, Bvalue);
                }
                else
                {
                    Serial.print("Setting LED "), Serial.print(ledIndex1), Serial.print(" to:");                  
                    ShowPixelColor(ledIndex1-1, Rvalue, Gvalue, Bvalue);                  
                }
  
                Serial.print(" R "); Serial.print(Rvalue); Serial.print(" G "); Serial.print(Gvalue); Serial.print(" B "); Serial.println(Bvalue);
             }
          }   
          else if(sscanf(RxBuff, "LED%d-%dR%dG%dB%d", &ledIndex1, &ledIndex2, &Rvalue, &Gvalue, &Bvalue)==5)       
          {
            if(ledIndex1 >= ledIndex2)
            {
              Serial.println("Invalid LED Range: First index must be lower then the second index"), invalid = true;  
            }
            if(ledIndex1 < 1 || ledIndex1 > 25 || ledIndex2 < 1 || ledIndex2 > 25)             
              Serial.println("Invalid LED Range: must be between 1 and 25"), invalid = true;            
            if(Rvalue < 0 || Rvalue > 255)
              Serial.println("Invalid R value: must be between 0 to 255"), invalid = true;
            if(Gvalue < 0 || Gvalue > 255)
              Serial.println("Invalid G value: must be between 0 to 255"), invalid = true;
            if(Bvalue < 0 || Bvalue > 255)
              Serial.println("Invalid B value: must be between 0 to 255"), invalid = true;

            if(!invalid)
             {                
                Serial.print("Setting LED "); Serial.print(ledIndex1); Serial.print(" to "); Serial.print(ledIndex2); Serial.print(":");
                Serial.print(" R "); Serial.print(Rvalue); Serial.print(" G "); Serial.print(Gvalue); Serial.print(" B "); Serial.println(Bvalue);
                ShowRangePixelColor(ledIndex1-1, ledIndex2-1, Rvalue, Gvalue, Bvalue);
             }
          }
          else if(strstr(RxBuff,",")!=NULL)       
          {
            // save a copy
            strncpy(RxBuff2,RxBuff,sizeof(RxBuff));
            
            // Get the RGB values first
            pch = strtok(RxBuff2,",");

            while((pch != NULL) && (sscanf(pch,"%*dR%dG%dB%d", &Rvalue, &Gvalue, &Bvalue) != 3))
            {              
              pch = strtok(NULL,",");
            }
            
            if(Rvalue < 0 || Rvalue > 255)
              Serial.println("Invalid R value: must be between 0 to 255"), invalid = true;
            if(Gvalue < 0 || Gvalue > 255)
              Serial.println("Invalid G value: must be between 0 to 255"), invalid = true;
            if(Bvalue < 0 || Bvalue > 255)
              Serial.println("Invalid B value: must be between 0 to 255"), invalid = true;            

            // Parse Led index per index
            pch = strtok(RxBuff,",");

            while(pch != NULL)
            {       
              sscanf(pch,"%d",&ledIndex1);
              
              if(ledIndex1 < 1 || ledIndex1 > 25)             
                  Serial.println("Invalid LED Range: must be between 1 and 25");              
              else if(!invalid)
              {                
                  Serial.print("Setting LED "); Serial.print(ledIndex1); Serial.print(" to:");
                  Serial.print(" R "); Serial.print(Rvalue); Serial.print(" G "); Serial.print(Gvalue); Serial.print(" B "); Serial.println(Bvalue);
                  ShowPixelColor(ledIndex1-1, Rvalue, Gvalue, Bvalue);
              }
              
              pch = strtok(NULL,",");
            }
          }
          else
          {
            Serial.println("Invalid command. (h) for help");
          }          
          
          cnt = 0;          
    }
   }      
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
  if( n < 24)
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

void SetFrontLedBrightness(uint8_t brightness)
{
  analogWrite(FLED, brightness);
}

// n is the index of pixel between 0-23 (0-NUMPIXELS-1)
void SetPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  if ((n >= 0) && (n < 24))
  {
    Wire.beginTransmission(DRIVERADDR[n]); // transmit to appropriate device
    // n is rgb pixel index [0-11]
    Wire.write(PWM_REG + DRIVERIDX[n]-1); // R LED Address. Autoincrements to G then B on successive writes
    Wire.write(pgm_read_byte(&gamma[r]));
    Wire.write(pgm_read_byte(&gamma[g]));
    Wire.write(pgm_read_byte(&gamma[b]));
    Wire.endTransmission();    // stop transmitting
  }
  else if( n==24 ) // Central LED
  {
    analogWrite(C_R, r);
    analogWrite(C_G, g);
    analogWrite(C_B, b);
  }
}

void ShowAllPixelColor(uint8_t r, uint8_t g, uint8_t b)
{
  for(int i=0;i<NUMPIXELS;i++)
  {
    SetPixelColor( i, r, g, b);
    showPixel(i);
  }

  // Central LED
  analogWrite(C_R, r);
  analogWrite(C_G, g);
  analogWrite(C_B, b);
}

void ShowPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{  
  SetPixelColor(n, r, g, b);
  showPixel(n);
}

void ShowRangePixelColor(uint16_t m, uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{  
  for(int i=m;i<=n;i++)
  {
    SetPixelColor(i, r, g, b);
    showPixel(i);
  }
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

void PrintHelp()
{
  Serial.println("(H)elp, LED(X)R(Y)G(Y)B(Y) with X the led index (1-25), Y the intensity value (0-255) of RGB leds. Led 25 is the central led.");
  Serial.println("Use LED(X-Z)R(Y)G(Y)B(Y) to set a range of leds, with X and Z led indexes (1-25), Y the intensity value (0-255) of RGB leds");
  Serial.println("Use LED(X,Z,W,V,etc.)R(Y)G(Y)B(Y) to set a multiple leds, with X, Z, W, V, etc a comma separated list of led indexes (1-25), Y the intensity value (0-255) of RGB leds");
  Serial.println("Use FLED(X) to set the front led brightness (0-255)");
}

