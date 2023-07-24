#define tone1kHz    1000
#define tone1_4kHz  714
#define tone2kHz    500
#define tone3kHz    333
#define tone4kHz    250
#define tone5kHz    200
#define tone6kHz    167


int speakerp = 9; 
int speakern = 10; 
long vel = 20000;
boolean hasplayed = false;
 
void setup() {
  pinMode(speakerp, OUTPUT);
  pinMode(speakern, OUTPUT);
  delay(2000);
}

//int melod[] = {tonec, toneG, toneE, toneA, toneB, toneBb, toneA, toneG, tonee, toneg, tonea, tonef, toneg, tonee, tonec, toned, toneB};
//int ritmo[] = {18, 18, 18, 12, 12, 6, 12, 8, 8, 8, 12, 6, 12, 12, 6, 6, 6};
 
void loop() {
  if (hasplayed == true){ return;}
  for (int i=0; i<17; i++) {
    int tom = tone4kHz;//melod[i];
    int tempo = 5;//ritmo[i];
 
    long tvalue = tempo * vel;

    tocar(tom, tvalue);

    delay(200);
    //delayMicroseconds(tvalue);
  }      delay(1000);

  hasplayed = true;
}
 
void tocar(int tom, long tempo_value) {
  long tempo_gasto = 0;
  while (tempo_gasto < tempo_value) {
    digitalWrite(speakerp, HIGH);
    digitalWrite(speakern, LOW);
    delayMicroseconds(tom / 2);
 
    digitalWrite(speakerp, LOW);
    digitalWrite(speakern, HIGH);
    delayMicroseconds(tom/2);  
    tempo_gasto += tom;
  }
}
