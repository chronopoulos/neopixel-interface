/*
 pillowroom_party
 Chris Chronopoulos
 January 24, 2014
 */

#include <Adafruit_NeoPixel.h>
//#include "WS2812_Definitions.h"

#define nleds 119
#define DATA_PIN 2

Adafruit_NeoPixel leds = Adafruit_NeoPixel(nleds, DATA_PIN, NEO_GRB + NEO_KHZ800);

int i = 0;
int j = 0;

byte rbuff[nleds];
byte gbuff[nleds];
byte bbuff[nleds];

byte tmpByte;

// state variables
byte flag, value;
byte mode = 0;
byte spd = 0;
double hue = 0.5;
double saturation = 0.5;
double intensity = 0.5;

byte r;
byte *rp = &r;
byte g;
byte *gp = &g;
byte b;
byte *bp = &b;
byte rgbarray[3] = {r, g, b};

///////////////////////////////////

void setWhite(void)
{
  for (i=0; i<nleds; i++)
  {
    leds.setPixelColor(i, 255, 255, 255);
  }
  leds.show();
}

void setRed(void)
{
  for (i=0; i<nleds; i++)
  {
    leds.setPixelColor(i, 255, 0, 0);
  }
  leds.show();
}

void setGreen(void)
{
  for (i=0; i<nleds; i++)
  {
    leds.setPixelColor(i, 0, 255, 0);
  }
  leds.show();
}

void setBlue(void)
{
  for (i=0; i<nleds; i++)
  {
    leds.setPixelColor(i, 0, 0, 255);
  }
  leds.show();
}

void handleOneMessage(void)
{
    tmpByte = Serial.read();
    if (tmpByte==254) // start byte
    {
      while (true) // keep reading until stop byte, which breaks out
      {
        while (Serial.available()==0) {} // wait until first byte arrives
        tmpByte = Serial.read();

        if (tmpByte==255) // stop byte
        {
          break;
        } 
        else
        {
          flag = tmpByte;
        }
        //-------------------------------
        while (Serial.available()==0) {} // wait until next byte arrives
        tmpByte = Serial.read();
        if (tmpByte==255) // stop byte
        {
          break;
        } 
        else
        {
          value = tmpByte;
        }
      };
      switch (flag)
      {
        case 0:
          mode = value;
          break;
        case 1: // hue
          r = value;
          break;
        case 2: // saturation
          g = value;
          break;
        case 3: // intensity
          b = value;
          break;
        case 4: // speed
          spd = value;
      }
    }  
}


void calculateFrame(void)
{
  switch (mode)
  {
    case 0: // uniform
      //HSV_to_RGB(hue, saturation, intensity, rp, gp, bp);
      //hsl2rgb(hue, saturation, intensity, rgbarray);
      for (i=0; i<nleds; i++)
      {
        rbuff[i] = r;
        gbuff[i] = g;
        bbuff[i] = b;
      }
      break;
  }
}


void postFrame(void)
{
  for (i=0; i<nleds; i++)
  {
    leds.setPixelColor(i, rbuff[i], gbuff[i], bbuff[i]);
  }
  leds.show();
}


void setup()
{
  Serial.begin(9600);
  leds.begin();  // Call this to start up the LED strip.
}


void loop()
{
    handleOneMessage();
    calculateFrame();
    postFrame();
}

