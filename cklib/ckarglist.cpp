#include "utils.h"
#include "cklib.h"
#include "color.h"
#include <iostream>

namespace CK {
//---------------------------------------------------------------
// Usage doc
//---------------------------------------------------------------
const char* kStdOptionsArgs = " --pds pdsinfo1 [--pds pdsinfo2 ...] [--verbose] [--time duration] [--rate rateval] [--color colorinfo]";
const char* kStdOptionsArgsDoc =
    "  pdsinfo describes the PDS IP and fixture port in the format IP/port(count)\n"
    "    For example, 172.24.22.51/1  or  172.24.22.51/2r(50).  'r' means reverse the order of those lights\n"
    "    If no PDS devices are specified, then they are auto detected.\n"
    "  duration is the running time in seconds. By default, animation continues forever.\n"
    "  rateval is the relative speed of the effect. Default is 1.0\n"
    "  colorinfo is description of a random mode."
    ;

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------
LBuffer*            gOutputBuffer   = NULL;
bool                gVerbose        = false;
float               gRunTime        = 0.0;
float               gRate           = 1.0;

// Force gOutputBuffer to be deleted at exit
struct UninitAtExit {
    UninitAtExit() {}
    ~UninitAtExit() {if (gOutputBuffer) {delete CK::gOutputBuffer; CK::gOutputBuffer = NULL;}}
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

    // Initialize CK::gOutputBuffer
    if (CK::gOutputBuffer) {delete CK::gOutputBuffer; CK::gOutputBuffer = NULL;}
    CKbuffer* ckbuffer = new CKbuffer();
    CK::gOutputBuffer = ckbuffer;

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
            CK::gVerbose = true;
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
            if (! CK::ParseColorMode(cstr, errmsg))
                return false;
        } else if (StrEQ(*argv, "--rate")) {
            const char* cstr = PopArg(argc, argv, true);
            CK::gRate = StrToFlt(cstr);
            if (CK::gRate <= 0) {
                if (errmsg) *errmsg = "--rate argument must be positive. Was " + string(cstr);
                return false;
            }
        } else if (StrEQ(*argv, "--time")) {
            const char* tstr = PopArg(argc,argv,true);
            if (!tstr) {
                if (errmsg) *errmsg = "Missing argument to --time";
                return false;
            }
            CK::gRunTime = atof(tstr);
            if (CK::gRunTime <= 0) {
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

    if (CK::gVerbose)
        cout << CK::gOutputBuffer->GetDescription() << endl;

    return true;
}

}; // namespace CK
