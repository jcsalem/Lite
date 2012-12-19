// Color data structures

#ifndef _COLOR_H
#define _COLOR_H

#include "utils.h"

class RGBColor; // fwd decl
class HSVColor; // fwd decl

// Abstract base class
class Color
    {
public:
    Color() {}
    virtual ~Color() {}

    // Create from a string.  Returns true if successful
    // RGB components are from 0 to 1 and separated by spaces or commas.
    static Color* AllocFromString(csref str, string* errmsg = NULL, bool ignoreRangeErrors = false);

    // Descriptiom
    virtual string ToString() const {return "unknown_color";}

    // Default conversion functions can be overridden
    // These return the components as 8-bit chars from 0 to 255
    virtual char rAsChar(void) const;
    virtual char gAsChar(void) const;
    virtual char bAsChar(void) const;
    virtual void ToRGBColor(RGBColor*) const = 0; // Must be implemented by all classes
    };

class RGBColor : public Color
  {
public:
    RGBColor(float rr, float gg, float bb) : Color() {r = rr; g = gg; b = bb;}
    RGBColor(const Color& color);
    RGBColor() : Color() {r = g = b = 0;}

    // Create from a string.  Returns true if successful
    // RGB components are from 0 to 1 and separated by spaces or commas.
    static bool FromString(csref str, RGBColor* rgb, string* errmsg = NULL, bool ignoreRangeErrors = false);

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
    virtual char rAsChar(void) const;
    virtual char gAsChar(void) const;
    virtual char bAsChar(void) const;
    virtual void ToRGBColor(RGBColor* colorptr) const {*colorptr = *this;}
    virtual string ToString() const;
  };

class HSVColor : public Color
  {
    friend class Color;
public:
    HSVColor(float hh, float ss, float vv) : Color() {h = hh; s = ss; v = vv;}
    HSVColor(const Color& color);
    HSVColor() : Color() {h = s = v = 0;}

    // Create from a string.  Returns true if successful
    // HSV components are from 0 to 1 and separated by spaces or commas.
    static bool FromString(csref str, HSVColor* hsv, string* errmsg = NULL, bool ignoreRangeErrors = false);

    // operations
    HSVColor operator*(const float& v) const;
    HSVColor operator/(const float& v) const;
    HSVColor& operator*=(const float& v);
    HSVColor& operator/=(const float& v);
    // Values and accessors
    float h;  // 0.0 is red. Wraps around, so that 1.0 is always the same as 0.0
    float s;  // 0.0 to 1.0; 0 is always gray. >= 1 is always fully saturated
    float v;  // 0.0 to 1.0; 0 is always black. >= 1 is always the max for that color
    virtual void ToRGBColor(RGBColor*) const;
    virtual string ToString() const;
    RGBColor ToRGBColor() const {RGBColor rgb; ToRGBColor(&rgb); return rgb;}
private:
    void SetFromRGB(const RGBColor& rgb);
  };

// A way of specifying a range of colors
class HSVColorRange
{
public:
    HSVColorRange(const Color& cc1, const Color& cc2);
    HSVColor GetRandomColor(void) const;  // returns a random color from the range
    HSVColor GetColor(float index) const; // returns a color from within the range (0 <= range <= 1)
    HSVColor c1;
    HSVColor c2;
    string GetDescription() const;

};

// Standard color names
#define BLACK   RGBColor(  0,  0,  0)
#define WHITE   RGBColor(1.0,1.0,1.0)
#define GRAY    RGBColor(.25,.25,.25)
//Brown needs help
#define BROWN   RGBColor(.60,.20,.05)

#define RED     RGBColor(1.0,  0,  0)
#define GREEN   RGBColor(  0,1.0,  0)
#define BLUE    RGBColor(  0,  0,1.0)

#define YELLOW  RGBColor(1.0,1.0,  0)
#define MAGENTA RGBColor(1.0,  0,1.0)
#define CYAN    RGBColor(  0,1.0,1.0)

#define ORANGE  RGBColor(1.0,.33,  0)
#define PURPLE  RGBColor(.63,.13,.94)

// Random color utilities (really part of LFramework.h)
// --color option
namespace L {
typedef enum {kRandomColorDefault = 0, kRandomColorBright = 1, kRandomColorRGB = 2,
            kRandomColorHalloween = 3, kRandomColorStarry = 4, kRandomColorRealStar = 5, kRandomColorRange = 6, kRandomColorExact = 7, kRandomColorChristmas = 8}
    RandomColorMode_t;
extern RandomColorMode_t    gRandomColorMode;
extern RGBColor             gRandomColor1;
extern RGBColor             gRandomColor2;
bool ParseColorMode(csref str, string* errmsg = NULL);
string CurrentColorModeAsString();
};

RGBColor RandomColor();


#endif
