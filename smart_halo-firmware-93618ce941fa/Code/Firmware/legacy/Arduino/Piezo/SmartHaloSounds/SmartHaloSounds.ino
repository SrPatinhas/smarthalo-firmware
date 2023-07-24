#include <Tone.h>

#define PIEZO_PIN   3

#define FMIN          20
#define FMAX          20000
#define NMIN          1
#define NMAX          100000
#define DURATIONMIN   0
#define DURATIONMAX   60000

#define FMIN_DEF      1400
#define FMAX_DEF      3200
#define F_DEF         2800
#define N_DEF         250
#define DURATION_DEF  0

int tone_f           = F_DEF;
int chirp_min_f      = FMIN_DEF;
int chirp_max_f      = FMAX_DEF;
int num_pts          = N_DEF;
int duration_ms      = DURATION_DEF;

#define BUFFERLEN 32
int cnt = 0;
char RxBuff[BUFFERLEN];

enum SOUNDMODE {OFF = 0, CHIRPLOG, CHIRPLIN, TONE, DUALTONE};
SOUNDMODE soundmode = OFF;

Tone MyTone;
 
void setup() {

  Serial.begin(115200);
  while(!Serial);
  Serial.println("SmartHaloSounds firmware");
  
  MyTone.begin(PIEZO_PIN);
}
 
void loop() {

      int value = 0;

      while( Serial.available() > 0 )
      {    
        char c = Serial.read();
        RxBuff[cnt++] = c;        
            
        if ((c == '\n') || (cnt == sizeof(RxBuff)-1))
        {
              RxBuff[cnt] = '\0';               
              for(int i=0; i<cnt; i++) RxBuff[i] = toupper(RxBuff[i]);

              // Word or dual letters
              if(cnt=3 && (RxBuff[0]=='C') && (RxBuff[1]=='G'))
              {
                soundmode = CHIRPLOG;                
                Serial.print("Chirp from "), Serial.print(chirp_min_f), Serial.print(" to "), Serial.print(chirp_max_f), Serial.print(" Hz, with "), Serial.print(num_pts), Serial.print(" points, for"); Serial.print(duration_ms); Serial.println(" ms, in log scale");                
              }
              else if(cnt=3 && (RxBuff[0]=='C') && (RxBuff[1]=='N'))
              {
                soundmode = CHIRPLIN;
                Serial.print("Chirp from "), Serial.print(chirp_min_f), Serial.print(" to "), Serial.print(chirp_max_f), Serial.print(" Hz, with "), Serial.print(num_pts), Serial.println(" points, for"); Serial.print(duration_ms); Serial.println(" ms, in lin scale");
              }
              else if(cnt=3 && (RxBuff[0]=='D') && (RxBuff[1]=='T'))
              {
                soundmode = DUALTONE;
                Serial.print("Dual tone at "), Serial.print(chirp_min_f), Serial.print(" and "), Serial.print(chirp_max_f), Serial.print(" Hz, for "), Serial.print(duration_ms), Serial.println(" ms");
              }
              // Single letters
              else if(cnt=2 && (RxBuff[0]=='H'))
              {
                PrintHelp();
              } 
              else if(cnt=2 && (RxBuff[0]=='T'))
              {
                soundmode = TONE;                
                Serial.print("Tone at "), Serial.print(tone_f), Serial.println(" Hz");
                
              }              
              else if(cnt=2 && (RxBuff[0]=='O'))
              {
                soundmode = OFF;
                Serial.println("Turning off");
                MyTone.stop();
              }
              else if(sscanf(RxBuff, "FMIN=%d", &value)==1)
              {
                if(value < FMIN || value >= chirp_max_f)
                  Serial.print("Invalid FMIN: must be between "), Serial.print(FMIN), Serial.print(" and "), Serial.println(chirp_max_f-1);
                else  
                {
                  chirp_min_f = value;
                  Serial.print("FMIN=");
                  Serial.println(chirp_min_f);
                }
              }
              else if(sscanf(RxBuff, "FMAX=%d", &value)==1)
              {
                if(value <= chirp_min_f || value > FMAX)
                  Serial.print("Invalid FMAX: must be between "), Serial.print(chirp_min_f+1), Serial.print(" and "), Serial.println(FMAX);
                else  
                {
                  chirp_max_f = value;
                  Serial.print("FMAX=");
                  Serial.println(chirp_max_f);
                }
              }
              else if(sscanf(RxBuff, "F=%d", &value)==1)
              {
                if(value < FMIN || value > FMAX)
                  Serial.print("Invalid F: must be between "), Serial.print(FMIN), Serial.print(" and "), Serial.println(FMAX);
                else  
                {
                  tone_f = value;
                  Serial.print("F=");
                  Serial.println(tone_f);
                }
              }
              else if(sscanf(RxBuff, "N=%d", &value)==1)
              {
                if(value < NMIN || value > NMAX)
                  Serial.print("Invalid N: must be between "), Serial.print(NMIN), Serial.print(" and "), Serial.println(NMAX);
                else  
                {
                  num_pts = value;
                  Serial.print("N=");
                  Serial.println(num_pts);
                }
              }
              else if(sscanf(RxBuff, "D=%d", &value)==1)
              {
                if(value < DURATIONMIN || value > DURATIONMAX)
                  Serial.print("Invalid D: must be between "), Serial.print(DURATIONMIN), Serial.print(" and "), Serial.println(DURATIONMAX);
                else  
                {
                  duration_ms = value;
                  Serial.print("D=");
                  Serial.println(duration_ms);
                }
              }
              else if(strncmp(RxBuff, "FMIN", 4)==0)
              {
                Serial.print("FMIN="); Serial.println(chirp_min_f);
              }
              else if(strncmp(RxBuff, "FMAX", 4)==0)
              {
                Serial.print("FMAX="); Serial.println(chirp_max_f);
              }
              else if(strncmp(RxBuff, "F", 1)==0)
              {
                Serial.print("F="); Serial.println(tone_f);
              }
              else if(strncmp(RxBuff, "N", 1)==0)
              {
                Serial.print("N="); Serial.println(num_pts);
              }
              else if(strncmp(RxBuff, "D", 1)==0)
              {
                Serial.print("D="); Serial.println(duration_ms);
              }
              else
              {
                Serial.println("Invalid command. (H) for help");
              }          
          
              cnt = 0;
        }
      }
      if(soundmode == TONE)
      {        
        MyTone.play(tone_f);      
      }
      if(soundmode == TONE)
      {        
        MyTone.play(tone_f);      
      }
      else if(soundmode == DUALTONE)
      {
        if(MyTone.isPlaying())
        {
          MyTone.stop();          
        }
        DualTone(chirp_min_f, chirp_max_f, duration_ms);
      }
      else if(soundmode == CHIRPLOG)
      {
        if(MyTone.isPlaying())
        {
          MyTone.stop();          
        }
        Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, true);
      }
      else if(soundmode == CHIRPLIN)
      {
        if(MyTone.isPlaying())
        {
          MyTone.stop();          
        }
        Chirp(chirp_min_f, chirp_max_f, num_pts, duration_ms, false);
      }
}
 
void tocar(int tom) {   
  digitalWrite(PIEZO_PIN, HIGH);
  delayMicroseconds(round((double)tom/(double)2));
  
  digitalWrite(PIEZO_PIN, LOW);
  delayMicroseconds(round((double)tom/(double)2));
}

void Chirp(int f_min, int f_max, int numpts, int duration_ms, bool logmode)
{
  double chirp_min_f  = logmode?log10((double)f_min):f_min;
  double chirp_max_f  = logmode?log10((double)f_max):f_max;
  double chirp_range  = chirp_max_f - chirp_min_f;
  double chirp_f_inc  = chirp_range/num_pts;

  double f_log = chirp_min_f;
  double f_lin = chirp_min_f;
  int toneduration_ms = round((double)duration_ms/(double)num_pts);
  
  for (int i=0; i<numpts; i++) 
  { 
    f_log += chirp_f_inc;
    f_lin = logmode?pow(10,f_log):(f_lin+chirp_f_inc);

    while((MyTone.isPlaying())&&(toneduration_ms>0));

    if(toneduration_ms > 0)
      MyTone.play(f_lin,toneduration_ms);
    else
      MyTone.play(f_lin);
  }   
}

void DualTone(int f_min, int f_max, int duration_ms)
{  
  MyTone.play(f_min,duration_ms);

  while(MyTone.isPlaying()&&(duration_ms>0));

  MyTone.play(f_max,duration_ms);

  while(MyTone.isPlaying()&&(duration_ms>0));
}

void PrintHelp()
{
  Serial.println("(H)elp, (C)hirp Li(n), (C)hirp Lo(g), (T)one, (D)ual(T)one, (O)ff");
  Serial.println("FMIN=Value, F=Value, FMAX=VALUE, N=VALUE, D=VALUE, all integers");
  Serial.println("FMIN to FMAX is the range for the chirp and F is the tone frequency. Frequencies valid from 20 to 20000");
  Serial.println("N is the number of points for the chirp. Valid from 1 to 100000");
  Serial.println("D is the duration in ms for the chirp. Valid from 100 to 60000");
}
