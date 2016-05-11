///////////////////////////////////////////////////////////////////////////////////////
// Arduino interface for the use of operated LEDs                                    //
// Uses Adalight protocol and is compatible with Boblight, Prismatik etc             // 
// "Magic Word" for synchronisation is 'Ada' followed by LED High, Low and Checksum  //
///////////////////////////////////////////////////////////////////////////////////////

// LEDs should be externally powered -- trying to run anymore than just a few off 
// the Arduino's 5V line is generally a Bad Idea.  LED ground should also be 
// connected to Arduino ground.

// --------------------------------------------------------------------
//   This file is part of Adalight.

//   Adalight is free software: you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as
//   published by the Free Software Foundation, either version 3 of
//   the License, or (at your option) any later version.

//   Adalight is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU Lesser General Public License for more details.

//   You should have received a copy of the GNU Lesser General Public
//   License along with Adalight.  If not, see
//   <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------

#include <FastLED.h>
  
  ///// User definitions /////

  // Define the number of LEDs. 
  #define NUM_LEDS 150
  // Define Data Pin
  #define DATA_PIN 11
  #define CLOCK_PIN 13
  // Baudrate, higher rate allows faster refresh rate and more LEDs (defined in /etc/boblight.conf)
  #define serialRate 115200
  
  // This controlls how long to wait till after communication is broken to turn off the lights
  // it's clock cycle dependent and not exact but generally good enough. Uncomment to enable. 
  // uint32_t serialTimeout=100000000;
  

  // If you intend to have more than 255 LEDs (!) and have the harware to pull it off 
  // you'll have to change the i below from int8_t to int16_t to be able to address more LEDs.
  uint8_t i;
  
  // Adalight sends a "Magic Word" before sending the pixel data to "syncronize" the data streams.
  uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk;
  
  // initialise LED-array
  CRGB leds[NUM_LEDS];

void setup()
{
    
    #define RGB_FLASH_TEST_OFF     // RGB flashes at boot to showcase all is working well by default. Uncomment to to TURN OFF this test
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Uncomment/edit one of the following lines to match your leds arrangement. Note the RGB portion 
    // should denote the order the chip embeded with your LEDs expects to recieve its colour data. If your 
    // not certain what the order is, just upload the code with the default of RGB and pay attention to the 
    // colours that flash when you power on the device. Whatever order it flashes the colours is the order 
    // of the LEDS. If it flashes RED GREEN BLUE for example it means no changes has to be made while GREEN
    // RED BLUE means you need to change it to GRB. Note that after you make the change to the correct order
    // it will flash RED GREEN BLUE at boot.
           
    // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
      
    // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<NEOPIXEL, DATA_PIN, RGB>(leds, NUM_LEDS);
      
    // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
      
    // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);

    FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
    // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   
       
    #ifndef RGB_FLASH_TEST_OFF       
      delay(800);
      LEDS.showColor(CRGB(255, 0, 0)); //Flash Red
      delay(800);
      LEDS.showColor(CRGB(0, 255, 0)); //Flash Green
      delay(800);
      LEDS.showColor(CRGB(0, 0, 255)); //Flash Blue
      delay(800);
    #endif
      
    LEDS.showColor(CRGB(0, 0, 0));
    
    Serial.begin(serialRate);
    Serial.print("Ada\n"); // Send "Magic Word" string to host
  
}

void loop() {
  
  // Syncronizes the data stream/checks the checksum for validity
  // to ensure the next byte from the serial buffer is the data for red.
  syncronize();
  
  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  // read the transmission data and set LED values
  for (i = 0; i < NUM_LEDS; i++) {
    byte r, g, b;    
    r = serialread();
    g = serialread();
    b = serialread();
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
  // shows new values
 FastLED.show();
}


// syncronizes the data to the Ada protocol
void syncronize(){
    // wait for first byte of Magic Word
  for(i = 0; i < sizeof prefix; ++i) {
    waitLoop: 
    // Check next byte in Magic Word
    if(prefix[i] == serialread()) continue;
    // otherwise, start over
    i = 0;
    goto waitLoop;
  }
  
  // Hi, Lo, Checksum
  
  hi=serialread();
  lo=serialread();
  chk=serialread();
  
  // if checksum does not match go back to wait
  if (chk != (hi ^ lo ^ 0x55))
  {
    i=0;
    goto waitLoop;
  }
}

byte serialread(){
  
  #ifdef serialTimeout        // loop code with timeout
    uint32_t t=0;
    // Will not leave this loop till the next stream of data comes in
    while (!Serial.available()){
      if (t>serialTimeout) {
        LEDS.showColor(CRGB(0, 0, 0));
        Serial.print("Ada\n"); // Send "Magic Word" string to host
        t=0;
      };
    t++;  
    }//endwhile  
  #else                      // loop code without timeout
     while (!Serial.available());
  #endif
 return Serial.read();   
}


// not yet implemented code for fading out the leds on timeout instead of hard shutoff
void fadeout()
{
  
}
