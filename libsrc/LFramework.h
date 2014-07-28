// Defines standard options used by all CK effects

#ifndef LFRAMEWORK_H_INCLUDED
#define LFRAMEWORK_H_INCLUDED
// TODO (jimsalem#1#): Change the ObjFcn and the GroupFcn to be methods on their respective object.

#include "Config.h"
#include "utilsTime.h"
#include "utilsOptions.h"
#include "Lproc.h"

class LBuffer;
class Lgroup;
class Lobj;

namespace L {
// --dev
extern LBuffer*     gOutputBuffer;
// --verbose
extern bool         gVerbose;
// --time
extern float        gRunTime;
// --color
// See color.h for the latest random color definitions
// --rate
extern float        gRate;
typedef enum {kRatePositive = 0, kRateNonZero = 1, kRateAny = 2} RateMode_t;    // Default is kRatePositive
void SetRateMode(RateMode_t mode); //Call this to allow different types of values for --rate
// --fade
extern float        gFade;  // Fade in/out time in seconds
// --filter   Output filters (used in creating the output buffer)
extern string       gGlobalFilters; // Output mapping
// --proc   Object operations to be applied
extern LprocList    gProcList;

// Global time variables
extern Milli_t      gTime;                  // Current time
extern Milli_t      gStartTime;
extern Milli_t      gEndTime;
extern Milli_t      gFrameDuration;	    // Length of a single frame

// These are requests from anywhere in the stack
extern bool         gTerminateNow;

// Standard loop functions
typedef void (*ObjCallback_t)   (Lobj* obj);    // Called for each object during L::Run
typedef void (*GroupCallback_t) (Lgroup* obj);  // Called once for the group at the end of L::Run

// This handles all of the startup functions. minPositionalArgs may be kVariable if any number of positional args are allowed or you can specify a range. 
// If maxPositionalArgs is not specified is defaulted to the value of minPositionalArgs
void Startup(int *argc, char** argv, int minPositionalArgs = 0, int maxPositionalArgs = -1);
//void Startup(); // Must have already initialized gOutputBuffer to call this one
void Run(Lgroup& objgroup, ObjCallback_t fcn = NULL, GroupCallback_t gfcn = NULL); // Delay between renders is based on gFrameDuration
void RunOnce(Lgroup& objgroup, GroupCallback_t gfcn = NULL);                // No delays built in
void Cleanup(bool eraseAtEnd = false);

void ErrorExit(csref message);
}; // namespace L

#endif // LFRAMEWORK_H_INCLUDED
