#include "utils.h"
#include "Color.h"
#include "cklib.h"
#include "ckcli.h"
#include <iostream>
#include <getopt.h>

//----------------------------------------------------------------------------
// Option Parsing
//----------------------------------------------------------------------------

string CKcli::gPDSInfoDoc =
    "  pdsinfo describes the PDS IP and fixture port in the format IP/port\n"
    "    For example, 172.24.22.51@1  or  172.24.22.51@2r.  'r' means reverse the order of those lights\n";

bool CKcli::ParseOptionsCheckArg(const char* param)
{
    if (optarg) return true;
    iLastError = "Missing argument for --" + string(param) + ". Was -" + string(param) + " used instead?";
    return false;
}

bool CKcli::ParseOptions(const char opt, const char* progname, int argc, char** argv)
{
    switch (opt)
        {
        case '?':
            // Error message already issues
            iLastError = "Bad argument";
            break;
        case 'p':
            if (ParseOptionsCheckArg("pds"))
                Buffer.AddDevice(string(optarg));
            break;
        default:
            return false;
        }
    return true;
}
