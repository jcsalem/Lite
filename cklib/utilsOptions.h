// Defines options
//
#ifndef UTILSOPTIONS_H_INCLUDED
#define UTILSOPTIONS_H_INCLUDED

#include "utils.h"

class Option {
public:
    // This is the callback function type for parsing a specific option.
    // Name is the option and arg is the value (if it takes a parameter).
    // Returns an error message if an error occurs.
    typedef string (*OptionParserFcn_t) (csref name, csref arg);
    typedef string (*OptionDefaultFcn_t) (csref name);

    Option(csref name, OptionParserFcn_t parserFcn, csref paramName, csref docstring, OptionDefaultFcn_t defaultFcn);

    csref               GetName     () const {return iName;}
    csref               GetParam    () const {return iParam;}
    bool                HasParam    () const {return !iParam.empty();}
    csref               GetDoc      () const {return iDoc;}
    OptionParserFcn_t   GetParser   () const {return iParser;}
    OptionDefaultFcn_t  GetDefaulter() const {return iDefaulter;}

    static const int kVariable = -1;
    // Returns false if an error occurred and a help message was printed.
    // numPositionArgs is the number of positional arguments after the options. Use kVariable if it can be more than one number.
    static void ParseArglist(int *argc, char** argv, int numPositionArgs = kVariable);
    static bool DeleteOption(csref name);
private:
    string              iName; // Fully decorated name in lower case
    string              iParam;
    string              iDoc;
    OptionParserFcn_t   iParser;
    OptionDefaultFcn_t  iDefaulter;
};

typedef enum {kPHprogram=0,kPHusage=1,kPHadditionalArgs=2,kPHhelp=3,kPHpreHelp=4, _kPHlimit=5} HelpType_t; // Help limit should always be +1 of the last help type


class ProgramHelp {
public:
    ProgramHelp(HelpType_t helpType, csref help);
    static string GetString(HelpType_t helpType);
    static string GetUsage(bool showOptions = false);
};

#define DefOption(name,parseCallback,paramName,docString, defaultValueCallback) \
  Option gOption_ ## name(#name, parseCallback, paramName, docString, defaultValueCallback)

#define DefOptionBool(name,parseCallback,docString) DefOption(name,callback,"",docString, NULL)

#define DefProgramHelp(helpKeyword, help) \
  ProgramHelp gProgramUsage ## helpKeyword(helpKeyword, help)

#endif //  UTILSOPTIONS_H_INCLUDED
