// Defines options
//
#ifndef UTILSOPTIONS_H_INCLUDED
#define UTILSOPTIONS_H_INCLUDED

#include "utils.h"

class Option {
public:
    // This is the callback function far parsoing a specific option.  Name is the option and arg is the value (if it takes a parameter)
    typedef string (*OptionParser_t) (csref name, csref arg);
    Option(csref name, OptionParser_t parser, csref paramName, csref docstring);

    bool HasParam() const {return !iParam.empty();}

    static bool ParseArglist(int *argc, char** argv, string* errmsg);
private:
    string          iName; // Fully decorated name in lower case
    string          iParam;
    string          iDoc;
    OptionParser_t  iParser;
};

class ProgramUsage {
public:
    ProgramUsage(csref usage, csref help);
};

#define DefOption(name,callback,paramName,docString) \
  Option gOption_ ## name(#name, callback, paramName, docString)

#define DefProgramHelp(oneLineUsageString, additionalHelp) \
  ProgramUsage gProgramUsage(oneLineUsageString, additionalHelp)

#endif //  UTILSOPTIONS_H_INCLUDED
