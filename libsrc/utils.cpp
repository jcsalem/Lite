// Cross-Platform Utilities
// Author: Jim Salem Feb 2012
//

#include "utils.h"
#include <sstream>
#include <iomanip>
#include <limits.h>
//#include <strings.h>

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

    if (spos > epos) return "";
    return str.substr(spos, epos - spos + 1);
}

string StrToLower(csref s) {
    string r(s);

    for (size_t i = 0; i < s.length(); ++i)
        r[i] = tolower(s[i]);
    return r;
}

string StrToUpper(csref s) {
    string r(s);
    for (size_t i = 0; i < s.length(); ++i)
        r[i] = toupper(s[i]);
    return r;
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

size_t StrSearch(csref str, csref matchString, size_t start)
	{
	size_t	slen	= str.length();
	size_t	wlen	= matchString.length();
	if (wlen == 0) return 0;

	const char	w0u		= toupper(matchString[0]);
	const char	w0l		= tolower(matchString[0]);
	const char*	sptr	= str.c_str();
	const char*	endptr	= sptr + slen - wlen;
	sptr += start;

	while (sptr <= endptr)
		{
		char sc = *sptr++;

		if (sc == w0l || sc == w0u)
			// Possible match
			{
			if (StrStartsWith(sptr, matchString.c_str()+1))
				return sptr - 1 - str.c_str();
			}
		}
	return string::npos;
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
    long val = strtol(str.c_str(), NULL, 0);
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

bool StrToUnsigned(csref str, unsigned* result) {
    errno = 0;
    unsigned long val = strtoul(str.c_str(), NULL, 0);
    if (val > UINT_MAX) {val = UINT_MAX; errno = ERANGE;}
    if (result) *result = val;
    return errno == 0;
}

unsigned StrToUnsigned(csref str) {
    unsigned val;
    StrToUnsigned(str, &val);
    return val;
}

//-------------------------------------------------------------------------
// Numeric to String conversions
//-------------------------------------------------------------------------

string IntToStr(int val)
    {
    stringstream ss;
    ss << val;
    return ss.str();
    }

string IntToHex(int val, bool noprefix) {
    stringstream ss;
    if (! noprefix) {
        if (val < 0) {ss << "-"; val = -val;}
        ss << "0x";
    }
    ss << hex << val;
    return ss.str();
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
#include <errno.h>
#include <string.h>
// Mac and Linux
string ErrorCodeStringInternal(int err)
    {
    const int kbuflen = 255;
    char buf[kbuflen];
    buf[0] = 0;
    char* retval = buf;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600 || __DARWIN_C_LEVEL >= 200112L) && ! _GNU_SOURCE
    // XSI version
    if  (strerror_r(errno, buf, kbuflen) == -1)
        return "Unknown Error Code";
#else
    retval = strerror_r(errno, buf, kbuflen);
    if (retval == NULL || retval == (char*)-1)
        return "Unknown Error Code";
#endif
    if (retval[0])
        return retval;
    else
        return "Missing Error Code";
	}

#endif //WIN32

string ErrorCodeString(int err)
    {
	return ErrorCodeStringInternal(err) +" (Code=" + IntToHex(err) + ")";
    }

#include <iostream>
// Environment variables
string GetEnvStr(csref name)
	{
#ifdef _MSC_VER
	// This version works with the MSVS secure CRT library
	// Get size needed
	size_t len = 0;
	getenv_s(&len, NULL, 0, name.c_str());
	if (len == 0) return "";
	// Allocate buffer 
	char* buffer = new char[len];
	buffer[0] = '\0';
	if (getenv_s(&len, buffer, len, name.c_str()) != NO_ERROR) return "";
	return string(buffer);
#else 
	// Standard portable version
	char* buffer = getenv(name.c_str());
	if (!buffer) return "";
	return string(buffer);
#endif
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

// Workarounds for Microsoft secure library
#if defined(_MSC_VER)
int snprintf(char* buffer, size_t count, const char* format, ...) {
	va_list args;
	va_start(args, format);
	int r = _vsnprintf_s(buffer, count, count, format, args);
	va_end(args);
	return r;
}
#endif // _MSC_VER
