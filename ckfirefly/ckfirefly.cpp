// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "LFramework.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

// Configuration
Milli_t     gFrameDuration  = 40;    // duration of each frame of animation (in MS)

// Fwd decls
void SetupNextCycle(LobjOld* lobj, Milli_t startTime);

//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << L::kStdOptionsArgs << endl;
    cerr << "Where:" << endl;
    cerr << L::kStdOptionsArgsDoc << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int* argc, char** argv)
{
    // Parse stamdard options
    string errmsg;
    bool success = L::StdOptionsParse(argc, argv, &errmsg);
    if (! success)
        Usage(progname, errmsg);

    // Parse remaining
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (*argc, argv, "", longOpts, &optIndex);
        if (c == (char) -1) break; // Done parsing
        switch (c)
            {
            case 'h':
                Usage(progname);
            default:
                cerr << "Internal error - unknown option: " << c << endl;
                Usage(progname);
            }
    }
}
//----------------------------------------------------------------
// Light Testing
//----------------------------------------------------------------

void TestLights()
{
    int count = L::gOutputBuffer->GetCount();

    for (int i = count-1; i >= 0; --i)
    {
        L::gOutputBuffer->Clear();
        for (int j = 0; j < count; ++j)
        {
            RGBColor c = WHITE;
            c *= ((float) j) / count;
            int pos = (i + j) % count;
            L::gOutputBuffer->SetRGB(pos, c);
        }
        L::gOutputBuffer->Update();
        SleepMilli(10);
    }
    SleepMilli(250);
    L::gOutputBuffer->Clear();
    L::gOutputBuffer->Update();
}

//----------------------------------------------------------------
// Firefly utilities
//----------------------------------------------------------------
Milli_t gTime;

int MaxFireflies () {
    return max(1, L::gOutputBuffer->GetCount() / 20);
}

// fwd decls
void FireflyMove(void);
void FireflyClip(void);
LobjOld* FireflyAlloc(void);

float RandomBell(float bnum, float mmin = 0.0, float mmax = 1.0) {
    int num = bnum;
    float retval = 0.0;
    for (int i = 0; i < num; ++i)
        retval += RandomFloat(mmin, mmax);
    if (bnum != num)
        retval += (bnum - num) * RandomFloat(mmin, mmax);
    return retval / bnum;
}

float RandomSpeed() {
    return L::gRate * RandomBell(2, .005, .4);
}

LobjOld* FireflyAlloc(void) {
  LobjOld* lobj = LobjOld::Alloc();
  if (! lobj) return NULL;
  lobj->pos = RandomFloat(L::gOutputBuffer->GetCount());
  lobj->speed = RandomSpeed();
  lobj->velocity = RandomFloat(-lobj->speed, lobj->speed);
  lobj->maxColor = RandomColor();
  lobj->color = lobj->maxColor;
  SetupNextCycle(lobj, gTime);
  return lobj;
}

void FireflyMoveOne(LobjOld* lobj) {
  lobj->pos += lobj->velocity;
  float delta = RandomBell(2, -lobj->speed/2, lobj->speed/2);
  lobj->velocity = lobj->velocity + delta;
}

void FireflyMove(void) {
  LobjOld::Map(FireflyMoveOne);
}

void FireflyClip(void) {
  for (short i = 0; i < LobjOld::GetNum();) {
    LobjOld* lobj = LobjOld::GetNth(i);
    if (lobj->pos <= -2 || lobj->pos >= L::gOutputBuffer->GetCount() + 1)
     LobjOld::Free(lobj);
    else
     ++i;
  }
}

//----------------------------------------------------------------------
// Brightness control
//----------------------------------------------------------------------

short gFFattack = 3;
short gFFhold   = 3;
short gFFrelease= 4;
short gFFSleepMin = 750;
short gFFSleepMax = 3000;
short gFFSleepFactor = 3;

short SmallRandInRange(short minval, short maxval, short factor) {
  factor = max(factor, (short) 1);
  short val = 32767;
  for (int i = 0; i < factor; ++i)
    val = min(val, (short) (RandomInt(maxval - minval) + minval));
  return val;
}

void SetupNextCycle(LobjOld* lobj, Milli_t startTime) {
  short attackDur = (RandomInt(150) + 0);
  short holdDur = (RandomInt(attackDur) + RandomInt(attackDur) + attackDur * 2);
  short releaseDur = min(RandomInt(200), RandomInt(200)) + 100;
  attackDur *= gFFattack;
  holdDur *= gFFhold;
  releaseDur *= gFFrelease;
  short sleepDur = SmallRandInRange(gFFSleepMin, gFFSleepMax, gFFSleepFactor);
  lobj->startAttack = startTime;
  lobj->startHold = lobj->startAttack + attackDur;
  lobj->startRelease = lobj->startHold + holdDur;
  lobj->startSleep = lobj->startRelease + releaseDur;
  lobj->startNext = lobj->startSleep + sleepDur;
}

void RampColor(LobjOld* lobj, Milli_t startTime, Milli_t endTime, bool reverse) {
  Milli_t startDiff = gTime - startTime;
  Milli_t totalDiff = endTime - startTime;
  if (totalDiff == 0) return;
  // Scale down so we don't overflow a short
  while (totalDiff > 127) {
      startDiff >>= 2;
      totalDiff >>= 2;
  }
  if (reverse) startDiff = totalDiff - startDiff;
  lobj->color = lobj->maxColor;
  lobj->color *= startDiff;
  lobj->color /= totalDiff;

  //cout << "current=" << gTime << "  start=" << startTime << "  end=" << endTime << " startDiff=" << startDiff << " totalDiff=" << totalDiff << endl;
}

void FireflyDimOne(LobjOld* lobj) {
  if (gTime < lobj->startHold)
    RampColor(lobj, lobj->startAttack, lobj->startHold, false);
  else if (gTime < lobj->startRelease)
    lobj->color = lobj->maxColor;
  else if (gTime < lobj->startSleep)
    RampColor(lobj, lobj->startRelease, lobj->startSleep, true);
  else if (gTime < lobj->startNext)
    lobj->color = BLACK;
  else {
    SetupNextCycle(lobj, lobj->startNext);
    RampColor(lobj, lobj->startAttack, max(gTime,lobj->startHold), false);
  }
}

void FireflyDim(void) {
  LobjOld::Map(FireflyDimOne);
}

void FireflyLoop()
{
    Milli_t startTime = Milliseconds();
    Milli_t runTimeMilli = L::gRunTime * 1000 + .5;

    while (true) {
        gTime = Milliseconds();
        FireflyMove();
        FireflyClip();
        // Maybe allocate
        short num = LobjOld::GetNum();
        if (num == 0 || (num < MaxFireflies() && num < LobjOld::GetMaxNum() && RandomInt(10) == 0))
          FireflyAlloc();
        FireflyDim();
        // Render
        L::gOutputBuffer->Clear();
        LobjOld::RenderAll(L::gOutputBuffer);
        L::gOutputBuffer->Update();
        // Exit if out of time, else delay until next frame
        Milli_t currentTime = Milliseconds();
        if (runTimeMilli != 0 && runTimeMilli < MilliDiff(currentTime, startTime))
            break;
        else {
            Milli_t elapsedSinceFrameStart = MilliDiff(currentTime, gTime);
            if (gFrameDuration > elapsedSinceFrameStart)
                SleepMilli(gFrameDuration - elapsedSinceFrameStart);
        }
    }
}

int main(int argc, char** argv)
{
    const char* progname = "ckfirefly";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

    // Parse arguments
    ParseArgs(progname, &argc, argv);

    // Parse command
    if (optind != argc)
    {
        cerr << "Too many arguments." << endl;
        Usage(progname);
    }

    // Test everything
    // TestLights();

    FireflyLoop();
}
