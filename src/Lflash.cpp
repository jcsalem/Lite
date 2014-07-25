// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include <iostream>
#include <stdio.h>

DefProgramHelp(kPHprogram, "Lflash");
DefProgramHelp(kPHusage, "Flashs all the lights");
DefProgramHelp(kPHadditionalArgs, "[color]");
DefProgramHelp(kPHhelp, "The color argument defauls to white.");

float gFlashSpeedFactor = 10;  // Default is 10 times per second

//----------------------------------------------------------------
// Option definitions
//----------------------------------------------------------------

float       gDuty        = 0.5;  // Average duty cycle of lights

string DutyCallback(csref name, csref val) {
    if (! StrToFlt(val, &gDuty))
        return "--duty wasn't a number: " + val;
    if (gDuty <= 0 || gDuty > 1)
        return "--duty argument must be between 0 and 1. Was " + val;
    return "";
}

string DutyDefaultCallback(csref name) {
    return FltToStr(gDuty);
}

DefOption(duty, DutyCallback, "duty", "What fraction of time the lights are on. Default is half the time.", DutyDefaultCallback);

enum {kFlash, kAccelerate, kSinusoidal} gFlashMode;

//----------------------------------------------------------------
// Initializing and running the lights
//----------------------------------------------------------------

Color* gColor;      // Always set
Color* gBlack = new BLACK;

Milli_t gStartTime;
Milli_t gPeriod;

void ObjCallback(Lobj* obj)
{
  Milli_t elapsed = TimeDiff(obj->nextTime, gStartTime);
  if (elapsed % gPeriod < gPeriod * gDuty)
    Lobj->color = gColor;
  else
    Lobj->color = gBlack;
}

Lobj* FlashAlloc(int idx, const void* ignore) 
    Lobj* obj = new Lobj(L::gTime);
    obj->x = idx;
    obj->color = gColor;
    return obj;
}

//----------------------------------------------------------------
// Main functions
//----------------------------------------------------------------

int main(int argc, char** argv)
{
    Option::DeleteOption("color");


    // Allocate objects
    Lgroup objects;
    objects.Add(L::gOutputBuffer->GetCount(), FlashAlloc, NULL);

    L::Startup(&argc, argv, 9, 1);
    gColor = new WHITE;
    if (argc > 0( 
      {
      }
    InitializeStars();
    L::Run(gObjs, StarryCallback);
    L::Cleanup();
    exit(EXIT_SUCCESS);
}
