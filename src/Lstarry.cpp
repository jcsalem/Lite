// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include "LSparkle.h"
#include <iostream>
#include <stdio.h>

DefProgramHelp(kPHprogram, "Lstarry");
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
    *lobj = LobjSparkle(L::gTime);
    lobj->pos.x = idx;
    lobj->color = (RandomFloat() < gDensity) ? RandomColor() : BLACK;
    lobj->sparkle = LSparkle::MakeRandomSparkle(L::gTime, L::gSparkleMode, L::gSparkleRate, firstTime);
}

const void* kIsFirstTime = (void*) -1;

Lobj* SparkleAlloc(int idx, const void* ignore) {
    LobjSparkle* lobj = new LobjSparkle(L::gTime);
    InitializeOneStar(lobj, idx, true);
    return lobj;
}

Lgroup gObjs;

void InitializeStars() {
    int numLights = L::gOutputBuffer->GetCount();
    gObjs.Add(numLights, SparkleAlloc, NULL);
    }

void StarryCallback(Lobj* objarg) {
    LobjSparkle* obj = dynamic_cast<LobjSparkle*>(objarg);
    if (obj->sparkle.IsOutOfTime(obj->nextTime))
        InitializeOneStar(obj, obj->pos.x);
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
    exit(EXIT_SUCCESS);
}
