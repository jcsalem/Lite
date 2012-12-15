// Defines standard options used by all CK effects

#ifndef LFRAMEWORK_H_INCLUDED
#define LFRAMEWORK_H_INCLUDED
#include "utilsTime.h"

class LBuffer;
class Lgroup;

namespace L {
// --pds
// --outmap   Output mapping function (used in creating the output buffer)
extern LBuffer*    gOutputBuffer;
// --verbose
extern bool        gVerbose;
// --time
extern float       gRunTime;
// --color
// See color.h for the latest random color definitions
// --rate
extern float       gRate;
// --fade
extern float       gFade;  // Fade in/out time in seconds

// Global time variables
extern Milli_t     gTime;                  // Current time
extern Milli_t     gStartTime;
extern Milli_t     gEndTime;

// Standard loop functions
typedef void (*Callback_t) (Lgroup& objGroup);
void Startup();
void Run(Lgroup& objgroup, Callback_t fcn);
void Cleanup(bool eraseAtEnd = true);

// Parses and initialize all of the standard options.  Modifies argc and argv
bool StdOptionsParse(int* argc, char** argv, string* errmsg = NULL);

// Help
extern const char*    kStdOptionsArgs;
extern const char*    kStdOptionsArgsDoc;

}; // namespace L
#endif // LFRAMEWORK_H_INCLUDED
