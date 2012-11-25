#include "utils.h"
#include "Color.h"
#include "utilsRandom.h"
#include <string>
#include <math.h>
#include <stdio.h>

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

    if (str.find_first_not_of("0123456789.eE+-") != string::npos) {
        if (errmsg) *errmsg = "Invalid color number (must be valid floating point)";
        return false;
    }
    *fptr = atof(str.c_str());
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
    pos = str.find_first_of(" ,", startpos);
    if (pos == string::npos) {
        if (errmsg) *errmsg = "Color must be specified with 3 components.";
        return false;
    }
    if (! ParseOneComponent(str.substr(startpos, pos - startpos), b, errmsg, ignoreRangeErrors))
        return false;

    // Third component
    startpos = SkipToNextComponent(strarg, pos, errmsg);
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
char Color::rAsChar(void) const {
    RGBColor rgb;
    ToRGBColor(&rgb);
    return rgb.rAsChar();
}

char Color::gAsChar(void) const {
    RGBColor rgb;
    ToRGBColor(&rgb);
    return rgb.gAsChar();
}

char Color::bAsChar(void) const {
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
        if (strEQ(str, gNameToColor[i].name)) {
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
    if (strStartsWith(str, "RGB")) {
        str = CheckAndRemoveParens(str.substr(3), errmsg);
        if (str.empty()) return false;
        if (ParseColorComponents(str, &a, &b, &c, errmsg, ignoreRangeErrors))
            return new RGBColor(a,b,c);
        else
            return NULL;
    } else if (strStartsWith(str, "HSV")) {
        str = CheckAndRemoveParens(str.substr(3), errmsg);
        if (str.empty()) return false;
        if (ParseColorComponents(str, &a, &b, &c, errmsg, ignoreRangeErrors))
            return new HSVColor(a,b,c);
        else
            return NULL;
    } else if (ParseNamedColor(str, &a, &b, &c)) {
        return new RGBColor(a,b,c);
    } else if (ParseColorComponents(str, &a, &b, &c, errmsg, ignoreRangeErrors))
        return new RGBColor(a,b,c);
    else
        return NULL;
}

//--------------------------------------------------------------------------
// RGBColor
//--------------------------------------------------------------------------
// Constructors
RGBColor::RGBColor(const Color& color) : Color() {
    color.ToRGBColor(this);
}

bool RGBColor::FromString(csref str, RGBColor* rgb, string* errmsg, bool ignoreRangeErrors) {
    return ParseColorComponents(str, &(rgb->r), &(rgb->g), &(rgb->b), errmsg, ignoreRangeErrors);
}

string RGBColor::ToString() const {
    char buffer[50];
    sprintf(buffer, "RGB(%f,%f,%f)", r, g, b);
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
    return ParseColorComponents(str, &(hsv->h), &(hsv->s), &(hsv->v), errmsg, ignoreRangeErrors);
}

string HSVColor::ToString() const {
    char buffer[50];
    sprintf(buffer, "HSV(%.3f,%.3f,%.3f)", h, s, v);
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

CK::RandomColorMode_t CK::gRandomColorMode = CK::kRandomColorDefault;
RGBColor CK::gRandomColor1 = BLACK;
RGBColor CK::gRandomColor2 = WHITE;

RGBColor RandomColor() {
    RGBColor rgb;
    HSVColor hsv;
    float temp;
    switch (CK::gRandomColorMode) {
        case CK::kRandomColorExact:
            return CK::gRandomColor2;
        case CK::kRandomColorRealStar:
            temp = RandomFloat(.333);
            hsv.h = (temp > .1666) ? temp + .5 : temp; // pick something in the red or blue spectrum
            hsv.s = RandomMin(4, 0, .5);
            hsv.v = RandomExponential(1.0, 1.0);
            return hsv.ToRGBColor();;
        case CK::kRandomColorStarry:
            temp = RandomFloat(.333);
            hsv.h = (temp > .1666) ? temp + .5 : temp; // pick something in the red or blue spectrum
            hsv.s = RandomMin(4, 0, .5);
            hsv.v = RandomFloat(1.0);
            return hsv.ToRGBColor();;
        case CK::kRandomColorRGB:
            rgb.r = RandomMax(2);
            rgb.g = RandomMax(2);
            rgb.b = RandomMax(2);
            return rgb;
        case CK::kRandomColorHalloween:
            rgb.r = RandomMax(2, 0.5, 1.0);
            rgb.g = RandomFloat (0.0, 0.4);
            rgb.b = RandomFloat (0.0, 0.1);
            return rgb;
        case CK::kRandomColorBright:
        default:
            hsv.h = RandomFloat(1.0);
            hsv.s = RandomMax(2);
            hsv.v = RandomMax(3);
            return hsv.ToRGBColor();
    }
}

namespace CK {
bool ParseColorMode(csref str, string* errmsg) {
    RGBColor rgb;
    string localError;
    if (RGBColor::FromString(str, &rgb, &localError)) {
        CK::gRandomColorMode = CK::kRandomColorExact;
        CK::gRandomColor2 = rgb;
        return true;
    }
    if      (strEQ(str, "RealStar"))            CK::gRandomColorMode = CK::kRandomColorRealStar;
    else if (strEQ(str, "Starry"))              CK::gRandomColorMode = CK::kRandomColorStarry;
    else if (strEQ(str, "RGB"))                 CK::gRandomColorMode = CK::kRandomColorRGB;
    else if (strEQ(str, "Halloween"))           CK::gRandomColorMode = CK::kRandomColorHalloween;
    else if (strEQ(str, "Bright"))              CK::gRandomColorMode = CK::kRandomColorBright;
    else {
        if (errmsg) *errmsg = "Invalid color parameter: " + str + ". " + localError;
        return false;
    }
    return true;
}

};//namespace
