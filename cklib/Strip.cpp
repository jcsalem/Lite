/* Jim Salem
  Sparkfun documentation:
  For the data pins, please pay attention to the arrow printed on the strip. You will need to connect to
  the end that is the begining of the arrows (data connection)--->

  New Connectors (4-pin plus 2-pin):
  Red = 5v
  Blue = CKI
  Green = SDI
  Yellow = GND

  If you have a 4-pin connection:
  Blue = 5V
  Red = SDI
  Green = CKI
  Black = GND

  If you have a split 5-pin connection:
  2-pin Red+Black = 5V/GND
  Green = CKI
  Red = SDI

 */
#include "WProgram.h"
#include "Strip.h"
#include "Buffer.h"

// Hardware declarations
int SDI = 2; //Red wire (not the red 5V wire!)
int CKI = 3; //Green wire

// Internal definitions for strip
typedef long StripColor;
#define MIN_CVAL 0
#define MAX_CVAL 255

// Gamma correction
#define USE_GAMMA
unsigned char gGamma[] = {
  0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,
2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,
5,5,5,5,5,6,6,6,6,7,7,7,8,8,8,8,
9,9,9,10,10,10,11,11,12,12,12,13,13,14,14,14,
15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,
23,24,24,25,25,26,27,27,28,29,29,30,31,31,32,33,
34,34,35,36,37,37,38,39,40,40,41,42,43,44,45,46,
46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,
62,63,64,65,66,68,69,70,71,72,73,74,76,77,78,79,
80,82,83,84,86,87,88,89,91,92,93,95,96,98,99,100,
102,103,105,106,108,109,111,112,114,115,117,118,120,122,123,125,
126,128,130,131,133,135,136,138,140,142,143,145,147,149,150,152,
154,156,158,160,162,163,165,167,169,171,173,175,177,179,181,183,
185,187,189,191,193,196,198,200,202,204,206,209,211,213,215,218,
220,222,224,227,229,231,234,236,238,241,243,246,248,251,253,255
};

StripColor gStripBuffer[STRIP_LENGTH];


// Color convert
StripColor RGBToStrip(const RGBColor& rgb)
  {
  StripColor scolor;
  int cval;

  if (rgb.r <= MIN_CVAL) cval = 0;
  else {
    if (rgb.r < MAX_CVAL) cval = rgb.r;
    else cval = 0xFF;
  }
#ifdef USE_GAMMA
  cval = gGamma[cval];
#endif
  scolor = cval;
  if (rgb.g <= MIN_CVAL) cval = 0;
  else {
    if (rgb.g < MAX_CVAL) cval = rgb.g;
    else cval = 0xFF;
  }
#ifdef USE_GAMMA
  cval = gGamma[cval];
#endif
  scolor = (scolor << 8) | cval;
  if (rgb.b <= MIN_CVAL) cval = 0;
  else {
    if (rgb.b < MAX_CVAL) cval = rgb.b;
    else cval = 0xFF;
  }
#ifdef USE_GAMMA
  cval = gGamma[cval];
#endif
  scolor = (scolor << 8) | cval;
  return scolor;
}

// Std setup
void StripInit(void)
  {
  pinMode(SDI, OUTPUT);
  pinMode(CKI, OUTPUT);
  digitalWrite(SDI, LOW);
  digitalWrite(CKI, LOW);

  for (int i = 0 ; i < STRIP_LENGTH ; ++i)
    gStripBuffer[i] = 0;
  StripUpdate();
  }

// Utilities
void StripClear (void) {
  for (int i = 0; i < STRIP_LENGTH; ++i)
    gStripBuffer[i] = 0;
}

void StripClearAndUpdate(void) {
  StripClear();
  StripUpdate();
}

void StripFill(const RGBColor& color) {
  for (int i = 0; i < STRIP_LENGTH; ++i)
    gStripBuffer[i] = RGBToStrip(color);
}

void StripSet (int pos, const RGBColor& color) {
    if (pos >= 0 && pos < STRIP_LENGTH)
      gStripBuffer[pos] = RGBToStrip(color);
  }

void StripRender(const Buffer& buffer) {
  short num = min(STRIP_LENGTH, buffer.Width);
  for (int i = 0; i < num; ++i)
    StripSet(i, buffer.colors[i][0]);
}

//Takes the current strip color array and pushes it out
void StripUpdate (void) {
  //Each LED requires 24 bits of data
  //MSB: R7, R6, R5..., G7, G6..., B7, B6... B0
  //Once the 24 bits have been delivered, the IC immediately relays these bits to its neighbor
  //Pulling the clock low for 500us or more causes the IC to post the data.

  for(int LED_number = 0 ; LED_number < STRIP_LENGTH ; LED_number++) {
    long this_led_color = gStripBuffer[LED_number]; //24 bits of color data

    for(byte color_bit = 23 ; color_bit != 255 ; color_bit--) {
      //Feed color bit 23 first (red data MSB)

      digitalWrite(CKI, LOW); //Only change data when clock is low

      long mask = 1L << color_bit;
      //The 1'L' forces the 1 to start as a 32 bit number, otherwise it defaults to 16-bit.

      if(this_led_color & mask)
        digitalWrite(SDI, HIGH);
      else
        digitalWrite(SDI, LOW);

      digitalWrite(CKI, HIGH); //Data is latched when clock goes high
    }
  }

  //Pull clock low to put strip into reset/post mode
  digitalWrite(CKI, LOW);
  delayMicroseconds(500); //Wait for 500us to go into reset
}

// Testing
void StripTest(void) {
#if 0
  for (int i = 0; i < STRIP_LENGTH; ++i) {
     StripClear();
     StripSet(i, WHITE);
     StripUpdate();
     delay(5);
  }
  for (int i = STRIP_LENGTH-1; i >= 0; --i) {
     StripClear();
     StripSet(i, WHITE);
     StripUpdate();
     delay(5);
  }
#endif
  for (int i = STRIP_LENGTH; i >= 0; --i) {
    StripClear();
    for (int j = 0; j < STRIP_LENGTH; ++j) {
      RGBColor c = WHITE;
      c *= j;
      c /= STRIP_LENGTH;
      short pos = (i + j) % STRIP_LENGTH;
      StripSet(pos, c);
    }
  StripUpdate();
  delay(10);
  }
  delay(250);
  StripClear();
  StripUpdate();
}
