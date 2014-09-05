#include "utilsTime.h"

//-------------------------------------------------------------
// Generic functions
//-------------------------------------------------------------
const uint32 kTimeValMax = 0xFFFFFFFFUL;
const uint32 kTimeValEndOfRangeMask = 0xF000000UL;

inline uint32 TimeDiff(uint32 newTime, uint32 oldTime) {
    if (newTime >= oldTime)
        return newTime - oldTime;
    else {
        oldTime = kTimeValMax - oldTime + 1;
        return oldTime + newTime;
    }
}

Milli_t MilliDiff(Milli_t newTime, Milli_t oldTime) {return TimeDiff(newTime, oldTime);}
Micro_t MicroDiff(Micro_t newTime, Micro_t oldTime) {return TimeDiff(newTime, oldTime);}

inline bool TimeLT(uint32 a, uint32 b) {
  if ((a & kTimeValEndOfRangeMask) == kTimeValEndOfRangeMask && (b & kTimeValEndOfRangeMask) == 0) 
    return true; 
  else 
    return a < b;
}

bool MilliLT(Milli_t a, Milli_t b) {return TimeLT(a, b);}
bool MicroLT(Micro_t a, Micro_t b) {return TimeLT(a, b);}

void SleepSec(float secs) {
    SleepMilli(secs * 1000);
}

//-------------------------------------------------------------
// OS-Specific Sleep Functions
//-------------------------------------------------------------
#if defined(OS_ARDUINO)

void SleepMilli(Milli_t millis) {
    delay(millis);
}

void SleepMicro(uint32 microsec) {
  // delayMicroseconds only works up to 16384
  if (microsec > 16383)
    SleepMilli((microsec + 500)/1000);
  else
    delayMicroseconds(microsec);
}

#elif defined(OS_WINDOWS)
#include "Windows.h"

void SleepMilli(Milli_t millis) {
    Sleep(millis);
}

// This is obviously totally inaccurate.
// Correct way to do this on Windows is to spin on QueryPerformanceCounter.
void SleepMicro(uint32 microsec) {
  Sleep(1);
}

#else // Linux and Mac
#include <unistd.h>

void SleepMilli(Milli_t millis) {
    usleep(millis * 1000);
}

// This version works on Linux and Mac
void SleepMicro(Micro_t microsec) {
  usleep(microsec);
}

#endif

//-------------------------------------------------------------
// OS-specific Time Functions
//-------------------------------------------------------------

#if defined(OS_ARDUINO)
#include "WProgram.h"
// Arduino
Milli_t Milliseconds()
{
    return millis();
}

Milli_t Microseconds()
{
    return micros();
}

#elif defined(OS_WINDOWS)
#include <ctime>

uint32 WindowsTime(double unitsPerSecond)
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
        return tt * unitsPerSecond / (double)(gQPFreq.QuadPart);
    }
    else
    {
     uint32 c = clock();
     uint32 m = unitsPerSecond / CLOCKS_PER_SEC + .5;
     return c * m;
    }
}


Milli_t Milliseconds() {return WindowsTime(1000.0);}
Micro_t Microseconds() {return WindowsTime(1000000.0);}

#elif defined(OS_MAC)
// #include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

uint32 MacTime(uint64_t nanosecsPerUnit)
{
    static mach_timebase_info_data_t timebaseInfo;
    static bool timebaseInfoReady = false;
    if (! timebaseInfoReady) {
        mach_timebase_info(&timebaseInfo);
        timebaseInfoReady = true;
    }

    uint64_t abstime = mach_absolute_time();
    abstime = abstime * timebaseInfo.numer / timebaseInfo.denom / nanosecsPerUnit;
    return (uint32) abstime;
}

Milli_t Milliseconds() {return MacTime(1000000);}
Micro_t Microseconds() {return MacTime(1000);}

#else
// POSIX version
#include <time.h>
#include <assert.h>

uint32 LinuxTime(uint32 unitsPerSecond)
{
    struct timespec current;
    int status = clock_gettime(CLOCK_MONOTONIC, &current);
    assert(status==0); // error if clock_gettime doesn't work
    uint32 nanosecsPerUnit = 1000000000 / unitsPerSecond;
    uint32 val = current.tv_sec * unitsPerSecond + current.tv_nsec / nanosecsPerUnit;
    return val;
}

Milli_t Milliseconds() {return LinuxTime(1000);}
Micro_t Microseconds() {return LinuxTime(1000000);}


// OBSOLETE Version (always returns 0 on XSI compatible clocks where CLOCKS_PER_SEC is 1000000)
// const Milli_t kClocksToMillis = 1000 / CLOCKS_PER_SEC;
//Milli_t Milliseconds()
//{
//     Milli_t c = clock();
//     return c * kClocksToMillis;
//}

#endif
