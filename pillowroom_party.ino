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
byte mode = 'u';
byte spd = 0;
double hue = 0.5;
double saturation = 0.5;
double intensity = 0.5;

byte rgbarray[3] = {r, g, b};

void calculateFrame(void)
{
  switch (mode)
  {
    case 'u': // uniform
      //HSV_to_RGB(hue, saturation, intensity, rp, gp, bp);
      hsl2rgb(hue, saturation, intensity, rgbarray);
      for (i=0; i<nleds; i++)
      {
        rbuff[i] = rgbarray[0];
        gbuff[i] = rgbarray[1];
        bbuff[i] = rgbarray[2];
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
        //Serial.println(tmpByte); // DEBUG!
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
        //Serial.println(tmpByte);  // DEBUG!
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
          for (i=0; i<nleds; i++)
          {
            leds.setPixelColor(i, 255, 255, 255);
          }
        case 'H': // hue
          // hue = (float) value*1.411764705882353; // 1.411764 = 360./250
          hue = (double) value/250.; // 1.411764 = 360./250
          for (i=0; i<nleds; i++)
          {
            leds.setPixelColor(i, 255, 0, 0);
          }
        case 'S': // saturation
          //saturation = (float) value/2.5;         // 2.5 = 100./250
          saturation = (double) value/250.;         // 2.5 = 100./250
          for (i=0; i<nleds; i++)
          {
            leds.setPixelColor(i, 0, 255, 0);
          }
        case 'I': // intensity
          //intensity = (float) value/2.5;          // 2.5 = 100./250
          intensity = (double) value/250.;          // 2.5 = 100./250
          for (i=0; i<nleds; i++)
          {
            leds.setPixelColor(i, 0, 0, 255);
          }
        case 's': // speed
          spd = value; // byte value!
      }

    }  
}

void setup()
{
  Serial.begin(57600);
  leds.begin();  // Call this to start up the LED strip.
}

void loop()
{
    handleOneMessage();
    calculateFrame();
    //postFrame();
  
}

void hsl2rgb(double h, double s, double l, byte rgb[]) {
    double r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        r = hue2rgb(p, q, h + 1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1/3);
    }

    rgb[0] = r * 255;
    rgb[1] = g * 255;
    rgb[2] = b * 255;
}

double hue2rgb(double p, double q, double t) {
    if(t < 0) t += 1;
    if(t > 1) t -= 1;
    if(t < 1/6) return p + (q - p) * 6 * t;
    if(t < 1/2) return q;
    if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
    return p;
}

