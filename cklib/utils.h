// Utilities
//

#ifndef __UTILS_H
#define __UTILS_H

// Shared includes
#include <stdlib.h>
#include <string>
#include <errno.h>
#include "utilsPrecomp.h"
using namespace std;

// Standard Data Types
typedef	unsigned	int 	uint32;
typedef				int 	int32;
typedef	unsigned	short	uint16;
typedef				short	int16;
typedef	unsigned	char	uint8;
typedef				char	int8;

//Note long is 4 bytes on Windows, 8 bytes on Mac

typedef const string&       csref;

// String functions
inline bool IsWhitespace(char c) {return c == ' ' || c == '\n' || c == '\r' || c == '\t';}
string TrimWhitespace(csref s);
string StrToLower(csref s);
string StrToUpper(csref s);


bool StrEQ(csref a, csref b); // Caseless string compare
size_t StrSearch(csref str, csref matchString, int start = 0); // Caseless string search
bool StrStartsWith(csref str, csref matchString); // Caseless compare to beginning of string
// Replace all occurances of the match string with substitute
string StrReplace(csref str, csref match, csref substitute);

// Numeric conversions
string IntToHex(int val, bool noprefix = false);
string IntToStr(int val);
string FltToStr(float  val, int maxWidth = 0);
string DblToStr(double val, int maxWidth = 0);
int StrToInt(csref str);
bool StrToInt(csref str, int* i); // returns false on error
float StrToFlt(csref str);
bool StrToFlt(csref str, float* i); // returns false on error

// Error related
string ErrorCodeString          (int err = errno);

// Windows specific utilities
#ifdef OS_WINDOWS
// Returns a pointer to a function in a windows DLL or NULL if it couldn't be found
void* GetDLLFunctionAddress(csref fcnName, csref dllName, string *errmsg = NULL);
#endif

#endif // __UTILS_H
