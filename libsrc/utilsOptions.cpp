// Option parsing code
//
// Note: Global variables inside this file are declared as statics inside functions to insure
// they are initialized irrespective of the load order of the callers.

#include "utilsOptions.h"
#include <vector>
#include <iostream>

//---------------------------------------------------------------------
// OptionList (a local class for storing all the options
//---------------------------------------------------------------------
class OptionList { // local to this file
public:
    OptionList() {}
    static void AddOption(const Option& option) {GetOptions().push_back(option);}
    static bool DeleteOption(csref name);
    static bool Exists(csref name);

    typedef vector<Option>::const_iterator const_iterator;
    static const_iterator begin() {return GetOptions().begin();}
    static const_iterator end()   {return GetOptions().end();}


private:
    static vector<Option>& GetOptions();
};

// Returns a reference to a global list of all options
vector<Option>& OptionList::GetOptions() {
    static vector<Option> allOptions;
    return allOptions;
    }

bool OptionList::DeleteOption(csref namearg) {
    string name = StrToLower(namearg);
    for (vector<Option>::iterator i = GetOptions().begin(); i != GetOptions().end(); ++i) {
        if (i->GetName() == name) {
            GetOptions().erase(i);
            return true;
        }
    }
    return false;
}

bool OptionList::Exists(csref namearg) {
    string name = StrToLower(namearg);
    for (vector<Option>::iterator i = GetOptions().begin(); i != GetOptions().end(); ++i) {
        if (i->GetName() == name) return true;
        }
    return false;
}

//---------------------------------------------------------------------
// Option class
//---------------------------------------------------------------------
Option::Option(csref name, OptionParserFcn_t parserCallback, csref paramName, csref docstring, OptionDefaultFcn_t defaultCallback)
{
    iName       = StrToLower(name); // Make lower case to simplify arg parsing
    iParam      = paramName;
    iDoc        = docstring;
    iParser     = parserCallback;
    iDefaulter  = defaultCallback;
    OptionList::AddOption(*this);
}

bool Option::DeleteOption(csref name) {
    return OptionList::DeleteOption(name);
}

bool Option::Exists(csref name) {
    return OptionList::Exists(name);
}

//---------------------------------------------------------------------
// Program Help support
//---------------------------------------------------------------------

vector<string>& getProgramHelp() {
    static vector<string> helpVector(_kPHlimit);
    return helpVector;
}

ProgramHelp::ProgramHelp(HelpType_t helpType, csref help) {
    if (helpType >= 0 && helpType < _kPHlimit)
        getProgramHelp()[helpType] = help;
}

vector<ProgramHelp::PostHelpFcn_t>& getPostHelpFcns() {
    static vector<ProgramHelp::PostHelpFcn_t> postHelpVector;
    return postHelpVector;
}

ProgramHelp::ProgramHelp(PostHelpFcn_t fcn)
    {
    getPostHelpFcns().push_back(fcn);
    }

string ProgramHelp::GetString(HelpType_t helpType) {
    string r;
    if (helpType >= 0 && helpType < _kPHlimit)
        r = getProgramHelp()[helpType];
    else
        r = "";
    // Handle the post help functions
    if (helpType == kPHpostHelp) {
        vector<PostHelpFcn_t>& fcns = getPostHelpFcns();
        for (size_t i = 0; i < fcns.size(); ++i) {
            string helpstr = fcns[i]();
            if (! r.empty() && !helpstr.empty()/* && r[r.size()-1] != '\n'*/) r += "\n";
            r += helpstr;
        }
    }
    return r;
}

string phGetOptions() {
    string r;
    for (OptionList::const_iterator i = OptionList::begin(); i != OptionList::end(); ++i) {
        r += "[--";
        r += i->GetName();
        if (i->HasParam())
            r += " " + i->GetParam();
        r += "] ";
    }
    return r;
}

string phGetOptionDoc() {
    string r;
    for (OptionList::const_iterator i = OptionList::begin(); i != OptionList::end(); ++i) {
        if (i->GetDoc().empty()) continue;
        if (!r.empty()) r += "\n";
        r += "  ";
        r += "--" + i->GetName();
        r += " ";
        r += StrReplace(i->GetDoc(), "\n", "\n  "); // indent
        if (i->HasParam() && i->GetDefaulter())
            r += " (default is " + (i->GetDefaulter())(i->GetName()) + ")";
    }
    return r;
}

string phGetUsageLine() {
    string r;
    r += "Usage: ";
    r += ProgramHelp::GetString(kPHprogram);
    r += " [--help] [options] ";
    r += ProgramHelp::GetString(kPHadditionalArgs);
    return r;
}

string ProgramHelp::GetUsage(bool showOptions) {
    string r;
    r+= phGetUsageLine();
    if (showOptions)
        r += "\nOptions: " + phGetOptions();
    return r;
    }

string phGetHelp(bool verbose) {
    string r;
    r += ProgramHelp::GetString(kPHusage) + "\n";
    r += phGetUsageLine() + "\n";
    // Show argument list and help
    r += "Options: " + phGetOptions();
    if (verbose) {
        r += "\n" + phGetOptionDoc();
        // Append help
        string temp = ProgramHelp::GetString(kPHhelp);
        if (!temp.empty()) r += "\n\n" + temp;
        temp = ProgramHelp::GetString(kPHpostHelp);
        if (!temp.empty()) r += "\n\n" + temp;
    }
    return r;
    }

void phHelpExit(bool verbose) {
    cerr << phGetHelp(verbose) << endl;
    exit(EXIT_SUCCESS);
}

void phErrorExit(csref errmsg, bool showOptions = false) {
    cerr << errmsg << endl;
    cerr << ProgramHelp::GetUsage(showOptions) << endl;
    exit (EXIT_FAILURE);
}
//---------------------------------------------------------------------
// Parsing the argument list
//---------------------------------------------------------------------
string PopArg(int* argc, char** argv, bool hasParameter)
// If hasParameter is true, returns the option's parameter argument. Else the empty string
{
    int amount = 1;
    string retval;
    if (hasParameter && *argc >= 2) {
        // Has a parameter (otherwise return NULL)
        retval = *(argv+1);
        amount = 2;
    }

    while (*(argv+amount))
    {
        *argv = *(argv + amount);
        ++argv;
    }
    *argv = NULL;
    *argc = *argc - amount;
    return retval;
}

string RemoveDir(csref str) {
    size_t pos = str.find_last_of("\\/");
    if (pos == string::npos)
        return str;
    else
        return str.substr(pos+1);
}

// Static (aka global) function for parsing arglist
void Option::ParseArglist(int *argc, char** argv, int minPositionalArgs, int maxPositionalArgs) {
    if (maxPositionalArgs < 0) maxPositionalArgs = minPositionalArgs;

    if (argc > 0 && argv != NULL && argv[0] != NULL) {
        ProgramHelp(kPHprogram, RemoveDir(argv[0]));
        ++argv;
    }

    string errmsg;
    string arg;
    while (*argv) {
        arg = *argv;

        if (arg == "-h" || arg == "-H" || arg == "-?")
            phHelpExit(false); // short help


        if (! StrStartsWith(arg, "--")) break; // end of switches

        string optionName = StrToLower(arg.substr(2));
        if (optionName == "help")
            phHelpExit(true);  // detailed help

        string errmsg = "Unknown option: " + arg;
        for (OptionList::const_iterator i = OptionList::begin(); i != OptionList::end(); ++i) {
            // Everything should be lower case now
            if (optionName == i->GetName()) {
                // We found a match
                string param = PopArg(argc, argv, i->HasParam()); // return an empty string if there is no parameter for option
                errmsg = i->iParser(optionName, param);
                break;
                }
        }

        if (! errmsg.empty())
            phErrorExit(errmsg, true);
    }

    // The remaining arguments must be positional
    if (minPositionalArgs != kVariable
        && !(minPositionalArgs == 0 && *argc == 0) // edge case for services
        ) {
	  // Need to check count of positional arguments
	  if (minPositionalArgs + 1 > *argc)
		phErrorExit("Not enough parameters.", false);
	  else if (maxPositionalArgs + 1 < *argc)
		phErrorExit("Too many parameters starting with: " + arg);
	  }

    // Everything checks out.  argv points to any remaining positional arguments
   }

