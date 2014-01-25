/*
 neopixel_hof
 hof = higher-order functions
 Chris Chronopoulos
 January 21, 2014
 NOTES:
 void loop ()
 {
   checkUSB_BUFFER();
   caculate_frame();
   post_frame();
 }
 */

#include <Adafruit_NeoPixel.h>
//#include "WS2812_Definitions.h"

#define LED_COUNT 11
#define DATA_PIN 4

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, DATA_PIN, NEO_GRB + NEO_KHZ800);

int i = 0;
int j = 0;

byte rbuff[LED_COUNT];
byte gbuff[LED_COUNT];
byte bbuff[LED_COUNT];

byte b, redness;

void setup()
{
  Serial.begin(14400);
  leds.begin();  // Call this to start up the LED strip.
  //Serial.write(0B11111110);
  //Serial.write('ohai');

}

void loop()
{

  if (Serial.available()>0)
  {
    b = Serial.read();
    if (b==254)
    {
      while (true)
      {
        while (Serial.available()==0) {} // wait
        b = Serial.read();
        if (b==255)
        {
          //Serial.write('B');
          break;
        } 
        else
        {
          //Serial.write('A');
          redness = b;
        }
      }
      Serial.println(redness);
      setAllUniform(redness, redness, redness);
    }
  }
}

void buff2leds(byte* r, byte* g, byte* b) // TODO: optimize this using pointers
{
  for (j=0; j<LED_COUNT; j++)
  {
    leds.setPixelColor(j, r[j], g[j], b[j]);
  }
  leds.show();
}

void setAllUniform(byte red, byte green, byte blue)
{
  for (j=0; j<LED_COUNT; j++)
  {
    leds.setPixelColor(j, red, green, blue);
  }
  leds.show();
}




