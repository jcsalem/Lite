// Implements various output filters

#include "utils.h"
#include "LBuffer.h"
#include "LFilter.h"
#include "LFramework.h"
#include "utilsParse.h"
#include <cmath>
#include "utilsRandom.h"
#include <iostream>

//-----------------------------------------------------------------------------
// Creating a filter
//-----------------------------------------------------------------------------

static LFilter* CreateError(string* errmsgptr, csref msg) {
    if (errmsgptr) *errmsgptr = msg;
    return NULL;
    }

LFilter* LFilter::Create(csref desc, string* errmsg) {
    size_t argpos = desc.find_first_of(":(");

    // Look up the filter
    string name = desc.substr(0,argpos);
    const LBufferType* type = LBufferType::Find(name);
    if (!type || !type->iIsFilter) return CreateError(errmsg, "No filter named: " + name);

    // Parse parameters (this should be shared with LBuffer::Create)
    vector<string> params; 
    string paramsString;
    if (argpos != string::npos) {
        paramsString = desc.substr(argpos+1);
        if (desc[argpos] == '(') {
            if (desc[desc.length()-1] != ')') return CreateError(errmsg, "Missing right parenthesis in arguments to " + name);
            paramsString = paramsString.substr(0, paramsString.length() - 1); // remove trailing parenthesis
        }
        // Parse the parameters
        string errmsg2;
        params = ParseParamList(paramsString, name, &errmsg2);
        if (params.empty() && !errmsg2.empty()) return CreateError(errmsg, errmsg2);
    }
    return type->iFilterCreateFcn(params, errmsg);
}

//-----------------------------------------------------------------------------
// LMapFilter -- Abstract class for any type that uses a map
//-----------------------------------------------------------------------------

// Default 1 to 1 map
void LMapFilter::InitializeMap() {
  for (int i = 0; i < iMap.size(); ++i) 
    iMap[i] = i;
}

//-----------------------------------------------------------------------------
// LShiftFilter -- Rotates the output a fixed amount
//-----------------------------------------------------------------------------
class LShiftFilter : public LMapFilter
{
public:
    LShiftFilter(int offset = 0) : iOffset(offset), LMapFilter() {} // Note that it's important to set iOffset first, so it will be set for the call to InitializeMap
    virtual ~LShiftFilter() {}
    virtual string GetDescriptor() const;
    void SetOffset(int offset) {iOffset = offset; InitializeMap();}
    int GetOffset() const {return iOffset;}
    virtual void InitializeMap();

private:
    int iOffset;
};

string LShiftFilter::GetDescriptor() const {
  if (iOffset == 0) return "shift"; 
  else return "shift:" + IntToStr(iOffset);
}

void LShiftFilter::InitializeMap() {
  int len = iMap.size();
  for (int i = 0; i < len; ++i) {
    iMap[i] = (i + iOffset) % len;
    }
}

LFilter* LShiftFilterCreate(cvsref params, string* errmsg) {
    int offset = 0;
    if (! ParamListCheck(params, "shift", errmsg, 0, 1)) return NULL;
    if (! ParseOptionalParam(&offset, params, 0, "shift offset", errmsg)) return NULL;
    return new LShiftFilter(offset);
}

DEFINE_LBUFFER_FILTER_TYPE(shift, LShiftFilterCreate, "shift[:amount]",
        "Rotates the order of the pixels by the specified amount (which should be an integer)");

//-----------------------------------------------------------------------------
// LRotateFilter -- Rotates the output an amount that changes over time
//-----------------------------------------------------------------------------

class LRotateFilter : public LShiftFilter
{
public:
    LRotateFilter(float speed = 1.0, float bounceAfter = 0.0) : LShiftFilter(0), iSpeed(speed), iBounceAfter(bounceAfter) {}
    virtual ~LRotateFilter() {}
    virtual string GetDescriptor() const;
    virtual bool Update();
    void  SetSpeed(float speed) {iSpeed = speed;}
    float GetSpeed() const {return iSpeed;}
    void  EnableBounce() {iBounceAfter = 1;}
    void  SetBounceAfter(float bounceAfter) {iBounceAfter = bounceAfter;}

private:
    float iSpeed;    // Speed of rotation in full buffer lengths per second
    float iBounceAfter; // Number of rotations after which we should rotate back the other way.  Set to 0 for no bound or 1 for bouncing after one full rotation
};

string LRotateFilter::GetDescriptor() const {
  if (iSpeed == 0) return "rotate"; 
  else return "rotate:" + FltToStr(iSpeed);
}

bool LRotateFilter::Update() {
  // This effectively updates the offset AFTER the update has been done
  double timeDiff = MilliDiff(L::gTime, L::gStartTime);
  double len = GetCount();
  double doffset = (timeDiff / 1000.0) * len * iSpeed; 
  int offset;
  if (iBounceAfter == 0.0) 
    offset = fmod((doffset + .5), (double) len);
  else {
    double bounceLen = len * iBounceAfter * 2 - 1;
    int midPoint = (bounceLen + 1.0) / 2.0;
    offset = fmod((doffset + .5), bounceLen);
    if (offset >= midPoint) offset = bounceLen - offset;
  }
  SetOffset(offset);
  return iBuffer->Update();
}

LFilter* LRotateFilterCreate(cvsref params, string* errmsg)
{
    float speed = 1;
    if (! ParamListCheck(params, "rotate", errmsg, 0, 1)) return NULL;
    if (! ParseOptionalParam(&speed, params, 0, "rotate speed", errmsg)) return NULL;
    return new LRotateFilter(speed);
}

DEFINE_LBUFFER_FILTER_TYPE(rotate, LRotateFilterCreate, "rotate[:speed]",
        "Rotates the display over time. Speed is the number of full rotations per second (default is 1)");

LFilter* LBounceFilterCreate(cvsref params, string* errmsg)
{
    float speed = 1;
    float bounceAfter = 1;
    if (! ParamListCheck(params, "bounce", errmsg, 0, 2)) return NULL;
    if (! ParseOptionalParam(&speed, params, 0, "bounce speed", errmsg)) return NULL;
    if (! ParseOptionalParam(&bounceAfter, params, 1, "bounce after", errmsg)) return NULL;
    return new LRotateFilter(speed, bounceAfter);
}

DEFINE_LBUFFER_FILTER_TYPE(bounce, LBounceFilterCreate, "bounce[:speed[,after]]",
        "Bounces the display back-and-forth over time. Speed is the number of bounces per second (default is 1). After is when to bounce.");

//-----------------------------------------------------------------------------
// ReverseBuffer
//-----------------------------------------------------------------------------
// Flips the order of lights in the buffer
class ReverseBuffer : public LFilter
{
public:
    ReverseBuffer() : LFilter() {}
    virtual ~ReverseBuffer() {}
    virtual string  GetDescriptor() const {return "flip";}

protected:
  virtual RGBColor&   GetRawRGB(int idx) {idx = iBuffer->GetCount() - idx - 1; return iBuffer->GetRawRGB(idx);}
};

LFilter* ReverseBufferCreate(cvsref params, string* errmsg)
{
    if (! ParamListCheck(params, "reverse", errmsg, 0)) return NULL;
    return new ReverseBuffer();
}

DEFINE_LBUFFER_FILTER_TYPE(flip, ReverseBufferCreate, "flip",
        "Reverses the order of pixels");

//-----------------------------------------------------------------------------
// RandomizedBuffer
//-----------------------------------------------------------------------------
// Randomizes the order of a buffer.
class RandomizedBuffer : public LMapFilter
{
public:
    RandomizedBuffer() : LMapFilter() {}
    virtual ~RandomizedBuffer() {}
    virtual string  GetDescriptor() const {return "random";}

    virtual void InitializeMap();

};

void RandomizedBuffer::InitializeMap()
{
    LMapFilter::InitializeMap();
    random_shuffle(iMap.begin(), iMap.end());
}


LFilter* RandomizedBufferCreate(cvsref params, string* errmsg)
{
   if (! ParamListCheck(params, "random", errmsg, 0)) return NULL;
   return new RandomizedBuffer();
}

DEFINE_LBUFFER_FILTER_TYPE(random, RandomizedBufferCreate, "random",
                    "Randomizes the order of pixels in the device.");

//-----------------------------------------------------------------------------
// SkipBuffer
//-----------------------------------------------------------------------------
const int kDefaultSkip = 2;

// Interleaves the pixel (e.g., skips every other).
class SkipBuffer : public LMapFilter
{
public:
    SkipBuffer(int skipnum = kDefaultSkip) : LMapFilter(), iSkip(skipnum) {}
    virtual ~SkipBuffer() {}
    virtual int GetCount() {int count = LMapFilter::GetCount(); return count - (count % max(iSkip,1));}
    virtual string  GetDescriptor() const {return iSkip == kDefaultSkip ? "skip" : ("skip:" + IntToStr(iSkip));}
    virtual void InitializeMap();
  private:
    int iSkip;
};

void SkipBuffer::InitializeMap() {
    int len = GetCount();
    int idx = 0;
    int startIndex = 0;
    for (int i = 0; i < len; ++i) {
      iMap[i] = idx;
      idx += iSkip;
      if (idx >= len) {++startIndex; idx = startIndex;}
    }
 }


LFilter* SkipBufferCreate(cvsref params, string* errmsg)
{
    if (! ParamListCheck(params, "skip", errmsg, 0, 1)) return NULL;
    int skipnum = 2;
    if (! ParseOptionalParam(&skipnum, params, 0, "skip amount", errmsg, 1)) return NULL;
    return new SkipBuffer(skipnum);
}

DEFINE_LBUFFER_FILTER_TYPE(skip, SkipBufferCreate, "skip[:amount]",
                    "Interleaves the order of pixels.");

//-----------------------------------------------------------------------------
// PlaneNavigationBuffer
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

const int gDefaultPlaneNavigationWidth = 9;

class PlaneNavigationBuffer : public LFilter
{
public:
  PlaneNavigationBuffer(int pixelWidth = gDefaultPlaneNavigationWidth) 
    : LFilter(), iNumPixels(pixelWidth) {}
  virtual ~PlaneNavigationBuffer() {}

  virtual int     GetCount() const;
  virtual string  GetDescriptor() const;
  virtual bool    Update();

protected:
  virtual RGBColor&   GetRawRGB(int idx);

private:
  int 	     iNumPixels;
};

int PlaneNavigationBuffer::GetCount() const {
  int len = LFilter::GetCount();
  return max(len - 2 * iNumPixels, 0);
}

string PlaneNavigationBuffer::GetDescriptor() const
{
  string desc = "plane";
  if (iNumPixels != gDefaultPlaneNavigationWidth) desc += "(" + IntToStr(iNumPixels) + ")";
  return desc;
}

bool PlaneNavigationBuffer::Update()
 {
   // Force beginning to be red and end to be green
   RGBColor green(GREEN);
   RGBColor red(RED);
   int count = iBuffer->GetCount();
   int stoppos = min(iNumPixels, count);
   for (int i = 0; i < stoppos; ++i)
     iBuffer->SetRGB(i, red);
   stoppos = max(count-iNumPixels,0);
   for (int i = count-1; i >= stoppos;--i)
     iBuffer->SetRGB(i, green);
   return iBuffer->Update();
 }

RGBColor& PlaneNavigationBuffer::GetRawRGB(int idx)
{
  int count = iBuffer->GetCount();
  idx += iNumPixels;
  if (idx >= count) return LFilter::GetRawRGB(max(count-1,0)); // Only happens if iNumPixels is too big
  else return LFilter::GetRawRGB(idx);
}

LFilter* PlaneNavigationBufferCreate(cvsref params, string* errmsg)
{
  if (! ParamListCheck(params, "plane", errmsg, 0, 1)) return NULL;

  int numPixels = gDefaultPlaneNavigationWidth;
  if (! ParseOptionalParam(&numPixels, params, 0, "plane pixel width", errmsg, 0)) return NULL;
  return new PlaneNavigationBuffer(numPixels);
}

DEFINE_LBUFFER_FILTER_TYPE(plane, PlaneNavigationBufferCreate, "plane(N)",
        "Adds green and red navigation lites for the last N pixels (default 10)");

//-----------------------------------------------------------------------------
// SparkleBuffer
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

// The fraction of pixels that are typically on
const float kDefaultSparkleFraction = 0.05;
// This is std dev of a Sparkle's on/off time (this is normalized to a duration of 1)
const float kDefaultSparkleSigma = 0.3; 
// The duration that a sparkle is on in seconds
const float kDefaultSparkleDuration = 0.02; // Sparkles generally turn on for just a single frame

class SparkleBuffer : public LFilter
{
public:
  SparkleBuffer();
  virtual ~SparkleBuffer() {
     if (iState) delete[] iState; iState = NULL; 
     if (iTimeToChange) delete[] iTimeToChange; iTimeToChange = NULL;
  }

  virtual void    SetBuffer(LBuffer* buffer) {LFilter::SetBuffer(buffer); InitializeArrays();}
  virtual string  GetDescriptor() const;
  virtual bool    Update();
  void SetColor(Color* color) {color->ToRGBColor(&iColor);}
  void SetParameters(float fraction, float onDuration, float sigma);

private:
  RGBColor   iColor;          // Should eventually be a ColorMode
  float      iOnFraction;       // Fraction of pixels that are sparkling at any one time
  float      iOnDuration;     // Time that a pixel should be on
  float      iOffDuration;    // Time between a single pixel's sparkle
  float      iSigma;          // Std. deviation of the on/off time (scale by duration)
  bool       iPixelsNeedInit; // true when iState needs initialization
  bool*      iState;
  Milli_t*   iTimeToChange;
  Milli_t    GetSparkleDuration(bool newState);
  void       InitializeArrays();
  void       InitializePixels();
};

string SparkleBuffer::GetDescriptor() const
{
  string desc = "sparkle";
  // TODO: Need to create the string
  return desc;
}

void SparkleBuffer::SetParameters(float onFraction, float onDuration, float sigma)
{
  iOnDuration = onDuration;
  if (onFraction < .001) onFraction = .001;
  iOnFraction = onFraction;
  iOffDuration = iOnDuration / iOnFraction - onDuration;
  iSigma = sigma;
  iPixelsNeedInit = true;
}

Milli_t SparkleBuffer::GetSparkleDuration(bool newState)
{
  float avDur = newState ? iOnDuration : iOffDuration;
  float sigma = avDur * iSigma * 2;
  float duration = RandomNormalBounded(avDur, sigma, 0.0, avDur * 100);
  duration = duration * 1000; // convert to milliseconds
  return (Milli_t) duration;
}

void SparkleBuffer::InitializeArrays() {
  int     count = GetCount();
  iState  = new bool[count];
  iTimeToChange = new Milli_t[count];
  iPixelsNeedInit = true;
}

SparkleBuffer::SparkleBuffer() : LFilter(), iColor(WHITE), iPixelsNeedInit(true)
{
  SetParameters(kDefaultSparkleFraction, kDefaultSparkleDuration, kDefaultSparkleSigma);
}

void SparkleBuffer::InitializePixels()
{
  int     count = GetCount();
  float   totalDuration = iOnDuration + iOffDuration;
  for (int i = 0; i < count; ++i) {
    bool state = RandomFloat(totalDuration) < iOnDuration;
    iState[i] = state;
    iTimeToChange[i] = L::gStartTime + GetSparkleDuration(state)/2;
  }
  iPixelsNeedInit = false;
}

bool SparkleBuffer::Update()
{
  if (iPixelsNeedInit) InitializePixels();

  int     count = GetCount();
  for (int i = 0; i < count; ++i) {
    // Update the states as necessary
    if (L::gTime > iTimeToChange[i])
      {
    	 iState[i] = !iState[i];
	     iTimeToChange[i] = L::gTime + GetSparkleDuration(iState[i]);
      }
    // If the light is sparkled, set it to the sparkle color
    if (iState[i])
      iBuffer->SetRGB(i, iColor);
  }
  return iBuffer->Update();
}

LFilter* SparkleBufferCreate(cvsref params, string* errmsg)
{
  if (! ParamListCheck(params, "sparkle", errmsg, 0, 4)) return NULL;

  // Parse the color
  Color* color = NULL;
  if (! ParseOptionalParam(&color, params, 0, "sparkle color", errmsg)) return NULL;

  // Now get other the parameters;
  float fraction = kDefaultSparkleFraction;
  float duration = kDefaultSparkleDuration;
  float sigma    = kDefaultSparkleSigma;
  
  if (!ParseOptionalParam(&fraction, params, 1, "sparkle fraction", errmsg, 0,  1)) return NULL;
  if (!ParseOptionalParam(&duration, params, 2, "sparkle duration", errmsg, 0, 30)) return NULL;
  if (!ParseOptionalParam(&sigma,    params, 3, "sparkle sigma",    errmsg, 0, 10)) return NULL;
  
  // Create the buffer
  SparkleBuffer* newbuf =  new SparkleBuffer();
  newbuf->SetParameters(fraction, duration, sigma);
  if (color) {
    newbuf->SetColor(color);
    delete color;
  }
  return newbuf;
}

DEFINE_LBUFFER_FILTER_TYPE(sparkle, SparkleBufferCreate, "sparkle([color],[frac],[ontime],[sigma])",
        "Sparkles the output. Frac is the fraction of time that should be spent sparkling");

