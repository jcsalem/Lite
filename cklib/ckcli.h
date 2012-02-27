// Argument parsing utilities

#include "cklib.h"

#ifndef _CLI_H
#define _CLI_H


#define CKCLI_PDSINFO_OPTIONS  {"pds", required_argument,  0, 'p'}

class CKcli
{
public:
    static string gPDSInfoDoc;
    CKcli() {}
    // Returns true if that character was handled
    bool    ParseOptions(const char opt, const char* progname, int argc, char** argv);
    string  GetDescription()    const {return Buffer.GetDescription();}

    bool    HasError()          const {return !iLastError.empty() || Buffer.HasError();}
    string  GetLastError()      const {return iLastError.empty() ? Buffer.GetLastError() : iLastError;}

    CKbuffer Buffer;
private:
    string iLastError;
    bool ParseOptionsCheckArg(const char* param);
    // Prevent copying
    CKcli(const CKcli&);
    CKcli& operator=(const CKcli&);
};

#endif // _ARGPARSE_H
