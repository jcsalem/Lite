// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "LFilter.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

// Configuration
float       gDensity        = 0.8;  // Average density of stars

//----------------------------------------------------------------------
// Creating Sparkle
//----------------------------------------------------------------------

typedef enum {kSparkleError = -1, kSparkleDefault = 0, kSparkleSparkle = 1, kSparkleFirefly = 2, kSparkleSlow = 3} SparkleMode_t;
SparkleMode_t gSparkleMode = kSparkleSlow;

SparkleMode_t StrToSparkle(csref str) {
    if      (StrEQ(str, "default"))     return kSparkleDefault;
    else if (StrEQ(str, "sparkle"))     return kSparkleSparkle;
    else if (StrEQ(str, "firefly"))     return kSparkleFirefly;
    else if (StrEQ(str, "slow"))        return kSparkleSlow;
    else return kSparkleError;
}

Lsparkle RandomSparkle (Milli_t currentTime, bool isFirstTime) {
    Lsparkle si;

    switch (gSparkleMode) {
        case kSparkleSlow:
            si.attack   = RandomInt(1000/L::gRate);
            si.hold     = RandomMax(3, 10000, 100000)/L::gRate;
            si.release  = RandomInt(1500/L::gRate);
            si.sleep    = RandomNormalBounded(333, 250, 50, 10000);
            break;
        case kSparkleFirefly:
            si.attack   = RandomInt(100, 450);
            si.hold     = si.attack * 2 + RandomInt(si.attack) + RandomInt(si.attack);
            si.release  = RandomMin(2, 0, 800) + 400;
            si.sleep    = RandomMin(3, 750, 3000);
            break;
        case kSparkleSparkle:
        default:
            si.attack   = min(RandomInt(100), RandomInt(150));
            si.hold     = si.attack/2 + RandomInt(200);
            si.release  = 10;
            si.sleep    = 100;
            break;
    }

    if (isFirstTime) {
        si.startTime = currentTime - RandomInt(si.GetEndTime());
    } else {
        si.startTime = currentTime;
    }

    return si;
}

void InitializeOneStar(LobjSparkle* lobj, int idx, bool firstTime = false) {
    lobj->pos.x = (idx != -1) ? idx : RandomInt(L::gOutputBuffer->GetCount());
    lobj->color = (RandomFloat() < gDensity) ? RandomColor() : BLACK;
    lobj->sparkle = RandomSparkle(L::gTime, firstTime);
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
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << L::kStdOptionsArgs << " [--density densityval] [--sparkle sparklemode]" << endl;
    cerr << "Where:" << endl;
    cerr << L::kStdOptionsArgsDoc << endl;
    cerr << "  densityval - Density of sparkles (default is 0.8)" << endl;
    cerr << "  sparkemode is SlowSparkle, Sparkle, or Firefly (default is SlowSparkle)" << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"sparkle", required_argument,  0, 's'},
        {"density", required_argument,  0, 'd'},
        {0,0,0,0}
    };

void ParseArgs(const char* progname, int* argc, char** argv)
{
    // Parse stamdard options
    string errmsg;
    bool success = L::StdOptionsParse(argc, argv, &errmsg);
    if (! success)
        Usage(progname, errmsg);

    // Parse remaining
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (*argc, argv, "", longOpts, &optIndex);
        if (c == (char) -1) break; // Done parsing
        switch (c)
            {
            case 'h':
                Usage(progname);
            case 'd':
                gDensity = StrToFlt(optarg);
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

    // Change the defaults for this
    L::gRandomColorMode = L::kRandomColorRealStar;
    L::gFade = 1.0;

    // Parse arguments
    ParseArgs(progname, &argc, argv);

    // Parse command
    if (optind != argc)
    {
        cerr << "Too many arguments." << endl;
        Usage(progname);
    }

    L::Startup();
    InitializeStars();
    L::Run(gObjs, StarryCallback);
    L::Cleanup();
    exit(0);
}
