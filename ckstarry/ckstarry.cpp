// Top level Starry Night code

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
float       gRate           = 1.0;  // Rate of sparkle creation
float       gDensity        = 0.8;  // Average density of stars

//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------
Milli_t gTime;

//----------------------------------------------------------------------
// Creating Sparkle
//----------------------------------------------------------------------

typedef enum {kSparkleError = -1, kSparkleDefault = 0, kSparkleSparkle = 1, kSparkleFirefly = 2, kSparkleSlow = 3} SparkleMode_t;
SparkleMode_t gSparkleMode = kSparkleSlow;

SparkleMode_t StrToSparkle(csref str) {
    if      (strEQ(str, "default"))     return kSparkleDefault;
    else if (strEQ(str, "sparkle"))     return kSparkleSparkle;
    else if (strEQ(str, "firefly"))     return kSparkleFirefly;
    else if (strEQ(str, "slow"))        return kSparkleSlow;
    else return kSparkleError;
}

Lsparkle RandomSparkle () {
    Lsparkle si;
    si.startTime = gTime;

    switch (gSparkleMode) {
        case kSparkleSlow:
            si.attack   = 5000/gRate;
            si.hold     = RandomMax(3, 15000, 300000)/gRate;
            si.release  = RandomInt(100, 250);
            si.sleepTime= RandomNormalBounded(333, 250, 50, 10000);
            break;
        case kSparkleFirefly:
            si.attack   = RandomInt(100, 450);
            si.hold     = si.attack * 2 + RandomInt(si.attack) + RandomInt(si.attack);
            si.release  = RandomMin(2, 0, 800) + 400;
            si.sleepTime= RandomMin(3, 750, 3000);
            break;
        case kSparkleSparkle:
        default:
            si.attack   = min(RandomInt(100), RandomInt(150));
            si.hold     = si.attack/2 + RandomInt(200);
            si.release  = 10;
            si.sleepTime= 100;
            break;
    }

    return si;
}

void InitializeOneStar(LobjSparkle* lobj, int idx) {
    lobj->pos.x = (idx != -1) ? idx : RandomInt(CK::gOutputBuffer->GetCount());
    lobj->color = (RandomFloat() < gDensity) ? RandomColor() : BLACK;
    lobj->sparkle = RandomSparkle();
}

LobjBase* SparkleAlloc(int idx, const void* ignore) {
    LobjSparkle* lobj = new LobjSparkle();
    InitializeOneStar(lobj, idx);
    return lobj;
}

void RestartExpired(LobjBase* objarg, const void* ignore) {
    LobjSparkle* obj = dynamic_cast<LobjSparkle*>(objarg);
    if (! obj || !obj->IsOutOfTime(gTime)) return;
    InitializeOneStar(obj, obj->pos.x);
}

Lgroup gObjs;

void InitializeStars() {
    int numLights = CK::gOutputBuffer->GetCount();
    gObjs.Add(numLights, SparkleAlloc, NULL);
    }

void StarryLoop()
{
    Milli_t startTime = Milliseconds();
//    int lastSec = startTime / 1000;
    Milli_t endTimeMilli =  CK::gRunTime > 0 ? startTime + (Milli_t) (CK::gRunTime * 1000 + .5) : 0;

    while (true) {
        gTime = Milliseconds();

        // Restart expired stars
        gObjs.Map(RestartExpired, NULL);
//        FreeIf(HasNoSparkleLeft, NULL);
//        if (IsTimeToAlloc())
//            gObjs.Add(SparkleAlloc());

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
    cerr << "Usage: " << progname << CK::kStdOptionsArgs << " [--rate rateval] [--sparkle sparklemode]" << endl;
    cerr << "Where:" << endl;
    cerr << CK::kStdOptionsArgsDoc << endl;
    cerr << "  rateval - Rate of sparkle creation (default is 1.0)" << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"rate",    required_argument,  0, 'r'},
        {"sparkle", required_argument,  0, 's'},
        {"density", required_argument,  0, 'd'},
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
            case 'r':
                gRate = atof(optarg);
                if (gRate <= 0)
                    Usage(progname, "--rate argument must be positive. Was " + string(optarg));
                break;
            case 'd':
                gDensity = atof(optarg);
                if (gDensity <= 0 || gDensity > 1)
                    Usage(progname, "--density argument must be between 0 and 1. Was " + string(optarg));
                break;
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
    CK::gRandomColorMode = CK::kRandomColorRealStar;

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

    gTime = Milliseconds();
    InitializeStars();

    StarryLoop();
}
