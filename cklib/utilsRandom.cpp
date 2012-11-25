#include "utils.h"
#include "utilsRandom.h"
#include <math.h>

//------------------------------------------------
// Random Utilities
//------------------------------------------------

// Returns a random number scaled from 0 to 0.99999
float GetNormalizedRand()
    {
    int rr = rand();
    return (float)rr / ((float)RAND_MAX + 1);
    }

// POSIX version
int32 RandomInt(int32 max)
{
    return RandomInt(0, max);
}

int32 RandomInt (int32 min, int32 max)
	{
	float r = GetNormalizedRand();
	float range = max - min + 1;
	r = r * range + 0.5;
	unsigned int ru = static_cast<unsigned int>(r);	// remove fractional part
	int ri = static_cast<int>(ru + min);
	return ri;
	}

float RandomFloat(float limit)
{
    return GetNormalizedRand() * limit;
}

float RandomFloat (float min, float limit)
{
    float r = GetNormalizedRand();
    r = r * (limit - min) + min;
    return r;
}

float RandomMax(int num, float mmax) {
    return RandomMax(num, 0.0, mmax);
    }
float RandomMax(int num, float mmin, float mmax) {
    float retval = RandomFloat(mmin, mmax);
    for (int i = 1; i < num; ++i)
        retval = max(RandomFloat(mmin, mmax), retval);
    return retval;
}

float RandomMin(int num, float mmax) {
    return RandomMin(num, 0.0, mmax);
    }
float RandomMin(int num, float mmin, float mmax) {
    float retval = RandomFloat(mmin, mmax);
    for (int i = 1; i < num; ++i)
        retval = min(RandomFloat(mmin, mmax), retval);
    return retval;
}

float _GetUniformRandom() {
    // Returns a random in the range (0, 1]
    int rr = rand();
    return ((float)rr + 1.0f) / ((float)RAND_MAX + 1.0f);
}

float RandomNormal(float mean, float sigma) {
    float r1 =  _GetUniformRandom();
    float r2 =  _GetUniformRandom();
    return mean + sigma * sqrt(-2*log(r1)) * cos(2*M_PI*r2);
}

// Probability is highest for numbers near zero
// This may return arbitrarily large numbers
float RandomExponential(float alpha, float valLimit) {
    if (alpha <=0) return 0.0f;
    while (true) {
        float r1 =  _GetUniformRandom();
        float val = -logf(r1)/alpha;
        if (valLimit == 0.0 || val < valLimit) return val;
    }
}

