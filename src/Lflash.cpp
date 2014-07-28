// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "Lobj.h"
#include "LFramework.h"
#include <iostream>

DefProgramHelp(kPHprogram, "Lflash");
DefProgramHelp(kPHusage, "Flashs all the lights");
DefProgramHelp(kPHadditionalArgs, "[color]");
DefProgramHelp(kPHhelp, "The color argument defauls to white.");

float gFlashSpeedFactor = .25;  // Default is 4 times per second

//----------------------------------------------------------------
// Option definitions
//----------------------------------------------------------------

float       gDensity        = 0.25;  // Average duty cycle of lights

string DensityCallback(csref name, csref val) {
    if (! StrToFlt(val, &gDensity))
        return "--duty wasn't a number: " + val;
    if (gDensity <= 0 || gDensity > 1)
        return "--duty argument must be between 0 and 1. Was " + val;
    return "";
}

string DensityDefaultCallback(csref name) {
    return FltToStr(gDensity);
}

DefOption(density, DensityCallback, "density", "The fraction of time the lights are on from 0 to 1. Default is 0.25.", DensityDefaultCallback);

//enum {kFlash, kAccelerate, kSinusoidal} gFlashMode; 

//----------------------------------------------------------------
// Initializing and running the lights
//----------------------------------------------------------------

Color* gColor = new WHITE;      // Always set
Color* gBlack = new BLACK;

Milli_t gPeriod;
Milli_t gLastPeriodStart;
bool gFlashOn = true;

void FlashGroupCallback(Lgroup* ignore)
{
  Milli_t timediff = MilliDiff(L::gTime, gLastPeriodStart);
  if (L::gEndTime != 0 && MilliGE(L::gTime + L::gFrameDuration, L::gEndTime))
    // Off if this is the last frame
    gFlashOn = false;
  else if (timediff > gPeriod)
    {
      // On if we're starting a new cycle
      gFlashOn = true;
      gLastPeriodStart = L::gTime;
    }
  else if (timediff > gPeriod * gDensity)
    // Off if it's too late in the current cycle
    gFlashOn = false;
}

void FlashCallback(Lobj* obj)
{
  if (gFlashOn)
    obj->color = *gColor;
  else
    obj->color = *gBlack;
}

Lobj* FlashAlloc(int idx, const void* ignore) 
{
  Lobj* obj = new Lobj();
    obj->pos.x = idx;
    obj->color = *gColor;
    return obj;
}


//----------------------------------------------------------------
// Main functions
//----------------------------------------------------------------

int main(int argc, char** argv)
{
    Option::DeleteOption("color");

    // Parse arguments
    L::Startup(&argc, argv, 0, 1);
    if (argc > 1)
      {
	string errmsg;
	gColor = Color::AllocFromString(argv[1], &errmsg);
	if (! gColor) L::ErrorExit(errmsg);
      }

    gPeriod = gFlashSpeedFactor / L::gRate * 1000.0;

    // Allocate objects
    Lgroup objects;
    objects.Add(L::gOutputBuffer->GetCount(), FlashAlloc, NULL);

    // Perform
    L::Run(objects, FlashCallback, FlashGroupCallback);
    L::Cleanup();
    exit(EXIT_SUCCESS);
}
