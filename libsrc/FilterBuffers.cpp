// Implements various output filters

#include "utils.h"
#include "LFramework.h"
#include "LBuffer.h"
#include "FilterBuffers.h"
#include "utilsRandom.h"
#include <algorithm>
#include <cfloat>
//#include <iostream> // for debugging

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

LBuffer* ReverseBufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
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


LBuffer* RandomizedBufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
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


LBuffer* Skip2BufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
    return new Skip2Buffer(buffer);
}

DEFINE_LBUFFER_FILTER_TYPE(skip2, Skip2BufferCreate, "skip2",
                    "Interleaves the order of pixels.");

//-----------------------------------------------------------------------------
// PlaneNavigationBuffer
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

const int gDefaultPlaneNavigationWidth = 10;

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
  if (iNumPixels != gDefaultPlaneNavigationWidth) desc += ":" + IntToStr(iNumPixels);
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

LBuffer* PlaneNavigationBufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
  unsigned numPixels;

  if (descStr.empty())
    return new PlaneNavigationBuffer(buffer);
  if (!StrToUnsigned(descStr, &numPixels))
    {
      if (errmsg) *errmsg = "The argument to \"plane\" must be a non-negative number.";
      return NULL;
    }
  return new PlaneNavigationBuffer(buffer, numPixels);
}

DEFINE_LBUFFER_FILTER_TYPE(plane, PlaneNavigationBufferCreate, "plane:N",
        "Adds green and red navigation lites for the last N pixels (default 10)");

//-----------------------------------------------------------------------------
// SparkleBuffer
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

// These are all in milliseconds
const float gTimeBetweenSparkles = 1000;
const float gTimeBetweenSparklesSigma = 300;
const float gSparkleFraction = .05;

class SparkleBuffer : public LFilter
{
public:
  SparkleBuffer(LBuffer* buffer, const RGBColor& color = WHITE);
  virtual ~SparkleBuffer() {
     if (iState) delete[] iState; iState = NULL; 
     if (iTimeToChange) delete[] iTimeToChange; iTimeToChange = NULL;
  }

  virtual string  GetDescriptor() const;
  virtual bool    Update();

private:
  RGBColor   iColor;
  bool*      iState;
  Milli_t*   iTimeToChange;
};

string SparkleBuffer::GetDescriptor() const
{
  string desc = "sparkle";
  //  if (iNumPixels != gDefaultSparkleWidth) desc += ":" + IntToStr(iNumPixels);
  desc += "|" + iBuffer->GetDescriptor();
  return desc;
}

Milli_t GetSparkleDuration(bool newState)
{
  float mult = newState ? gSparkleFraction : 1.0 - gSparkleFraction;
  float duration = RandomNormalBounded(gTimeBetweenSparkles * mult, gTimeBetweenSparklesSigma * mult * 2, 0.0, 1000000.0);
  return (Milli_t) duration;
}

SparkleBuffer::SparkleBuffer(LBuffer* buffer, const RGBColor& color)
  : LFilter(buffer), iColor(color)
{
  int     count = GetCount();
  iState 	= new bool[count];
  iTimeToChange = new Milli_t[count];

  for (int i = 0; i < count; ++i) {
    bool state = RandomFloat() < gSparkleFraction;
    iState[i] = state;
    iTimeToChange[i] = L::gStartTime + GetSparkleDuration(state)/2;
  }
}

bool SparkleBuffer::Update()
{
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

LBuffer* SparkleBufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
  if (descStr.empty())
    return new SparkleBuffer(buffer);
#if 0
  if (!StrToUnsigned(descStr, &numPixels))
    {
      if (errmsg) *errmsg = "The argument to \"sparkle\" must be a non-negative number.";
      return NULL;
    }
#endif
  return new SparkleBuffer(buffer);
}

DEFINE_LBUFFER_FILTER_TYPE(sparkle, SparkleBufferCreate, "sparkle:[frac],[color]",
        "Sparkles the output. When implemented, frac is the fraction of time that should be spent sparkling");

