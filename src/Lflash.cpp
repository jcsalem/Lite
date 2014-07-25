// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "Lobj.h"
#include "LFramework.h"

DefProgramHelp(kPHprogram, "Lflash");
DefProgramHelp(kPHusage, "Flashs all the lights");
DefProgramHelp(kPHadditionalArgs, "[color]");
DefProgramHelp(kPHhelp, "The color argument defauls to white.");

float gFlashSpeedFactor = .25;  // Default is 4 times per second

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

DefOption(duty, DutyCallback, "duty", "The fraction of time the lights are on from 0 to 1. Default is 0.5.", DutyDefaultCallback);

//enum {kFlash, kAccelerate, kSinusoidal} gFlashMode; 

//----------------------------------------------------------------
// Initializing and running the lights
//----------------------------------------------------------------

Color* gColor = new WHITE;      // Always set
Color* gBlack = new BLACK;

Milli_t gStartTime;
Milli_t gPeriod;

void FlashCallback(Lobj* obj)
{
  Milli_t elapsed = MilliDiff(obj->nextTime, gStartTime);
  if (elapsed % gPeriod < gPeriod * gDuty)
    obj->color = *gColor;
  else
    obj->color = *gBlack;
}

Lobj* FlashAlloc(int idx, const void* ignore) 
{
    Lobj* obj = new Lobj(L::gTime);
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

    gStartTime = L::gTime;
    gPeriod = (gFlashSpeedFactor / L::gRate) * 1000;

    // Allocate objects
    Lgroup objects;
    objects.Add(L::gOutputBuffer->GetCount(), FlashAlloc, NULL);

    // Perform
    L::Run(objects, FlashCallback);
    L::Cleanup();
    exit(EXIT_SUCCESS);
}
