#include "utils.h"
#include "Color.h"
#include "utilsRandom.h"
#include <string>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "utilsOptions.h"
#include "utilsParse.h"

//--------------------------------------------------------------------------
// Utilities
//--------------------------------------------------------------------------
// Make these local declarations
namespace {

size_t SkipToNextComponent(csref str, size_t seppos, string* errmsg) {
    // str[seppos] must be either a comma or space.
    // Returns the position of the next char that's not a space or comma
    bool hasComma = false;
    while (true)
    {
        char c = str[seppos++];
        if (c == ' ') continue;
        if (c == ',') {
            if (hasComma) {
                if (errmsg) *errmsg = "Missing color component (too many commas in a row)";
                return string::npos;
            }
            hasComma = true;
            continue;
        }
        if (c == '\0') {
            if (errmsg) *errmsg = "Fewer than three color components";
            return string::npos;
        }
        return seppos - 1;
    }
}

bool ParseOneComponent(csref str, float* fptr, string* errmsg, bool ignoreRangeErrors) {
    if (str.empty()) {
        if (errmsg) *errmsg = "Missing color component";
        return false;
    }

    if (str.find_first_not_of("0123456789+-.eE") != string::npos) {
        if (errmsg) *errmsg = "Invalid color: " + str + " (must be valid floating point)";

        return false;
    }

    if (! StrToFlt(str, fptr)) {
        if (errmsg) *errmsg = "Invalid color: " + str + " (must be valid floating point): " + ErrorCodeString();
        return false;
    }
    //*fptr = StrToFlt()atof(str.c_str());
    if (!ignoreRangeErrors && *fptr < 0) {
        if (errmsg) *errmsg = "Negative color component";
        return false;
    }
    if (!ignoreRangeErrors && *fptr > 1.0) {
        if (errmsg) *errmsg = "Color component greater than one";
        return false;
    }
    return true;
}

bool ParseColorComponents(csref strarg, float* a, float* b, float* c, string* errmsg, bool ignoreRangeErrors) {
    *a = *b = *c = 0.0; // clear values in case error occurs
    string str = TrimWhitespace(strarg);
    if (str.empty()) {
        if (errmsg) *errmsg = "Empty string used to specify color.";
        return false;
    }
    // First component
    size_t startpos, pos;
    pos = str.find_first_of(" ,");
    if (pos == string::npos) {
        if (errmsg) *errmsg = "Color must be specified with 3 components.";
        return false;
    }
    if (! ParseOneComponent(str.substr(0, pos), a, errmsg, ignoreRangeErrors))
        return false;

    // Second component
    startpos = SkipToNextComponent(strarg, pos, errmsg);
    if (startpos == string::npos) return false;
    pos = str.find_first_of(" ,", startpos);
    if (pos == string::npos) {
        if (errmsg) *errmsg = "Color must be specified with 3 components.";
        return false;
    }
    if (! ParseOneComponent(str.substr(startpos, pos - startpos), b, errmsg, ignoreRangeErrors))
        return false;

    // Third component
    startpos = SkipToNextComponent(strarg, pos, errmsg);
    if (startpos == string::npos) return false;
    if (! ParseOneComponent(str.substr(startpos), c, errmsg, ignoreRangeErrors))
        return false;

    return true;
}

string CheckAndRemoveParens(csref strarg, string* errmsg) {
    string str = TrimWhitespace(strarg);
    if (str.empty()) {
        if (errmsg) *errmsg = "Missing color components.";
        return string();
    }
    size_t len = str.length();
    if (str[0] != '(' || str[len-1] != ')') {
        if (errmsg) *errmsg = "Missing color components.";
        return string();
    }
    return TrimWhitespace(str.substr(1, len-2));
}

};  // namespace


//--------------------------------------------------------------------------
// Base Color stuff
//--------------------------------------------------------------------------
unsigned char Color::rAsChar(void) const {
    RGBColor rgb;
    ToRGBColor(&rgb);
    return rgb.rAsChar();
}

unsigned char Color::gAsChar(void) const {
    RGBColor rgb;
    ToRGBColor(&rgb);
    return rgb.gAsChar();
}

unsigned char Color::bAsChar(void) const {
    RGBColor rgb;
    ToRGBColor(&rgb);
    return rgb.bAsChar();
}

struct NameToColor_t {string name; RGBColor color;} gNameToColor[] = {
    {"white",   WHITE},
    {"black",   BLACK},
    {"gray",    GRAY},
    {"red",     RED},
    {"green",   GREEN},
    {"blue",    BLUE},
    {"yellow",  YELLOW},
    {"magenta", MAGENTA},
    {"cyan",    CYAN},
    {"orange",  ORANGE},
    {"purple",  PURPLE},
    {"brown",   BROWN},
    };

bool ParseNamedColor(csref str, float* r, float* g, float* b) {
    int num = sizeof(gNameToColor) / sizeof(struct NameToColor_t);
    for (int i = 0; i < num; ++i) {
        if (StrEQ(str, gNameToColor[i].name)) {
            *r = gNameToColor[i].color.r;
            *g = gNameToColor[i].color.g;
            *b = gNameToColor[i].color.b;
            return true;
        }
    }
    return false;
}

Color* Color::AllocFromString(csref strarg, string* errmsg, bool ignoreRangeErrors) {
    string str = TrimWhitespace(strarg);
    float a, b, c;
    RGBColor rgb;
    if (StrStartsWith(str, "RGB")) {
        str = CheckAndRemoveParens(str.substr(3), errmsg);
        if (str.empty()) return NULL;
        if (ParseColorComponents(str, &a, &b, &c, errmsg, ignoreRangeErrors))
            return new RGBColor(a,b,c);
        else
            return NULL;
    } else if (StrStartsWith(str, "HSV")) {
        str = CheckAndRemoveParens(str.substr(3), errmsg);
        if (str.empty()) return NULL;
        if (ParseColorComponents(str, &a, &b, &c, errmsg, ignoreRangeErrors))
            return new HSVColor(a,b,c);
        else
            return NULL;
    } else if (ParseNamedColor(str, &a, &b, &c)) {
        return new RGBColor(a,b,c);
    } 
    // Doesn't make sense to support this old syntax
    //else if (ParseColorComponents(str, &a, &b, &c, errmsg, ignoreRangeErrors))
    //    return new RGBColor(a,b,c);
    else {
        if (errmsg) *errmsg = "unknown color: \"" + str + "\"";
        return NULL;
    }
}

// IMPORTANT!  This allocats a color object
template<>
  bool ParseParam<Color*>(Color** out, csref param, csref paramName, string* errmsg) {
    Color* color = Color::AllocFromString(param, errmsg);
    if (color) {
      *out = color;
      return true;
    } else {
      if (errmsg) *errmsg = "While parsing " + paramName + ", " + *errmsg;
      return false;
    } 
  }


//--------------------------------------------------------------------------
// RGBColor
//--------------------------------------------------------------------------
// Constructors
RGBColor::RGBColor(const Color& color) : Color() {
    color.ToRGBColor(this);
}

bool RGBColor::FromString(csref str, RGBColor* rgb, string* errmsg, bool ignoreRangeErrors) {
    Color* color = Color::AllocFromString(str, errmsg, ignoreRangeErrors);
    if (! color) return false;
    if (rgb) color->ToRGBColor(rgb);
    delete color;
    return true;
}

string RGBColor::ToString() const {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "RGB(%f,%f,%f)", r, g, b);
    return string(buffer);
}

// Operators
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

// Convert to 8 bit color values
unsigned char RGBColor::rAsChar(void) const {
  if (r < 0) return 0;
  else if (r >= 1.0) return 255;
  short ret = (r * 255.0) + 0.5;
  return ret;
}

unsigned char RGBColor::gAsChar(void) const {
  if (g < 0) return 0;
  else if (g >= 1.0) return 255;
  short ret = (g * 255.0) + 0.5;
  return ret;
}

unsigned char RGBColor::bAsChar(void) const {
  if (b < 0) return 0;
  else if (b >= 1.0) return 255;
  short ret = (b * 255.0) + 0.5;
  return ret;
}

//--------------------------------------------------------------------------
// HSVColor
//--------------------------------------------------------------------------
// HSV conversion utilities

void HSVColor::ToRGBColor(RGBColor* rgb) const {
    // Normalize hsv to 0 to 1 range
    float hue = fmodf(h,1.0F);
    if (hue < 0) hue = 1.0 + hue;
    float sat = s;
    if (sat < 0) sat = 0;
    else if (sat > 1) sat = 1;
    float val = v;
    if (val < 0) val = 0;
    else if (val > 1) val = 1;

    // Compute values
    float hueScaled = hue * 6.0;
    int hueSector = hueScaled;
    float chroma = sat * val;
    float offchroma = chroma * fabsf(fmodf(hueScaled, 2.0F) - 1.0F);

    switch (hueSector)
    {
        case 0:
        case 6:
            rgb->r = v;
            rgb->g = v - offchroma;
            rgb->b = v - chroma;
            break;
        case 1:
            rgb->r = v - offchroma;
            rgb->g = v;
            rgb->b = v - chroma;
            break;
        case 2:
            rgb->r = v - chroma;
            rgb->g = v;
            rgb->b = v - offchroma;
            break;
        case 3:
            rgb->r = v - chroma;
            rgb->g = v - offchroma;
            rgb->b = v;
            break;
        case 4:
            rgb->r = v - offchroma;
            rgb->g = v - chroma;
            rgb->b = v;
            break;
        case 5:
            rgb->r = v;
            rgb->g = v - chroma;
            rgb->b = v - offchroma;
            break;
    }
 }

void HSVColor::SetFromRGB(const RGBColor& rgb)
{
    // Normalize RGB
    float red = rgb.r;
    if (red < 0.0f) red = 0.0;
    else if (red > 1.0f) red = 1.0;
    float green = rgb.g;
    if (green < 0.0f) green = 0.0;
    else if (green > 1.0f) green = 1.0;
    float blue = rgb.b;
    if (blue < 0.0f) blue = 0.0;
    else if (blue > 1.0f) blue = 1.0;

    // Value
    float cmax = fmaxf(red,fmaxf(green,blue));
    v = cmax;
    // Special case for black
    if (v == 0) {
        h = s = 0;
        return;
    }

    // Saturation
    float cmin = fminf(red,fminf(green,blue));
    float chroma = cmax - cmin;
    s = chroma/v;
    // Special case for gray/white
    if (s == 0) {
        h = 0;
        return;
    }

    // Hue
    float hue;
    if (red == cmax) {
        hue = 0 + (green - blue)  / chroma;
        if (hue < 0) hue += 6.0f;
    } else if (green == cmax) {
        hue = 2 + (blue  - red)   / chroma;
    } else {
        hue = 4 + (red   - green) / chroma;
    }
    h = hue / 6.0f;
}

// Constructors
HSVColor::HSVColor(const Color& color) : Color() {
    const HSVColor* hsv = dynamic_cast<const HSVColor*>(&color);
    if (hsv)
        *this = *hsv;
    else {
        RGBColor rgb;
        color.ToRGBColor(&rgb);
        SetFromRGB(rgb);
    }
}

bool HSVColor::FromString(csref str, HSVColor* hsv, string* errmsg, bool ignoreRangeErrors) {
    Color* color = Color::AllocFromString(str, errmsg, ignoreRangeErrors);
    if (! color) return false;
    if (hsv) {
        RGBColor rgb;
        color->ToRGBColor(&rgb);
        hsv->SetFromRGB(rgb);
    }
    delete color;
    return true;
}

string HSVColor::ToString() const {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "HSV(%.3f,%.3f,%.3f)", h, s, v);
    return string(buffer);
}

HSVColor   HSVColor::operator*(const float& m) const {
  HSVColor ret (*this);
  ret.v *= m;
  return ret;
}

HSVColor& HSVColor::operator*=(const float& m) {
  v *= m;
  return *this;
}

HSVColor HSVColor::operator/(const float& m) const {
  HSVColor ret (*this);
  ret.v /= m;
  return ret;
}

HSVColor& HSVColor::operator/=(const float& m) {
  v /= m;
  return *this;
}

//--------------------------------------------------------------------------
// HSVColorRange
//--------------------------------------------------------------------------

HSVColorRange::HSVColorRange(const Color& cc1, const Color& cc2) {
    c1 = HSVColor(cc1);
    c2 = HSVColor(cc2);
}

HSVColor HSVColorRange::GetColor(float index) const {
    if      (index < 0) index = 0.0;
    else if (index > 1) index = 1.0;
    HSVColor hsv;
    if (c1.h < c2.h)
        hsv.h = (c2.h - c1.h) * index + c1.h;
    else
        hsv.h = (1 + c1.h - c2.h) * index + c2.h;
    if (hsv.h > 1) hsv.h = hsv.h - 1;
    hsv.s = (c2.s - c1.s) * index + c1.s;
    hsv.v = (c2.v - c1.v) * index + c1.v;
    return hsv;
}

HSVColor HSVColorRange::GetRandomColor() const {
    return GetColor(RandomFloat(1.0));
}

//--------------------------------------------------------------------------
// Picking a random color
//--------------------------------------------------------------------------

L::RandomColorMode_t L::gRandomColorMode = L::kRandomColorDefault;
RGBColor L::gRandomColor1 = BLACK;
RGBColor L::gRandomColor2 = WHITE;

RGBColor RandomColor() {
    RGBColor rgb;
    HSVColor hsv;
    float temp;

    switch (L::gRandomColorMode) {
        case L::kRandomColorExact:
            return L::gRandomColor2;
        case L::kRandomColorRealStar:
            temp = RandomFloat(0, .2);
            hsv.h = temp < 0 ? temp + 1 : temp; // pick something in the red or blue spectrum
            hsv.s = RandomMin(2, .05, .4);
            hsv.v = RandomExponential(6, 1.0);
            return hsv.ToRGBColor();
        case L::kRandomColorStarry:
            temp = RandomFloat(.333);
            hsv.h = (temp > .1666) ? temp + .5 : temp; // pick something in the red or blue spectrum
            hsv.s = RandomMin(4, 0, .5);
            hsv.v = RandomFloat(1.0);
            return hsv.ToRGBColor();
        case L::kRandomColorChristmas:
            if (RandomInt(1)) // pick red or green
                temp = RandomNormalBounded(0, .01, -.025, .05);
            else
                temp = RandomNormalBounded(.33333, .01, .2533, .40);
            hsv.h = temp < 0 ? temp + 1 : temp;
            hsv.s = RandomMax(3, .9, 1);
            hsv.v = RandomExponential(2, 1.0);
            return hsv.ToRGBColor();
        case L::kRandomColorRGB:
            rgb.r = RandomMax(2);
            rgb.g = RandomMax(2);
            rgb.b = RandomMax(2);
            return rgb;
        case L::kRandomColorHalloween:
            rgb.r = RandomMax(2, 0.5, 1.0);
            rgb.g = RandomFloat (0.0, 0.4);
            rgb.b = RandomFloat (0.0, 0.1);
            return rgb;
        case L::kRandomColorRange:
            {
            HSVColorRange range(L::gRandomColor1, L::gRandomColor2);
            return range.GetRandomColor().ToRGBColor();
            }
        case L::kRandomColorBright:
        default:
            hsv.h = RandomFloat(1.0);
            hsv.s = RandomMax(2);
            hsv.v = RandomMax(3);
            return hsv.ToRGBColor();
    }
}

namespace L {
bool ParseColorMode(csref strarg, string* errmsg) {
    string str = TrimWhitespace(strarg);
    string localError;

    if (str.empty()) {
        if (errmsg) *errmsg = "Empty color parameter";
        return false;
    }

    Color* color = Color::AllocFromString(str, &localError);
    if (color) {
        L::gRandomColorMode = L::kRandomColorExact;
        color->ToRGBColor(&L::gRandomColor2);
        delete color;
        return true;
    }

    if (StrStartsWith(str,"range:"))
        {
        L::gRandomColorMode = L::kRandomColorRange;
        string cs1 = str.substr(6);
        size_t semipos = cs1.find(';');
        if (semipos == string::npos) {
            if (errmsg) *errmsg = "Missing semicolon in color range: " + str;
            return false;
        }
        string cs2 = cs1.substr(semipos+1);
        cs1 = cs1.substr(0,semipos);
        Color* color = Color::AllocFromString(cs1, &localError);
        if (color) {
            color->ToRGBColor(&L::gRandomColor1);
            delete color;
            color = Color::AllocFromString(cs2, &localError);
            if (color) {
                color->ToRGBColor(&L::gRandomColor2);
                delete color;
                return true;
                }
            }
        // If we get here the color parsing failed
        if (errmsg) *errmsg = "Error parsing color range: " + localError;
        return false;
        }

    if      (StrEQ(str, "RealStar"))            L::gRandomColorMode = L::kRandomColorRealStar;
    else if (StrEQ(str, "Starry"))              L::gRandomColorMode = L::kRandomColorStarry;
    else if (StrEQ(str, "RGB"))                 L::gRandomColorMode = L::kRandomColorRGB;
    else if (StrEQ(str, "Halloween"))           L::gRandomColorMode = L::kRandomColorHalloween;
    else if (StrEQ(str, "Bright"))              L::gRandomColorMode = L::kRandomColorBright;
    else if (StrEQ(str, "xmas") || StrEQ(str, "Christmas")) L::gRandomColorMode = L::kRandomColorChristmas;
    else {
        if (errmsg) {
            *errmsg = "Invalid color parameter: " + str + ". ";
            // Add in the other error if it's a type we recognize
            if (isdigit(str[0]) || str[0] == '.' || StrStartsWith(str,"rgb") || StrStartsWith(str,"hsv"))
                *errmsg += localError;
        }
        return false;
    }
    return true;
}

string CurrentColorModeAsString() {
    switch (gRandomColorMode) {
        case kRandomColorRealStar:      return "realstar";
        case kRandomColorStarry:        return "starry";
        case kRandomColorRGB:           return "RGB";
        case kRandomColorHalloween:     return "Halloween";
        case kRandomColorChristmas:     return "Christmas";
        case kRandomColorBright:        return "Bright";
        case kRandomColorRange:         return "Range";
        case kRandomColorExact:         return "Exact";
        default: return "UnknownMode";
        }
}

string ColorModeHelp() {
    if (Option::Exists("color"))
        return "colormode is one of: rgb,realstar,starry,halloween,christmas,bright,(R,G,B),hsv(H,S,V),range:C1,C2";
    else
        return "";
}

ProgramHelp PHColorHelp(ColorModeHelp);
};//namespace
