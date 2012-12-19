// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include "LSparkle.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

LobjSparkle* SparkleAlloc(void) {
    LobjSparkle* lobj = new LobjSparkle();
    lobj->pos.x = RandomInt(L::gOutputBuffer->GetCount());
    lobj->color = RandomColor();
    lobj->sparkle = LSparkle::MakeRandomSparkle(L::gTime, L::gSparkleMode, L::gSparkleRate);
    return lobj;
}

bool HasNoSparkleLeft(LobjBase* objarg, const void* ignore) {
    const LobjSparkle* obj = dynamic_cast<const LobjSparkle*>(objarg);
    if (! obj) return false;
    return obj->IsOutOfTime();
}

bool IsTimeToAlloc() {
    static Milli_t lastTime = L::gTime;
    Milli_t millisSinceLast = MilliDiff(L::gTime,lastTime);
    lastTime = L::gTime;

    // Default probability is that every light has a 50% chance to flash every 10 seconds
    float lightProb = (millisSinceLast / 20000.0F);
    lightProb *= L::gOutputBuffer->GetCount();
    lightProb *= L::gRate;

    switch (L::gSparkleMode) {
        case LSparkle::kFirefly:
            // Slower for firefly
            lightProb /= 4;
            break;
        default:
            break;
    }

    return lightProb > RandomFloat();
}

void SparkleCallback(Lgroup& objgroup)
{
    // Deallocate and Allocate
    objgroup.FreeIf(HasNoSparkleLeft, NULL);
    if (IsTimeToAlloc())
        objgroup.Add(SparkleAlloc());

    // Move (needed for time update
    objgroup.MoveAll(L::gTime);
}

//----------------------------------------------------------------
// Main loop
//----------------------------------------------------------------

DefProgramHelp(kPHprogram, "cksparkle");
DefProgramHelp(kPHusage, "Displays a sparkly effect.");

int main(int argc, char** argv)
{
    // Change the default for this
    L::gRandomColorMode = L::kRandomColorStarry;
    Option::DeleteOption("rate");

    Lgroup objs;
    L::Startup(&argc, argv);
    L::Run(objs, SparkleCallback);
    L::Cleanup();
}
