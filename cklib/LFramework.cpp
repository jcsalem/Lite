#include "utils.h"
#include "cklib.h"
#include "Color.h"
#include "LFramework.h"
#include "LFilter.h"
#include "utilsOptions.h"
#include "Lobj.h"
#include <iostream>

namespace L {
//---------------------------------------------------------------
// Usage doc
//---------------------------------------------------------------
const char* kStdOptionsArgs = " --pds pdsinfo1 [--pds pdsinfo2 ...] [--verbose] [--time duration] [--rate rateval] [--color colorinfo] [--fade fadeduration]  [--outmap random|normal]";
const char* kStdOptionsArgsDoc =
    "  duration is the running time in seconds. By default, animation continues forever.\n"
    "  rateval is the relative speed of the effect. Default is 1.0\n"
    "  colorinfo is description of a random mode.\n"
    "  fadeduration is the time to fade in and out in seconds (default is 1.0)\n"
    "  outmap specifies if any special mapping is to be done."
    ;

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------

// Global time variables
Milli_t     gTime;                   // Current time
Milli_t     gStartTime;              // Loop start time
Milli_t     gEndTime;                // Ending time (set to gTime to end prematurely)


// Not externally accessible
Milli_t     gFrameDuration  = 40;    // duration of each frame of animation (in MS)
LFilterList gFilters;

// Force gOutputBuffer to be deleted at exit
struct UninitAtExit {
    UninitAtExit() {}
    ~UninitAtExit() {if (gOutputBuffer) {delete gOutputBuffer; gOutputBuffer = NULL;}}
};

UninitAtExit _gUninitAtExit;

//---------------------------------------------------------------
// Output Device Parsing
//---------------------------------------------------------------

LBuffer*    gOutputBuffer   = NULL;

string InitializeOneDevice(csref devstr) {
    // TO DO: Add support for comma separated values and autock
    if (! StrStartsWith(devstr, "ck://"))
        return "Unknown output device: " + devstr + " (For a ColorKinetics PDS device start with CK://)";
    CKbuffer *ckbuffer = dynamic_cast<CKbuffer*>(gOutputBuffer);
    if (gOutputBuffer == NULL) gOutputBuffer = ckbuffer = new CKbuffer();
    if (! ckbuffer->AddDevice(devstr.substr(5)))
        return ckbuffer->GetLastError();
    // All good
    return "";
    }

string InitializeOutputDevice(csref devstrarg) {
    string errmsg;
    string devstr = TrimWhitespace(devstrarg);
    if (StrEQ(devstr, "autock")) {
        vector<CKdevice> devices = CKpollForDevices(&errmsg);
        CKbuffer *ckbuffer = dynamic_cast<CKbuffer*>(gOutputBuffer);
        if (! gOutputBuffer) gOutputBuffer = ckbuffer = new CKbuffer();
        for (size_t i = 0; i < devices.size(); ++i)
            ckbuffer->AddDevice(devices[i]);
        if (! errmsg.empty())
            errmsg = "Error discovering CK devices: " + errmsg;
        }
    else {
        while (! devstr.empty()) {
            size_t commapos = devstr.find(',');
            string onedev = TrimWhitespace(devstr.substr(0, commapos));
            if (commapos == string::npos)
                devstr = "";
            else
                devstr = devstr.substr(commapos+1);
            errmsg = InitializeOneDevice(onedev);
            if (! errmsg.empty()) break;
            }
    }
    return errmsg;
}

string DeviceCallback(csref name, csref val) {
    return InitializeOutputDevice(val);
}

DefOption(dev, DeviceCallback, "deviceURLs", "is a comma separated list of URLs describing the output devices", NULL);

DefProgramHelp(kPHpostHelp, "If --dev is not specified, the environment variable LDEV is checked.\n"
                "A deviceURL begins with a protocol name.  Supported protocols and their URL format are:\n"
                "   ck://ipaddress/port(size) A ColorKinetics device. Port can be followed by 'r' to reverse the port mapping\n"
                "         For example, ck://172.24.22.51/1  or  ck://172.24.22.51/2r(50).\n"
                "   autock  Automatically configures all CK devices on the local area network."
                );

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
enum {kOutMapNormal = 0, kOutMapRandom = 1};
int gOutMap = kOutMapNormal;

string OutMapDefaultCallback(csref name) {
    if (gOutMap == kOutMapRandom) return "random";
    return "normal";
}

string OutMapCallback(csref name, csref val) {
    if      (StrEQ(val, "random")) gOutMap = kOutMapRandom;
    else if (StrEQ(val, "normal")) gOutMap = kOutMapNormal;
    else return "Invalid --outmap value: " + val + " (should be normal or random)";
    return "";
}

DefOption(outmap, OutMapCallback, "outmap", "Enables mapping the order of the output pixels to something else. Either normal or random.", OutMapDefaultCallback);

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

DefOption(color, ColorCallback, "colormode", "identifies the type of color. Options: bright, halloween, realstar, starry, etc.", ColorDefaultCallback);

//------------
float gRunTime        = 0.0;

string TimeCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gRunTime))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    if (gRunTime < 0)
        return "--" + name + " cannot be less than zero.";
    return "";
}

DefOption(time, TimeCallback, "duration", "is the running time in seconds. By default, animation continues forever.", NULL);

//------------
float gRate           = 1.0;
bool gAllowNegativeRate = false;

string RateDefaultCallback(csref name) {
    return FltToStr(gRate);
    }

string RateCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gRate))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    if (gRate <= 0 && !gAllowNegativeRate)
        return "--" + name + " must be positive.";
    return "";
}

void AllowNegativeRate() {
    gAllowNegativeRate = true;
}

DefOption(rate, RateCallback, "rateval", " is the relative speed of the effect.", RateDefaultCallback);

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

DefOption(fade, FadeCallback, "fadeduration", " is the time to fade in and out in seconds.", FadeDefaultCallback);



//---------------------------------------------------------------
// Startup
//---------------------------------------------------------------

void errorExit(csref msg) {
    cerr << ProgramHelp::GetString(kPHprogram) << ": " << msg << endl;
    cerr << ProgramHelp::GetUsage();
    exit(EXIT_FAILURE);
    }

void Startup(int *argc, char** argv, int numPositionalArgs) {
    // Parse the argument list
    Option::ParseArglist(argc, argv, numPositionalArgs);

    // If no --dev, try LDEV environment variable
    if (! gOutputBuffer) {
        const char* envval = getenv("LDEV");
        if (envval && envval[0] != '\0') {
            string errmsg = InitializeOutputDevice(envval);
            if (! errmsg.empty()) errorExit(errmsg);
        }
    }

    // Check for valid output buffer
    if (! gOutputBuffer)
        errorExit("No output device was specified via --dev or the LDEV environment variable.");

    if (gOutputBuffer->HasError())
        errorExit(gOutputBuffer->GetLastError());

    if (gOutputBuffer->GetCount() == 0)
        errorExit("Empty output device.");

    if (gVerbose)
        cout << gOutputBuffer->GetDescription() << endl;

    // Other random setup
    if (gOutMap == kOutMapRandom)
        gOutputBuffer->RandomizeMap();

    // Set up time variables
    gStartTime  = gTime = Milliseconds();
    gEndTime = 0; // Runs forever
    if (gRunTime > 0)
        gEndTime = gStartTime + (Milli_t) (gRunTime * 1000 + .5);

    // Handle fade effect (this should really be automated from a list of filters added during arg parsing
    if (gFade > 0) {
        gFilters.AddFilter(LFilterFadeIn(false,gStartTime, gStartTime + gFade * 1000));
        if (gEndTime)
            gFilters.AddFilter(LFilterFadeOut(false,gEndTime - gFade * 1000, gEndTime));
    }
}

void Cleanup(bool eraseAtEnd)
{
    if (eraseAtEnd)
    {
        // Clear the lights
        gOutputBuffer->Clear();
        gOutputBuffer->Update();
    }
}

void Run(Lgroup& objGroup, L::Callback_t fcn)
{
    while (true) {
        gTime = Milliseconds();

        // Call the callback
        fcn(objGroup);

        // Render
        gOutputBuffer->Clear();
        objGroup.RenderAll(gOutputBuffer, gFilters);
        gOutputBuffer->Update();

        // Exit if out of time, else delay until next frame
        Milli_t currentTime = Milliseconds();
        if (gEndTime != 0 && MilliLE(gEndTime, currentTime)) break;

        Milli_t elapsedSinceFrameStart = MilliDiff(currentTime, gTime);
        if (gFrameDuration > elapsedSinceFrameStart)
            SleepMilli(gFrameDuration - elapsedSinceFrameStart);
    }
}


}; // namespace CK
