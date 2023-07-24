/*
 * animationsLibrary.c
 *
 *  Created on: Jun 5, 2019
 *      Author: Nzo
 */

#include <AnimationsLibrary.h>
#include "math.h"
#include "Shell.h"
#include "reboot.h"

#define MILLISECONDS_PER_UPDATE 10
#define LED_COUNT 24
#define RGB_LED_COUNT (LED_COUNT*3)

#define ALARM_BLINK_COUNT 4*2 //4 times 2 fade in/out durations

uint8_t maxVal = MAX_LED_BRIGHTNESS;
uint8_t minVal = MIN_LED_BRIGHTNESS;

#if !defined(WWDG_FOUND)
#define LEDS(idx)   ({ *ledp(leds, (idx), __LINE__); })
#else
#define LEDS(idx) leds[idx]
#endif

static inline uint8_t *ledp(uint8_t *leds, uint8_t idx, int line)
{
    if (idx > RGB_LED_COUNT) {
        log_Shell("WTF!!! leds index out of range: %u", idx);
        soft_crash(eANIMATIONSLIBRARY << 16 | line);
    }
    return leds + idx;
}

void setMaxBrightness_AnimationsLibrary(uint8_t brightness){
    maxVal = brightness > minVal ? brightness : minVal;
}

uint8_t getMaxBrightness_AnimationsLibrary(){
    return maxVal;
}

void HsvToRgb(HsvColour_t hsv, RgbColour_t * rgb)
{
    unsigned char region, remainder, p, q, t;

    hsv.v = maxVal/255.f*hsv.v;
    if (hsv.s == 0){
        rgb->r = hsv.v;
        rgb->g = hsv.v;
        rgb->b = hsv.v;
        return;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region){
        case 0:
            rgb->r = hsv.v; rgb->g = t; rgb->b = p;
            break;
        case 1:
            rgb->r = q; rgb->g = hsv.v; rgb->b = p;
            break;
        case 2:
            rgb->r = p; rgb->g = hsv.v; rgb->b = t;
            break;
        case 3:
            rgb->r = p; rgb->g = q; rgb->b = hsv.v;
            break;
        case 4:
            rgb->r = t; rgb->g = p; rgb->b = hsv.v;
            break;
        default:
            rgb->r = hsv.v; rgb->g = p; rgb->b = q;
            break;
    }
}

void RgbToHsv(RgbColour_t rgb, HsvColour_t * hsv)
{
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv->v = rgbMax > maxVal ? maxVal : rgbMax;
    if (hsv->v == 0){
        hsv->h = 0;
        hsv->s = 0;
        return;
    }

    hsv->s = 255 * (uint16_t)(rgbMax - rgbMin) / hsv->v;
    if (hsv->s == 0){
        hsv->h = 0;
        return;
    }

    if (rgbMax == rgb.r)
        hsv->h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv->h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv->h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
}

static float getLEDFromAngle(float angle){
    return (angle*LED_COUNT)/360.f;
}

static LEDAnimationData_t getLEDAnimationData(float startAngle, float endAngle, const HsvColour_t * startHsv, const HsvColour_t * endHsv){
    LEDAnimationData_t animationData;
    float startLEDFloat = getLEDFromAngle(startAngle);
    float endLEDFloat = getLEDFromAngle(endAngle);
    animationData.startLED = ceil(startLEDFloat);
    animationData.endLED = floor(endLEDFloat);
    animationData.startFraction =  -1*(startLEDFloat - animationData.startLED);
    animationData.endFraction = endLEDFloat - animationData.endLED;
    animationData.ledGap = abs(animationData.endLED-animationData.startLED+round(animationData.endFraction+animationData.startFraction));
    animationData.hueSweep = endHsv->h - startHsv->h;
    animationData.direction = animationData.endLED > animationData.startLED ? 1 : -1;
    return animationData;
}

void showArc(uint8_t * leds, float startAngle, float endAngle, const HsvColour_t *  startHsv, const HsvColour_t * endHsv){
    LEDAnimationData_t animationData = getLEDAnimationData(startAngle, endAngle, startHsv, endHsv);
    for(int i=0;i<animationData.ledGap;i++){
        HsvColour_t thisColor = {.h = startHsv->h+(i*animationData.hueSweep/animationData.ledGap), .s = startHsv->s, .v = startHsv->v};
        RgbColour_t thisRGB;
        HsvToRgb(thisColor, &thisRGB);
        uint8_t idx = ((animationData.startLED+i*animationData.direction)*3+0)%RGB_LED_COUNT;
        LEDS(idx + 0) = thisRGB.r;
        LEDS(idx + 1) = thisRGB.g;
        LEDS(idx + 2) = thisRGB.b;
    }
}

void growArc(uint8_t * leds, uint16_t clock, float divisor, float startAngle, float endAngle, const HsvColour_t * startHsv, const HsvColour_t * endHsv){
    LEDAnimationData_t animationData = getLEDAnimationData(startAngle, endAngle, startHsv, endHsv);
    uint8_t currentLEDProgress = clock/divisor;
    int8_t position = animationData.startLED*3 + 3*currentLEDProgress * animationData.direction;
    position -= position%3;
    for(int i=animationData.startLED*3;i<=position;i+=3){
        HsvColour_t thisColor = {.h = startHsv->h+(((i-animationData.startLED*3)%RGB_LED_COUNT/3)*animationData.hueSweep/animationData.ledGap),
                .s = startHsv->s,
                .v = startHsv->v};
        RgbColour_t thisRGB;
        HsvToRgb(thisColor, &thisRGB);
        LEDS((i+0)%RGB_LED_COUNT) = thisRGB.r;
        LEDS((i+1)%RGB_LED_COUNT) = thisRGB.g;
        LEDS((i+2)%RGB_LED_COUNT) = thisRGB.b;
    }
}

void showLEDsSpin(uint8_t * leds, uint16_t clock, float divisor, float startAngle, float endAngle, uint16_t tailAngle, const HsvColour_t * startHsv, const HsvColour_t * endHsv, bool isFresh){
    LEDAnimationData_t animationData = getLEDAnimationData(startAngle, endAngle, startHsv, endHsv);
    uint8_t currentLEDProgress = clock/divisor;
    uint8_t tailLEDCount = getLEDFromAngle(tailAngle);
    if(isFresh){
        for(int i = 0;i<RGB_LED_COUNT;i++){
            leds[i] = 0;
        }
    }
    uint8_t position = animationData.startLED*3 + 3*currentLEDProgress * animationData.direction;
    position -= position%3;
    HsvColour_t thisColor = {.h = startHsv->h+(currentLEDProgress*animationData.hueSweep/animationData.ledGap),
            .s = startHsv->s,
            .v = startHsv->v};
    RgbColour_t thisRGB;
    HsvToRgb(thisColor, &thisRGB);
    LEDS((position++)%RGB_LED_COUNT) = thisRGB.r;
    LEDS((position++)%RGB_LED_COUNT) = thisRGB.g;
    LEDS((position++)%RGB_LED_COUNT) = thisRGB.b;
    if(tailLEDCount){
        position -= (tailLEDCount+animationData.direction)*3*animationData.direction;
        for(int i=0;i<tailLEDCount;i++){
            HsvColour_t thisColor = {.h = startHsv->h+((position%RGB_LED_COUNT/3)*animationData.hueSweep/animationData.ledGap),
                    .s = startHsv->s,
                    .v = startHsv->v>>(tailLEDCount-i)};
            RgbColour_t thisRGB;
            HsvToRgb(thisColor, &thisRGB);
            LEDS((position++)%RGB_LED_COUNT) = thisRGB.r;
            LEDS((position++)%RGB_LED_COUNT) = thisRGB.g;
            LEDS((position++)%RGB_LED_COUNT) = thisRGB.b;
            if(animationData.direction == -1)
                position -= 6;
        }
    }
}

#if 0
void showLEDsSpiral(uint8_t * leds, uint16_t clock){
    float divisor = 20/3;
    for(int i = 0;i<RGB_LED_COUNT;i++){
        leds[i] = 0;
    }
    uint8_t position = clock/divisor;
    position += (3 - position%3);
    leds[(position+24)%RGB_LED_COUNT] = 255;
    leds[(position+48)%RGB_LED_COUNT] = 255;
    leds[position++] = 255;
    leds[(position+24)%RGB_LED_COUNT] = 255;
    leds[(position+48)%RGB_LED_COUNT] = 255;
    leds[position++] = 255;
    leds[(position+24)%RGB_LED_COUNT] = 255;
    leds[(position+48)%RGB_LED_COUNT] = 255;
    leds[position++] = 255;
}
#endif

/**
 * @brief Display the SmartHalo Logo Animation
 */
int logo_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount){
    int fade = 0;
    int done = 0;
    // All of this timing is taken from SmartHalo 1's original animation
    uint8_t spinTicks = 330/MILLISECONDS_PER_UPDATE; // 330ms, 332ms on SmartHalo 1.
    uint8_t growTicks = 400/MILLISECONDS_PER_UPDATE; // 400ms, 398ms on SmartHalo 1.
    uint8_t whiteFadeTicks = 500/MILLISECONDS_PER_UPDATE; // 500ms, 498ms on Smarthalo 1.
    float logoFadeTicks = 1000.f/MILLISECONDS_PER_UPDATE; // 1000ms, 996ms on SmartHalo 1.
    float spinLEDs = 19.f;
    float growLEDs = 24.f;
    if(animationClockCount <= spinTicks){
        int clock = animationClockCount;
        done = 0;
        HsvColour_t spinHsv = {.h = 14, .s = 0, .v = 255};
        // the 18 here is how many LEDs will pass in the time of the spin ticks
        showLEDsSpin(leds,clock,spinTicks/spinLEDs,60,360,60,&spinHsv,&spinHsv, true);
    }else if(animationClockCount <= (spinTicks + growTicks)){
        int clock = animationClockCount-spinTicks;
        HsvColour_t spinHsv = {.h = 14, .s = 0, .v = 255};
        growArc(leds,clock,growTicks/growLEDs,0,360,&spinHsv,&spinHsv);
    }else if(animationClockCount <= (spinTicks + growTicks + whiteFadeTicks)){
        int clock = animationClockCount-(spinTicks + growTicks);
        fade = 255-clock*255/whiteFadeTicks;
        if(fade < 0){
            fade = 0;
        }
        HsvColour_t showHsv = {.h = 14, .s = 0, .v = fade};
        showArc(leds,90,450,&showHsv,&showHsv);
    }else if(!done){
        int clock = animationClockCount-(spinTicks + growTicks + whiteFadeTicks);
        fade = clock*255/logoFadeTicks;
        if(fade > 255){
            fade = 255 - (clock-logoFadeTicks)*255/logoFadeTicks;
            if(fade < 0){
                fade = 0;
                done = 1;
            }
        }
        HsvColour_t hsvs[24] = {{.h=167,.s=125,.v=fade},
                {.h=151,.s=187,.v=fade},
                {.h=144,.s=237,.v=fade},
                {.h=139,.s=225,.v=fade},
                {.h=131,.s=210,.v=fade},
                {.h=121,.s=196,.v=fade},
                {.h=111,.s=188,.v=fade},
                {.h=101,.s=181,.v=fade},
                {.h=91,.s=175,.v=fade},
                {.h=77,.s=167,.v=fade},
                {.h=65,.s=182,.v=fade},
                {.h=56,.s=195,.v=fade},
                {.h=49,.s=208,.v=fade},
                {.h=43,.s=220,.v=fade},
                {.h=38,.s=233,.v=fade},
                {.h=32,.s=219,.v=fade},
                {.h=24,.s=203,.v=fade},
                {.h=14,.s=187,.v=fade},
                {.h=2,.s=171,.v=fade},
                {.h=245,.s=200,.v=fade},
                {.h=237,.s=237,.v=fade},
                {.h=232,.s=209,.v=fade},
                {.h=221,.s=168,.v=fade},
                {.h=196,.s=133,.v=fade},
        };
        for(int i=0;i<24;i++){
            showArc(leds,270+15*i,285+15*i,&hsvs[i],&hsvs[i]);
        }
    }

    return done;
}

/**
 * @brief Display the SmartHalo Disconnect Animation
 */
int disconnect_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount){
    int fade = 0;
    int done = 0;

    float fadeInTicks = 500.f/MILLISECONDS_PER_UPDATE;
    float fadeOutTicks =  1000.f/MILLISECONDS_PER_UPDATE;
    if(!done){
        int clock = animationClockCount;
        fade = clock*255/fadeInTicks;
        if(fade > 255){
            fade = 255 - (clock-50)*255/fadeOutTicks;
            if(fade < 0){
                fade = 0;
                done = 1;
            }
        }
        HsvColour_t hsvs[24] = {{.h=167,.s=125,.v=fade},
                {.h=151,.s=187,.v=fade},
                {.h=144,.s=237,.v=fade},
                {.h=139,.s=225,.v=fade},
                {.h=131,.s=210,.v=fade},
                {.h=121,.s=196,.v=fade},
                {.h=111,.s=188,.v=fade},
                {.h=101,.s=181,.v=fade},
                {.h=91,.s=175,.v=fade},
                {.h=77,.s=167,.v=fade},
                {.h=65,.s=182,.v=fade},
                {.h=56,.s=195,.v=fade},
                {.h=49,.s=208,.v=fade},
                {.h=43,.s=220,.v=fade},
                {.h=38,.s=233,.v=fade},
                {.h=32,.s=219,.v=fade},
                {.h=24,.s=203,.v=fade},
                {.h=14,.s=187,.v=fade},
                {.h=2,.s=171,.v=fade},
                {.h=245,.s=200,.v=fade},
                {.h=237,.s=237,.v=fade},
                {.h=232,.s=209,.v=fade},
                {.h=221,.s=168,.v=fade},
                {.h=196,.s=133,.v=fade},
        };
        for(int i=0;i<24;i++){
            showArc(leds,270+15*i,285+15*i,&hsvs[i],&hsvs[i]);
        }
    }

    return done;
}

int pointer_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct PointerAnimationData * data){
    float shiftTicks = 830/MILLISECONDS_PER_UPDATE;
    float fadeTicks = 160/MILLISECONDS_PER_UPDATE;
    uint8_t angleAlignmentAdjustment = 7;
    memset(leds,0,RGB_LED_COUNT);
    uint16_t heading = data->heading + 360 + angleAlignmentAdjustment;
    int currentValue = data->colour.v;
    HsvColour_t currentColour = {data->colour.h,data->colour.s,data->colour.v};
    if(data->off){
        if(animationClockCount < fadeTicks){
            int clock = animationClockCount;
            currentValue = (1-clock/fadeTicks)*currentValue;
            currentColour.v = currentValue;
        }else{
            return true;
        }
    }
    if(data->standby){
        int clock = animationClockCount%((int)shiftTicks);
        heading = clock/shiftTicks*360 + data->heading;
    }else if(animationClockCount < shiftTicks){
        int clock = animationClockCount;
        if(data->headingGap > 180){
            heading += (1-clock/shiftTicks)*(360-data->headingGap);
        }else if(data->headingGap < -180){
            heading -= (1-clock/shiftTicks)*(360+data->headingGap);
        }else{
            heading -= (1-clock/shiftTicks)*data->headingGap;
        }
    }
    float startAngle = heading - 45/2;
    float endAngle = heading + 45/2;
    LEDAnimationData_t animationData = getLEDAnimationData(startAngle, endAngle, &currentColour, &currentColour);
    showArc(leds,startAngle,endAngle,&currentColour,&currentColour);
    currentColour.v = currentValue * animationData.startFraction;
    showArc(leds,animationData.startLED*15-15,animationData.startLED*15,&currentColour,&currentColour);
    currentColour.v = currentValue * animationData.endFraction;
    showArc(leds,animationData.endLED*15,animationData.endLED*15+15,&currentColour,&currentColour);


    return false;
}

uint16_t pointerStandbyLocation_AnimationsLibrary(uint16_t animationClockCount, uint16_t pointerHeading){
    float shiftTicks = 830/MILLISECONDS_PER_UPDATE;
    return animationClockCount/shiftTicks*360+pointerHeading;
}

float speedometerIntroTicks_AnimationLibrary(){
    return 660/MILLISECONDS_PER_UPDATE;
}

int speedometer_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct SpeedometerAnimationData * data){
    float fadeTicks = 500/MILLISECONDS_PER_UPDATE;
    float moveTicks = 500/MILLISECONDS_PER_UPDATE;
    uint8_t offsetLEDs = LED_COUNT*2/3;
    float hueRange = 90.f;
    uint16_t clock = animationClockCount;
    memset(leds, 0, RGB_LED_COUNT);
    float fade = 1.f;
    if(data->off && animationClockCount > fadeTicks){
        return 1;
    }
    float position;
    if(data->off)
        fade = 1.f-clock/fadeTicks;
    position = !data->off && animationClockCount < moveTicks && data->gap != 0
            ? (data->percentage-data->gap*(1.f-clock/moveTicks))/100.f : data->percentage/100.f;
    HsvColour_t start = {hueRange,255,255*fade};
    HsvColour_t end = {hueRange*(1.f-position),255,255};
    uint16_t startAngle = offsetLEDs*15;
    uint16_t endAngle = offsetLEDs*15+17*position*15;
    LEDAnimationData_t animationData = getLEDAnimationData(startAngle, endAngle, &start, &end);
    HsvColour_t fractionalStart = {start.h,start.s,start.v*animationData.endFraction};
    HsvColour_t fractionalEnd = {end.h,end.s,end.v*animationData.endFraction};
    showArc(leds,startAngle,animationData.endLED*15+15,&fractionalStart,&fractionalEnd);
    showArc(leds,startAngle,animationData.endLED*15,&start,&end);
    return 0;
}

int averageSpeed_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AverageSpeedAnimationData * data){
    float moveTicks = 500/MILLISECONDS_PER_UPDATE;
    float fadeTicks = 500/MILLISECONDS_PER_UPDATE;
    float brightness = 1.f;
    if(data->off)
        brightness = 1.f-animationClockCount/fadeTicks;

    uint32_t speed = animationClockCount < moveTicks ?
            data->metersPerHour - data->gap*(1.f-animationClockCount/moveTicks) :
            data->metersPerHour;
    HsvColour_t averageSpeedColour = data->averageHsv;
    averageSpeedColour.v *= brightness;
    showArc(leds,353,367,&averageSpeedColour,&averageSpeedColour);
    uint16_t angle = 0;
    HsvColour_t speedColour;
    if(speed < data->averageMetersPerHour){
        angle = 360-180*(1.f-((float)speed)/data->averageMetersPerHour);
        speedColour = data->lowerHsv;
    }else{
        angle = 360+180*(((float)speed/data->averageMetersPerHour)-1.f);
        speedColour = data->higherHsv;
    }
    speedColour.v *= brightness;
    showArc(leds,angle-7,angle+7,&speedColour,&speedColour);

    if(brightness <= 0.0)
        return 1;
    return 0;
}

int fractionalData_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, struct FractionalData data, bool fractionalDataOff){
    float fadeTicks = 500/MILLISECONDS_PER_UPDATE;
    float brightness = 1.f;
    if(fractionalDataOff)
        brightness = 1.f-animationClockCount/fadeTicks;
    data.colour.v *= brightness;
    uint16_t angle = 180 + 360*data.percentage/100;

    showArc(leds,angle-7,angle+7,&data.colour,&data.colour);
    // light up neighbouring LEDs with half brightness
    data.colour.v *= 0.5;
    showArc(leds,angle-7-15,angle+7-15,&data.colour,&data.colour);
    showArc(leds,angle-7+15,angle+7+15,&data.colour,&data.colour);

    if(brightness <= 0.0)
        return 1;
    return 0;
}

float zeroToOne()
{
    return (float)rand() / (float)(RAND_MAX) ;
}

int fire_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, HsvColour_t colour, bool fireOff){
    float decayTicks = 5000/MILLISECONDS_PER_UPDATE;
    float fadeTicks = 500/MILLISECONDS_PER_UPDATE;
    float brightness = 1.f;
    if(fireOff)
        brightness = 1.f-animationClockCount/fadeTicks;
    colour.v *= brightness;

    uint8_t decayingAngle = 100*(1.f-fmin(1.f,animationClockCount/decayTicks));
    uint8_t angleSize = 35;
    LEDAnimationData_t animationData = getLEDAnimationData(180-angleSize*zeroToOne()-decayingAngle,
            180+angleSize*zeroToOne()+decayingAngle,
            &colour,
            &colour);
    showArc(leds,animationData.startLED*15,animationData.endLED*15+15,&colour,&colour);
    colour.v = animationData.startFraction;
    showArc(leds,animationData.startLED*15-15,animationData.startLED*15,&colour,&colour);
    colour.v = animationData.endFraction;
    showArc(leds,animationData.endLED*15+15,animationData.endLED*15+30,&colour,&colour);

    if(brightness <= 0.0)
        return 1;
    return 0;
}

int nightLightIntro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount){
    int introTicks = 380/MILLISECONDS_PER_UPDATE;
    if(animationClockCount < introTicks){
        int clock = animationClockCount;
        int introLEDCount = 14;
        HsvColour_t spinHsv = {.h = 38, .s = 234, .v = 255};
        int progress = introLEDCount*clock/introTicks;
        spinHsv.v = progress <= 10 ? 255 : progress < 13 ? 255 >> (progress - 10) : 0;
        int tail = progress <= 10 ? 30 : 30 - (progress-10)*15;
        clock = progress > 10 ? 10*introTicks/introLEDCount : clock;
        showLEDsSpin(leds,clock,introTicks/(introLEDCount*1.f),150,0,tail,&spinHsv,&spinHsv, true);
        showLEDsSpin(leds,clock,introTicks/(introLEDCount*1.f),210,360,tail,&spinHsv,&spinHsv, false);
        return 0;
    }
    return 1;
}

int nightLight_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, uint8_t * nightLEDPercentage, const struct FrontLightAnimationData * data){
    int fadingTicks = 1000/MILLISECONDS_PER_UPDATE;
    int blinkTicks = 500/MILLISECONDS_PER_UPDATE;
    if(*nightLEDPercentage != data->percentage || data->percentage > 0){
        int clock = animationClockCount;
        if(clock < fadingTicks){
            float progress = data->gap*clock*1.f/fadingTicks;
            *nightLEDPercentage = data->percentage-(data->gap-progress);
        }else{
            *nightLEDPercentage = data->percentage;
        }

        if(data->isBlinking){
            clock = animationClockCount%blinkTicks;
            *nightLEDPercentage = clock < blinkTicks/2 ? *nightLEDPercentage : 0;
        }

        return 0;
    }
    return 1;
}

int nightLightOutro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount){
    int outroTicks = 380/MILLISECONDS_PER_UPDATE;
    if(animationClockCount < outroTicks){
        int clock = animationClockCount;
        int outroLEDCount = 14;
        HsvColour_t spinHsv = {.h = 38, .s = 234, .v = 255};
        int progress = outroLEDCount*clock/outroTicks;
        spinHsv.v = progress <= 10 ? 255 : progress < 13 ? 255 >> (progress - 10) : 0;
        int tail = progress <= 10 ? 30 : 30 - (progress-10)*15;
        clock = progress > 10 ? 10*outroTicks/outroLEDCount : clock;
        showLEDsSpin(leds,clock,outroTicks/(outroLEDCount*1.f),330,180,tail,&spinHsv,&spinHsv, true);
        showLEDsSpin(leds,clock,outroTicks/(outroLEDCount*1.f),30,180,tail,&spinHsv,&spinHsv, false);
        return 0;
    }
    return 1;
}

int progressIntro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct ProgressAnimationData * data){
    float growTicks = 500/MILLISECONDS_PER_UPDATE;
    uint16_t clock = animationClockCount;
    if(clock < growTicks){
        growArc(leds,clock,growTicks/24.f,180,540,&data->colour1,&data->colour2);
        return 0;
    }else if(clock < growTicks *2){
        growArc(leds,growTicks*2-clock,growTicks/24.f,180,540,&data->colour1,&data->colour2);
        return 0;
    }
    return 1;
}

int progress_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct ProgressAnimationData * data){
    float fadeTicks = 500/MILLISECONDS_PER_UPDATE;
    float moveTicks = 500/MILLISECONDS_PER_UPDATE;
    float shadowTicks = 5000/MILLISECONDS_PER_UPDATE;
    uint8_t offsetLEDs = LED_COUNT/2;
    uint16_t clock = animationClockCount;
    memset(leds, 0, RGB_LED_COUNT);
    float fade = 1.f;
    if(data->off && animationClockCount > fadeTicks){
        return 1;
    }
    float position;
    position = !data->off && animationClockCount < moveTicks && data->gap != 0
            ? (data->progress-data->gap*(1.f-clock/moveTicks))/100.f : data->progress/100.f;
    if(data->off)
        fade = 1.f-clock/fadeTicks;
    if(data->progress == 100){
        uint16_t flashClock = clock%((int)fadeTicks*2);
        if(flashClock < fadeTicks)
            fade = 1.f-flashClock/fadeTicks;
        else if(flashClock < fadeTicks*2)
            fade = (flashClock-fadeTicks)/fadeTicks;
    }
    HsvColour_t fadedColour1 = {data->colour1.h,data->colour1.s,data->colour1.v*fade};
    HsvColour_t fadedColour2 = {data->colour2.h,data->colour2.s,data->colour2.v*fade};
    uint16_t startAngle = offsetLEDs*15;
    uint16_t endAngle = offsetLEDs*15+LED_COUNT*position*15;
    LEDAnimationData_t animationData = getLEDAnimationData(startAngle, endAngle, &fadedColour1, &fadedColour2);
    if(data->progress == 100){
        showArc(leds,startAngle,animationData.endLED*15,&fadedColour1,&fadedColour2);
        return 0;
    }
    float divisor = (moveTicks)/(animationData.endLED-12.f);
    uint8_t tail = 15*floor(clock%((int)shadowTicks)/divisor);
    if(tail > 15*(animationData.endLED-12.f-1))
        tail = 15*(animationData.endLED-12.f-1);
    if(animationData.endLED*15 > 180 && clock%((int)shadowTicks) < moveTicks){
        showLEDsSpin(leds,clock%((int)moveTicks),divisor,180,animationData.endLED*15,tail,&fadedColour1,&fadedColour2,false);
    }else if(animationData.endLED*15 > 180 && clock%((int)shadowTicks) < moveTicks + (moveTicks/(animationData.endLED-12.f+1))*moveTicks/divisor){
        uint8_t reducedLEDs = 1+floor((clock%((int)shadowTicks) - moveTicks)/divisor);
        HsvColour_t dimming = fadedColour1;
        dimming.v = dimming.v >> 1*reducedLEDs;
        showLEDsSpin(leds,clock%((int)shadowTicks),divisor,180-reducedLEDs*15,animationData.endLED*15,tail,&dimming,&dimming,false);
    }
    if(animationData.endFraction)
        showArc(leds,animationData.endLED*15,animationData.endLED*15+15,&fadedColour1,&fadedColour2);
    else if(animationData.endLED > 12)
        showArc(leds,animationData.endLED*15-15,animationData.endLED*15,&fadedColour1,&fadedColour2);
    return 0;
}

RgbColour_t adjustBrightnessFromArrayLocation(uint8_t * array, uint8_t position, uint8_t brightness){
    RgbColour_t currentRgb = {array[position],array[position+1],array[position+2]};
    HsvColour_t currentHsv;
    RgbToHsv(currentRgb, &currentHsv);
    currentHsv.v = brightness;
    HsvToRgb(currentHsv, &currentRgb);
    return currentRgb;
}

int customCircle_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct CustomCircleAnimationData *circleData){

    HsvColour_t mainColour = circleData->mainColour;
    HsvColour_t otherColour = circleData->otherColour;
    uint8_t count = circleData->count < MAX_ANGLES ? circleData->count : MAX_ANGLES;
    uint16_t width = circleData->width;
    bool flash = circleData->flash;
    uint8_t style = circleData->style;

    // copy array to fix any alignment problems
    int16_t angles[MAX_ANGLES];
    memcpy(angles, circleData->angles, sizeof(angles));

    const uint16_t beginTicks = 500/MILLISECONDS_PER_UPDATE;
    uint8_t circleDesign[LED_COUNT*3];
    float generalBrightness = 0.7f;
    int8_t direction = style == 0 ? 1 : style == 1 ? - 1 : 0;
    if(direction == 0 && !circleData->ready && animationClockCount < beginTicks)
        generalBrightness = 0.7f*animationClockCount/beginTicks;
    HsvColour_t white = {0,0,generalBrightness*255};
    showArc(circleDesign,0,360,&white,&white);
    otherColour.v *= generalBrightness;
    mainColour.v *= generalBrightness;
    for(int i=1;i<count;i++){
        //standardize angle
        int angle = (angles[i]/15)*15 + 360;
        showArc(circleDesign,angle,angle+width,&otherColour,&otherColour);
    }
    int mainAngle = (angles[0]/15)*15 + 360;
    showArc(circleDesign,mainAngle,mainAngle+width,&mainColour,&mainColour);

    uint8_t LEDOffset = 3*LED_COUNT/2;
    if(!circleData->ready){
        uint16_t clock = animationClockCount;
        uint8_t shownLEDs = LED_COUNT*clock/beginTicks;
        if (shownLEDs > LED_COUNT) shownLEDs = LED_COUNT;
        if(direction != 0){
            for(int i=0;i<shownLEDs;i++){
                uint8_t position = (3*(LEDOffset+i*direction))%(LED_COUNT*3);
                LEDS(position) = circleDesign[position];
                position = ((3*(LEDOffset+i*direction))+1)%(LED_COUNT*3);
                LEDS(position) = circleDesign[position];
                position = ((3*(LEDOffset+i*direction))+2)%(LED_COUNT*3);
                LEDS(position) = circleDesign[position];
            }
        }else{
            memcpy(leds,circleDesign,LED_COUNT*3);
            return clock > beginTicks;
        }
        if(shownLEDs == LED_COUNT)
            return 1;
        return 0;
    }

    memcpy(leds,circleDesign,LED_COUNT*3);

    if(circleData->ready){
        float blinkTicks = 500/MILLISECONDS_PER_UPDATE;
        float pulseTicks = 500/MILLISECONDS_PER_UPDATE;
        if(direction == 0){
            if(flash){
                uint16_t clock = animationClockCount%((int)blinkTicks*2);
                float pulse;
                if(clock < blinkTicks){
                    pulse = 1.0f - 1.0f*clock/blinkTicks;
                }else{
                    pulse = 1.0f*(clock-blinkTicks)/blinkTicks;
                }
                mainColour.v *= pulse;
                showArc(leds,mainAngle,mainAngle+width,&mainColour,&mainColour);
            }
        }else{
            uint16_t clock = animationClockCount%((int)pulseTicks*2);
            uint8_t shownLEDs;
            uint8_t animLEDs = direction == 1 ? (mainAngle - LEDOffset*15  + 360 + width)/15 : (LEDOffset*15 - mainAngle + 15)/15;
            if(clock < pulseTicks){
                shownLEDs = animLEDs*clock/pulseTicks;
            }else{
                shownLEDs = animLEDs;
            }
            if (shownLEDs > LED_COUNT) shownLEDs = LED_COUNT;
            uint8_t fade = 255;
            if(clock > pulseTicks){
                fade = 255*(0.7f + 0.3f*(1-(clock-pulseTicks)/pulseTicks));
            }
            for(int i=0;i<shownLEDs;i++){
                uint8_t position = (3*(LEDOffset+i*direction))%(LED_COUNT*3);
                RgbColour_t newRgb = adjustBrightnessFromArrayLocation(circleDesign, position, fade);
                LEDS(position) = newRgb.r;
                position = ((3*(LEDOffset+i*direction))+1)%(LED_COUNT*3);
                LEDS(position) = newRgb.g;
                position = ((3*(LEDOffset+i*direction))+2)%(LED_COUNT*3);
                LEDS(position) = newRgb.b;
            }
        }
    }
    if(circleData->off)
        memset(leds, 0, RGB_LED_COUNT);
    return circleData->off;
}

/**
 * @brief Display the state of charge animation
 */
int battery_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct StateOfChargeAnimationData * data){
    int fillTicks = 500/MILLISECONDS_PER_UPDATE;
    int retractTicks = 500/MILLISECONDS_PER_UPDATE;
    int pulseOffsetTicks = 6000/MILLISECONDS_PER_UPDATE;
    int idleTicks = 2000/MILLISECONDS_PER_UPDATE;
    if(animationClockCount < fillTicks){
        HsvColour_t startHsv = {.h = 0, .s = 255, .v = 85};
        HsvColour_t endHsv = {.h = 85, .s = 255, .v = 85};
        if(animationClockCount < fillTicks/2)
            growArc(leds,animationClockCount,(fillTicks/2)/12.f,180,60*360/100+180, &startHsv, &endHsv);
        else if(animationClockCount >= fillTicks/2){
            showArc(leds,180,375,&startHsv,&endHsv);
            growArc(leds,animationClockCount-fillTicks/2,(fillTicks/2)/11.f,15,180, &endHsv, &endHsv);
        }
    }else if(animationClockCount < (fillTicks + retractTicks)){
        uint8_t clock = retractTicks - (animationClockCount - fillTicks);
        float offLEDs = 23*(100-data->stateOfCharge)/100.f;
        HsvColour_t startHsv = {.h = 0, .s = 255, .v = 85};
        HsvColour_t endHsv = {.h = 85, .s = 255, .v = 85};
        if(offLEDs < 11){
            uint16_t minimum = retractTicks*(11-offLEDs)/11;
            clock = clock*(offLEDs)/11 + minimum;
            showArc(leds,180,375,&startHsv,&endHsv);
            growArc(leds,clock,retractTicks/11.f,15,180, &endHsv, &endHsv);
        }else{
            uint16_t minimum = retractTicks*(13-(offLEDs-11))/13.f;
            uint16_t firstClock = (float)clock/retractTicks < 1-11.f/offLEDs ? minimum + (clock/(1-11.f/offLEDs))*(50.f-minimum)/50.f : retractTicks;
            uint16_t secondClock = (float)clock/retractTicks > 1-11.f/offLEDs ? (clock - (retractTicks*(1-11.f/offLEDs)))/(11.f/offLEDs) : 0;
            growArc(leds,firstClock,retractTicks/(13.f),180,60*360/100+180, &startHsv, &endHsv);
            if(secondClock)
                growArc(leds,secondClock,retractTicks/11.f,15,180, &endHsv, &endHsv);
        }
    }else if(animationClockCount < (fillTicks + retractTicks+ idleTicks) || data->showIdle){
        float offLEDs = 23*(100-data->stateOfCharge)/100.f;
        HsvColour_t startHsv = {.h = 0, .s = 255, .v = 85};
        HsvColour_t endHsv = {.h = 85, .s = 255, .v = 85};
        if(offLEDs < 11){
            uint16_t minimum = retractTicks*(11-offLEDs)/11;
            showArc(leds,180,375,&startHsv,&endHsv);
            growArc(leds,minimum,retractTicks/11.f,15,180, &endHsv, &endHsv);
        }else{
            uint16_t minimum = retractTicks*(13-(offLEDs-11))/13;
            growArc(leds,minimum,retractTicks/13.f,180,60*360/100+180, &startHsv, &endHsv);
        }
    }else if(animationClockCount > (fillTicks + retractTicks+ idleTicks) && !data->charging){
        return 1;
    }else if(data->charging){
        uint16_t clock = (animationClockCount-(fillTicks+retractTicks+310))%pulseOffsetTicks;
        HsvColour_t startHsv = {.h = 0, .s = 255, .v = 85};
        HsvColour_t endHsv = {.h = 85, .s = 255, .v = 85};
        HsvColour_t offHsv = {.h = 85, .s = 255, .v = 0};
        HsvColour_t startAuraHsv = {.h = 0, .s = 255, .v = 100};
        HsvColour_t endAuraHsv = {.h = 85, .s = 255, .v = 100};
        HsvColour_t startPulseHsv = {.h = 0, .s = 255, .v = 150};
        HsvColour_t endPulseHsv = {.h = 85, .s = 255, .v = 150};

        //Display the state of charge without a pulse
        showArc(leds,180,60*360/100+180,&startHsv,&endHsv);
        if(data->stateOfCharge >= 60)
            showArc(leds, 0,15+(data->stateOfCharge-51)*360/100,&endHsv,&endHsv);
        //Remove unfilled state of charge LEDs
        if(data->stateOfCharge < 100)
            showArc(leds,525,195+(360*data->stateOfCharge/100 - 15),&offHsv,&offHsv);

        //leading aura
        if(clock/5 - 1 < data->stateOfCharge*24/100 && clock/5 < 24){
            if(clock/5 - 0 >= 0 && clock/5 - 1 < 55*24/100)
                showLEDsSpin(leds,clock,60/(12.f),180,60*360/100+180,0,&startAuraHsv,&endAuraHsv,false);
            if(clock/5 - 11 >= 0)
                showLEDsSpin(leds,clock,60/(12.f),180,60*360/100+180,0,&endAuraHsv,&endAuraHsv,false);
        }
        //pulse
        if(clock/5 - 2 < data->stateOfCharge*24/100 && clock/5 - 1 < 24){
            if(clock/5 - 1 >= 0 && clock/5 - 2 < 55*24/100)
                showLEDsSpin(leds,clock-5*1,60/(12.f),180,60*360/100+180,0,&startPulseHsv,&endPulseHsv,false);
            if(clock/5 - 12 >= 0)
                showLEDsSpin(leds,clock-5*1,60/(12.f),180,60*360/100+180,0,&endPulseHsv,&endPulseHsv,false);
        }
        //trailing aura
        if(clock/5 - 3 < data->stateOfCharge*24/100 && clock/5 -2 < 24){
            if(clock/5 - 2 >= 0 && clock/5 - 3 < 55*24/100)
                showLEDsSpin(leds,clock-5*2,60/(12.f),180,60*360/100+180,0,&startAuraHsv,&endAuraHsv,false);
            if(clock/5 - 13 >= 0)
                showLEDsSpin(leds,clock-5*2,60/(12.f),180,60*360/100+180,0,&endAuraHsv,&endAuraHsv,false);
        }
    }
    return 0;
}

static void angleShow(uint8_t * leds, int16_t angle, uint16_t width, HsvColour_t colour, HsvColour_t backgroundColour){
    float modifier = 360.f/LED_COUNT;
    int8_t location = round(angle/modifier);
    int16_t center = location*modifier;
    float start = width >= 15 ? 360+center-(width/2.f-7.5) : 360+center-(width/2.f);
    float end = width >= 15 ? 360+center+(width/2.f-7.5) : 360+center+(width/2.f);
    LEDAnimationData_t animationData = getLEDAnimationData(start, end+15, &colour, &colour);
    if(width >= 15){
        showArc(leds,animationData.startLED*modifier,animationData.endLED*modifier,&colour,&colour);
        if(animationData.startFraction != 0){
            HsvColour_t display = {.h = round(colour.h*animationData.startFraction + backgroundColour.h*(1-animationData.startFraction)),
                    .s = round(colour.s*animationData.startFraction + backgroundColour.s*(1-animationData.startFraction)),
                    .v = round(colour.v*animationData.startFraction + backgroundColour.v*(1-animationData.startFraction))};
            showArc(leds,animationData.startLED*15-15,animationData.startLED*15,&display,&display);
        }
        if(animationData.endFraction != 0){
            HsvColour_t display = {.h = round(colour.h*animationData.endFraction + backgroundColour.h*(1-animationData.endFraction)),
                    .s = round(colour.s*animationData.endFraction + backgroundColour.s*(1-animationData.endFraction)),
                    .v = round(colour.v*animationData.endFraction + backgroundColour.v*(1-animationData.endFraction))};
            showArc(leds,animationData.endLED*15,animationData.endLED*15+15,&display,&display);
        }
    }else{
        float progress = width/15.f;
        HsvColour_t display = {.h = colour.h*progress + backgroundColour.h*(1-progress), .s = colour.s*progress + backgroundColour.s*(1-progress), .v = colour.v*progress + backgroundColour.v*(1-progress)};
        showArc(leds,animationData.startLED*modifier,animationData.endLED*modifier,&display,&display);
    }
}

/**
 * @brief Show angles with a background
 */
int angleTwoBackground_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AnglesAnimationData * data){
    angleBackground_AnimationsLibrary(leds,animationClockCount,&data->angleData);
    struct AngleAnimationData nextAngle = {
            .angle = data->nextAngle,
            .colour = data->nextColour,
            .progress = data->nextProgress,
            .gap = data->nextGap,
            .width = data->nextWidth,
            .background = data->angleData.background,
            .backgroundColour = data->angleData.backgroundColour,
            .backgroundReady = data->angleData.backgroundReady,
            .repeat = data->angleData.repeat,
            .complete = data->angleData.complete
    };
    return angleBackground_AnimationsLibrary(leds,animationClockCount,&nextAngle);
}

/**
 * @brief Show angle with a background
 */
int angleBackground_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AngleAnimationData * data){
    float fillTicks = 500.f/MILLISECONDS_PER_UPDATE;
    float fadeTicks = 250.f/MILLISECONDS_PER_UPDATE;
    if(!data->backgroundReady){
        if(animationClockCount < fillTicks){
            uint16_t clock = animationClockCount;
            float currentWidth = data->width*clock/fillTicks;
            angleShow(leds,data->angle,round(currentWidth),data->backgroundColour, data->backgroundColour);
            return 0;
        }else{
            return 1;
        }
    }else if((data->progress >= 100 && animationClockCount >= fillTicks) || (data->progress >= 100 && data->gap == 0)){
        uint16_t clock = data->gap != 0 ? animationClockCount - fillTicks : animationClockCount;
        uint16_t ticksMax = fadeTicks*2;
        clock = clock%ticksMax;
        float fade;
        if(clock < fadeTicks){
            fade = 1.0f-clock/fadeTicks;
        }else{
            fade = (clock-fadeTicks)/fadeTicks;
        }
        HsvColour_t fadingColour = {data->colour.h,data->colour.s,data->colour.v*fade};
        angleShow(leds,data->angle,data->width,fadingColour,fadingColour);
    }else if(animationClockCount < fillTicks){
        angleShow(leds,data->angle,data->width,data->backgroundColour,data->backgroundColour);
        uint16_t clock = animationClockCount;
        float currentWidth = data->width*(data->progress/100.f-(data->gap/100.f)*(1-clock/fillTicks));
        angleShow(leds,data->angle,round(currentWidth),data->colour,data->colour);
        return 0;
    }else{
        angleShow(leds,data->angle,data->width,data->backgroundColour,data->backgroundColour);
        float currentWidth = data->width*(data->progress/100.f);
        angleShow(leds,data->angle,round(currentWidth),data->colour,data->colour);
        return 0;
    }
    return 0;
}


int angleTwo_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AnglesAnimationData * data){
    float maxTicks = ((1+data->angleData.repeat)*500.f + 700.f)/MILLISECONDS_PER_UPDATE;
    HsvColour_t blankColour = {0,0,0};
    if(!data->angleData.complete){
        struct AngleAnimationData adjustedData = data->angleData;
        adjustedData.colour = blankColour;
        angleBackground_AnimationsLibrary(leds,animationClockCount,&adjustedData);
        struct AngleAnimationData nextAngle = {
                .angle = data->nextAngle,
                .colour = blankColour,
                .progress = data->nextProgress,
                .gap = data->nextGap,
                .width = data->nextWidth,
                .background = data->angleData.background,
                .backgroundColour = data->angleData.backgroundColour,
                .backgroundReady = data->angleData.backgroundReady,
                .repeat = data->angleData.repeat,
                .complete = data->angleData.complete
        };
        return angleBackground_AnimationsLibrary(leds,animationClockCount,&nextAngle);
    }else{
        struct AngleAnimationData adjustedData = data->angleData;
        adjustedData.backgroundColour = blankColour;
        angleBackground_AnimationsLibrary(leds,animationClockCount,&adjustedData);
        struct AngleAnimationData nextAngle = {
                .angle = data->nextAngle,
                .colour = data->nextColour,
                .progress = data->nextProgress,
                .gap = data->nextGap,
                .width = data->nextWidth,
                .background = data->angleData.background,
                .backgroundColour = blankColour,
                .backgroundReady = data->angleData.backgroundReady,
                .repeat = data->angleData.repeat,
                .complete = data->angleData.complete
        };
        angleBackground_AnimationsLibrary(leds,animationClockCount, &nextAngle);
        if(animationClockCount > maxTicks){
            return 1;
        }
    }
    return 0;
}

int angle_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct AngleAnimationData * data){
    float maxTicks = ((1+data->repeat)*500.f + 700.f)/MILLISECONDS_PER_UPDATE;
    HsvColour_t blankColour = {0,0,0};
    struct AngleAnimationData adjustedData = *data;
    if(!data->backgroundReady){
        adjustedData.colour = blankColour;
        return angleBackground_AnimationsLibrary(leds,animationClockCount,&adjustedData);
    }else{
        adjustedData.backgroundColour = blankColour;
        angleBackground_AnimationsLibrary(leds,animationClockCount,&adjustedData);
        if(animationClockCount > maxTicks){
            return 1;
        }
    }
    return 0;
}

int angleIntro_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount){
    float sequenceTicks = 6*LED_COUNT/4.f;
    float durationTicks = sequenceTicks*3;
    if(animationClockCount < durationTicks){
        uint16_t clock = animationClockCount%((int)(sequenceTicks+1));
        int introLEDCount = LED_COUNT/4+3;
        HsvColour_t spinHsv = {.h = 67, .s = 255, .v = 255};
        int progress = introLEDCount*clock/(sequenceTicks);
        int8_t endGap = progress + 3 - introLEDCount;
        spinHsv.v = progress < 3 ? 255 >> (2 - progress) : progress >= introLEDCount - 2 ? 255 >> (endGap) : 255;
        int tail = progress < 3 ? progress*15 : progress >= introLEDCount - 2 ? (introLEDCount-progress-1)*15 : 30;
        if(progress < introLEDCount - 2){
            showLEDsSpin(leds,clock,sequenceTicks/(introLEDCount*1.f),90,0,tail,&spinHsv,&spinHsv, true);
            showLEDsSpin(leds,clock,sequenceTicks/(introLEDCount*1.f),270,360,tail,&spinHsv,&spinHsv, false);
        }else{
            showLEDsSpin(leds,clock,sequenceTicks/(introLEDCount*1.f),90 + endGap*15,0,tail,&spinHsv,&spinHsv, true);
            showLEDsSpin(leds,clock,sequenceTicks/(introLEDCount*1.f),270 - endGap*15,360,tail,&spinHsv,&spinHsv, false);
        }
        return 0;
    }else{
        return 1;
    }
}

/**
 * @brief Spiral Animation
 */
int spiral_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct SpiralAnimationData * data){
    showLEDsSpin(leds,animationClockCount,data->speed,data->tail+360*(1-data->clockwise),data->tail+360*data->clockwise,data->tail,&data->colour,&data->colour,true);
    if(!(animationClockCount%(LED_COUNT*3)))
        return 1;
    return 0;
}

/**
 * @brief Swipe animation
 */
int swipe_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, bool isRight, bool connectionState){
    uint8_t swipeTicks = 200/MILLISECONDS_PER_UPDATE;
    uint16_t clock = animationClockCount;
    HsvColour_t spinHSV = {.h = 21, .s = connectionState ? 0 : 209, .v = 255};
    float ledCount = 6.f;
    uint8_t tail = 15*clock/(swipeTicks/ledCount);
    showLEDsSpin(leds,clock,swipeTicks/ledCount,isRight?225:135,isRight?135:225,tail,&spinHSV,&spinHSV,true);
    return clock > swipeTicks;
}

/**
 * @brief Tap animations
 */
void taps_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, uint8_t code, uint8_t length, bool connectionState, eStandbyReason_t standbyReason, bool isShadow){
    if(length){
        uint8_t longTap = code & 0x1 ? 1 : 0;
        HsvColour_t startHsv = {.h = 21, .s = connectionState ? 0 : 209, .v = 255};
        HsvColour_t endHsv = {.h = 21, .s = connectionState ? 0 : 209, .v = 255};
        if(standbyReason == StandbySleep){
            startHsv.h = endHsv.h = 143;
            startHsv.s = endHsv.s = 219;
        }
        if(isShadow)
        	startHsv.v = endHsv.v = 30;
        showArc(leds, 270, 285+15*longTap, &startHsv, &endHsv);
        if(length > 1){
            longTap = (code >> 1) & 0x1 ? 1 : 0;
            showArc(leds, 315, 330+15*longTap, &startHsv, &endHsv);
        }
        if(length > 2){
            longTap = (code >> 2) & 0x1 ? 1 : 0;
            showArc(leds, 360, 375+15*longTap, &startHsv, &endHsv);
        }
        if(length > 3){
            longTap = (code >> 3) & 0x1 ? 1 : 0;
            showArc(leds, 405, 420+15*longTap, &startHsv, &endHsv);
        }
        if(length > 4){
            longTap = (code >> 4) & 0x1 ? 1 : 0;
            showArc(leds, 450, 465+15*longTap, &startHsv, &endHsv);
        }
    }
}

int alarmSettingChange_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, bool alarmEnabled){
    uint8_t growTicks = 500/MILLISECONDS_PER_UPDATE;
    if(alarmEnabled){
        HsvColour_t enableHsv = {.h = 13, .s =207, .v = 255};
        if(animationClockCount <= growTicks){
            uint16_t clock = animationClockCount;
            growArc(leds, clock, growTicks/8.f,60,180,&enableHsv,&enableHsv);
            growArc(leds, clock, growTicks/8.f,180,300,&enableHsv,&enableHsv);
            growArc(leds, clock, growTicks/8.f,300,420,&enableHsv,&enableHsv);
        }else if(animationClockCount <= growTicks*2){
            float clock = animationClockCount - growTicks;
            float fadePercentage = clock/growTicks;
            enableHsv.v = 255 - 255*fadePercentage;
            showArc(leds,0,360,&enableHsv,&enableHsv);
        }else{
            return true;
        }
        return false;
    }else{
        float transitionTicks = 250/MILLISECONDS_PER_UPDATE;
        HsvColour_t enableHsv = {.h = 13, .s =207, .v = 255};
        if(animationClockCount <= transitionTicks){
            float progress = animationClockCount/transitionTicks;
            enableHsv.v = 255*progress;
            showArc(leds,0,360,&enableHsv,&enableHsv);
        }else if(animationClockCount <= transitionTicks*2){
            uint16_t clock = animationClockCount - transitionTicks;
            float progress = clock/transitionTicks;
            enableHsv.h = 13+(80-13)*progress;
            showArc(leds,0,360,&enableHsv,&enableHsv);
        }else if(animationClockCount <= transitionTicks*2+growTicks){
            HsvColour_t disableHsv = {.h = 80, .s =207, .v = 255};
            uint16_t clock = growTicks - (animationClockCount - transitionTicks*2);
            growArc(leds, clock, growTicks/8.f,60,180,&disableHsv,&disableHsv);
            growArc(leds, clock, growTicks/8.f,180,300,&disableHsv,&disableHsv);
            growArc(leds, clock, growTicks/8.f,300,420,&disableHsv,&disableHsv);
        }else{
            return true;
        }
        return false;
    }
    return true;
}

int alarmStateChange_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, uint8_t alarmProgress, uint8_t * frontLEDPercentage){
    uint8_t alarmStepFlash = 500/MILLISECONDS_PER_UPDATE;
    uint16_t alarmTriggerLength = 15000/MILLISECONDS_PER_UPDATE;
    HsvColour_t enableHsv = {.h = 13, .s =207, .v = 255};
    if(alarmProgress < 100 && alarmProgress > 0){
        if(animationClockCount%(alarmStepFlash*2)){
            uint8_t clock = animationClockCount%(alarmStepFlash*2);
            float progress = 0.1+0.9*(clock%alarmStepFlash)*1.f/alarmStepFlash;
            enableHsv.v = 0;
            showArc(leds,0,360,&enableHsv,&enableHsv);
            uint8_t brightness = clock < alarmStepFlash ? 255*progress : 255-255*progress;
            enableHsv.v = brightness;
            int i;
            for(i=0;i<=8*alarmProgress/100;i++){
                showArc(leds,60,75+i*15,&enableHsv,&enableHsv);
                showArc(leds,180,195+i*15,&enableHsv,&enableHsv);
                showArc(leds,300,315+i*15,&enableHsv,&enableHsv);
            }
        }
        *frontLEDPercentage = 0;
        if(animationClockCount < alarmStepFlash * ALARM_BLINK_COUNT){
            return false;
        }else{
            return true;
        }
    }else if(alarmProgress != 0 && animationClockCount < alarmTriggerLength){
        uint8_t clock = animationClockCount%(alarmStepFlash*2);
        enableHsv.v = 128;
        if(clock < 2){
            enableHsv.s = 0;
            showArc(leds,0,360,&enableHsv,&enableHsv);
        }else if(clock <= 5){
            showArc(leds,0,360,&enableHsv,&enableHsv);
        }else{
            enableHsv.v = 0;
            showArc(leds,0,360,&enableHsv,&enableHsv);
        }

        *frontLEDPercentage = clock >= 25 && clock <= 30 ? 99 : 0;
        return false;
    }else{
        enableHsv.v = 0;
        showArc(leds,0,360,&enableHsv,&enableHsv);
    }
    return true;
}

int clock_AnimationsLibrary(uint8_t * leds, uint16_t animationClockCount, const struct ClockAnimationData * clockSettings){
    uint16_t hourAngle = 360*(clockSettings->hour%12)/12 + 15*(clockSettings->minute>32);
    uint16_t minuteAngle = 360*clockSettings->minute/60;
    uint8_t angleGap = abs(hourAngle-minuteAngle);
    uint8_t fadeInTicks = 631/MILLISECONDS_PER_UPDATE;
    uint8_t fadeOutTicks = 631/MILLISECONDS_PER_UPDATE;
    uint8_t blinkTicks = 166/MILLISECONDS_PER_UPDATE;
    blinkTicks = angleGap < 15 ? blinkTicks*2 : blinkTicks;
    uint16_t durationTick = clockSettings->duration/MILLISECONDS_PER_UPDATE;
    if(clockSettings->duration != -1 && animationClockCount > durationTick){
        uint16_t clock = animationClockCount%(durationTick+1);
        float fadeOut = (float)clock/fadeOutTicks;
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255-255*fadeOut};
        HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255-255*fadeOut};
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
        showArc(leds,minuteAngle,minuteAngle+15,&minuteColour,&minuteColour);
        if(fadeOut == 1){
            return true;
        }
        return false;
    }

    bool overlapping = ceil(hourAngle/15.f) == ceil(minuteAngle/15.f);
    uint16_t loopClock = overlapping ? (animationClockCount - fadeInTicks)%(blinkTicks*2) :(animationClockCount-fadeInTicks)%(fadeOutTicks/2 + blinkTicks + fadeInTicks/2 + blinkTicks);
    loopClock = overlapping ? loopClock > blinkTicks ? loopClock + fadeOutTicks : loopClock + fadeOutTicks/2 : loopClock;
    uint16_t minuteFinalAngle = minuteAngle > hourAngle ? minuteAngle : minuteAngle+360;
    uint16_t minuteSpinTicks = 2*(minuteFinalAngle - hourAngle)/15;
    if(clockSettings->intro && animationClockCount < 2*hourAngle/15){
        uint16_t clock = animationClockCount;
        float fadeIn = clock/fadeInTicks;
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255*fadeIn};
        showLEDsSpin(leds,clock,(2*hourAngle/15)/((hourAngle)/15),0,hourAngle,0,&hourColour,&hourColour,true);
    }else if(clockSettings->intro && animationClockCount <= (2*hourAngle/15 + minuteSpinTicks)){
        uint16_t clock = animationClockCount - 2*hourAngle/15;
        float fadeIn = (float)MIN((float)animationClockCount/fadeInTicks,1);
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255*fadeIn};
        HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255*fadeIn};
        uint16_t minuteSpin = minuteAngle > hourAngle ? ceil((minuteAngle-hourAngle)/15.f) : ceil((360+minuteAngle-hourAngle)/15.f);
        showLEDsSpin(leds,clock,(float)(minuteSpinTicks)/minuteSpin,hourAngle,minuteFinalAngle,0,&minuteColour,&minuteColour,true);
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
    }else if(animationClockCount < fadeInTicks){
        uint16_t clock = animationClockCount;
        float fadeIn = (float)clock/fadeInTicks;
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255*fadeIn};
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
        HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255*fadeIn};
        showArc(leds,minuteAngle,minuteAngle+15,&minuteColour,&minuteColour);
    }else if(loopClock < fadeOutTicks/2){
        uint16_t clock = loopClock;
        float fadeOut = (float)clock/fadeOutTicks;
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255};
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
        HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255-255*fadeOut};
        showArc(leds,minuteAngle,minuteAngle+15,&minuteColour,&minuteColour);
    }else if(loopClock < fadeOutTicks/2 + blinkTicks){
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255};
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
        if(!overlapping){
            HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255/2};
            showArc(leds,minuteAngle,minuteAngle+15,&minuteColour,&minuteColour);
        }
    }else if(loopClock < fadeOutTicks/2 + blinkTicks + fadeInTicks/2){
        uint16_t clock = loopClock - (fadeOutTicks/2 + blinkTicks);
        float fadeIn = (float)clock/fadeInTicks;
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255};
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
        HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255*(0.5+fadeIn)};
        showArc(leds,minuteAngle,minuteAngle+15,&minuteColour,&minuteColour);
    }else if(loopClock < fadeOutTicks/2 + blinkTicks + fadeInTicks/2 + blinkTicks){
        HsvColour_t hourColour = {clockSettings->hourHsv.h,clockSettings->hourHsv.s, 255};
        HsvColour_t minuteColour = {clockSettings->minuteHsv.h,clockSettings->minuteHsv.s, 255};
        showArc(leds,hourAngle,hourAngle+15,&hourColour,&hourColour);
        showArc(leds,minuteAngle,minuteAngle+15,&minuteColour,&minuteColour);
    }
    return false;
}

static void enableLED(uint8_t * leds, uint8_t led, uint8_t brightness, uint8_t ignore){
    //This is a test to check if we want to deliberately turn off LEDs during the tests to check if our software detects the errors.
    uint8_t ledsCapped = led%RGB_LED_COUNT;
    uint8_t ignoreMappedToRGB = ignore*3;
    if(ledsCapped == ignoreMappedToRGB || ledsCapped == ignoreMappedToRGB + 1 || ledsCapped == ignoreMappedToRGB + 2) return;

    LEDS(led%RGB_LED_COUNT) = brightness;
}

/**
 * @brief Display the halo tests
 */
void ledsTest_AnimationsLibrary(uint8_t * leds, uint8_t stage, uint8_t brightness, uint8_t ledsOffLED){
    //test stages 4 and up are for solid colour tests.
    uint8_t solidLEDTestStage = 4;
    if(stage >= solidLEDTestStage){
        for(int i=stage-solidLEDTestStage;i<RGB_LED_COUNT+stage-solidLEDTestStage; i+=3){
            enableLED(leds, i, brightness, ledsOffLED);
        }
    }else{
        for(int i=(stage-1)*3; i<RGB_LED_COUNT+(stage-1)*3; i+=9){
            enableLED(leds, i+0, brightness, ledsOffLED);
            enableLED(leds, i+4, brightness, ledsOffLED);
            enableLED(leds, i+8, brightness, ledsOffLED);
        }
    }
}


