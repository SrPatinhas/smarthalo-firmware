#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 1
#define HALOPIN   7
#define SPKR_PIN  5
#define BZR_PIN   A0

Adafruit_NeoPixel HaloPix = Adafruit_NeoPixel(NUMPIXELS, HALOPIN, NEO_GRB + NEO_KHZ800);

// Musical tones
#define toneC4      1911
#define toneDb4     1804
#define toneD4      1703
#define toneEb4     1607
#define toneE4      1517
#define toneF4      1432
#define toneGb4     1351
#define toneG4      1276
#define toneAb4     1204
#define toneA4      1136
#define toneBb4     1073
#define toneB4      1012
#define toneC5      1911
#define toneDb5     1804
#define toneD5      1703
#define toneEb5     1607
#define toneE5      1517
#define toneF5      1432
#define toneGb5     1351
#define toneG5      1276
#define toneAb5     1204
#define toneA5      1136
#define toneBb5     1073
#define toneB5      1012
#define toneC6       955
#define toneDb6      902
#define toneD6       851
#define toneEb6      803
#define toneE6       758
#define toneF6       716
#define toneGb6      676
#define toneG6       638
#define toneAb6      602
#define toneA6       568
#define toneBb6      536
#define toneB6       506
#define toneC7       478
#define toneDb7      451
#define toneD7       426
#define toneEb7      402
#define toneE7       379
#define toneF7       358
#define toneGb7      338
#define toneG7       319
#define toneAb7      301
#define toneA7       284
#define toneBb7      268
#define toneB7       253
#define toneC8       239
#define toneDb8      225
#define toneD8       213
#define toneEb8      201
#define toneE8       190
#define toneF8       179
#define toneGb8      169
#define toneG8       159
#define toneAb8      150
#define toneA8       142
#define toneBb8      134
#define toneB8       127

// Special tone for pauses
#define tonep       0


// Music logic
long vel = 20000;
 
void setup() {

  Serial.begin(115200);
  while(!Serial);
  Serial.println("PiezoTune firmware");
  
  pinMode(SPKR_PIN, OUTPUT);
  pinMode(BZR_PIN, OUTPUT);

  HaloPix.begin();
  HaloPix.setBrightness(230);

  LedOn();
  delay(200);
  LedOff();
  delay(200);
}

// Call sound
int melodCall[] = {toneEb7, toneAb7, toneEb7, toneAb7, toneEb7, toneAb7, toneEb7, toneAb7, toneEb7, toneAb7, tonep};
int ritmoCall[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 100};
// Sms sound
int melodSMS[] = {toneE6, toneB6, toneE7, tonep};
int ritmoSMS[] = {4, 4, 4, 100};
// Alarm sound
int melodAlarm[] = {toneB7, toneC8, tonep};
int ritmoAlarm[] = {12, 12, 2};
 
void loop() {

//  if(callon == HIGH)
//  {   
//    for (int i=0; i<11; i++) 
//    {
//      int tom = melodCall[i];
//      int tempo = ritmoCall[i];
//   
//      long tvalue = tempo * vel;
//  
//      if(tom==tonep)
//        delay(tvalue/1000);
//      else
//        tocar(tom, tvalue);
//
//      delayMicroseconds(1000);
//    }     
//  }
//
//  if(smson == HIGH)
//  {   
//    for (int i=0; i<4; i++) 
//    {
//      int tom = melodSMS[i];
//      int tempo = ritmoSMS[i];
//   
//      long tvalue = tempo * vel;
//  
//      if(tom==tonep)
//        delay(tvalue/1000);
//      else
//        tocar(tom, tvalue);
//
//      delayMicroseconds(1000);
//    }
//  }

//  if(alarmon == HIGH)
//  {
    
      for (int i=0; i<3; i++) 
      {
        int tom = melodAlarm[i];
        int tempo = ritmoAlarm[i];
     
        long tvalue = tempo * vel;
    
        if(tom==tonep)
          delay(tvalue/1000);
        else
          tocar(tom, tvalue);
  
        delayMicroseconds(1000);
      }        
 // }  
}
 
void tocar(int tom, long tempo_value) {
  long tempo_gasto = 0;
  while (tempo_gasto < tempo_value) {
    digitalWrite(SPKR_PIN, HIGH);
    digitalWrite(BZR_PIN, HIGH);
    delayMicroseconds(tom / 2);
 
    digitalWrite(SPKR_PIN, LOW);
    digitalWrite(BZR_PIN, LOW);
    delayMicroseconds(tom/2);  
    tempo_gasto += tom;
  }
}

void LedOn()
{
  HaloPix.setPixelColor(0, HaloPix.Color(255,255,255)); 
  HaloPix.show();
}

void LedOff()
{
  HaloPix.setPixelColor(0, HaloPix.Color(0,0,0)); 
  HaloPix.show();
}
