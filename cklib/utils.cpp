// Cross-Platform Utilities
// Author: Jim Salem Feb 2012
//

#include "utils.h"

#ifdef WIN32
#include "stdarg.h"
#include "windef.h"
#include "winbase.h"
#endif //WIN32

// String Functions
string IntToStr(int val)
    {
    char buffer[32]; // good enough for 64 bit ints
    itoa(val, buffer, 10);
    return string(buffer);
    }

string IntToHex(int val, bool noprefix)
    {
    char buffer[32]; // good enough for 64 bit ints
    char* bptr = buffer;
    if (! noprefix)
        {
        if (val < 0)
            {
            *bptr++ = '-';
            val = -val;
            }
        *bptr++ = '0';
        *bptr++ = 'x';
        }
    itoa(val, bptr, 16);
    return string(buffer);
    }

// Error functions
#ifdef WIN32
string ErrorCodeStringInternal(int err)
    {
    LPTSTR	lpMsgBuf = NULL;
	FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL );
    string message;
	if (! lpMsgBuf)
		{
		message = "(no error text)";
		}
	else
		{
		message = lpMsgBuf;
		LocalFree( lpMsgBuf );
		}
    return message;
    }
#else // WIN32
string ErrorCodeStringInternal(int err)
    {
	if (err < 0 || err >= sys_nerr)
		return "Unknown Error";
	else
		return sys_errlist[errno];
	}
#endif //WIN32

string ErrorCodeString(int err)
    {
	return ErrorCodeStringInternal(err) +" (Code=" + IntToHex(err) + ")";
    }
