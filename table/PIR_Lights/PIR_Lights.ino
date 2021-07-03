/*******************
 * Program: PIR Lights
 * Author:  Casey McGraw-Herdeg
 * Inputs:  PIR Motion Sensor (VCC, GND, Pin 2)
 * Outputs: LED Strip (VCC, GND, Pin 7) 
 * Notes:   Turns on lights with motion, adds sparkles with continued PIR activity, turns off lights after timeout period of no motion
 *******************/

// Handy Configs
bool logging = true;
int pauseThreshold = 5;

// Control Variables for Fade Pattern 
int lightness = 150; 
int saturate = 200; 
int maxBright = 255; 
int minBright = 100; 
int dimmer = 1; 
int bHue = 137;
int endHue = 190; 
int shifter = 1; 

// FastLED - Version: 3.3.2
#include <FastLED.h>
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>

int inputPin = 2; // PIR sensor pin
int val = 0;   
bool movement = true;

// LED Strip Config
#define LED_PIN     7 // Led output pin
#define NUM_LEDS    144
#define BRIGHTNESS  15
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define UPDATES_PER_SECOND 100

// Measure duration of movement
int moveTimer = 0; 
int pauseTimer = 0;
int lightTimer = 0;

void setup() {
	Serial.begin(9600);
	FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); 
	FastLED.setBrightness(  BRIGHTNESS ); 

	pinMode(inputPin, INPUT);     // declare sensor as input
}

void loop() {
	// PIR Sensing
	val = digitalRead(inputPin);  // read input value
	if (val == HIGH) {            // check if the input is HIGH      
		movement = true; 
	} else {
		movement = false; 
	}
		
	// Movement detected
	if (movement) {

		// Turn on lights
		fill_solid( leds, NUM_LEDS, CHSV(bHue,200,lightness)); 
		
		// Add Glitter while actively moving
		if( random8() < 200) {
			leds[ random16(NUM_LEDS) ] += CRGB::White;
		}

		// Add to movement timer
		moveTimer++;
        lightTimer++;

		// Reset time since last movement
		pauseTimer = 0; 
	}

	// No movement
	else {

		// Allow a brief pause
		if (pauseTimer < pauseThreshold) {
			
			// Turn on lights
			fill_solid( leds, NUM_LEDS, CHSV(bHue,200,lightness)); 
			
			// Add to pause duration
			pauseTimer++;
			lightTimer++;
		}

		// After pause threshold exceeded
		else {

		// Turn off lights
		fill_solid( leds, NUM_LEDS, CHSV(bHue,200,0)); 

		// // If it had been moving for a while, and stopped
		// if (lightTimer > 100){
		//     // Flash Red (maybe knock in future?) 
		//     FastLED.delay(500); 
		//     fill_solid( leds, NUM_LEDS, CHSV(1,255,200)); 
		//     FastLED.delay(100);
		// }

		// Reset movement timers
		moveTimer = 0;
		}
	}

        if (logging) {
            Serial.print("Move: "); 
            Serial.print(moveTimer); 
            Serial.print("\t\tPause: ");
            Serial.print(pauseTimer);
            Serial.print("\t\tLights on: ");
            Serial.print(lightTimer);
            Serial.println();  
        }
         

		// Smooth out pauses / bumps
		if (moveTimer < 5) {
			pauseThreshold = 2; 
		}
		else if (moveTimer < 20) {
			pauseThreshold = 20; 
		}
		else {
			pauseThreshold = 100; 
		}
		
	FastLED.show();

	// Fade brightness
	lightness = lightness + (3 * dimmer); 
	if(lightness >= maxBright){
		dimmer = -1;
	}
	else if (lightness <= minBright){
		dimmer = 1; 
	}

	// Fade hue
	bHue = bHue + (shifter); 
	if (bHue >= endHue){
		shifter = -1; 
	}
	else if (bHue <= 137)
	{
		shifter = 1; 
	}

	FastLED.delay(100);
}
