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

RGBColor   RGBColor::operator*(const float& v) const {
  RGBColor ret;
  ret.r = r * v;
  ret.g = g * v;
  ret.b = b * v;
  return ret;
}

RGBColor& RGBColor::operator*=(const float& v) {
  r = r * v;
  g = g * v;
  b = b * v;
  return *this;
}

RGBColor RGBColor::operator/(const float& v) const {
  RGBColor ret;
  ret.r = r / v;
  ret.g = g / v;
  ret.b = b / v;
  return ret;
}

RGBColor& RGBColor::operator/=(const float& v) {
  r = r / v;
  g = g / v;
  b = b / v;
  return *this;
}

char RGBColor::rAsChar(void) const {
  if (r < 0) return 0;
  else if (r >= 1.0) return 255;
  short ret = (r * 255.0) + 0.5;
  return ret;
}

char RGBColor::gAsChar(void) const {
  if (g < 0) return 0;
  else if (g >= 1.0) return 255;
  short ret = (g * 255.0) + 0.5;
  return ret;
}

char RGBColor::bAsChar(void) const {
  if (b < 0) return 0;
  else if (b >= 1.0) return 255;
  short ret = (b * 255.0) + 0.5;
  return ret;
}
