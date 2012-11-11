#include "utils.h"

#ifndef _UTILSTIME_H
#define _UTILSTIME_H

typedef uint32 Milli_t;
const uint32 kMaxMilli_t = 0xFFFFFFFFUL;

Milli_t Milliseconds();
Milli_t MillisecondsDiff(Milli_t newTime, Milli_t oldTime);

void SleepMilli(Milli_t milliseconds);
void SleepSec(Milli_t milliseconds);

#endif // _UTILSTIME_H
