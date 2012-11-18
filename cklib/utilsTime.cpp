#include <time.h>
#include "utilsTime.h"


Milli_t MilliDiff(Milli_t newTime, Milli_t oldTime)
{
    if (newTime >= oldTime)
        return newTime - oldTime;
    else
    {
        oldTime = kMaxMilli_t - oldTime + 1;
        return oldTime + newTime;
    }
}

#ifdef _WIN32
#include "Windows.h"
const Milli_t kClocksToMillis = 1000 / CLOCKS_PER_SEC;

Milli_t Milliseconds()
{
    static bool gFirstTime = true;
    static bool gHasQPF;
    static LARGE_INTEGER gQPFreq;

    if (gFirstTime)
    {
        gHasQPF = QueryPerformanceFrequency(&gQPFreq);
        gFirstTime = false;
    }
    LARGE_INTEGER t;
    if (gHasQPF && QueryPerformanceCounter(&t))
    {
        double tt = t.QuadPart;
        return tt * 1000.0 / (double)(gQPFreq.QuadPart);
    }
    else
    {
     Milli_t c = clock();
     return c * kClocksToMillis;
    }
}

void SleepMilli(Milli_t millis) {
    Sleep(millis);
}

void SleepSec(int secs) {
    Sleep(secs * 1000);
}

#elif defined(__AVR__)
#include "WProgram.h"
// Arduino
Milli_t Milliseconds()
{
    return millis();
}

#else
#include <unistd.h>
// POSIX version
const Milli_t kClocksToMillis = 1000 / CLOCKS_PER_SEC;
Milli_t Milliseconds()
{
     Milli_t c = clock();
     return c * kClocksToMillis;
}

void SleepMilli(Milli_t millis) {
    usleep(millis * 1000);
}

void SleepSec(int secs) {
    usleep(secs * 1000000);
}

#endif
