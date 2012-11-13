// Defines standard options used by all CK effects

#ifndef STDOPTIONS_H_INCLUDED
#define STDOPTIONS_H_INCLUDED

class LBuffer;

namespace CK {
extern LBuffer*    gOutputBuffer;
extern bool        gVerbose;
extern float       gRunTime;

// Parses and initialize all of the standard options.  Modifies argc and argv
bool StdOptionsParse(int* argc, char** argv, string* errmsg = NULL);

// Help
extern const char*    kStdOptionsArgs;
extern const char*    kStdOptionsArgsDoc;

}; // namespace CK
#endif // STDOPTIONS_H_INCLUDED
