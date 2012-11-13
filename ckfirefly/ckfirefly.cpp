// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "stdOptions.h"
#include <iostream>
#include <getopt.h>
#include <stdio.h>

// Configuration
Milli_t     gFrameDuration = 25;    // duration of each frame of animation (in MS)

// Fwd decls
void SetupNextCycle(Lobj* lobj, Milli_t startTime);

//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << CK::kStdOptionsArgs << endl;
    cerr << "Where:" << endl;
    cerr << CK::kStdOptionsArgsDoc << endl;
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
    bool success = CK::StdOptionsParse(argc, argv, &errmsg);
    if (! success)
        Usage(progname, errmsg);

    // Parse remaining
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (*argc, argv, "", longOpts, &optIndex);
        if (c == -1) break; // Done parsing
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
    int count = CK::gOutputBuffer->GetCount();

    for (int i = count-1; i >= 0; --i)
    {
        CK::gOutputBuffer->Clear();
        for (int j = 0; j < count; ++j)
        {
            RGBColor c = WHITE;
            c *= ((float) j) / count;
            int pos = (i + j) % count;
            CK::gOutputBuffer->SetRGB(pos, c);
        }
        CK::gOutputBuffer->Update();
        Sleep(10);
    }
    Sleep(250);
    CK::gOutputBuffer->Clear();
    CK::gOutputBuffer->Update();
}

//----------------------------------------------------------------
// Firefly utilities
//----------------------------------------------------------------
Milli_t gTime;

// fwd decls
void FireflyMove(void);
void FireflyClip(void);
Lobj* FireflyAlloc(void);

const short kMaxFireflies = 5;
//const short kMillisPerFrame = 16;

static const float gMaxColor = 1.0;
float RandomCVal(void) {
  return max(RandomFloat(gMaxColor), RandomFloat(gMaxColor));
}

RGBColor RandomColor(void) {
    RGBColor rgb;
#if 0
    rgb.r = max(RandomFloat(0.5, 1.0), RandomFloat(0.5, 1.0));
    rgb.g = RandomFloat (0.0, 0.4);
    rgb.b = RandomFloat (0.0, 0.1);
#else
    rgb.r = RandomCVal();
    rgb.g = RandomCVal();
    rgb.b = RandomCVal();
#endif

    return rgb;
}

Lobj* FireflyAlloc(void) {
  Lobj* lobj = Lobj::Alloc();
  if (! lobj) return NULL;
  lobj->pos = RandomInt(CK::gOutputBuffer->GetCount() * Lobj::kPosIncr);
//  static short lastpos = 0;
//  lobj->pos = lastpos * Lobj::kPosIncr;
//  lastpos += 3;
//  if (lastpos >= CK::gOutputBuffer->GetCount()) lastpos = lastpos - CK::gOutputBuffer->GetCount();
//  cout << lobj->pos << endl;
  short maxSpeed = Lobj::kPosIncr/21;
  short minSpeed = (Lobj::kPosIncr+63)/64;
  lobj->speed = RandomInt(maxSpeed-minSpeed) + minSpeed;
  lobj->velocity = RandomInt(lobj->speed + 1) - (lobj->speed / 2);
  lobj->maxColor = RandomColor();
  lobj->color = lobj->maxColor;
  SetupNextCycle(lobj, gTime);
  return lobj;
}

void FireflyMoveOne(Lobj* lobj) {
#if 1
  lobj->pos += lobj->velocity;
  short delta = RandomInt(lobj->speed + 1) - (lobj->speed+1)/2;
  lobj->velocity = lobj->velocity + delta;
#else
  short delta = RandomInt(2 * lobj->speed + 1) - lobj->speed;
  lobj->pos += delta + lobj->velocity;
  lobj->velocity = delta;
#endif
}

void FireflyMove(void) {
  Lobj::Map(FireflyMoveOne);
}

void FireflyClip(void) {
  for (short i = 0; i < Lobj::GetNum();) {
    Lobj* lobj = Lobj::GetNth(i);
    short bufpos = lobj->pos / Lobj::kPosIncr;
    if (bufpos <= -2 || bufpos >= CK::gOutputBuffer->GetCount() + 1)
     Lobj::Free(lobj);
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

void SetupNextCycle(Lobj* lobj, Milli_t startTime) {
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

void RampColor(Lobj* lobj, Milli_t startTime, Milli_t endTime, bool reverse) {
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

void FireflyDimOne(Lobj* lobj) {
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
  Lobj::Map(FireflyDimOne);
}

// The time for each frame (in milliseconds
const float gTimePerFrame = 25;

void FireflyLoop()
{
    while (true)
    {
        gTime = Milliseconds();
        FireflyMove();
        FireflyClip();
        // Maybe allocate
        short num = Lobj::GetNum();
        if (num == 0 || (num < kMaxFireflies && num < Lobj::GetMaxNum() && RandomInt(10) == 0))
          FireflyAlloc();
        FireflyDim();
        // Render
        CK::gOutputBuffer->Clear();
        Lobj::RenderAll(CK::gOutputBuffer);
        CK::gOutputBuffer->Update();
        // Delay (should be based on clock)
        Sleep(40);

        //Millis_t curTime = Milliseconds();
        //short delayAmount = curTime - gTime;
        //if (delayAmount > 0)
        //  delay(delayAmount);
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
    TestLights();

    FireflyLoop();
}
