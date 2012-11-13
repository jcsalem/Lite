// Random number functions

#ifndef _UTILSRANDOM_H
#define _UTILSRANDOM_H

#include "utils.h"

// These return a random value from min through max
int32 RandomInt (int32 min, int32 max);
int32 RandomInt (int32 max);

// These return a random float from min up to but not including limit
float RandomFloat(float min, float limit);
float RandomFloat(float limit = 1.0);

#endif // _UTILSRANDOM_H
