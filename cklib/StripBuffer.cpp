// Implements an LBuffer variant using the SparkFun addressable LED Strip
//    http://www.sparkfun.com/products/10312
//  Only supported on Raspberry Pi
//
// Pin Assignments
//  Pin    OldStrips  NewStrips  Connector   RPi Pin   BCM GPIO#  WiringPi
//  5v      Red         Red        Blue        4
//  CLK     Green       Blue       Green      16/11      23/17       4/0
//  SDI     Red         Green      Yellow     18/15      24/22       5/6
//  Gnd     Blue        Yellow     Red        6 or 14
// This is compatible with both revision 1 and revision 2 boards/
// Two standard strips are supported and labeled with A and B.
// Strip A pins are list before the slash and strip B pins are listed after.
//
// On the new strips, the blue color comes first. On the old strips, red came first.

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

#include "utilsPrecomp.h"

// Dummy function to force this file to be linked in.
void ForceLinkStrip() {}

#ifndef OS_WINDOWS
//#ifdef __arm__
#include "utils.h"
#include "StripBuffer.h"
#include "Color.h"
#include "utilsGPIO.h"

//-----------------------------------------------------------------------------
// Creation function
//-----------------------------------------------------------------------------

LBuffer* StripBufferCreate(csref descStrArg, string* errmsg) {
    string descStr = descStrArg;
    // Parse size if it exists
    unsigned size = 32;
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
        if (!StrToUnsigned(descStr.substr(leftPos+1, rightPos-leftPos-1), &size)) {
            if (errmsg) *errmsg = "Failed to parse size of strip buffer.";
            return NULL;
        }
        descStr = descStr.substr(0, leftPos) + descStr.substr(rightPos+1);
    }
    // Flip
    bool flip = false;

    if (! descStr.empty() && tolower(descStr[descStr.size()-1]) == 'x') {
        flip = true;
        descStr.substr(0, descStr.size()-1);
    }
    // Standard names
    descStr = TrimWhitespace(descStr);
    if (descStr.empty() || StrEQ(descStr, "A")) descStr = "24/23";
    else if (StrEQ(descStr, "B")) descStr = "22/17";)

    // Now parse SDI and CLK
    size_t slashPos = descStr.find('/');
    if (slashPos == string::npos) {
        if (errmsg) *errmsg = "Unrecognized strip name or missing slash: " + descStrArg;
        return NULL;
    }

    unsigned sdiGPIO, clkGPIO;
    if (!StrToUnsigned(descStr.substr(0, slashPos), &sdiGPIO) || !StrToUnsigned(descStr.substr(slashPos+1), &clkGPIO)) {
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

    StripBuffer* buffer = new StripBufferWS2801(count, sdiGPIO, clkGPIO);
    buffer->SetCreateString("strip:"+descStrArg);
    buffer->SetColorFlip(flip);
    return buffer;
    }

DEFINE_LBUFFER_TYPE(strip, StripBufferCreate, "strip:stripInfo(size)",
        "Outputs to a WS2801-based LED strip. stripInfo is SDI/CLK or one of the aliases.\n"
        "Aliases: A is 24/23; B is 22/17.  'strip' by itself is the same as 'strip:A'\n"
        "Size defaults to 32.  Follow the description with 'X' to flip the color order (for older sparkfun strips)\n"
        "  Examples: strip:A, strip:22/17(32), strip:BX");

//-----------------------------------------------------------------------------
// WS2801 Specific Support
//-----------------------------------------------------------------------------

int PackRGB(const RGBColor& color, bool flip) {
    int r = min(255, max(0, ((int) color.r * 256)));
    int g = min(255, max(0, ((int) color.g * 256)));
    int b = min(255, max(0, ((int) color.b * 256)));

    if (flip)
        return r * 0x10000 + g * 0x100 + b;
    else
        return r + g * 0x100 + b * 0x10000;
}

bool StripBufferWS2801::Update() {
    const_iterator bufBegin = const_cast<const StripBufferWS2801*>(this)->begin();
    const_iterator bufEnd   = const_cast<const StripBufferWS2801*>(this)->end();

    for (const_iterator i = bufBegin; i != bufEnd; ++i) {
        int colordata = PackRGB(*i, GetColorFlip());
        for (int j = 23; j >= 0; --j) {
            // Serialize this color. Clock in the data
            int mask = 1 << j;
            GPIO::Write(iCLKgpio, false);
            GPIO::Write(iSDIgpio, colorword & mask);
            GPIO::Write(iCLKgpio, true);
        }
    }
    // Now wait at least 500us to latch in the data
    GPIO::Write(iCLKgpio, false);
    SleepMilli(1);
}

#endif // __arm__

