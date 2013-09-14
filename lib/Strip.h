// Strip data structures
/*
  The SparkFun (individually controllable) RGB strip contains a bunch of WS2801 ICs. These
  are controlled over a simple data and clock setup. The WS2801 is really cool! Each IC has its
  own internal clock so that it can do all the PWM for that specific LED for you. Each IC
  requires 24 bits of 'greyscale' data. This means you can have 256 levels of red, 256 of blue,
  and 256 levels of green for each RGB LED. REALLY granular.
 
  To control the strip, you clock in data continually. Each IC automatically passes the data onto
  the next IC. Once you pause for more than 500us, each IC 'posts' or begins to output the color data
  you just clocked in. So, clock in (24bits * 32LEDs = ) 768 bits, then pause for 500us. Then
  repeat if you wish to display something new.
  
    You will need to connect 5V/Gnd from the Arduino (USB power seems to be sufficient).
  
*/

#ifndef _STRIP_H
#define _STRIP_H

#include "Color.h"
#include "Buffer.h"

#define STRIP_LENGTH 32 //32 LEDs on this strip

void StripRender(const Buffer& buffer);
void StripClear(void);
void StripClearAndUpdate(void);
void StripSet(int pos, const RGBColor& color);
void StripFill(const RGBColor& color);
void StripUpdate(void);
void StripInit(void);
void StripTest(void);

#endif  //_STRIP_H

