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
string IntToHex(int val, bool noprefix = false);
string IntToStr(int val);
inline bool IsWhitespace(char c) {return c == ' ' || c == '\n' || c == '\r' || c == '\t';}

bool strEQ(csref a, csref b); // Caseless string compare

// Error related
string ErrorCodeString          (int err = errno);

#endif // __UTILS_H
