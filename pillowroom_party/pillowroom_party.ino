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
byte mode = 'f'; // fade mode is default
byte spd = 20;

byte r;
byte g;
byte b;

int rgb[3];


float h=352; // crazy pink
float s=100;
float v=2;

///////////////////////////////////
// Rainbow shit

int autoRainbowOffset = 0;
int manualRainbowOffset = 0;
int rainbowShiftCounter = 0;
//int rainbowScale = 192/nleds;
int totalOffset;
int rainbowScale = 6;
uint32_t tmpColor;

///////////////////////////////////

////////////////////////
///// Wheel and Fade
float wheelHue = 0; // max 360
float fadeValue = 0; // 0 to 2 pi

///////////////////////
///// rAndom walk
float velocity = 0;
int acc = 100;
float max_speed = (float)spd/250;

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


// Function example takes H, S, I, and a pointer to the
// returned RGB colorspace converted vector. It should
// be initialized with:
//
// int rgb[3];
//
// in the calling function. After calling hsi2rgb
// the vector rgb will contain red, green, and blue
// calculated values.

void HSI_to_RGB(float H, float S, float I, int* rgb) {
  int r, g, b;
  H = fmod(H,360); // cycle H around to 0-360 degrees
  H = 3.14159*H/(float)180; // Convert to radians.
  S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
  I = I>0?(I<1?I:1):0;

  // Math! Thanks in part to Kyle Miller.
  if(H < 2.09439) {
    r = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    g = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    b = 255*I/3*(1-S);
  } else if(H < 4.188787) {
    H = H - 2.09439;
    g = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    b = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    r = 255*I/3*(1-S);
  } else {
    H = H - 4.188787;
    b = 255*I/3*(1+S*cos(H)/cos(1.047196667-H));
    r = 255*I/3*(1+S*(1-cos(H)/cos(1.047196667-H)));
    g = 255*I/3*(1-S);
  }
  rgb[0]=r;
  rgb[1]=g;
  rgb[2]=b;
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
        case 'a': // acceleration
          acc = value;
          break;
        case 'o': // rainbow offset
          manualRainbowOffset = (int)((float)value*191/250);
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
        HSI_to_RGB((rainbowScale*i+autoRainbowOffset+manualRainbowOffset)%360, 1.0, 1.0, rgb);
        rbuff[i] = (byte) rgb[0];
        gbuff[i] = (byte) rgb[1];
        bbuff[i] = (byte) rgb[2];
        // HSV_to_RGB((rainbowScale*i+autoRainbowOffset+manualRainbowOffset)%360, 100.0, 100.0, &r, &g, &b);
        // tmpColor = rainbowColor((rainbowScale*i+(totalOffset))%191);
        // rbuff[i] = tmpColor >> 16;
        // gbuff[i] = tmpColor >> 8;
        // bbuff[i] = tmpColor;
      }
      rainbowShiftCounter += spd; // counter increments according to speed
      if (rainbowShiftCounter > 249) // only auto-shift the rainbow when counter is high enough
      {
        autoRainbowOffset = (autoRainbowOffset + 1)%360;
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
    case 'i': // random inertial hue walk
      max_speed = (float)spd / 250;
      // in case max_speed just changed
      if (velocity > max_speed)
      {
        velocity = max_speed;
      }
      else if (velocity < (-1 * max_speed))
      {
        velocity = max_speed * -1;
      }
      velocity = velocity + ((float) random(-1 * acc, acc))/10000; // max change of acc/10000
      if (velocity > max_speed)
      {
        velocity = max_speed * 2 - velocity;
      }
      else if (velocity < (-1 * max_speed))
      {
        velocity = max_speed * -2 - velocity;
      }
      h = h + velocity;
      if (h >= 360)
      {
        h = h - 360;
      }
      else if (h < 0)
      {
        h = 360 + h;
      }
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
  randomSeed(analogRead(0)); // seed random number generator
}


void loop()
{
    handleOneMessage();
    calculateFrame();
    postFrame();
}

