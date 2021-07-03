/*******************
 * Program: AcceLEDS
 * Author:  Casey McGraw-Herdeg
 * Inputs:  Accelerometer (VCC, GND, XCL, XCA)
 * Outputs: LED Strip (VCC, GND, Pin 5) 
 * Notes:   Combined acceleration along axes affects brightness of LEDs. Flashes red when stopped after longer period of motion
 *******************/


    #include "Wire.h" // This library allows you to communicate with I2C devices.
    const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
    int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t xPrev, yPrev, zPrev; 
int16_t xDif, yDif, zDif; 
int16_t sumDif; 
    int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
    int16_t temperature; // variables for temperature data
    char tmp_str[7]; // temporary variable used in convert function
    char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
      sprintf(tmp_str, "%6d", i);
      return tmp_str;
    }
#define THRESHOLD 1000

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
    #include <FastLED.h>
    #include <Wire.h>
    
#define LED_PIN     3
#define NUM_LEDS    17
#define BRIGHTNESS  100
    #define LED_TYPE    WS2811
    #define COLOR_ORDER GRB
    CRGB leds[NUM_LEDS];
    
    #define UPDATES_PER_SECOND 100

// Control Variables for Fade Pattern 
int lightness = 150; 
int maxBright = 255; 
int minBright = 100; 
int dimmer = 1; 
int bHue = 137;
int endHue = 190; 
int shifter = 1; 

// Measure duration of movement
int moveTimer = 0; 
int pauseTimer = 0; 
int pause = 10; 

    
    void setup() {
      Serial.begin(9600);
      Wire.begin();
      Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
      Wire.write(0x6B); // PWR_MGMT_1 register
      Wire.write(0); // set to zero (wakes up the MPU-6050)
      Wire.endTransmission(true);
      
      // Initialize comparison variables
      xPrev = yPrev = zPrev = 0; 
      xDif = yDif = zDif = 0; 

      FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip ); 
      FastLED.setBrightness(  BRIGHTNESS ); 

    }
    void loop() {
      
      Wire.beginTransmission(MPU_ADDR);
      Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
      Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
      Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
      
      accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
      accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
      accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
   
      // Compare to previous readings
      xDif = (accelerometer_x - xPrev);
      yDif = (accelerometer_y - yPrev); 
      zDif = (accelerometer_z - zPrev); 
      sumDif = abs(xDif) + abs(yDif) + abs(zDif);

//      // Calculate motion-based brightness
      lightness = (sumDif/50 +85); 
      if (lightness > 255) {
        lightness = 255; 
      }



      fill_solid( leds, NUM_LEDS, CHSV(bHue, 200, 0)); 

      // Shared Threshold Trigger
      if (sumDif > THRESHOLD) {
          Serial.print("\t\t"); 
          Serial.print(lightness); 
          Serial.print("\tMovement!"); 

          // Turn on lights
          fill_solid( leds, NUM_LEDS, CHSV(bHue,200,lightness)); 
         

          // Add to movement timer
          moveTimer++; 
          Serial.print("\t"); 
          Serial.print(moveTimer); 
          pauseTimer = 0; 

          Serial.println(); 
      }
      // No movement
      else {

          // Allow a brief pause
          if (pauseTimer < pause) {
           
            // Turn on lights
            fill_solid( leds, NUM_LEDS, CHSV(bHue,200,lightness)); 
            
//            Serial.print("\tPause:"); 
//            Serial.print(pauseTimer);
//            Serial.println();   

            
            // Add to pause duration
            pauseTimer++;
            moveTimer++; 
      
            }

          // After pause period
          else {
        
          // Turn off lights
          fill_solid( leds, NUM_LEDS, CHSV(bHue,200,0)); 
  
          // If it had been moving for a while, and stopped
          if (moveTimer > 20 && moveTimer < 60){
            // Chance of flash on stop 
            int rando= random(1,10); 
              if( rando < 6) {
                // Flash Red (maybe knock in future?) 
                FastLED.delay(500); 
                fill_solid( leds, NUM_LEDS, CHSV(1,255,150)); 
                FastLED.delay(100);
                fill_solid( leds, NUM_LEDS, CHSV(1,255,0)); 
                FastLED.delay(400);
              }
            Serial.print(rando); 
          }

          else if (moveTimer > 60){
                // Flash Red (maybe knock in future?) 
                FastLED.delay(500); 
                fill_solid( leds, NUM_LEDS, CHSV(1,255,150)); 
                FastLED.delay(100);
                fill_solid( leds, NUM_LEDS, CHSV(1,255,0)); 
                FastLED.delay(400);
          }
          

  
          // Reset movement timers
          moveTimer = 0; 
        }
      }

      // Smooth out pauses / bumps
      if (moveTimer < 5) {
        pause = 2; 
      }
      else if (moveTimer > 60){
        pause = 5;
      }
      else {
        pause = 10; 
      }

      
      // Store readings for comparison
      xPrev = accelerometer_x; 
      yPrev = accelerometer_y; 
      zPrev = accelerometer_z; 
      
  FastLED.show();

//  // Dim lights
//  lightness = lightness + (3 * dimmer); 
//  if(lightness >= maxBright){
//    dimmer = -1;
//  }
//  else if (lightness <= minBright){
//    dimmer = 1; 
//  }

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
