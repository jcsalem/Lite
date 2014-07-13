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
// Makes one end red and the other end green

class PlaneNavigationBuffer : public LBuffer
{
public:
  PlaneNavigationBuffer(LBuffer* buffer) 
    : LBuffer(), iBuffer(buffer), iNumPixels(-1) {}
    virtual ~PlaneNavigationBuffer() {}

    virtual int     GetCount() const {return iBuffer->GetCount();}
    virtual string  GetDescriptor() const {return "plane|" + iBuffer->GetDescriptor();}
    virtual bool    Update() {return iBuffer->Update();}

protected:
  virtual RGBColor&   GetRawRGB(int idx);

private:
    LBuffer* iBuffer;
    int iNumPixels;
};

#include <iostream>
RGBColor& PlaneNavigationBuffer::GetRawRGB(int idx)
{
  static RGBColor temp[1000];

  int numPixels = (iNumPixels < 0) ? 10 : iNumPixels;
  temp[idx].r = 255; temp[idx].g = 1.0; temp[idx].b = 1.0;
  return temp[idx];
#if 0
  if (idx < numPixels) {temp = GREEN; cout << "GREEN" << endl; temp.g = 255.0; return temp;}
  else if (idx >= iBuffer->GetCount() - numPixels) {temp = RED; return temp;}
  else return iBuffer->GetRawRGB(idx);
#endif
}

LBuffer* PlaneNavigationBufferCreate(csref descStr, LBuffer* buffer, string* errmsg)
{
    return new PlaneNavigationBuffer(buffer);
}

DEFINE_LBUFFER_FILTER_TYPE(plane, PlaneNavigationBufferCreate, "plane:N",
        "Adds green and red navigation lites for the last N pixels (default 10)");

