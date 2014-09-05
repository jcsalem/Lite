#ifndef _UTILSTIME_H
#define _UTILSTIME_H
#include "utils.h"

typedef uint32 Milli_t;
Milli_t Milliseconds(); // This is a monotonically increasing millisecond count
Milli_t MilliDiff(Milli_t newTime, Milli_t oldTime);

bool MilliLT(Milli_t a, Milli_t b);
inline bool MilliLE(Milli_t a, Milli_t b) {return a == b || MilliLT(a, b);}
inline bool MilliGT(Milli_t a, Milli_t b) {return MilliLE(b, a);}
inline bool MilliGE(Milli_t a, Milli_t b) {return MilliLT(b, a);}

typedef uint32 Micro_t;
Micro_t Microseconds(); // This is a monotonically increasing microsecond count
Micro_t MicroDiff(Micro_t newTime, Micro_t oldTime);

void SleepSec(float seconds);
void SleepMilli(Milli_t milliseconds);
void SleepMicro(Micro_t microseconds);

#endif // _UTILSTIME_H
