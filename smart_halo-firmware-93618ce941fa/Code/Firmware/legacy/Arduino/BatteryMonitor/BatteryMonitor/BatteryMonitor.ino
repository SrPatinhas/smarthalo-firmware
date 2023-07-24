#define IBAT_PIN  A5
#define VBAT_PIN  A1

// ADC
#define ADC_BITS  10
#define ADC_RANGE 5

// SENSE
const double ADC_SENS = (double)((ADC_RANGE)/pow(2,ADC_BITS));
const double VGAIN = 0.75;
const double IGAIN = 110 * 0.01;
const double IOFFSET = ADC_RANGE/(float)2;

double VBAT_TOT = 0;
double IBAT_TOT = 0;
int N_TOT = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Battery Monitor firmware");

  //pinMode(IBAT_PIN,INPUT);
  //pinMode(VBAT_PIN,INPUT); 
}

void loop() {
  
  // Read analog pins
  int VBAT_N = analogRead(VBAT_PIN);
  int IBAT_N = analogRead(IBAT_PIN);

  // Convert ADC values to physical values
  float VBAT = VBAT_N * ADC_SENS / VGAIN + 0.03;
  float IBAT = 1000 * ((float)(IBAT_N * ADC_SENS) - IOFFSET)/ IGAIN - 57.71;  
  VBAT_TOT += VBAT;
  IBAT_TOT += IBAT;
  N_TOT++;
  
  Serial.print("VBAT: "); Serial.print(VBAT_N); Serial.print("\t"); Serial.print(VBAT); Serial.println(" V");
  Serial.print("IBAT: "); Serial.print(IBAT_N); Serial.print("\t"); Serial.print(IBAT); Serial.println(" mA");
  Serial.print("VBAT average: "); Serial.print(VBAT_TOT/N_TOT); Serial.println(" V");
  Serial.print("IBAT average: "); Serial.print(IBAT_TOT/N_TOT); Serial.println(" mA");
  
  delay(2000);
}
