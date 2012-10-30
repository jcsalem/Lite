// Color data structures

#ifndef _COLOR_H
#define _COLOR_H

struct RGBColor
  {
    RGBColor(float rr, float gg, float bb) {r = rr; g = gg; b = bb;}
    RGBColor() {r = g = b = 0;}

    // operations
    RGBColor operator+(const RGBColor& c) const;
    RGBColor operator-(const RGBColor& c) const;
    RGBColor operator*(const float& v) const;
    RGBColor operator/(const float& v) const;
    RGBColor& operator+=(const RGBColor& c);
    RGBColor& operator-=(const RGBColor& c);
    RGBColor& operator*=(const float& v);
    RGBColor& operator/=(const float& v);
    // Values and accessors
    float r;
    float g;
    float b;
    // These return the components as 8-bit chars from 0 to 255
    char rAsChar(void) const;
    char gAsChar(void) const;
    char bAsChar(void) const;
  };

#define BLACK RGBColor(0,0,0)
#define RED   RGBColor(1,0,0)
#define GREEN RGBColor(0,1,0)
#define BLUE  RGBColor(0,0,1)
#define WHITE RGBColor(1,1,1)

#define GRAY  RGBColor(.25,.25,.25)

#endif
