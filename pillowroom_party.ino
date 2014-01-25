/*
 pillowroom_party
 Chris Chronopoulos
 January 24, 2014
 */

#include <Adafruit_NeoPixel.h>
//#include "WS2812_Definitions.h"

#define nleds 59
#define DATA_PIN 2

Adafruit_NeoPixel leds = Adafruit_NeoPixel(nleds, DATA_PIN, NEO_GRB + NEO_KHZ800);

int i = 0;
int j = 0;

byte rbuff[nleds];
byte gbuff[nleds];
byte bbuff[nleds];

byte tmpByte;

byte r;
byte *rp = &r;
byte g;
byte *gp = &g;
byte b;
byte *bp = &b;

// GLOBAL VARIABLES:
byte flag, value;
byte mode, spd;
float hue, saturation, intensity;

void calculateFrame(void)
{
  switch (mode)
  {
    case 'u': // uniform
      HSV_to_RGB(hue, saturation, intensity, rp, gp, bp);
      for (i=0; i<nleds; i++)
      {
        rbuff[i] = *rp;
        gbuff[i] = *gp;
        bbuff[i] = *bp;
      }
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

void handleOneMessage(void)
{
    tmpByte = Serial.read();
    Serial.println(tmpByte); // DEBUG!
    if (tmpByte==254) // start bit
    {
      while (true) // keep reading until stop bit, which breaks out
      {
        while (Serial.available()==0) {} // wait until first byte arrives
        tmpByte = Serial.read();
        Serial.println(tmpByte); // DEBUG!
        if (tmpByte==255) // stop bit
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
        Serial.println(tmpByte);  // DEBUG!
        if (tmpByte==255) // stop bit
        {
          break;
        } 
        else
        {
          value = tmpByte;
        }
      }
      switch (flag)
      {
        case 'm':
          mode = value;
        case 'H': // hue
          hue = (float) value*1.411764705882353; // 1.411764 = 360./250
        case 'S': // saturation
          saturation = (float) value/2.5;         // 2.5 = 100./250
        case 'I': // intensity
          intensity = (float) value/2.5;          // 2.5 = 100./250
        case 's': // speed
          spd = value; // byte value!
      }

    }  
}

void setup()
{
  Serial.begin(14400);
  leds.begin();  // Call this to start up the LED strip.
}

void loop()
{

  if (Serial.available()>0)
  {
    handleOneMessage();
    calculateFrame();
    postFrame();
  }
  
}




