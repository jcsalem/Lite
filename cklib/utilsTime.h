#include "utils.h"

#ifndef _UTILSTIME_H
#define _UTILSTIME_H

typedef uint32 Milli_t;
const uint32 kMaxMilli_t = 0xFFFFFFFFUL;
const uint32 kMilliEndOfRangeMask = 0xF000000UL;

Milli_t Milliseconds();
Milli_t MilliDiff(Milli_t newTime, Milli_t oldTime);

inline bool MilliLT(Milli_t a, Milli_t b) {if ((a & kMilliEndOfRangeMask) == kMilliEndOfRangeMask && (b & kMilliEndOfRangeMask) == 0) return true; else return a < b;}
inline bool MilliLE(Milli_t a, Milli_t b) {return a == b || MilliLT(a, b);}
inline bool MilliGT(Milli_t a, Milli_t b) {return MilliLT(b, a);}
inline bool MilliGE(Milli_t a, Milli_t b) {return MilliLE(b, a);}

void SleepMilli(Milli_t milliseconds);
void SleepSec(Milli_t milliseconds);

#endif // _UTILSTIME_H
