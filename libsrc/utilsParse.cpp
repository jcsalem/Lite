// Parsing utilities

#include "utils.h"
#include "utilsParse.h"

//----------------------------------------------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------------------------------------------
bool ParamListCheck(cvsref params, csref bufferName, string* errmsg, int minArgs, int maxArgs)
{
    if (maxArgs == -1) maxArgs = minArgs;
    if (params.size() < minArgs) {
        if (errmsg) *errmsg = "Too few arguments to " + bufferName;
        return false;
    }
    if (params.size() > maxArgs) {
        if (errmsg) *errmsg = "Too many arguments to " + bufferName;
        return false;
    }
    return true;
}

bool ParamListHasValue(cvsref paramList, int index, csref paramName, string* errmsg)
{
    if (index < 0 || index >= paramList.size() || paramList[index].empty()) {
        if (errmsg) *errmsg = "Missing required argument: " + paramName;
        return false;
    } else
    return true;
}

string ParamListToString(cvsref params)
{
    string out;
    for (size_t i = 0; i < params.size(); ++i) {
        if (i != 0) out += ",";
        out += params[i];
    }
    return out;
}

bool ParamErrmsgSet(string* errmsg, csref contextName, csref msg)
{
    if (errmsg) *errmsg = "Error parsing " + contextName + ": " + msg;
    return false;
}

bool ParamErrmsgSet(string* errmsg, csref contextName, csref msg, csref value)
{
    ParamErrmsgSet(errmsg, contextName, msg);
    if (errmsg) *errmsg += " (value given was \"" + value + "\")";
    return false;
}


//----------------------------------------------------------------------------------------------------------------
// Parameter list parsing
//----------------------------------------------------------------------------------------------------------------

vector<string> PPLerror(string* errmsg, csref contextName, csref msg)
{
    ParamErrmsgSet(errmsg, contextName + " parameters", msg);
    return vector<string>();
}

// Returns the location of the matching end character.
// If an error occurs, returns string::npos
// curpos is the position of the character (either ' or ") that we want to skip to the next one of.
// slash is the only escape character
size_t PPLskipToNext(csref paramString, size_t curpos)
{
    char endChar = paramString[curpos];
    size_t endPos = paramString.length();
    for (size_t i = curpos + 1; i < endPos; ++i)
    {
        char c = paramString[i];
        if (c == endChar) return i;
        if (c == '\\') ++i;
    }
    return string::npos;
}

string CharName(char c, int num = 1)
{
    switch (c)
    {
        case '(':
        case ')':
            return num == 1 ? "parenthesis" : "parentheses";
        case '[':
        case ']':
            return PluralStr("square bracket", num);
        case '\'':
            return PluralStr("single quote", num);
        case '"':
            return PluralStr("double quote", num);
        default:
            return "unknown";
    }
}

char MatchingChar(char c)
{
    switch (c) {
        case '[': return ']'; case ']': return '[';
        case '(': return ')'; case ')': return '(';
        case '{': return '}'; case '}': return '{'; // not yet used
        default: return '\0'; //not used
    }
}

vector<string> ParseParamList(csref paramString, csref context, string* errmsg)
{
    // Special case zero length strings
    if (TrimWhitespace(paramString).empty()) {
        if (errmsg) *errmsg = "";
        return vector<string>();
    }
    vector<string> params;
    int startPos = 0;
    vector<char> groupStack;

    for (size_t i = 0; i < paramString.size(); ++i) {
        char c = paramString[i]; 
        switch (c)
        {
        case '[':
        case '(':
            groupStack.push_back(c);
            break;
        case ']':
        case ')':
            
            if (groupStack.empty())
                return PPLerror(errmsg, context, "Too many right " + CharName(c,2));
            else if (groupStack.back() != MatchingChar(c))
                return PPLerror(errmsg, context, "Mismatched " + CharName(groupStack.back(),2) + ", got right " + CharName(c) + " instead");
            else
                groupStack.pop_back();
            break;
        case ',':
            if (groupStack.empty()) {
                params.push_back(TrimWhitespace(paramString.substr(startPos, i - startPos)));
                startPos = i+1;
            }
            break;
        case '"':
        case '\'':
            i = PPLskipToNext(paramString, i);
            if (i == string::npos)
                return PPLerror(errmsg, context, "Mismatched " + CharName(c));
            break;
        }
    }

    if (!groupStack.empty()) {
        return PPLerror(errmsg, context, "Missing right " + CharName(groupStack.back()));
    }
    params.push_back(TrimWhitespace(paramString.substr(startPos, paramString.size() - startPos)));
    return params;
}

//----------------------------------------------------------------------------------------------------
// Parsing string parameters
//----------------------------------------------------------------------------------------------------

template<>
  bool ParseParam<string>(string* out, csref paramString, csref paramName, string* errmsg) {
    if (paramString.empty()) {
        *out = "";
        return true;
    }
    char endChar = paramString[0];

    if (endChar != '"' && endChar != '\'') {
        *out = paramString[0];
        return true;
    }
    // Handle quoted string
    *out = "";
    out->reserve(paramString.size() - 2);
    size_t pos = 1;
    for (; pos < paramString.size(); ++pos) {
        char c = paramString[pos];
        if (c == endChar) break;
        if (c == '\\') {
            ++pos;
            if (pos == paramString.size()) break;
        }
        out->push_back(paramString[pos]);
    }
    if (pos >= paramString.size()) return ParamErrmsgSet(errmsg, paramName, "Mismatched " + CharName(endChar)); // This should never happen with strins parsed using ParseParamList
    if (pos < paramString.size()-1) return ParamErrmsgSet(errmsg, paramName, "Unexpected characters after " + CharName(endChar));
    return true;
}

//----------------------------------------------------------------------------------------------------
// Parsing Numeric parameters
//----------------------------------------------------------------------------------------------------
// Generic definition for four argument version
template<typename T>
  bool ParseParam(T*  out, csref paramString, csref paramName, string* errmsg) {
    return ParseParam(out, paramString, paramName, errmsg, numeric_limits<T>::min(), numeric_limits<T>::max());
}
template<> bool ParseParam<float>(float*  out, csref paramString, csref paramName, string* errmsg);
template<> bool ParseParam<int>  (int*    out, csref paramString, csref paramName, string* errmsg);

// Float
template<>
  bool ParseParam<float>(float*  out, csref paramString, csref paramName, string* errmsg, double lowBound, double highBound) {
    if (! StrToFlt(paramString, out)) 
        return ParamErrmsgSet(errmsg, paramName, "Expected a number");
    if (*out < lowBound) 
        return ParamErrmsgSet(errmsg, paramName, "Must be greater than or equal to " + FltToStr(lowBound));
    if (*out >= highBound) 
        return ParamErrmsgSet(errmsg, paramName, "Must be less than " + FltToStr(highBound));
    return true;
}

// Int
template<>
  bool ParseParam<int>(int*    out, csref paramString, csref paramName, string* errmsg, double lowBound, double highBound) {
    if (! StrToInt(paramString, out)) 
        return ParamErrmsgSet(errmsg, paramName, "Expected a number");
    if (*out < lowBound) 
        return ParamErrmsgSet(errmsg, paramName, "Must be greater than or equal to " + IntToStr(lowBound));
    if (*out >= highBound) 
        return ParamErrmsgSet(errmsg, paramName, "Must be less than " + IntToStr(highBound));
    return true;
}
