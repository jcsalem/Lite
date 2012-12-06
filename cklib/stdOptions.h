// Defines standard options used by all CK effects

#ifndef STDOPTIONS_H_INCLUDED
#define STDOPTIONS_H_INCLUDED

class LBuffer;

namespace CK {
// --pds
extern LBuffer*    gOutputBuffer;
// --verbose
extern bool        gVerbose;
// --time
extern float       gRunTime;
// --color
// See color.h for the latest random color definitions
// --rate
extern float       gRate;
// --outmap   Output mapping function (used in creating the output buffer)

// Parses and initialize all of the standard options.  Modifies argc and argv
bool StdOptionsParse(int* argc, char** argv, string* errmsg = NULL);

// Help
extern const char*    kStdOptionsArgs;
extern const char*    kStdOptionsArgsDoc;

}; // namespace CK
#endif // STDOPTIONS_H_INCLUDED
