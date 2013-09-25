// For Windows to force the definition of rand_s in stdlib.h (only supported on XP/2003 and later)
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

float RandomNormalBounded(float mean, float sigma, float minVal, float maxVal) {
    float rval;
    do {
        rval = RandomNormal(mean, sigma);
    } while (rval < minVal || rval > maxVal);
    return rval;
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

//------------------------------------------------
// Initialization
//------------------------------------------------

// Pid value
#if defined(OS_WINDOWS)
#include "Windows.h"
#include <ctime>
// RtlGenRandom and rand_s are not defined by default in MING. It's been in Windows since XP
typedef BOOLEAN (WINAPI *RtlGenRandom_t)(PVOID randomBuffer, ULONG length);
RtlGenRandom_t gRtlGenRandom = NULL;
RandomSeed_t RandomGenerateSeed() {
    static bool isSetup = false;
    if (! isSetup) {
        gRtlGenRandom = (RtlGenRandom_t) GetDLLFunctionAddress("SystemFunction036", "advapi32");
        isSetup = true;
    }
    RandomSeed_t seed = 0;
    if (gRtlGenRandom && gRtlGenRandom(&seed,sizeof(RandomSeed_t)) && seed) return seed;
    // RtlGenRandom failed
    seed = GetCurrentThreadId() ^ GetCurrentProcessId() ^ time(NULL) ^ clock();
    return seed;
}
#elif defined(OS_ARDUINO)
#warn "Arduino RandomGenerateSeed needs work"
RandomSeed_t RandomGenerateSeed() {
    return 0x12345678;
}
#elif defined(__posix__) || defined(OS_LINUX) || defined(OS_MAC)
#include <unistd.h>
#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
namespace{
RandomSeed_t _ReadURandom() {
    FILE* file = fopen("/dev/urandom", "rb");
    if (!file) return 0;
    RandomSeed_t seed = 0;
    fread(&seed, sizeof(seed), 1, file);
    fclose(file);
    return seed;
}
#pragma GCC diagnostic pop

#ifdef OS_LINUX
#include <sys/syscall.h>
pid_t my_gettid() {
    return (pid_t) syscall(SYS_gettid);
}
#elif defined(OS_MAC)
pid_t my_gettid() {
    return 0; // Could use pthread_self but that's more work
}
#else
pid_t my_gettid() {
    return gettid();
}
#endif


}; // namespace

RandomSeed_t RandomGenerateSeed() {
    RandomSeed_t seed = _ReadURandom();
    if (seed) return seed;

    // No /dev/urandom.  Fall back to something simple
    pid_t pid = getpid();
    pid_t tid = my_gettid(); // If this isn't defined syscall(SYS_gettid) can be used or pthread_self()
    if (tid != pid) pid ^= tid;
    seed ^= pid;
    seed ^= gethostid();
    seed ^= time(NULL);
    seed ^= clock();
    return seed;
}
#else
#error "No RandomGenerateSeed implementation for this architecture"
#endif

// If it's zero, it uses a system noise value
RandomSeed_t RandomInitialize(RandomSeed_t seed) {
    if (seed == 0) seed = RandomGenerateSeed();
    srand(seed);
    return seed;
}

#ifndef NO_RANDOM_INIT
namespace { // make private
class RandomInitAtStartup {
    public:
        RandomInitAtStartup() {RandomInitialize();}
};

RandomInitAtStartup foo;

};

#endif // NO_RANDOM_INIT
