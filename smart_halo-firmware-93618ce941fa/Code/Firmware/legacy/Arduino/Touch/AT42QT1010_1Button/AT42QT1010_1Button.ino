#include <Adafruit_NeoPixel.h>

#define TOUCHPIN  A3 
#define HALOPIN   7

#define NUMPIXELS 1
Adafruit_NeoPixel HaloPix = Adafruit_NeoPixel(NUMPIXELS, HALOPIN, NEO_GRB + NEO_KHZ800);

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);
  Serial.println("AT42QT1010 1 Button Test firmware");
  pinMode(TOUCHPIN, INPUT);

  HaloPix.begin();
  HaloPix.setBrightness(230);

  LedOn();
  delay(200);
  LedOff();
  delay(200);
}

void loop() 
{  
  int touch = digitalRead(TOUCHPIN); 
  
  if(touch == HIGH)
    LedOn();
  else
    LedOff();
    
  delay(100);
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

