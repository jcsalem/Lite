// Cross-Platform Utilities
// Author: Jim Salem Feb 2012
//

#include "utils.h"
#include <sstream>
#include <iomanip>
#include "limits.h"

#ifdef OS_WINDOWS
#include "Windows.h"
#endif //OS_WINDOWS

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

bool StrEQ(csref a, csref b)
{
    return strcasecmp(a.c_str(), b.c_str()) == 0;
}

bool StrStartsWith(csref str, csref matchString)
{
    if (matchString.empty()) return true;
    if (matchString.length() > str.length()) return false;
    return strncasecmp(str.c_str(), matchString.c_str(), matchString.length()) == 0;
}

string StrReplace(csref str, csref match, csref subst) {
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

//-------------------------------------------------------------------------
// String to number conversions
//-------------------------------------------------------------------------
bool StrToFlt(csref str, float* result) {
    errno = 0;
    float val = strtof(str.c_str(), NULL);
    if (result) *result = val;
    return errno == 0;
}

float StrToFlt(csref str) {
    float val;
    StrToFlt(str, &val);
    return val;
}

bool StrToInt(csref str, int* result) {
    errno = 0;
    long int val = strtol(str.c_str(), NULL, 0);
    if (val < INT_MIN) {val = INT_MIN; errno = ERANGE;}
    if (val > INT_MAX) {val = INT_MAX; errno = ERANGE;}
    if (result) *result = val;
    return errno == 0;
}

int StrToInt(csref str) {
    int val;
    StrToInt(str, &val);
    return val;
}

//-------------------------------------------------------------------------
// Numeric to String conversions
//-------------------------------------------------------------------------

string IntToStr(int val)
    {
#ifdef OS_WINDOWS
    char buffer[32]; // good enough for 64 bit ints
    itoa(val, buffer, 10);
    return string(buffer);
#else
    // Runs everywhere
    stringstream ss;
    ss << val;
    return ss.str();
#endif
    }

string IntToHex(int val, bool noprefix) {
#ifdef OS_WINDOWS
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
#else
    // Runs everywhere
    stringstream ss;
    if (noprefix) {
        if (val < 0) {ss << "-"; val = -val;}
        ss << "0x";
    }
    ss << hex << val;
    return ss.str();
#endif
}

string FltToStr(float val, int maxWidth) {
    return DblToStr(val, maxWidth);
}

string DblToStr(double val, int maxWidth) {
    ostringstream ss;
    if (maxWidth == 0)
        ss << val;
    else
        ss << setw(maxWidth) << val; // consider precision instead of setw
    return ss.str();
}

// Error functions
#ifdef OS_WINDOWS
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
#else
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

//-------------------------------------------------------------------------
// Windows specific stuff
//-------------------------------------------------------------------------
#ifdef OS_WINDOWS
// Returns a pointer to a library function or NULL if it couldn't be found
void* GetDLLFunctionAddress(csref fcnName, csref dllName, string *errmsg)
	{
	// Get a pointer to the function
	HMODULE module = GetModuleHandle(dllName.c_str());
	if (module == NULL)
		{
        // Try to load it
        module = LoadLibrary(dllName.c_str());
        if (module == NULL) {
            if (errmsg) *errmsg = "Couldn't locate " + dllName + " module: " + ErrorCodeString(GetLastError());
            return NULL;
            }
		}
	void* fcnPtr = (void*) GetProcAddress(module, fcnName.c_str());
	if (! fcnPtr)
		{
		if (errmsg) *errmsg = fcnName + " was not defined in " + dllName + " module: " + ErrorCodeString(GetLastError());
		return NULL;
		}
	else
		return fcnPtr;
	}
#endif
