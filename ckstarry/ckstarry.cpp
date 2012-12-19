// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "LFilter.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include "LSparkle.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

DefProgramHelp(kPHprogram, "ckstarry");
DefProgramHelp(kPHusage, "Displays a starry night effect.");

//----------------------------------------------------------------
// Option definitions
//----------------------------------------------------------------
float       gDensity        = 0.8;  // Average density of stars

string DensityCallback(csref name, csref val) {
    if (! StrToFlt(val, &gDensity))
        return "--density wasn't a number: " + val;
    if (gDensity <= 0 || gDensity > 1)
        return "--density argument must be between 0 and 1. Was " + val;
    return "";
}

string DensityDefaultCallback(csref name) {
    return FltToStr(gDensity);
}

DefOption(density, DensityCallback, "density", "controls the density of stars.", DensityDefaultCallback);

//----------------------------------------------------------------
// Initialization
//----------------------------------------------------------------

void InitializeOneStar(LobjSparkle* lobj, int idx, bool firstTime = false) {
    lobj->pos.x = (idx != -1) ? idx : RandomInt(L::gOutputBuffer->GetCount());
    lobj->color = (RandomFloat() < gDensity) ? RandomColor() : BLACK;
    lobj->sparkle = LSparkle::MakeRandomSparkle(L::gTime, L::gSparkleMode, L::gSparkleRate, firstTime);
}

const void* kIsFirstTime = (void*) -1;

LobjBase* SparkleAlloc(int idx, const void* ignore) {
    LobjSparkle* lobj = new LobjSparkle();
    InitializeOneStar(lobj, idx, true);
    return lobj;
}

void RestartExpired(LobjBase* objarg, const void* ignore) {
    LobjSparkle* obj = dynamic_cast<LobjSparkle*>(objarg);
    if (! obj || !obj->IsOutOfTime()) return;
    InitializeOneStar(obj, obj->pos.x);
}

Lgroup gObjs;

void InitializeStars() {
    int numLights = L::gOutputBuffer->GetCount();
    gObjs.Add(numLights, SparkleAlloc, NULL);
    }


void StarryCallback(Lgroup& objGroup)
{
    // Restart expired stars
    objGroup.Map(RestartExpired, NULL);

    // Move (needed for time update
    objGroup.MoveAll(L::gTime);
}

//----------------------------------------------------------------
// Main functions
//----------------------------------------------------------------

int main(int argc, char** argv)
{
    // Change the defaults for this
    L::gRandomColorMode = L::kRandomColorRealStar;
    L::gFade = 1.0;
    L::gSparkleMode = LSparkle::kSlow;

    L::Startup(&argc, argv);
    InitializeStars();
    L::Run(gObjs, StarryCallback);
    L::Cleanup();
    return 0;
}
