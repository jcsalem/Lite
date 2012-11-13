#include "utils.h"
#include "cklib.h"
#include <iostream>

namespace CK {
//---------------------------------------------------------------
// Usage doc
//---------------------------------------------------------------
const char* kStdOptionsArgs = " --pds pdsinfo1 [--pds pdsinfo2 ...] [--verbose] [--time duration]";
const char* kStdOptionsArgsDoc =
    "  pdsinfo describes the PDS IP and fixture port in the format IP/port(count)\n"
    "    For example, 172.24.22.51/1  or  172.24.22.51/2r(50).  'r' means reverse the order of those lights\n"
    "    If no PDS devices are specified, then they are auto detected.\n"
    "  duration is the running time in seconds. By default, animation continues forever."
    ;

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------
LBuffer*    gOutputBuffer   = NULL;
bool        gVerbose        = false;
float       gRunTime        = 0.0;

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

    // Initialize CK::gOutputBuffer
    if (CK::gOutputBuffer) {delete CK::gOutputBuffer; CK::gOutputBuffer = NULL;}
    CKbuffer* ckbuffer = new CKbuffer();
    CK::gOutputBuffer = ckbuffer;

    while (*argv)
    {
        if (strEQ(*argv, "--pds"))
        {
            const char* pds = PopArg(argc, argv, true);
            if (!pds) {
                if (errmsg) *errmsg = "Missing argument to --pds";
                return false;
            }
            ckbuffer->AddDevice(string(pds));
            foundPDS = true;
        } else if (strEQ(*argv, "--verbose")) {
            CK::gVerbose = true;
            PopArg(argc,argv,false);
        }  else if (strEQ(*argv, "--time")) {
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
    if (!foundPDS) {
        // Auto
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

    if (CK::gOutputBuffer->GetCount() == 0) {
        if (errmsg) *errmsg = "Not output device specified and couldn't locate one on network.";
        return false;
    }

    if (CK::gVerbose)
        cout << CK::gOutputBuffer->GetDescription() << endl;

    return ! CK::gOutputBuffer->HasError();
}

}; // namespace CK
