// Implements various output filters

#include "utils.h"
#include "LFramework.h"
#include "LBuffer.h"
#include "FilterBuffers.h"
#include "utilsRandom.h"
#include "utilsParse.h"
#include <algorithm>
#include <iostream> // for debugging

// Dummy function to force this file to be linked in.
void ForceLinkFilters() {}

//-----------------------------------------------------------------------------
// LMapFilter -- Abstract class for any type that uses a map
//-----------------------------------------------------------------------------

LMapFilter::LMapFilter(LBuffer* buffer) : LFilter(buffer)
{
    int len = iBuffer->GetCount();
    iMap = vector<int>(len);
    for (int i = 0; i < len; ++i) {
        iMap[i] = i;
    }
}

//-----------------------------------------------------------------------------
// ReverseBuffer
//-----------------------------------------------------------------------------
// Flips the order of lights in the buffer
class ReverseBuffer : public LFilter
{
public:
    ReverseBuffer(LBuffer* buffer) : LFilter(buffer) {}
    virtual ~ReverseBuffer() {}

    virtual string  GetDescriptor() const {return "flip:" + iBuffer->GetDescriptor();}

protected:
  virtual RGBColor&   GetRawRGB(int idx) {idx = iBuffer->GetCount() - idx - 1; return iBuffer->GetRawRGB(idx);}
};

LBuffer* ReverseBufferCreate(cvsref params, LBuffer* buffer, string* errmsg)
{
    if (! ParamListCheck(params, "reverse", errmsg, 0)) return NULL;
    return new ReverseBuffer(buffer);
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
    RandomizedBuffer(LBuffer* buffer) : LMapFilter(buffer) {RandomizeMap();}
    virtual ~RandomizedBuffer() {}
    virtual string  GetDescriptor() const {return "random:" + iBuffer->GetDescriptor();}

    void RandomizeMap();

};

void RandomizedBuffer::RandomizeMap()
{
    random_shuffle(iMap.begin(), iMap.end());
}


LBuffer* RandomizedBufferCreate(cvsref params, LBuffer* buffer, string* errmsg)
{
   if (! ParamListCheck(params, "random", errmsg, 0)) return NULL;
   return new RandomizedBuffer(buffer);
}

DEFINE_LBUFFER_FILTER_TYPE(random, RandomizedBufferCreate, "random",
                    "Randomizes the order of pixels in the device.");

//-----------------------------------------------------------------------------
// Skip2Buffer
//-----------------------------------------------------------------------------
// Interleaves the pixel (e.g., skips every other).  It would be nice to make this more generic (e.g., skip3, skip4, etc.)
class Skip2Buffer : public LMapFilter
{
public:
    Skip2Buffer(LBuffer* buffer);
    virtual ~Skip2Buffer() {}
    virtual string  GetDescriptor() const {return "skip2:" + iBuffer->GetDescriptor();}
};

Skip2Buffer::Skip2Buffer(LBuffer* buffer) : LMapFilter(buffer)
{
    int len = buffer->GetCount();
    int idx = 0;
    for (int i = 0; i < len; i += 2)
        iMap[idx++] = i;
    for (int i = 1; i < len; i += 2)
        iMap[idx++] = i;
}


LBuffer* Skip2BufferCreate(cvsref params, LBuffer* buffer, string* errmsg)
{
    if (! ParamListCheck(params, "skip", errmsg, 0)) return NULL;
    return new Skip2Buffer(buffer);
}

DEFINE_LBUFFER_FILTER_TYPE(skip2, Skip2BufferCreate, "skip2",
                    "Interleaves the order of pixels.");

//-----------------------------------------------------------------------------
// PlaneNavigationBuffer
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

const int gDefaultPlaneNavigationWidth = 9;

class PlaneNavigationBuffer : public LFilter
{
public:
  PlaneNavigationBuffer(LBuffer* buffer, int pixelWidth = gDefaultPlaneNavigationWidth) 
    : LFilter(buffer), iNumPixels(pixelWidth) {}
  virtual ~PlaneNavigationBuffer() {}

  virtual int     GetCount() const {return max(iBuffer->GetCount() - 2 * iNumPixels, 0);}
  virtual string  GetDescriptor() const;
  virtual bool    Update();

protected:
  virtual RGBColor&   GetRawRGB(int idx);

private:
  int 	     iNumPixels;
};

string PlaneNavigationBuffer::GetDescriptor() const
{
  string desc = "plane";
  if (iNumPixels != gDefaultPlaneNavigationWidth) desc += "(" + IntToStr(iNumPixels) + ")";
  desc += "|" + iBuffer->GetDescriptor();
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

LBuffer* PlaneNavigationBufferCreate(cvsref params, LBuffer* buffer, string* errmsg)
{
  if (! ParamListCheck(params, "plane", errmsg, 0, 1)) return NULL;

  int numPixels = gDefaultPlaneNavigationWidth;
  if (! ParseOptionalParam(&numPixels, params, 0, "plane pixel width", errmsg, 0)) return NULL;
  return new PlaneNavigationBuffer(buffer, numPixels);
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
  SparkleBuffer(LBuffer* buffer, csref params);
  virtual ~SparkleBuffer() {
     if (iState) delete[] iState; iState = NULL; 
     if (iTimeToChange) delete[] iTimeToChange; iTimeToChange = NULL;
  }

  virtual string  GetDescriptor() const;
  virtual bool    Update();
  void SetColor(Color* color) {color->ToRGBColor(&iColor);}
  void SetParameters(float fraction, float onDuration, float sigma);

private:
  string     iParamString;
  RGBColor   iColor;          // Should eventually be a ColorMode
  float      iOnDuration;     // Fraction of pixels that are sparkling at any one time
  float      iOffDuration;    // Time between a single pixel's sparkle
  float      iSigma;          // Std. deviation of the on/off time (scale by duration)
  bool       iPixelsNeedInit; // true when iState needs initialization
  bool*      iState;
  Milli_t*   iTimeToChange;
  Milli_t    GetSparkleDuration(bool newState);
  void       InitializePixels();
};

string SparkleBuffer::GetDescriptor() const
{
  string desc = "sparkle";
  if (! iParamString.empty()) desc += "(" + iParamString + ")";
  desc += ":" + iBuffer->GetDescriptor();
  return desc;
}

void SparkleBuffer::SetParameters(float onFraction, float onDuration, float sigma)
{
  iOnDuration = onDuration;
  if (onFraction < .001) onFraction = .001;
  iOffDuration = onDuration / onFraction - onDuration;
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

SparkleBuffer::SparkleBuffer(LBuffer* buffer, csref paramString)
  : LFilter(buffer), iColor(WHITE), iParamString(paramString), iPixelsNeedInit(true)
{
  int     count = GetCount();
  iState 	= new bool[count];
  iTimeToChange = new Milli_t[count];
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

LBuffer* SparkleBufferCreate(cvsref params, LBuffer* buffer, string* errmsg)
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
  SparkleBuffer* newbuf =  new SparkleBuffer(buffer, ParamListToString(params));
  newbuf->SetParameters(fraction, duration, sigma);
  if (color) {
    newbuf->SetColor(color);
    delete color;
  }
  return newbuf;
}

DEFINE_LBUFFER_FILTER_TYPE(sparkle, SparkleBufferCreate, "sparkle([color],[frac],[ontime],[sigma])",
        "Sparkles the output. Frac is the fraction of time that should be spent sparkling");

