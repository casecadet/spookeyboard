/*******************
* Program: Knobby Clock
* Author:  Casey McGraw-Herdeg
* Inputs:  RTC Clock (VCC, GND, SDA [A4], SCL [A5])
*          KY-040 Encoder knob (VCC, GND, DT [3], CLK [2])
* Outputs: LED Strip (VCC, GND, [5]) 
* Notes:   Marks the passage of time with individual LED markers for hour, minute, and second.
*          Customizable: number of LEDs, brightness and hue fade settings
*******************/

// Handy customizations
int brightness = 40;
int hue = 137;
#define startLED    0
#define NUM_LEDS    144
bool logging = false;

// Dimmer settings
int minBright = 40;
int maxBright = 120;
int dimmer = 0;
int dimmerFactor = 3;

// Color fade settings
int minHue = 100;
int maxHue = 200; 
int hueShifter = 0;
double hueShiftFactor = 3;

    /** 
    *   Todo: 
    *   - wrap hues around color wheel
    *   - use encoder button to toggle hue / brightness control 
    *   - modes for adjusting speed of hue / brightness fade?
    **/ 

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <FastLED.h>
#include <Wire.h>
#include "RTClib.h"

RTC_PCF8523 rtc;

#define LED_PIN     5
#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 120
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

// Encoder Vars
#include <NSEncoder.h>
#define ENCODER_S1_PIN 3 //Define you encoder connection pin Here
#define ENCODER_S2_PIN 2
int enc_position = 0;
int last_knob_movement = 0;
int knob_movement = 0;
NSEncoder enc(ENCODER_S1_PIN, ENCODER_S2_PIN, 4);

// Internal math vars
int secondBrightness = brightness + 10;

// LEDs per minute interval
double LEDpm = NUM_LEDS / 60.0;

// location of LED to be lit
int hourMark;
int minuteMark; 
int secondMark;
int prevSecond;

void setup () {
    
    // Check for clock connection
    Serial.begin(57600);
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    // Initialize Clock
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
    if (! rtc.initialized()) {
        Serial.println("RTC is NOT running!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Initialize LEDs
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    FastLED.clear();  
}

void loop () {

    /* Knob input */
    knob_movement = 0;
    
    // If movement value updated
    if((knob_movement = enc.get_diffPosition()) != 0)
      {
        if(logging){
            Serial.print("\tEncoder Diff: ");
            Serial.println(knob_movement);
        }
    
        last_knob_movement = knob_movement;
      }
    /* End knob input */

    // Use knob to adjust settings
    // Control hue
    if (knob_movement){
        hue += (knob_movement * hueShiftFactor);
    }

    // Set brightness
    if(dimmer){
        // Dim lights
        brightness = brightness + (dimmer); 
        if(brightness >= maxBright){
            dimmer = -1;
        }
        else if (brightness <= minBright){
            dimmer = 1; 
        }
    }
    
    // Cap at max & min brightness
    brightness = brightness > maxBright ? maxBright : brightness;
    brightness = brightness < minBright ? minBright : brightness;
    
    secondBrightness = brightness + 10;
    secondBrightness = secondBrightness > maxBright ? maxBright : secondBrightness;
    brightness = secondBrightness < minBright ? minBright : secondBrightness;
    
    //  Set background hue
    if (hueShifter){
          hue = hue + (hueShifter); 
          if (hue >= maxHue){
            hueShifter = -1; 
          }
          else if (hue <= minHue)
          {
            hueShifter = 1; 
          }
    }
    
    // Cap at max & min hues
    hue = hue > maxHue ? maxHue : hue;
    hue = hue < minHue ? minHue : hue;

    // Fill background LEDs
    fill_solid( leds, NUM_LEDS, CHSV(hue,200,brightness));

    // Get current time
    DateTime now = rtc.now();

    if(logging){
        Serial.print(now.hour());
        Serial.print(":"); 
        Serial.print(now.minute()); 
        Serial.print(":"); 
        Serial.print(now.second());
        Serial.print('\n');
    }
    
    // Calculate time locations
    hourMark = (((now.hour()%12) *5 ) * LEDpm); 
    minuteMark = now.minute() * LEDpm; 
    secondMark = now.second() * LEDpm; 

    // Adjust for clock orientation
    if(startLED){
        hourMark = (hourMark + startLED) % NUM_LEDS;
        minuteMark = (minuteMark +startLED) % NUM_LEDS; 
        secondMark = (secondMark + startLED) % NUM_LEDS;
        prevSecond = (secondMark + NUM_LEDS -1) % NUM_LEDS; 
    }
    
    // Set colors for time markers
    leds[hourMark] = CRGB(50, 30, 0);
    leds[minuteMark] = CRGB(100, 0, 40);
    leds[secondMark] = CHSV(hue, 100, secondBrightness);
    
    FastLED.delay(100);
}
