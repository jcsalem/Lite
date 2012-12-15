// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

//----------------------------------------------------------------------
// Creating Sparkle
//----------------------------------------------------------------------

typedef enum {kSparkleError = -1, kSparkleDefault = 0, kSparkleSparkle = 1, kSparkleFirefly = 2} SparkleMode_t;
SparkleMode_t gSparkleMode = kSparkleDefault;

SparkleMode_t StrToSparkle(csref str) {
    if      (StrEQ(str, "default"))     return kSparkleDefault;
    else if (StrEQ(str, "sparkle"))     return kSparkleSparkle;
    else if (StrEQ(str, "firefly"))     return kSparkleFirefly;
    else return kSparkleError;
}

Lsparkle RandomSparkle () {
    Lsparkle si;
    si.startTime = L::gTime;

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
    lobj->pos.x = RandomInt(L::gOutputBuffer->GetCount());
    lobj->color = RandomColor();
    lobj->sparkle = RandomSparkle();
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
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << L::kStdOptionsArgs << " [--sparkle sparklemode]" << endl;
    cerr << "Where:" << endl;
    cerr << L::kStdOptionsArgsDoc << endl;
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
    L::gRandomColorMode = L::kRandomColorStarry;

    // Parse arguments
    ParseArgs(progname, &argc, argv);

    // Parse command
    if (optind != argc)
    {
        cerr << "Too many arguments." << endl;
        Usage(progname);
    }

    Lgroup objs;
    L::Startup();
    L::Run(objs, SparkleCallback);
    L::Cleanup();
}
