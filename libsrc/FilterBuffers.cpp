// Implements various output filters

#include "utils.h"
#include "LBuffer.h"
#include "FilterBuffers.h"
#include <algorithm>

// Dummy function to force this file to be linked in.
void ForceLinkFilters() {}

//-----------------------------------------------------------------------------
// ReverseBuffer
//-----------------------------------------------------------------------------
// Flips the order of lights in the buffer
class ReverseBuffer : public LBuffer
{
public:
    ReverseBuffer(LBuffer* buffer) : LBuffer(), iBuffer(buffer) {}
    virtual ~ReverseBuffer() {}

    virtual int     GetCount() const {return iBuffer->GetCount();}
    virtual string  GetDescriptor() const {return "flip:" + iBuffer->GetDescriptor();}
    virtual bool    Update() {return iBuffer->Update();}

protected:
    virtual RGBColor&   GetRawRGB(int idx) {idx = iBuffer->GetCount() - idx - 1; return iBuffer->GetRawRGB(idx);}

private:
    LBuffer* iBuffer;
};

LBuffer* ReverseBufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
    return new ReverseBuffer(buffer);
}

DEFINE_LBUFFER_FILTER_TYPE(flip, ReverseBufferCreate, "flip",
        "Reverses the order of pixels");

//-----------------------------------------------------------------------------
// LBufferMap -- Abstract class for any type that uses a map
//-----------------------------------------------------------------------------

LBufferMap::LBufferMap(LBuffer* buffer) : LBuffer(), iBuffer(buffer)
{
    int len = iBuffer->GetCount();
    iMap = vector<int>(len);
    for (int i = 0; i < len; ++i) {
        iMap[i] = i;
    }
}

//-----------------------------------------------------------------------------
// RandomizedBuffer
//-----------------------------------------------------------------------------
// Randomizes the order of a buffer.
class RandomizedBuffer : public LBufferMap
{
public:
    RandomizedBuffer(LBuffer* buffer) : LBufferMap(buffer) {RandomizeMap();}
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
class Skip2Buffer : public LBufferMap
{
public:
    Skip2Buffer(LBuffer* buffer);
    virtual ~Skip2Buffer() {}
    virtual string  GetDescriptor() const {return "skip2:" + iBuffer->GetDescriptor();}
};

Skip2Buffer::Skip2Buffer(LBuffer* buffer) : LBufferMap(buffer)
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

class PlaneNavigationBuffer : public LBuffer
{
public:
  PlaneNavigationBuffer(LBuffer* buffer, int pixelWidth = gDefaultPlaneNavigationWidth) 
    : LBuffer(), iBuffer(buffer), iNumPixels(pixelWidth) {}
  virtual ~PlaneNavigationBuffer() {}

  virtual int     GetCount() const {return max(iBuffer->GetCount() - 2 * iNumPixels, 0);}
  virtual string  GetDescriptor() const;
  virtual bool    Update();

protected:
  virtual RGBColor&   GetRawRGB(int idx);

private:
  LBuffer*   iBuffer;
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
  if (idx >= count) return iBuffer->GetRawRGB(max(count-1,0)); // Only happens if iNumPixels is too big
  else return iBuffer->GetRawRGB(idx);
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

