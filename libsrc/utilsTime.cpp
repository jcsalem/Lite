#include "utilsTime.h"

//-------------------------------------------------------------
// Generic functions
//-------------------------------------------------------------

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

void SleepSec(float secs) {
    SleepMilli(secs * 1000);
}


//-------------------------------------------------------------
// OS-specific ones
//-------------------------------------------------------------

#if defined(OS_ARDUINO)
#include "WProgram.h"
// Arduino
Milli_t Milliseconds()
{
    return millis();
}

#elif defined(OS_WINDOWS)
#include "Windows.h"
#include <ctime>

const Milli_t kClocksToMillis = 1000 / CLOCKS_PER_SEC;

Milli_t Milliseconds()
{
    static bool gFirstTime = true;
    static bool gHasQPF;
    static LARGE_INTEGER gQPFreq;

    if (gFirstTime)
    {
		gHasQPF = (QueryPerformanceFrequency(&gQPFreq) != 0);
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

#elif defined(OS_MAC)
// #include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h> // Required for usleep

void SleepMilli(Milli_t millis) {
    usleep(millis * 1000);
}


Milli_t Milliseconds()
{
    static mach_timebase_info_data_t timebaseInfo;
    static bool timebaseInfoReady = false;
    if (! timebaseInfoReady) {
        mach_timebase_info(&timebaseInfo);
        timebaseInfoReady = true;
    }

    uint64_t abstime = mach_absolute_time();
    abstime = abstime * timebaseInfo.numer / timebaseInfo.denom / 1000000;
    return (Milli_t) abstime;
}

#else
// POSIX version
#include <time.h>
#include <unistd.h>
#include <assert.h>

void SleepMilli(Milli_t millis) {
    usleep(millis * 1000);
}

Milli_t Milliseconds()
{
    struct timespec current;
    int status = clock_gettime(CLOCK_MONOTONIC, &current);
    assert(status==0); // error if clock_gettime doesn't work
    Milli_t val = current.tv_sec * 1000 + current.tv_nsec / 1000000;
    return val;
}

// OBSOLETE Version (always returns 0 on XSI compatible clocks where CLOCKS_PER_SEC is 1000000)
// const Milli_t kClocksToMillis = 1000 / CLOCKS_PER_SEC;
//Milli_t Milliseconds()
//{
//     Milli_t c = clock();
//     return c * kClocksToMillis;
//}

#endif
