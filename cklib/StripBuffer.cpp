// Implements an LBuffer variant using the SparkFun addressable LED Strip
//    http://www.sparkfun.com/products/10312
//  Only supported on Raspberry Pi
//
// Pin Assignments
//  Pin    OldStrips  NewStrips  Connector    RPi Pin   GPIO
//  5v      Red         Red        Blue        4
//  CLK     Green       Blue       Green      16/11      23/17
//  SDI     Red         Green      Yellow     18/15      24/22
//  Gnd     Blue        Yellow     Red        6 or 14
// This is compatible with both revision 1 and revision 2 boards

// On the new strips, the blue color comes first. On the old strips, red came first.

// URLish naming is: strip:[gpioInfo/size][?opt]],...
//  Where:
//  gpioInfo
//    <empty>      Same as "STD"
//    STD or STD1  SDI=18, CLK=16 size=32
//    STD2         SDI=15, CLK=11 size=32
//    <SDIpin>@<CLKpin>
//  size is the pixel count
//  opt is a collection of single character options
//    r means reverse the ordering of lights in this strip
//    c flip red and blue (needed because new strips have blue first while old strips had red first)
//

#include "utilsPrecomp.h"

// Dummy function to force this file to be linked in.
void ForceLinkStrip() {}

#ifndef OS_WINDOWS
#include "utils.h"
#include "StripBuffer.h"
#include "Color.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>

//-----------------------------------------------------------------------------
// Creation function
//-----------------------------------------------------------------------------

LBuffer* StripBufferCreate(csref descStr, string* errmsg) {
    size_t slashPos = descStr.find('/');
    size_t qPos = descStr.find('?');

    string gpio, countStr, opts;
    if (slashPos != string::npos) {
        if (qPos != string::npos) {
            if (qPos < slashPos) {
                *errmsg = "Couldn't parse strip description: " + descStr;
                return NULL;
            } else {
                gpio = descStr.substr(0, slashPos);
                countStr = descStr.substr(slashPos+1,qPos - slashPos - 1);
                opts = descStr.substr(qPos+1);
            }
        } else {
            gpio = descStr.substr(0, slashPos);
            countStr = descStr.substr(slashPos + 1);
        }
    } else {
        if (qPos != string::npos) {
            gpio = descStr.substr(0, qPos);
            opts = descStr.substr(qPos + 1);
        } else {
            gpio = descStr;
            }
        }

    gpio        = TrimWhitespace(gpio);
    countStr    = TrimWhitespace(countStr);
    opts        = TrimWhitespace(opts);

    int count = 0;
    if (! countStr.empty()) {
        if (! StrToInt(countStr, &count)) {
            *errmsg = "Couldn't parse size: " + descStr;
            return NULL;
        }
    }

    if      (StrEQ(gpio, "std") || StrEQ(gpio, "std1")) {gpio = "24@23"; if (count != 0) count = 32;}
    else if (StrEQ(gpio, "std2")) {gpio = "22@17"; if (count != 0) count = 32;}

    size_t aPos = gpio.find('@');
    int sdi, clk;
    if (aPos == string::npos || !StrToInt(gpio.substr(0, aPos), &sdi) || !StrToInt(gpio.substr(aPos+1), &clk) ||
        sdi <= 0 || sdi > 127 || clk <= 0 || clk > 127)
        *errmsg = "Couldn't parse gpio info. Format is SDI@CLK or one of std, std1, or std2.";
        return NULL;
    }

    Strip::Options_t options = Strip::kOptNone;
    if (StrSearch(opts, "r")) options |= Strip::kOptReverse;
    if (StrSearch(opts, "c")) options |= Strip::kOptFlipColor;

    if (count <= 0) {
        *errmsg = "Size must be specified and positive: " + descStr;
        return NULL;
    }

    StripBuffer* buffer = new StripBufferWS2801(count, sdi, clk);
    buffer->SetOptions(opts);
    buffer->SetCreateString(descString);
    return buffer;
    }

DEFINE_LBUFFER_TYPE(strip, StripBufferCreate, "strip:gpioInfo[/size[?options]]",
        "Outputs to a WS2801-based LED strip. gpioInfo is STD, STD2, or SDI@CLK. STD is 24@23/32. STD2 is 22@17/32\n"
        "Options: r is reverse, and c is flip colors (for old strips)\n"
        "  Examples: strip:std, strip:22@17/32?rc");

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

namespace GPIO {
void SetModeInput(int gpio) {
    // Clear the GPIO controls
    *(gGPIO+(gpio/10)) &= ~(7<<(((gpio)%10)*3))
}

void SetModeOutput(int gpio) {
    // Clear the GPIO controls
    *(gGPIO+(gpio/10)) &= ~(7<<((gpio%10)*3))
    // Now set output mode
    *(gGPIO+(gpio/10)) |=  (1<<((gpio%10)*3));
}

void Write(int gpio, bool value) {
    if (value)
        *(gGPIO +  7) = 1 << gpio;
    else
        *(gGPIO + 10) = 1 << gpio;
    }
};

// ARM GPIO access
const unsigned kGPIO_baseaddr = 0x20200000; // for BCM2708
volatile unsigned* gGPIO = NULL;

bool StripBufferWS2801::MaybeInitializeStrip() {
    static bool firstTime = true;
    if (firstTime) {
        int     mem_fd;
        void*   gpio_map;

        // Map the GPIO Controller address
        if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
            iLastError = "Failed to open /dev/mem. Run via 'sudo'");
            return false;
        }

       /* mmap GPIO */
       gpio_map = mmap(
          NULL,             //Any adddress in our space will do
          4 * 1024,         //Map length
          PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
          MAP_SHARED,       //Shared with other processes
          mem_fd,           //File to map
          kGPIO_baseaddr    //Offset to GPIO peripheral
       );

       close(mem_fd); //No need to keep mem_fd open after mmap

       if (gpio_map == MAP_FAILED) {
          iLastError("mmap error %d\n", (int)errno);//errno also set!
          return false;
       }

       // Always use volatile pointer!
       gGPIO = (volatile unsigned *)gpio_map;

       // Now setup SDI and CLK as outputs
       GPIO::SetModeOutput(iSDIgpio);
       GPIO::SetModeOutput(iCLKgpio);
       return true;
    }
}



bool StripBufferWS2801::Update() {
    MaybeInitializeStrip();
    return true;
}

#endif
