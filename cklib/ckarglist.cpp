#include "utils.h"
#include "cklib.h"

const char* CKbuffer::kArglistArgs = " --pds pdsinfo1 [--pds pdsinfo2 ...]";
const char* CKbuffer::kArglistDoc =
    "  pdsinfo describes the PDS IP and fixture port in the format IP/port(count)\n"
    "    For example, 172.24.22.51/1  or  172.24.22.51/2r(50).  'r' means reverse the order of those lights\n";


static void ShiftArgv(char** argv, int amount)
// Argv points to the first element of the shifted array
{
    if (amount < 0) return;
    while (*(argv+amount))
    {
        *argv = *(argv+amount);
        ++argv;
    }
    *argv = NULL;
}

bool CKbuffer::CreateFromArglist(CKbuffer* buffer, int* argc, char** argv)
{
    while (*argv)
    {
        if (strEQ(*argv, "--pds"))
        {
            const char* pds = *(argv+1);
            if (!pds)
            {
                buffer->iLastError = "Missing argument to --pds";
                --argc;ShiftArgv(argv,1);
                return false;
            }
            buffer->AddDevice(string(pds));
            *argc = *argc - 2;
            ShiftArgv(argv,2);
        }
        else
            ++argv;
    }
    return ! buffer->HasError();
}
