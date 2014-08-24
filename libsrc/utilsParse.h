// Parsing Utilities
// These functions parse comma separated argument lists. 

#ifndef __UTILSPARSE_H
#define __UTILSPARSE_H
#include <utils.h>
#include <limits>
#include <vector>
typedef const vector<string>& cvsref;  // Convenience type for passing around parsed parameter lists

// Parses a comma separated list into a vector of strings.
// Whitespace is removed around each string.
// An empty string returns an empty vector. 
// Escaping:
//   \  escapes next character
//   () [] commas inside paratheses or brackets are ignored
//   '' escapes everything but \
//   "" escapes everything but \
// If parsing errors occur, an empty vector is returned and errmsg is set to an error message.
vector<string> ParseParamList(csref paramString, csref contextName = "", string* errmsg = NULL);

// Helper functions
//  Checks the number of parameters. 
bool ParamListCheck(cvsref paramList, csref contextName, string* errmsg, int minArgs, int maxArgs = -1); 
//  Creates a string with a comma-separated list of the parameters
string ParamListToString(cvsref params);
//  Sets errmsg to the standard error message. Always returns false.
bool ParamErrmsgSet(string* errmsg, csref contextName, csref msg);
bool ParamErrmsgSet(string* errmsg, csref contextName, csref msg, csref value);

// Parses a single parameter.
// Assumes the strings have been trimmed of whitespace (which ParseParamList does)
// Returns true on success or false on error.  An empty string is always an error
// out: value
// paramString: input
// paramName: the name of the parameter
// errmsg: error message output string (may be NULL)
// low_bound <= value < high_bound

template<typename T>
  bool ParseParam(T* out, csref paramString, csref paramName, string* errmsg = NULL);
template<typename T>
  bool ParseOptionalParam(T* out, csref paramString, csref paramName, string* errmsg = NULL);
// With bounds checking for numeric types 
template<typename T>  
  bool ParseParam(T* out, csref paramString, csref paramName, string* errmsg, double lowBound, double highBound = numeric_limits<T>::max());  // Used doubles rather than T because the implicit conversion for templates sucks
template<typename T>  
  bool ParseOptionalParam(T* out, csref paramString, csref paramName, string* errmsg, double lowBound, double highBound = numeric_limits<T>::max());  // Used doubles rather than T because the implicit conversion for templates sucks

#endif //  __UTILSPARSE_H
