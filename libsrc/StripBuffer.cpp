// Implements an LBuffer variant using the SparkFun addressable LED Strip
//    This is based on the WS2801 chip and other WS2801 devices should similarly.
//  Only supported on Raspberry Pi (both v1 and v2)
// More doc here:
//   https://docs.google.com/document/d/1gNWRNP_ibS9p0BvzpeZA4dg787PQppPVYeafXvptNWk
//
// WS2801 Strip versions:
//   V3   NooElec (RGB order)
//   V2   http://www.sparkfun.com/products/10312 (RGB order)
//   V1   http://www.sparkfun.com/products/10312 (BGR order)

// Pin Assignments
//  Pin  V1orig  V1jim   V2     PlaneConn  OldCable?  
//  5v   Red     Blue    Red      Blue      Blue
//  CLK  Green   Green   Blue     Green     Red 
//  SDI  Red     Yellow  Green    Yellow    Green
//  Gnd  Blue    Red     Yellow   Red       Black

//  Pin   V3     V1jim   Cable  
//  5v   Red     Blue    Blue
//  SDI  Yellow  Green   Red
//  CLK  Green   Yellow  Green
//  Gnd  Blue    Red     Black

// GPIO Pins assignments
// Pin   RPi Pin  BCM GPIO#  WiringPi 
// ACLK   16         23        4
// ASDI   18         24        5
// BCLK   11         17        0
// BSDI   15         22        6
// +5v     4
// Gnd  6 or 15


// Two standard strips are supported and labeled with A and B.
// Strip A pins are list before the slash and strip B pins are listed after.
//

// URLish naming is: strip:[<stripInfo>][X][(size)]
//  Where:
//  stripInfo
//    <empty>      Same as "A"
//    A            SDI=18, CLK=16 size=32
//    B            SDI=15, CLK=11 size=32
//    <SDIpin>/<CLKpin>
//  size is the pixel count
//  If X, is present flip red and blue (needed because new strips have blue first while old strips had red first)
//

#include "StripBuffer.h"

// Dummy function to force this file to be linked in.
void ForceLinkStrip() {}

#ifdef HAS_GPIO
#include "utils.h"
#include "Color.h"
#include "utilsGPIO.h"
#include "utilsParse.h"

//-----------------------------------------------------------------------------
// Creation function
//-----------------------------------------------------------------------------

LBuffer* StripBufferCreate(cvsref params, string* errmsg) {
    if (! ParamListCheck(params, "LED strip", errmsg, 0, 1)) return NULL;
    string descStrArg, descStr;
    if (params.size() > 0) descStrArg = descStr = params[0];

    int size = 32;
    size_t leftPos = descStr.find('(');
    size_t rightPos = descStr.find(')');
    if (leftPos != string::npos || rightPos != string::npos) {
        if (leftPos == string::npos || rightPos == string::npos || rightPos < leftPos) {
            if (errmsg) *errmsg = "Mismatched parenthesis in strip buffer description.";
            return NULL;
        }
        // Looks reasonable, check that size is at end
        if (rightPos < descStr.length() - 2) {
            if (errmsg) *errmsg = "Size was not at end of strip buffer description.";
            return NULL;
        }
        if (!StrToUnsigned(descStr.substr(leftPos+1, rightPos-leftPos-1), (unsigned*) &size)) {
            if (errmsg) *errmsg = "Failed to parse size of strip buffer.";
            return NULL;
        }
        descStr = descStr.substr(0, leftPos) + descStr.substr(rightPos+1);
    }
    // Flip
    bool flip = false;

    if (! descStr.empty() && tolower(descStr[descStr.size()-1]) == 'x') {
        flip = true;
        descStr = descStr.substr(0, descStr.size()-1);
    }
    // Standard names
    descStr = TrimWhitespace(descStr);
    if (descStr.empty() || StrEQ(descStr, "A")) descStr = "24/23";
    else if (StrEQ(descStr, "B")) descStr = "22/17";

    // Now parse SDI and CLK
    size_t slashPos = descStr.find('/');
    if (slashPos == string::npos) {
        if (errmsg) *errmsg = "Unrecognized strip name or missing slash: " + descStrArg;
        return NULL;
    }

    int sdiGPIO, clkGPIO;
    if (!StrToUnsigned(descStr.substr(0, slashPos), (unsigned*) &sdiGPIO) || !StrToUnsigned(descStr.substr(slashPos+1),  (unsigned*) &clkGPIO)) {
        if (errmsg) *errmsg = "Couldn't parse strip SDI/CLK: " + descStr;
        return NULL;
    }
    if (sdiGPIO > GPIO::kMaxGPIO || clkGPIO > GPIO::kMaxGPIO) {
        if (errmsg) *errmsg = "Strip SDI/CLK was out of range: " + descStr;
        return NULL;
    }

    // Now see if we can initialize the GPIO system
    if (! GPIO::InitializeGPIO(errmsg))
        return NULL;
    GPIO::Write(sdiGPIO, 0);
    GPIO::SetModeOutput(sdiGPIO);
    GPIO::Write(clkGPIO, 0);
    GPIO::SetModeOutput(clkGPIO);

    StripBufferWS2801* buffer = new StripBufferWS2801(size, sdiGPIO, clkGPIO);
    buffer->SetCreateString("strip:"+descStrArg);
    buffer->SetColorFlip(flip);
    return buffer;
    }

DEFINE_LBUFFER_DEVICE_TYPE(strip, StripBufferCreate, "strip:stripInfo(size)",
        "Outputs to a WS2801-based LED strip. stripInfo is SDI/CLK or one of the aliases.\n"
        "  Aliases: A is 24/23; B is 22/17.  'strip' by itself is the same as 'strip:A'\n"
        "  Size defaults to 32.  Follow the description with 'X' to flip the color order (for older sparkfun strips)\n"
        "    Examples: strip:A, strip:22/17(32), strip:BX");

//-----------------------------------------------------------------------------
// WS2801 Specific Support
//-----------------------------------------------------------------------------

int PackRGB(const RGBColor& color, bool flip) {
    int r = min(255, max(0, (int) (color.r * 256)));
    int g = min(255, max(0, (int) (color.g * 256)));
    int b = min(255, max(0, (int) (color.b * 256)));

    if (flip)
        return r * 0x10000 + g * 0x100 + b;
    else
        return r + g * 0x100 + b * 0x10000;
}

Micro_t kMinTimeBetweenUpdates = 500; // In Microseconds. This is a requirement of the WS2801 chip

bool StripBufferWS2801::Update() {
    const_iterator bufBegin = const_cast<const StripBufferWS2801*>(this)->begin();
    const_iterator bufEnd   = const_cast<const StripBufferWS2801*>(this)->end();

    // Check if we need to sleep
    Micro_t timeSinceLast = MicroDiff(Microseconds(), iLastTime);
    if (timeSinceLast < kMinTimeBetweenUpdates)
      // Need to wait more time
      SleepMicro(kMinTimeBetweenUpdates - timeSinceLast);

    for (const_iterator i = bufBegin; i != bufEnd; ++i) {
        int colorword = PackRGB(*i, GetColorFlip());
        for (int j = 23; j >= 0; --j) {
            // Serialize this color. Clock in the data
            int mask = 1 << j;
            GPIO::Write(iCLKgpio, false);
            GPIO::Write(iSDIgpio, colorword & mask);
            GPIO::Write(iCLKgpio, true);
        }
    }
    // This marks the end of updates to this strip (note that the 500us delay betwen frames is done above)
    GPIO::Write(iCLKgpio, false);
    return true;
}

#endif // __arm__

