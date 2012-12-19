// Defines standard options used by all CK effects

#ifndef LFRAMEWORK_H_INCLUDED
#define LFRAMEWORK_H_INCLUDED
#include "utilsTime.h"
#include "utilsOptions.h"

class LBuffer;
class Lgroup;

namespace L {
// --pds
// --outmap   Output mapping function (used in creating the output buffer)
extern LBuffer*     gOutputBuffer;
// --verbose
extern bool         gVerbose;
// --time
extern float        gRunTime;
// --color
// See color.h for the latest random color definitions
// --rate
extern float        gRate;
void AllowNegativeRate(); //Call this to allow negative values
// --fade
extern float        gFade;  // Fade in/out time in seconds
// --outmap
extern int          gOutMap; // Output mapping

// Global time variables
extern Milli_t      gTime;                  // Current time
extern Milli_t      gStartTime;
extern Milli_t      gEndTime;

// Standard loop functions
typedef void (*Callback_t) (Lgroup& objGroup);
void Startup(int *argc, char** argv, int numPositionalArgs = 0);
//void Startup(); // Must have already initialized gOutputBuffer to call this one
void Run(Lgroup& objgroup, Callback_t fcn);
void Cleanup(bool eraseAtEnd = true);

}; // namespace L
#endif // LFRAMEWORK_H_INCLUDED
