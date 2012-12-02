// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "stdOptions.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

// Configuration
Milli_t     gFrameDuration  = 40;    // duration of each frame of animation (in MS)

//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------
Milli_t gTime;

//----------------------------------------------------------------------
// Creating Sparkle
//----------------------------------------------------------------------

typedef enum {kSparkleError = -1, kSparkleDefault = 0, kSparkleSparkle = 1, kSparkleFirefly = 2} SparkleMode_t;
SparkleMode_t gSparkleMode = kSparkleDefault;

SparkleMode_t StrToSparkle(csref str) {
    if      (strEQ(str, "default"))     return kSparkleDefault;
    else if (strEQ(str, "sparkle"))     return kSparkleSparkle;
    else if (strEQ(str, "firefly"))     return kSparkleFirefly;
    else return kSparkleError;
}

Lsparkle RandomSparkle () {
    Lsparkle si;
    si.startTime = gTime;

    switch (gSparkleMode) {
        case kSparkleFirefly:
            si.attack = RandomInt(100, 450);
            si.hold = si.attack * 2 + RandomInt(si.attack) + RandomInt(si.attack);
            si.release = RandomMin(2, 0, 800) + 400;
            break;
        case kSparkleSparkle:
        default:
            si.attack = min(RandomInt(100), RandomInt(150));
            si.hold = si.attack/2 + RandomInt(200);
            si.release = 10;
            break;
    }

    return si;
}

LobjSparkle* SparkleAlloc(void) {
    LobjSparkle* lobj = new LobjSparkle();
    lobj->pos.x = RandomInt(CK::gOutputBuffer->GetCount());
    lobj->color = RandomColor();
    lobj->sparkle = RandomSparkle();
    return lobj;
}

bool HasNoSparkleLeft(LobjBase* objarg, const void* ignore) {
    const LobjSparkle* obj = dynamic_cast<const LobjSparkle*>(objarg);
    if (! obj) return false;
    return obj->IsOutOfTime(gTime);
}

bool IsTimeToAlloc() {
    static Milli_t lastTime = gTime;
    Milli_t millisSinceLast = MilliDiff(gTime,lastTime);
    lastTime = gTime;

    // Default probability is that every light has a 50% chance to flash every 10 seconds
    float lightProb = (millisSinceLast / 20000.0F);
    lightProb *= CK::gOutputBuffer->GetCount();
    lightProb *= CK::gRate;

    switch (gSparkleMode) {
        case kSparkleFirefly:
            // Slower for firefly
            lightProb /= 4;
            break;
        case kSparkleSparkle:
        default:
            break;
    }

    return lightProb > RandomFloat();
}

Lgroup gObjs;

void SparkleLoop()
{
    Milli_t startTime = Milliseconds();
//    int lastSec = startTime / 1000;
    Milli_t endTimeMilli =  CK::gRunTime > 0 ? startTime + (Milli_t) (CK::gRunTime * 1000 + .5) : 0;

    while (true) {
        gTime = Milliseconds();

        // Deallocate and Allocate
        gObjs.FreeIf(HasNoSparkleLeft, NULL);
        if (IsTimeToAlloc())
            gObjs.Add(SparkleAlloc());

        // Move (needed for time update
        gObjs.MoveAll(gTime);

        // Describe
//        if (CK::gVerbose && lastSec != int(gTime/1000)) {
//            lastSec = gTime/1000;
//            int sinceSec = lastSec - (startTime/1000);
//            cout << sinceSec << ": " << gObjs.GetDescription(CK::gVerbose) << endl;
//        }

        // Render
        CK::gOutputBuffer->Clear();
        gObjs.RenderAll(CK::gOutputBuffer);
        CK::gOutputBuffer->Update();

        // Exit if out of time, else delay until next frame
        Milli_t currentTime = Milliseconds();
        if (endTimeMilli != 0 && MilliLE(endTimeMilli, currentTime)) break;

        Milli_t elapsedSinceFrameStart = MilliDiff(currentTime, gTime);
        if (gFrameDuration > elapsedSinceFrameStart)
            SleepMilli(gFrameDuration - elapsedSinceFrameStart);
    }
}

//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << CK::kStdOptionsArgs << " [--sparkle sparklemode]" << endl;
    cerr << "Where:" << endl;
    cerr << CK::kStdOptionsArgsDoc << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"sparkle", required_argument,  0, 's'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int* argc, char** argv)
{
    // Parse stamdard options
    string errmsg;
    bool success = CK::StdOptionsParse(argc, argv, &errmsg);
    if (! success)
        Usage(progname, errmsg);

    // Parse remaining
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (*argc, argv, "", longOpts, &optIndex);
        if (c == -1) break; // Done parsing
        switch (c)
            {
            case 'h':
                Usage(progname);
            case 's':
                gSparkleMode = StrToSparkle(optarg);
                if (gSparkleMode == kSparkleError)
                    Usage(progname, "--sparkle must be either sparkle or firefly");
                break;
            default:
                cerr << "Internal error - unknown option: " << c << endl;
                Usage(progname);
            }
    }
}

int main(int argc, char** argv)
{
    const char* progname = "cksparkle";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

    // Change the default for this
    CK::gRandomColorMode = CK::kRandomColorStarry;

    // Parse arguments
    ParseArgs(progname, &argc, argv);

    // Parse command
    if (optind != argc)
    {
        cerr << "Too many arguments." << endl;
        Usage(progname);
    }

    // Test everything
    // TestLights();

    SparkleLoop();
}
