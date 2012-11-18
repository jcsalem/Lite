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
#include <math.h>

// Configuration
Milli_t     gFrameDuration  = 40;    // duration of each frame of animation (in MS)
float       gRate           = 1.0;  // Rate of sparkle creation

//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << CK::kStdOptionsArgs << " [--rate rateval]" << endl;
    cerr << "Where:" << endl;
    cerr << CK::kStdOptionsArgsDoc << endl;
    cerr << "  rateval - Rate of sparkle creation (default is 1.0)" << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"rate",    required_argument,  0, 'r'},
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
            default:
                cerr << "Internal error - unknown option: " << c << endl;
                Usage(progname);
            }
    }
}

//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------
Milli_t gTime;

float RandomMax(int num, float mmin = 0.0, float mmax = 1.0) {
    float retval = RandomFloat(mmin, mmax);
    for (int i = 1; i < num; ++i)
        retval = max(RandomFloat(mmin, mmax), retval);
    return retval;
}

float RandomMin(int num, float mmin = 0.0, float mmax = 1.0) {
    float retval = RandomFloat(mmin, mmax);
    for (int i = 1; i < num; ++i)
        retval = min(RandomFloat(mmin, mmax), retval);
    return retval;
}

float RandomBell(float bnum, float mmin = 0.0, float mmax = 1.0) {
    int num = bnum;
    float retval = 0.0;
    for (int i = 0; i < num; ++i)
        retval += RandomFloat(mmin, mmax);
    if (bnum != num)
        retval += (bnum - num) * RandomFloat(mmin, mmax);
    return retval / bnum;
}

typedef enum {kCrngDefault = 0, kCrngBrightHSV = 1, kCrngRandomRGB = 2, kCrngHalloween = 3, kCrngStarry = 4} ColorRNG_t;
ColorRNG_t gColorRNGmode = kCrngStarry;

RGBColor RandomColor(void) {
    RGBColor rgb;
    HSVColor hsv;
    float temp;
    switch (gColorRNGmode) {
        case kCrngStarry:
            temp = RandomFloat(.333);
            hsv.h = (temp > .1666) ? temp + .5 : temp; // pick something in the red or blue spectrum
            hsv.s = RandomMin(4, 0, .5);
            hsv.v = RandomFloat(1.0);
            return hsv.ToRGBColor();;
        case kCrngRandomRGB:
            rgb.r = RandomMax(2);
            rgb.g = RandomMax(2);
            rgb.b = RandomMax(2);
            return rgb;
        case kCrngHalloween:
            rgb.r = RandomMax(2, 0.5, 1.0);
            rgb.g = RandomFloat (0.0, 0.4);
            rgb.b = RandomFloat (0.0, 0.1);
            return rgb;
        case kCrngBrightHSV:
        default:
            hsv.h = RandomFloat(1.0);
            hsv.s = RandomMax(2);
            hsv.v = RandomMax(3);
            return hsv.ToRGBColor();
    }
}

//----------------------------------------------------------------------
// Creating Sparkle
//----------------------------------------------------------------------

typedef enum {kSparkleDefault = 0, kSparkleSparkle = 1} SparkleRNG_t;
SparkleRNG_t gSparkleRNGmode = kSparkleSparkle;

Lsparkle RandomSparkle () {
    Lsparkle si;
    si.attack = min(RandomInt(100), RandomInt(150));
    si.hold = si.attack/2 + RandomInt(200);
    si.release = 10;
    si.startTime = gTime;
    return si;
}

LobjSparkle* SparkleAlloc(void) {
    LobjSparkle* lobj = new LobjSparkle();
    lobj->pos.x = RandomInt(CK::gOutputBuffer->GetCount());
    lobj->color = RandomColor();
    lobj->sparkle = RandomSparkle();
    return lobj;
}

//----------------------------------------------------------------------
// Sparkle color computation
//----------------------------------------------------------------------

RGBColor Lsparkle::ComputeColor(const RGBColor& referenceColor, Milli_t currentTime) const {
//    cout << "Color: " << referenceColor.ToString() << " start: " << startTime << " currentTime: " << currentTime << "  attack: " << attack << "  hold: " << hold << endl;

    if (MilliGE(startTime, currentTime))
        return BLACK;
    else if (MilliGT(startTime + attack, currentTime))
        return referenceColor * ((float)(currentTime - startTime) / attack);
    else if (MilliGE(startTime + attack + hold, currentTime))
        return referenceColor;
    else if (MilliGT(startTime + attack + hold + release, currentTime))
        return referenceColor * ((float) (release - (currentTime - startTime - attack - hold)) / release);
    else
        return BLACK;
}

bool HasNoSparkleLeft(LobjBase* objarg, const void* ignore) {
    const LobjSparkle* obj = dynamic_cast<const LobjSparkle*>(objarg);
    if (! obj) return false;

    return MilliLT(obj->sparkle.startTime + obj->sparkle.attack + obj->sparkle.hold + obj->sparkle.release, gTime);
}

bool IsTimeToAlloc() {
    return (gRate * CK::gOutputBuffer->GetCount() / 25.0) > RandomFloat(10);
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

int main(int argc, char** argv)
{
    const char* progname = "cksparkle";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

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
