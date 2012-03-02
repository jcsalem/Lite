#include "utils.h"
#include "utilsRandom.h"

//------------------------------------------------
// Random Utilities
//------------------------------------------------

#ifndef __AVR__
// POSIX version
int32 Random(int32 max)
{
    return Random(0, max);
}

int32 Random (int32 min, int32 max)
	{
    int rr = rand();
    // normalize to between 0 and 0.999999
	double r = (double)rr / ((double)RAND_MAX + 1);
	double range = max - min + 1;
	r = r * range;
	unsigned int ru = static_cast<unsigned int>(r);	// remove fractional part
	int ri = static_cast<int>(ru + min);
	return ri;
	}
#endif
