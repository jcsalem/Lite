// Implements various output filters

#include "utils.h"
#include "MapFilters.h"
#include "LFramework.h"
#include <cmath>
#include "utilsParse.h"

void ForceLinkMapFilters() {} // This is reference in LFilters.cpp to force the link of this file into all binaries

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

string LShiftFilter::GetDescriptor() const {
  return "shift:" + IntToStr(iOffset);
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

const float LRotateFilter::gDefaultSpeed = 2.0;

string LRotateFilter::GetDescriptor() const {
  if (iBounceAfter == 0) 
    return "rotate:" + FltToStr(iSpeed); 
  else 
    return "bounce(" + FltToStr(iSpeed) + "," + FltToStr(iBounceAfter) + ")";
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
    float speed = LRotateFilter::gDefaultSpeed;
    if (! ParamListCheck(params, "rotate", errmsg, 0, 1)) return NULL;
    if (! ParseOptionalParam(&speed, params, 0, "rotate speed", errmsg)) return NULL;
    return new LRotateFilter(speed);
}

DEFINE_LBUFFER_FILTER_TYPE(rotate, LRotateFilterCreate, "rotate[:speed]",
        "Rotates the display over time. Speed is the number of full rotations per second (default is 1)");

LFilter* LBounceFilterCreate(cvsref params, string* errmsg)
{
    float speed = LRotateFilter::gDefaultSpeed;
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

