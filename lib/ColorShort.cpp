#include "Color.h"

// Color code

RGBColor   RGBColor::operator+(const RGBColor& c) const {
  RGBColor ret;
  ret.r = r + c.r;
  ret.g = g + c.g;
  ret.b = b + c.b;
  return ret;
}
RGBColor&  RGBColor::operator+=(const RGBColor& c) {
  r += c.r;
  g += c.g;
  b += c.b;
  return *this;
}

RGBColor   RGBColor::operator-(const RGBColor& c) const {
  RGBColor ret;
  ret.r = r - c.r;
  ret.g = g - c.g;
  ret.b = b - c.b;
  return ret;
}
RGBColor&  RGBColor::operator-=(const RGBColor& c) {
  r -= c.r;
  g -= c.g;
  b -= c.b;
  return *this;
}

RGBColor   RGBColor::operator*(const short& v) const {
  RGBColor ret;
  ret.r = r * v;
  ret.g = g * v;
  ret.b = b * v;
  return ret;
}

RGBColor& RGBColor::operator*=(const short& v) {
  r = r * v;
  g = g * v;
  b = b * v;
  return *this;
}

RGBColor RGBColor::operator/(const short& v) const {
  RGBColor ret;
  ret.r = r / v;
  ret.g = g / v;
  ret.b = b / v;
  return ret;
}

RGBColor& RGBColor::operator/=(const short& v) {
  r = r / v;
  g = g / v;
  b = b / v;
  return *this;
}
