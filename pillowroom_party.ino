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
byte mode = 'u';
byte spd = 0;
double hue = 0.5;
double saturation = 0.5;
double intensity = 0.5;

byte r;
//byte *rp = &r;
byte g;
//byte *gp = &g;
byte b;
//byte *bp = &b;

float h, s, v;

///////////////////////////////////

void HSV_to_RGB(float h, float s, float v, byte *r, byte *g, byte *b)
{
  int i;
  float f,p,q,t;
  
  h = max(0.0, min(360.0, h));
  s = max(0.0, min(100.0, s));
  v = max(0.0, min(100.0, v));
  
  s /= 100;
  v /= 100;
  
  if(s == 0) {
    // Achromatic (grey)
    *r = *g = *b = round(v*255);
    return;
  }
 
  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch(i) {
    case 0:
      *r = round(255*v);
      *g = round(255*t);
      *b = round(255*p);
      break;
    case 1:
      *r = round(255*q);
      *g = round(255*v);
      *b = round(255*p);
      break;
    case 2:
      *r = round(255*p);
      *g = round(255*v);
      *b = round(255*t);
      break;
    case 3:
      *r = round(255*p);
      *g = round(255*q);
      *b = round(255*v);
      break;
    case 4:
      *r = round(255*t);
      *g = round(255*p);
      *b = round(255*v);
      break;
    default: // case 5:
      *r = round(255*v);
      *g = round(255*p);
      *b = round(255*q);
    }
}


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
    //setWhite();
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
        case 'm':
          mode = value;
          break;
        case 'H': // hue
          h = value*1.44;
          break;
        case 'S': // saturation
          s = value*0.4;
          break;
        case 'I': // intensity
          v = value*0.4;
          break;
        case 's': // speed
          spd = value;
      }
    }  
}


void calculateFrame(void)
{
  switch (mode)
  {
    case 'u': // uniform
      //HSV_to_RGB(hue, saturation, intensity, rp, gp, bp);
      //hsl2rgb(hue, saturation, intensity, rgbarray);
      HSV_to_RGB(h, s, v, &r, &g, &b);
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

