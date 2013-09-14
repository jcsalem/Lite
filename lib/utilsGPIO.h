// Utilities for accessing the GPIO on a Raspberry PI
// Used by StripBuffer
// Requires code to be run as root.
// Alternatives to this code are:
//   A library like WiringPi (too much code and overhead)
//   sysfs (easy but requires a context switch)
//

#include "utilsPrecomp.h"

// This ifndef is just for testing the compilation on Linux/OSX
#ifndef OS_WINDOWS
//#ifdef __arm__
#include <string>
#include "utils.h"

namespace GPIO {
// There are 54 GPIO pins on the BCM2835
const int       kMaxGPIO    = 54;

bool InitializeGPIO(string* errmsg = NULL);
void SetModeInput(int gpio);
void SetModeOutput(int gpio);
void Write(int gpio, bool value);
};

#endif // __arm__
