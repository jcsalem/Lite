#include "utils.h"
#include "Color.h"
#include "LFramework.h"
#include "ComboBuffer.h"
#include "Lproc.h"
#include "utilsOptions.h"
#include "Lobj.h"
#include "utilsStats.h"
#include "LFilter.h"
#include <iostream>

namespace L {

//---------------------------------------------------------------
// Global variables
//---------------------------------------------------------------

// Global time variables
Milli_t     gTime;                   // Current time
Milli_t     gStartTime;              // Loop start time
Milli_t     gEndTime;                // Ending time (set to gTime to end prematurely)
bool        gTerminateNow;           // Set to exit asap
Milli_t     gFrameDuration  = 40;    // duration of each frame of animation (in MS)

// Public list of procs
LprocList       gProcs;

// Publically visible output pipeline
Pipeline        gOutput;

// Not externally Accessible
LBuffer*        gOutputBuffer   = NULL;
deque<LFilter*> gFilters; // This is organized in the order they are applied to the output

// Force things to be deleted at exit
struct UninitializeAtExit {
    UninitializeAtExit() {}
    ~UninitializeAtExit();
};

UninitializeAtExit _gUninitAtExit;

UninitializeAtExit::~UninitializeAtExit() {
    if (gOutputBuffer) {delete gOutputBuffer; gOutputBuffer = NULL;}
    for (size_t i = 0; i < gFilters.size(); ++i)
        delete gFilters[i];
    gFilters.clear();
    gOutput.SetBuffer(NULL);
}

//---------------------------------------------------------------
// Trap Ctrl-C to set terminate flag
//---------------------------------------------------------------
bool CtrlCHandler()
{
    if (gTerminateNow)
        // If Ctrl-C recieved while trying to terminate, let it propogate up the state
        return false;
    gTerminateNow = true;
    return true;
}

//---------------------------------------------------------------
// Stats Buffer (for reporting framerate stats in verbose mode)
//---------------------------------------------------------------

class StatsBuffer : public LFilter
{
public:
  StatsBuffer() : LFilter(), iIsFirstTime(true), iLastFrameTime(0) {}
  string GetDescriptor() const {return "VerboseStats";}
  virtual bool Update();
  const StatsCollector<long>& GetCollector() {return iCollector;}
private:
  StatsCollector<long>  iCollector;
  bool iIsFirstTime;
  Milli_t iLastFrameTime;
};

bool  StatsBuffer::Update()
{
  static bool iIsFirstTime = true;
  static Milli_t lastFrameTime = Milliseconds();
  if (iIsFirstTime) 
    iIsFirstTime = false;
  else {
    Milli_t newTime = Milliseconds();
    iCollector.Record(newTime - lastFrameTime);
    lastFrameTime = newTime;
  }
  return iBuffer->Update();
}

//---------------------------------------------------------------
// Output Device Parsing
//---------------------------------------------------------------

string gOutputDeviceArg;

LBuffer* CreateOutputBuffer(csref devstr, string* errmsg) {
    LBuffer* buffer = ComboBuffer::Create(devstr, errmsg);
    if (! buffer) return NULL;
    ComboBuffer* cbuffer = dynamic_cast<ComboBuffer*>(buffer);
    if (cbuffer) {// should always be true
        switch (cbuffer->GetNumBuffers())
        {
        case 0: //Should never happen?
            if (errmsg) *errmsg = "Missing device descriptor. Empty?";
            return NULL;
        case 1: // No combo, get rid of top-level combo
            buffer = cbuffer->PopLastBuffer();
            delete cbuffer;
        }
    }
    return buffer;
}

string DeviceCallback(csref name, csref val) {
    gOutputDeviceArg = TrimWhitespace(val);
    return "";
}

DefOption(dev, DeviceCallback, "deviceInfo", "specifies the output device, optionally prefaced by filterInfos.", NULL);

string GetDevHelp() {
    string r, tmp;
    r = "Environment variable LDEV is used if --dev is missing.\n"
        "\nfilterInfo is pipe-separated list of filter_type:parameters\n"
        "   Examples: random, skip2|flip\n"
        "The supported filter types are: \n  ";
    tmp = LBufferType::GetDocumentation(true);
    r += StrReplace(tmp, "\n", "\n  ") + "\n";

    r += "\ndeviceInfo is device_type:arguments, optionally preceded by filterInfos\n"
         "   Examples: ck:192.168.5.5(50), flip|console\n"
        "The supported device types are: \n  ";
    tmp = LBufferType::GetDocumentation(false);
    r += StrReplace(tmp, "\n", "\n  ") + "\n";
    return r;
}

ProgramHelp PHDevHelp(GetDevHelp);

//---------------------------------------------------------------
// Filters
//---------------------------------------------------------------


bool AddFilter(csref filterDescription, string* errmsg) {
    LFilter* filter = LFilter::Create(filterDescription, errmsg);
    if (! filter) return false;
    AddFilter(filter);
    return true;
}

void AddFilter(LFilter* filter) {
    gFilters.push_back(filter);
}

bool PrependFilter(csref filterDescription, string* errmsg) {
    LFilter* filter = LFilter::Create(filterDescription, errmsg);
    if (! filter) return false;
    PrependFilter(filter);
    return true;
}

void PrependFilter(LFilter* filter) {
    gFilters.push_front(filter);
}

//---------------------------------------------------------------
// Output pipeline
//---------------------------------------------------------------

void InitializeOutputPipeline() { 
    LBuffer* lastBuffer = gOutputBuffer;
    for (int i = ((int) gFilters.size()) - 1; i >= 0; --i) {
        gFilters[i]->SetBuffer(lastBuffer);
        lastBuffer = gFilters[i];
    }
    gOutput.SetBuffer(lastBuffer);
}

//---------------------------------------------------------------
// Various command line options
//---------------------------------------------------------------

bool        gVerbose        = false;

string VerboseCallback(csref name, csref val) {
    gVerbose = true;
    return "";
    }
DefOptionBool(verbose, VerboseCallback, "enables verbose status messages");

//------------

string FilterCallback(csref name, csref valarg) {
    string value = TrimWhitespace(valarg);
    if (value.empty()) return "";
    string errmsg;
    AddFilter(valarg, &errmsg);
    return errmsg;
}

DefOption(filter, FilterCallback, "filterInfo", "adds post-rendering filters.", NULL);

//------------
string ColorDefaultCallback(csref name)
{
    return CurrentColorModeAsString();
}

string ColorCallback(csref name, csref val) {
    string errmsg;
    ParseColorMode(val, &errmsg);
    return errmsg;
}

DefOption(color, ColorCallback, "colormode", "chooses a color scheme.", ColorDefaultCallback);

//------------
float gRunTime        = -1.0; // any number below zero means run forever

string TimeCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gRunTime))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    if (gRunTime < 0)
        return "--" + name + " cannot be less than zero.";
    return "";
}

DefOption(time, TimeCallback, "duration", "specifies the running time in seconds. By default, animation continues forever.", NULL);

//------------
float gRate     = 1.0;
int gRateMode  = kRatePositive;

string RateDefaultCallback(csref name) {
    return FltToStr(gRate);
    }

string RateCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gRate))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    switch (gRateMode) {
        case kRatePositive:
            if (gRate > 0)  break;
            else return "--" + name + " must be positive.";
        case kRateNonZero:
            if (gRate != 0)  break;
            else return "--" + name + " must be non-zero.";
        case kRateAny: break;
    }
    return "";
}

void SetRateMode(RateMode_t mode) {
    gRateMode = mode;
}

void SetRateDoc(csref docString) {
    Option opt;
    if (Option::GetOption(&opt, "rate")) {
        opt.SetDoc(docString);
        Option::ReplaceOption(opt);
    }
}

DefOption(rate, RateCallback, "rateval", "specifies the relative speed of the effect.", RateDefaultCallback);

//------------

float       gFade           = 0.0;  // Fade in/out time in seconds
string FadeDefaultCallback(csref name) {
    return FltToStr(gFade);
    }

string FadeCallback(csref name, csref val) {
    string errmsg;
    if (! StrToFlt(val, &gFade))
        return "The --" + name + " parameter, " + val + ", was not a number.";
    if (gFade < 0)
        return "--" + name + " cannot be less than zero.";
    return "";
}

DefOption(fade, FadeCallback, "fadeduration", "sets the fade in and out in time seconds.", FadeDefaultCallback);

//---------------------------------------------------------------
// Startup
//---------------------------------------------------------------
StatsBuffer* gStatsBuffer = NULL;

void ErrorExit(csref msg) {
    cerr << ProgramHelp::GetString(kPHprogram) << ": " << msg << endl;
    cerr << ProgramHelp::GetUsage() << endl;
    exit(EXIT_FAILURE);
    }

  void Startup(int *argc, char** argv, int minPositionalArgs, int maxPositionalArgs) {
    // Set up time variables
    gStartTime  = gTime = Milliseconds();

    // Parse the argument list
    Option::ParseArglist(argc, argv, minPositionalArgs, maxPositionalArgs);

    // Create the OutputBuffer
    if (gOutputDeviceArg.empty()) 
		gOutputDeviceArg = TrimWhitespace(GetEnvStr("LDEV"));
    if (gOutputDeviceArg.empty())
        ErrorExit("No output device was specified via --dev or the LDEV environment variable.");

    string errmsg;
    gOutputBuffer = CreateOutputBuffer("[" + gOutputDeviceArg + "]", &errmsg);
    if (!gOutputBuffer) ErrorExit("Error initializing output device: " + errmsg);
    if (gOutputBuffer->HasError())
        ErrorExit(gOutputBuffer->GetLastError());
    if (gOutputBuffer->GetCount() == 0)
        ErrorExit("Empty output device.");

    // Add StatsBuffer if desired
    if (gVerbose) {
        gStatsBuffer = new StatsBuffer();
        PrependFilter(gStatsBuffer);
    }

    // Now create the output pipeline (gOutput)
    InitializeOutputPipeline();

    if (gVerbose)
	    cout << "Output Pipeline: " << gOutput.GetDescription() << endl;

    CtrlCHandler::Add(CtrlCHandler);
}

void Cleanup(bool eraseAtEnd)
{
    CtrlCHandler::Delete(CtrlCHandler);
    if (eraseAtEnd)
      {
        // Clear the lights
        gOutput.Clear();
        gOutput.Update();
      }

    if (gVerbose)
      {
	   cout << "Framerate Statistics" << endl;
	   cout << "  Target time between frames: " << gFrameDuration << "ms" << endl;
	   cout << "  Actual " << gStatsBuffer->GetCollector().GetSummaryString() << endl;
       cout << "  Samples: " << gStatsBuffer->GetCollector().GetSamplesString() << endl;
      }
}

void RunOnce(Lgroup& objGroup, GroupCallback_t groupfcn)
{
  gTime = Milliseconds();
  gOutput.Clear();
  if (groupfcn) groupfcn(&objGroup);
  objGroup.RenderAll(gTime, gProcs, &gOutput);
  gOutput.Update();
}

void Run(Lgroup& objGroup, L::ObjCallback_t objfcn, L::GroupCallback_t groupfcn)
{
    // Set up end time variables
    gEndTime = 0; // Runs forever
    if (gRunTime >= 0)
        gEndTime = gStartTime + (Milli_t) (gRunTime * 1000 + .5);

    // Handle fade effect (this should really be automated from a list of filters added during arg parsing
    if (gFade > 0) {
        gProcs.AddProc(LprocFadeIn(true,gStartTime, gStartTime + gFade * 1000));
        if (gEndTime)
            gProcs.AddProc(LprocFadeOut(true,gEndTime - gFade * 1000, gEndTime - 1));
    }

    // If callback, install it
    if (objfcn) {
        LprocFcn proc(objfcn);
        gProcs.PrependProc(proc);
    }

    // Main loop
    while (true) {
      if (gTerminateNow) break;  // Exit if we're done

	  RunOnce(objGroup, groupfcn);

	  Milli_t currentTime = Milliseconds();
	  if (gEndTime != 0 && MilliLE(gEndTime, currentTime)) break;
	  
	  Milli_t elapsedSinceFrameStart = MilliDiff(currentTime, gTime);
	  if (gFrameDuration > elapsedSinceFrameStart)
		SleepMilli(gFrameDuration - elapsedSinceFrameStart);
    }
}


}; // namespace L

