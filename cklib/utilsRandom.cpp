#include "utils.h"
#include "utilsRandom.h"

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
