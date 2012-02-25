// Color data structures

#ifndef _COLOR_H
#define _COLOR_H

#define MAX_COLOR 255

struct RGBColor 
  {
    RGBColor(short rr, short gg, short bb) {r = rr; g = gg; b = bb;}
    RGBColor() {r = g = b = 0;}
    
    // operations
    RGBColor operator+(const RGBColor& c) const;
    RGBColor operator-(const RGBColor& c) const;
    RGBColor operator*(const short& v) const;
    RGBColor operator/(const short& v) const;
    RGBColor& operator+=(const RGBColor& c);
    RGBColor& operator-=(const RGBColor& c);
    RGBColor& operator*=(const short& v);
    RGBColor& operator/=(const short& v);
    short r;
    short g;
    short b;
  };
  
#define BLACK RGBColor(0,0,0)
#define RED   RGBColor(MAX_COLOR,0,0)
#define GREEN RGBColor(0,MAX_COLOR,0)
#define BLUE  RGBColor(0,0,MAX_COLOR)
#define WHITE RGBColor(MAX_COLOR,MAX_COLOR,MAX_COLOR)

#define GRAY  RGBColor(MAX_COLOR/4,MAX_COLOR/4,MAX_COLOR/4)

#endif
