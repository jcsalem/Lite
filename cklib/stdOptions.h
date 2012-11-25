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
// See color.h for the random color definitions
//typedef enum {kRandomColorDefault = 0, kRandomColorBrightHSV = 1, kRandomColorRGB = 2, kRandomColorHalloween = 3, kRandomColorStarry = 4, kRandomColorRealStar = 5, kRandomColorRange = 6}
//    RandomColor_t;
//extern RandomColor_t gRandomColorMode;

// Parses and initialize all of the standard options.  Modifies argc and argv
bool StdOptionsParse(int* argc, char** argv, string* errmsg = NULL);

// Help
extern const char*    kStdOptionsArgs;
extern const char*    kStdOptionsArgsDoc;

}; // namespace CK
#endif // STDOPTIONS_H_INCLUDED
