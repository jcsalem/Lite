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
string TrimWhitespace(csref str)
{
    int len = str.length();
    int spos, epos;
    for (spos = 0; spos < len; ++spos)
    {
        if (!IsWhitespace(str[spos])) break;
    }
    for (epos = len - 1; epos > spos; --epos)
        if (!IsWhitespace(str[epos])) break;

    if (spos >= epos) return "";
    return str.substr(spos, epos - spos + 1);
}

bool strEQ(csref a, csref b)
{
    return strcasecmp(a.c_str(), b.c_str()) == 0;
}

bool strStartsWith(csref str, csref matchString)
{
    if (matchString.empty()) return true;
    if (matchString.length() > str.length()) return false;
    return strncasecmp(str.c_str(), matchString.c_str(), matchString.length()) == 0;
}

string strReplace(csref str, csref match, csref subst) {
    string retval;
    size_t startpos = 0;
    size_t pos;
    while ((pos = str.find(match, startpos)) != string::npos) {
        retval += str.substr(startpos, pos - startpos) + subst;
        startpos = pos + match.length();
    }
    retval += str.substr(startpos);
    return retval;
}

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
// Mac and Linux
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
