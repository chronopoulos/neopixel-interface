/*
 neopixel_interface
 Chris Chronopoulos
 January 20, 2014
 */

#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"

#define LED_COUNT 60
#define DATA_PIN 4

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, DATA_PIN, NEO_GRB + NEO_KHZ800);

int i = 0;
int j = 0;

byte rbuff[LED_COUNT];
byte gbuff[LED_COUNT];
byte bbuff[LED_COUNT];

void setup()
{
  Serial.begin(57600);
  leds.begin();  // Call this to start up the LED strip.
  /*for (i=0;i<LED_COUNT;i++)
  {
    rbuff[i] = 127;
    gbuff[i] = 0;
    bbuff[i] = 255;
  }
  buff2leds(rbuff, gbuff, bbuff);*/

}

void loop()
{
   while (Serial.available() == 0) {}
   rbuff[i] = Serial.read();
   while (Serial.available() == 0) {}
   gbuff[i] = Serial.read();
   while (Serial.available() == 0) {}
   bbuff[i] = Serial.read();
   i++;
   if (i==LED_COUNT)
   {
     buff2leds(rbuff, gbuff, bbuff);
     //delay(1000);
     /*for (j=0;j<LED_COUNT;j++)
     {
       Serial.print(rbuff[j]); Serial.print(", ");
       Serial.print(gbuff[j]); Serial.print(", ");
       Serial.print(bbuff[j]); Serial.print(", ");
     }*/
   i=0;
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


