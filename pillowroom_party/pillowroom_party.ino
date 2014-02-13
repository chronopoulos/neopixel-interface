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

const float pi = 3.1415;

byte rbuff[nleds];
byte gbuff[nleds];
byte bbuff[nleds];

byte tmpByte;

// state variables
byte flag, value;
byte mode = 'u';
byte spd = 0;

byte r;
byte g;
byte b;


float h=0;
float s=100;
float v=50;

///////////////////////////////////
// rainbow shit

int autoRainbowOffset = 0;
int manualRainbowOffset = 0;
int rainbowShiftCounter = 0;
//int rainbowScale = 192/nleds;
int totalOffset;
int rainbowScale = 8;
uint32_t tmpColor;

///////////////////////////////////

////////////////////////
///// wheel and fade
float wheelHue = 0; // max 360
float fadeValue = 0; // 0 to 2 pi

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
          h = value*1.43426; // evenly space 251 values around the hue wheel
          break;
        case 'S': // saturation
          s = value*0.4;
          break;
        case 'I': // intensity
          v = value*0.4;
          break;
        case 's': // speed
          spd = value;
          break;
        case 'o': // rainbow offset
          manualRainbowOffset = value*191/250;
          break;
      }
    }
}

uint32_t rainbowColor(byte position)
{
  // 6 total zones of color change:
  if (position < 31)  // Red -> Yellow (Red = FF, blue = 0, green goes 00-FF)
  {
    return leds.Color(0xFF, position * 8, 0);
  }
  else if (position < 63)  // Yellow -> Green (Green = FF, blue = 0, red goes FF->00)
  {
    position -= 31;
    return leds.Color(0xFF - position * 8, 0xFF, 0);
  }
  else if (position < 95)  // Green->Aqua (Green = FF, red = 0, blue goes 00->FF)
  {
    position -= 63;
    return leds.Color(0, 0xFF, position * 8);
  }
  else if (position < 127)  // Aqua->Blue (Blue = FF, red = 0, green goes FF->00)
  {
    position -= 95;
    return leds.Color(0, 0xFF - position * 8, 0xFF);
  }
  else if (position < 159)  // Blue->Fuchsia (Blue = FF, green = 0, red goes 00->FF)
  {
    position -= 127;
    return leds.Color(position * 8, 0, 0xFF);
  }
  else  //160 <position< 191   Fuchsia->Red (Red = FF, green = 0, blue goes FF->00)
  {
    position -= 159;
    return leds.Color(0xFF, 0x00, 0xFF - position * 8);
  }
}


void calculateFrame(void)
// perhaps this function should just call leds.setPixelColor()?
{
  switch (mode)
  {
    case 'u': // uniform
      HSV_to_RGB(h, s, v, &r, &g, &b);
      for (i=0; i<nleds; i++)
      {
        rbuff[i] = r;
        gbuff[i] = g;
        bbuff[i] = b;
      }
      break;
    case 'r': // automatic rainbow
      totalOffset = autoRainbowOffset+manualRainbowOffset;
      for (i=0; i<nleds; i++)
      {
//        HSV_to_RGB((rainbowScale*i+autoRainbowOffset+manualRainbowOffset)%360, 100.0, 100.0, &r, &g, &b);
        tmpColor = rainbowColor((rainbowScale*i+(totalOffset))%191);
        rbuff[i] = tmpColor >> 16;
        gbuff[i] = tmpColor >> 8;
        bbuff[i] = tmpColor;
      }
      rainbowShiftCounter += spd; // counter increments according to speed
      if (rainbowShiftCounter > 249) // only auto-shift the rainbow when counter is high enough
      {
        autoRainbowOffset = (autoRainbowOffset + 1)%191;
        rainbowShiftCounter = rainbowShiftCounter % 250;
      }
      break;
    case 'R': // manual rainbow
      for (i=0; i<nleds; i++)
      {
        tmpColor = rainbowColor((rainbowScale*(i+manualRainbowOffset))%192);
        rbuff[i] = tmpColor >> 16 ;
        gbuff[i] = tmpColor >> 8;
        bbuff[i] = tmpColor;
      }
      break;
    case 'w': // wheel
      HSV_to_RGB(wheelHue, s, v, &r, &g, &b);
      for (i=0; i<nleds; i++)
      {
        rbuff[i] = r;
        gbuff[i] = g;
        bbuff[i] = b;
      }
      wheelHue = wheelHue + (float)spd/250;
      if (wheelHue >= 360)
      {
        wheelHue = wheelHue - 360;
      }
      break;
    case 'f': // fade with sine
      HSV_to_RGB(h, s, (sin(fadeValue)/2 + 0.5)*v, &r, &g, &b);
      for (i=0; i<nleds; i++)
      {
        rbuff[i] = r;
        gbuff[i] = g;
        bbuff[i] = b;
      }
      fadeValue += .01;
      if (fadeValue > (10*pi)) // ranges from 0 to 10pi
      {
        fadeValue = 0;
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

