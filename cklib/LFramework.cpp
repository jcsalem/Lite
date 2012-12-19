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
    "  pdsinfo describes the PDS IP and fixture port in the format IP/port(count)\n"
    "    For example, 172.24.22.51/1  or  172.24.22.51/2r(50).  'r' means reverse the order of those lights\n"
    "    If no PDS devices are specified, then they are auto detected.\n"
    "  duration is the running time in seconds. By default, animation continues forever.\n"
    "  rateval is the relative speed of the effect. Default is 1.0\n"
    "  colorinfo is description of a random mode.\n"
    "  fadeduration is the time to fade in and out in seconds (default is 1.0)\n"
    "  outmap specifies if any special mapping is to be done."
    ;

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------
LBuffer*    gOutputBuffer   = NULL;
bool        gVerbose        = false;
float       gRunTime        = 0.0;
float       gRate           = 1.0;
float       gFade           = 0.0;  // Fade in/out time in seconds

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
// Parse command line arguments
//---------------------------------------------------------------

const char* PopArg(int* argc, char** argv, bool hasParameter = false)
// Argv points to the first element of the shifted array
{
    int amount = 1;
    const char* retval = NULL;
    if (hasParameter && *argc >= 2) {
        // Has a parameter (otherwise return NULL)
        retval = *(argv+1);
        amount = 2;
    }

    while (*(argv+amount))
    {
        *argv = *(argv + amount);
        ++argv;
    }
    *argv = NULL;
    *argc = *argc - amount;
    return retval;
}

bool StdOptionsParse(int* argc, char** argv, string* errmsg)
{
    bool foundPDS = false;
    const int kOutMapRandom = -1;
    int outputMapping = 0;

    // Initialize gOutputBuffer
    if (gOutputBuffer) {delete gOutputBuffer; gOutputBuffer = NULL;}
    CKbuffer* ckbuffer = new CKbuffer();
    gOutputBuffer = ckbuffer;

    while (*argv)
    {
        if (StrEQ(*argv, "--pds")) {
            const char* pds = PopArg(argc, argv, true);
            if (!pds) {
                if (errmsg) *errmsg = "Missing argument to --pds";
                return false;
            }
            ckbuffer->AddDevice(string(pds));
            foundPDS = true;
        } else if (StrEQ(*argv, "--verbose")) {
            gVerbose = true;
            PopArg(argc,argv,false);
        } else if (StrEQ(*argv, "--outmap")) {
            const char* cstr = PopArg(argc, argv, true);
            if      (StrEQ(cstr, "normal")) outputMapping = 0;
            else if (StrEQ(cstr, "random")) outputMapping = kOutMapRandom;
            else {
                if (errmsg) *errmsg = "--outmap given unknown option. Was " + string(cstr);
                return false;
            }

        } else if (StrEQ(*argv, "--color")) {
            const char* cstr = PopArg(argc, argv, true);
            if (! ParseColorMode(cstr, errmsg))
                return false;
        } else if (StrEQ(*argv, "--rate")) {
            const char* cstr = PopArg(argc, argv, true);
            gRate = StrToFlt(cstr);
            if (gRate <= 0) {
                if (errmsg) *errmsg = "--rate argument must be positive. Was " + string(cstr);
                return false;
            }
        } else if (StrEQ(*argv, "--fade")) {
            const char* cstr = PopArg(argc, argv, true);
            gFade = StrToFlt(cstr);
            if (gFade < 0) {
                if (errmsg) *errmsg = "--fade argument must be zero or greater. Was " + string(cstr);
                return false;
            }
        } else if (StrEQ(*argv, "--time")) {
            const char* tstr = PopArg(argc,argv,true);
            if (!tstr) {
                if (errmsg) *errmsg = "Missing argument to --time";
                return false;
            }
            gRunTime = atof(tstr);
            if (gRunTime <= 0) {
                if (errmsg) *errmsg = "--time argument must be positive. Was " + string(tstr);
                return false;
            }
        } else
            ++argv;
    }

    // If no --pds, try environment variable
    if (!foundPDS) {
        const char* envval = getenv("PDS");
        string devstr = TrimWhitespace(string(envval ? envval : ""));
        if (! devstr.empty()) {
            foundPDS = true;
            ckbuffer->AddDevice(devstr);
        }
    }

    // Poll for PDS if necessary
    if (!foundPDS) {
        string myerrmsg;
        string* errmsgarg = errmsg ? errmsg : &myerrmsg;
        vector<CKdevice> devices = CKpollForDevices(errmsgarg);
        for (size_t i = 0; i < devices.size(); ++i) {
            ckbuffer->AddDevice(devices[i]);
        }
        if (! errmsgarg->empty()) {
            if (errmsg) *errmsg = "Error finding CK devices: " + *errmsg;
            return false;
        }
    }

    if (ckbuffer->HasError()) {
        if (errmsg) *errmsg = ckbuffer->GetLastError();
        return false;
    }

    if (ckbuffer->GetCount() == 0) {
        if (errmsg) *errmsg = "Not output device specified and couldn't locate one on network.";
        return false;
    }

    if (outputMapping == kOutMapRandom)
        ckbuffer->RandomizeMap();

    if (gVerbose)
        cout << gOutputBuffer->GetDescription() << endl;

    return true;
}


void Startup()
{
    // Sanity testing
    if (! gOutputBuffer || gOutputBuffer->GetCount() == 0)
    {
        cerr << ProgramHelp::GetString(kPHprogram) + ": Missing or empty output display" << endl;
        cerr << ProgramHelp::GetUsage();
        exit(1);
    }

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

void Startup(int *argc, char** argv, int numPositionalArgs)
{
    // Parse the argument list
    Option::ParseArglist(argc, argv, numPositionalArgs);
    Startup();
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
