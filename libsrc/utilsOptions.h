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
    Option() : iParser(NULL), iDefaulter(NULL) {}

    csref               GetName     () const {return iName;}
    csref               GetParam    () const {return iParam;}
    bool                HasParam    () const {return !iParam.empty();}
    csref               GetDoc      () const {return iDoc;}
    OptionParserFcn_t   GetParser   () const {return iParser;}
    OptionDefaultFcn_t  GetDefaulter() const {return iDefaulter;}

    static const int kVariable = -1;
    // Returns false if an error occurred and a help message was printed.
    // minPositionArgs is the minimum number of positional arguments after the options. Use kVariable if it can be any number of arguments..
    // maxPositionArgs is the maximum number of positional arguments after the options. Defaults to minPositionalArgs
    static void ParseArglist(int *argc, char** argv, int minPositionArgs = 0, int maxPositionalArgs = -1);
    static void AddOption(const Option& option);
    static bool ReplaceOption(const Option& option);  // Returns true if the option was new. False if it was replaced.
    static bool DeleteOption(csref name);
    static bool Exists(csref name);
private:
    string              iName; // Fully decorated name in lower case
    string              iParam;
    string              iDoc;
    OptionParserFcn_t   iParser;
    OptionDefaultFcn_t  iDefaulter;
};

typedef enum {kPHprogram=0,kPHusage=1,kPHadditionalArgs=2,kPHhelp=3,kPHpostHelp=4, _kPHlimit=5} HelpType_t; // Help limit should always be +1 of the last help type


class ProgramHelp {
public:
    ProgramHelp(HelpType_t helpType, csref help);
    typedef string (*PostHelpFcn_t)();
    ProgramHelp(PostHelpFcn_t fcn);  // Defines a function that can be used to generate
    static string GetString(HelpType_t helpType);
    static string GetUsage(bool showOptions = false);
};

#define DefOption(name,parseCallback,paramName,docString, defaultValueCallback) \
  Option gOption_ ## name(#name, parseCallback, paramName, docString, defaultValueCallback)

#define DefOptionBool(name,parseCallback,docString) DefOption(name,parseCallback,"",docString, NULL)

#define DefProgramHelp(helpKeyword, help) \
  ProgramHelp gProgramUsage ## helpKeyword(helpKeyword, help)

#endif //  UTILSOPTIONS_H_INCLUDED
