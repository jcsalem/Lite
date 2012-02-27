// Top level firefly code

#include "../cklib/utils.h"
#include "../cklib/Color.h"
#include "../cklib/cklib.h"
#include "../cklib/ckcli.h"
#include "../cklib/Lobj.h"
#include <iostream>
#include <getopt.h>

CKcli gCLI;

//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname)
    {
    cerr << "Usage: " << progname << " --pds pdsinfo1 [--pds pdsinfo2 ...]" << endl;
    cerr << "Where:" << endl;
    cerr << CKcli::gPDSInfoDoc << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        CKCLI_PDSINFO_OPTIONS,
        {"help",    no_argument,        0, 'h'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int argc, char** argv)
{
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (argc, argv, "", longOpts, &optIndex);
        if (c == -1) break; // Done parsing

        if (! gCLI.ParseOptions(c, progname, argc, argv))
        {
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
    if (gCLI.HasError())
    {
        cerr << gCLI.GetLastError() << endl;
        Usage(progname);
    }
    if (gCLI.Buffer.GetCount() == 0)
    {
        cerr << "You must specify at least one CK device" << endl;
        Usage(progname);
    }
}
//----------------------------------------------------------------
// Light Testing
//----------------------------------------------------------------

void TestLights()
{
    int count = gCLI.Buffer.GetCount();

    for (int i = count-1; i >= 0; --i)
    {
        gCLI.Buffer.Clear();
        for (int j = 0; j < count; ++j)
        {
            RGBColor c = WHITE;
            c *= j;
            c /= count;
            int pos = (i + j) % count;
            gCLI.Buffer.SetRGB(pos, c);
        }
        gCLI.Buffer.Update();
        Sleep(10);
    }
    Sleep(250);
    gCLI.Buffer.Clear();
    gCLI.Buffer.Update();
}

//----------------------------------------------------------------
// Firefly utilities
//----------------------------------------------------------------

#if 0
// fwd decls
void FireflyMove(void);
void FireflyClip(void);
Lobj* FireflyAlloc(void);

const short kMaxFireflies = 5;
const short kMillisPerFrame = 16;
Buffer gBuffer;
unsigned long gTime;

static const short gMaxColor = 128;
short RandomCVal(void) {
  return max(random(gMaxColor), random(gMaxColor));
}

Lobj* FireflyAlloc(void) {
  Lobj* lobj = Lobj::Alloc();
  if (! lobj) return NULL;
  lobj->pos = random(Buffer::Width * Lobj::kPosIncr);
  short maxSpeed = Lobj::kPosIncr/12;
  short minSpeed = (Lobj::kPosIncr+63)/64;
  lobj->speed = random(maxSpeed-minSpeed) + minSpeed;
  lobj->velocity = random(lobj->speed + 1) - (lobj->speed / 2);
  lobj->maxColor.r = RandomCVal();
  lobj->maxColor.g = RandomCVal();
  lobj->maxColor.b = RandomCVal();
  lobj->color = lobj->maxColor;
  SetupNextCycle(lobj, gTime);
  return lobj;
}

void FireflyMoveOne(Lobj* lobj) {
#if 0
  lobj->pos += lobj->velocity;
  short delta = random(2 * lobj->speed + 1) - lobj->speed;
  lobj->velocity = lobj->velocity + delta;
#else
  short delta = random(2 * lobj->speed + 1) - lobj->speed;
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
    if (bufpos <= -2 || bufpos >= Buffer::Width + 1)
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
  factor = max(factor, 1);
  short val = 32767;
  for (int i = 0; i < factor; ++i)
    val = min(val, random(maxval - minval) + minval);
  return val;
}

void SetupNextCycle(Lobj* lobj, unsigned long startTime) {
  short attackDur = (random(150) + 0);
  short holdDur = (random(attackDur) + random(attackDur) + attackDur * 2);
  short releaseDur = min(random(200), random(200)) + 100;
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

void RampColor(Lobj* lobj, unsigned long startTime, unsigned long endTime, bool reverse) {
  unsigned long startDiff = gTime - startTime;
  unsigned long totalDiff = endTime - startTime;
  // Scale down so we don't overflow a short
  while (totalDiff > 127) {
      startDiff >>= 2;
      totalDiff >>= 2;
  }
  if (reverse) startDiff = totalDiff - startDiff;
  lobj->color = lobj->maxColor;
  lobj->color *= startDiff;
  lobj->color /= totalDiff;
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

#endif

void FireflyLoop()
{
    while (true)
    {
        gTime = millis();
        FireflyMove();
        FireflyClip();
        // Maybe allocate
        short num = Lobj::GetNum();
        if (num == 0 || (num < kMaxFireflies && num < Lobj::GetMaxNum() && random(10) == 0))
          FireflyAlloc();
        FireflyDim();
        // Render
        gBuffer.Clear();
        Lobj::RenderAll(&gBuffer);
        StripRender(gBuffer);
        StripUpdate();
        // Delay (should be based on clock)
        unsigned long curTime = millis();
        short delayAmount = curTime - gTime;
        if (delayAmount > 0)
          delay(delayAmount);
    }
}

int main(int argc, char** argv)
{
    const char* progname = "cktool";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

    // Parse arguments
    ParseArgs(progname, argc, argv);

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
