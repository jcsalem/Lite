#include "utils.h"
#include "Color.h"
#include "LFramework.h"
#include "ComboBuffer.h"
#include "Lproc.h"
#include "utilsOptions.h"
#include "Lobj.h"
#include <iostream>

namespace L {

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------

// Global time variables
Milli_t     gTime;                   // Current time
Milli_t     gStartTime;              // Loop start time
Milli_t     gEndTime;                // Ending time (set to gTime to end prematurely)
bool        gTerminateNow;           // Set to exit asap

// Not externally accessible
Milli_t     gFrameDuration  = 40;    // duration of each frame of animation (in MS)
LprocList   gProcs;


// Force gOutputBuffer to be deleted at exit
struct UninitAtExit {
    UninitAtExit() {}
    ~UninitAtExit() {if (gOutputBuffer) {delete gOutputBuffer; gOutputBuffer = NULL;}}
};

UninitAtExit _gUninitAtExit;

//---------------------------------------------------------------
// Output Device Parsing
//---------------------------------------------------------------

LBuffer* gOutputBuffer   = NULL;
string gOutputDeviceArg;

LBuffer* CreateOutputBuffer(csref devstr, string* errmsg) {
    LBuffer* buffer = ComboBuffer::Create(devstr, errmsg);
    if (! buffer) return NULL;
    ComboBuffer* cbuffer = dynamic_cast<ComboBuffer*>(buffer);
    if (cbuffer) {// should always be true
        switch (cbuffer->GetNumBuffers())
        {
        case 0: //Should never happen?
            if (errmsg) *errmsg = "Missing device descriptor. Empty?";
            return NULL;
        case 1: // No combo, get rid of top-level combo
            buffer = cbuffer->PopLastBuffer();
            delete cbuffer;
        }
    }
    return buffer;
}

string DeviceCallback(csref name, csref val) {
    gOutputDeviceArg = TrimWhitespace(val);
    return "";
}

DefOption(dev, DeviceCallback, "deviceInfo", "specifies the output device, optionally prefaced by filterInfos.", NULL);

string GetDevHelp() {
    string r, tmp;
    r = "Environment variable LDEV is used if --dev is missing.\n"
        "\nfilterInfo is pipe-separated list of filter_type:parameters\n"
        "   Examples: random, skip2|flip\n"
        "The supported filter types are: \n  ";
    tmp = LBufferType::GetDocumentation(true);
    r += StrReplace(tmp, "\n", "\n  ") + "\n";

    r += "\ndeviceInfo is device_type:arguments, optionally preceded by filterInfos\n"
         "   Examples: ck:192.168.5.5(50), flip|console\n"
        "The supported device types are: \n  ";
    tmp = LBufferType::GetDocumentation(false);
    r += StrReplace(tmp, "\n", "\n  ") + "\n";
    return r;
}

ProgramHelp PHDevHelp(GetDevHelp);


//---------------------------------------------------------------
// Other command line options
//---------------------------------------------------------------

bool        gVerbose        = false;

string VerboseCallback(csref name, csref val) {
    gVerbose = true;
    return "";
    }
DefOptionBool(verbose, VerboseCallback, "enables verbose status messages");

//------------
string gGlobalFilters;

string FilterCallback(csref name, csref valarg) {
    string value = TrimWhitespace(valarg);
    if (value.empty()) return "";
    if (value[value.size()-1] != '|') value += "|";
    gGlobalFilters = gGlobalFilters + value;
    return "";
}

DefOption(filter, FilterCallback, "filterInfo", "adds post-rendering filters.", NULL);

//------------
string ColorDefaultCallback(csref name)
{
    return CurrentColorModeAsString();
}

string ColorCallback(csref name, csref val) {
    string errmsg;
    ParseColorMode(val, &errmsg);
    return errmsg;
}

DefOption(color, ColorCallback, "colormode", "chooses a color scheme.", ColorDefaultCallback);

//------------
float gRunTime        = -1.0; // any number below zero means run forever

string TimeCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gRunTime))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    if (gRunTime < 0)
        return "--" + name + " cannot be less than zero.";
    return "";
}

DefOption(time, TimeCallback, "duration", "specifies the running time in seconds. By default, animation continues forever.", NULL);

//------------
float gRate     = 1.0;
int gRateMode  = kRatePositive;

string RateDefaultCallback(csref name) {
    return FltToStr(gRate);
    }

string RateCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gRate))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    switch (gRateMode) {
        case kRatePositive:
            if (gRate > 0)  break;
            else return "--" + name + " must be positive.";
        case kRateNonZero:
            if (gRate != 0)  break;
            else return "--" + name + " must be non-zero.";
        case kRateAny: break;
    }
    return "";
}

void SetRateMode(RateMode_t mode) {
    gRateMode = mode;
}

DefOption(rate, RateCallback, "rateval", "specifies the relative speed of the effect.", RateDefaultCallback);

//------------

float       gFade           = 0.0;  // Fade in/out time in seconds
string FadeDefaultCallback(csref name) {
    return FltToStr(gFade);
    }

string FadeCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gFade))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    if (gFade < 0)
        return "--" + name + " cannot be less than zero.";
    return "";
}

DefOption(fade, FadeCallback, "fadeduration", "sets the fade in and out in time seconds.", FadeDefaultCallback);



//---------------------------------------------------------------
// Startup
//---------------------------------------------------------------

void ErrorExit(csref msg) {
    cerr << ProgramHelp::GetString(kPHprogram) << ": " << msg << endl;
    cerr << ProgramHelp::GetUsage();
    exit(EXIT_FAILURE);
    }

void Startup(int *argc, char** argv, int numPositionalArgs) {
    // Parse the argument list
    Option::ParseArglist(argc, argv, numPositionalArgs);

    // Create the OutputBuffer
    if (gOutputDeviceArg.empty()) {
        const char* envval = getenv("LDEV");
        if (envval && envval[0] != '\0') gOutputDeviceArg = TrimWhitespace(envval);
    }
    if (gOutputDeviceArg.empty())
        ErrorExit("No output device was specified via --dev or the LDEV environment variable.");

    string errmsg;
    string devstr = gOutputDeviceArg;
    if (!gGlobalFilters.empty())
        devstr = gGlobalFilters + "[" + gOutputDeviceArg + "]";
    gOutputBuffer = CreateOutputBuffer(devstr, &errmsg);
    if (!gOutputBuffer) ErrorExit("Error creating output buffer: " + errmsg);

    if (gOutputBuffer->HasError())
        ErrorExit(gOutputBuffer->GetLastError());

    if (gOutputBuffer->GetCount() == 0)
        ErrorExit("Empty output device.");

    if (gVerbose)
        cout << gOutputBuffer->GetDescription() << endl;

    // Set up time variables
    gStartTime  = gTime = Milliseconds();
}

void Cleanup(bool eraseAtEnd)
{
    if (eraseAtEnd)
    {
        // Clear the lights
        // Note!  This doesn't work reliably
        gOutputBuffer->Clear();
        gOutputBuffer->Update();
    }
}

void RunOnce(Lgroup& objGroup) {
    gTime = Milliseconds();
    gOutputBuffer->Clear();
    objGroup.RenderAll(gTime, gProcs, gOutputBuffer);
    gOutputBuffer->Update();
}

void Run(Lgroup& objGroup, L::ObjCallback_t objfcn, L::GroupCallback_t groupfcn)
{
    // Set up end time variables
    gEndTime = 0; // Runs forever
    if (gRunTime >= 0)
        gEndTime = gStartTime + (Milli_t) (gRunTime * 1000 + .5);

    // Handle fade effect (this should really be automated from a list of filters added during arg parsing
    if (gFade > 0) {
        gProcs.AddProc(LprocFadeIn(false,gStartTime, gStartTime + gFade * 1000));
        if (gEndTime)
            gProcs.AddProc(LprocFadeOut(false,gEndTime - gFade * 1000, gEndTime));
    }

    // If callback, install it
    if (objfcn) {
        LprocFcn proc(objfcn);
        gProcs.PrependProc(proc);
    }

    // Main loop
    while (true) {
        RunOnce(objGroup);
        if (groupfcn) groupfcn(&objGroup);

        // Exit if out of time, else delay until next frame
        if (gTerminateNow) break;
        Milli_t currentTime = Milliseconds();
        if (gEndTime != 0 && MilliLE(gEndTime, currentTime)) break;

        Milli_t elapsedSinceFrameStart = MilliDiff(currentTime, gTime);
        if (gFrameDuration > elapsedSinceFrameStart)
            SleepMilli(gFrameDuration - elapsedSinceFrameStart);
    }
}


}; // namespace L
