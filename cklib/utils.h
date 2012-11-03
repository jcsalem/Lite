// Utilities
//

#ifndef __UTILS_H
#define __UTILS_H

// Shared includes
#include <stdlib.h>
#include <string>
#include <errno.h>
using namespace std;

// Standard Data Types
typedef	unsigned	long	uint32;
typedef				long	int32;
typedef	unsigned	short	uint16;
typedef				short	int16;
typedef	unsigned	char	uint8;
typedef				char	int8;

typedef const string&       csref;

// String functions
inline bool IsWhitespace(char c) {return c == ' ' || c == '\n' || c == '\r' || c == '\t';}
string TrimWhitespace(csref s);

bool strEQ(csref a, csref b); // Caseless string compare
bool strStartsWith(csref str, csref matchString); // Caseless compare to beginning of string

// Numeric conversions
string IntToHex(int val, bool noprefix = false);
string IntToStr(int val);
inline int StrToInt(csref str);
bool StrToInt(csref str, int* i); // returns false on error
inline float StrToFloat(csref str);
bool StrToFloat(csref str, float* i); // returns false on error

// Error related
string ErrorCodeString          (int err = errno);

#endif // __UTILS_H
