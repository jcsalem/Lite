// Random number functions

#ifndef _UTILSRANDOM_H
#define _UTILSRANDOM_H

#include "utils.h"

// These return a random value from min through max
int32 RandomInt (int32 min, int32 max);
int32 RandomInt (int32 max);

// These return a random float from min up to but not including limit (uniform distribution)
float RandomFloat(float min, float limit);
float RandomFloat(float limit = 1.0);

// This returns the max of <num> random floats
float RandomMax(int num, float min, float limit);
float RandomMax(int num, float limit = 1.0);
// This returns the min of <num> random floats
float RandomMin(int num, float min, float limit);
float RandomMin(int num, float limit = 1.0);

// This returns a normally distributed random number
float RandomNormal(float mean = 0.0, float sigma = .5);
// This returns a normally distributed random number that is bounded
float RandomNormalBounded(float mean, float sigma, float minVal, float maxVal);
// Returns a positive number
float RandomExponential(float alpha = 1.0, float maxVal = 0.0);


#endif // _UTILSRANDOM_H
