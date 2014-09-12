// Implements various output filters

#include "utils.h"
#include "MapFilters.h"
#include "LFramework.h"
#include <cmath>
#include <algorithm>  // random_shuffle
#include "utilsParse.h"

void ForceLinkMapFilters() {} // LFilters.cpp refers to this to force linking of this file into all binaries

//-----------------------------------------------------------------------------
// MapFilter -- Abstract class for any type that uses a map
//-----------------------------------------------------------------------------

// Default 1 to 1 map
void MapFilter::InitializeMap() {
  for (int i = 0; i < iMap.size(); ++i) 
    iMap[i] = i;
}

//-----------------------------------------------------------------------------
// ShiftFilter -- Rotates the output a fixed amount
//-----------------------------------------------------------------------------

string ShiftFilter::GetDescriptor() const {
  return "shift:" + IntToStr(iOffset);
}

void ShiftFilter::InitializeMap() {
  int len = iMap.size();
  for (int i = 0; i < len; ++i) {
    iMap[i] = (i + iOffset) % len;
    }
}

LFilter* ShiftFilterCreate(cvsref params, string* errmsg) {
    int offset = 0;
    if (! ParamListCheck(params, "shift", errmsg, 0, 1)) return NULL;
    if (! ParseOptionalParam(&offset, params, 0, "shift offset", errmsg)) return NULL;
    return new ShiftFilter(offset);
}

DEFINE_LBUFFER_FILTER_TYPE(shift, ShiftFilterCreate, "shift[:amount]",
        "Rotates the order of the pixels by the specified amount (which should be an integer)");

//-----------------------------------------------------------------------------
// RotateFilter -- Rotates the output an amount that changes over time
//-----------------------------------------------------------------------------

const float RotateFilter::kDefaultSpeed = 1.0;

string RotateFilter::GetDescriptor() const {
  return "rotate:" + FltToStr(iSpeed); 
}

bool RotateFilter::Update() {
  // This effectively updates the offset AFTER the update has been done
  double timeDiff = MilliDiff(L::gTime, L::gStartTime);
  double len = GetCount();
  double doffset = (timeDiff / 1000.0) * len * iSpeed; 
  int offset = fmod((doffset + .5), (double) len);
  SetOffset(offset);
  return iBuffer->Update();
}

LFilter* RotateFilterCreate(cvsref params, string* errmsg)
{
    float speed = RotateFilter::kDefaultSpeed;
    if (! ParamListCheck(params, "rotate", errmsg, 0, 1)) return NULL;
    if (! ParseOptionalParam(&speed, params, 0, "rotate speed", errmsg)) return NULL;
    return new RotateFilter(speed);
}

DEFINE_LBUFFER_FILTER_TYPE(rotate, RotateFilterCreate, "rotate[:speed]",
        "Rotates the display over time. Speed is the number of full rotations per second (default is " + FltToStr(RotateFilter::kDefaultSpeed) + ")");

//-----------------------------------------------------------------------------
// BounceFilter -- Rotates the output an amount that changes over time
//-----------------------------------------------------------------------------

const float BounceFilter::kDefaultSpeed = 1.0;

string BounceFilter::GetDescriptor() const {
  return "bounce(" + FltToStr(iSpeed) + "," + FltToStr(iBounceAfter) + "," + FltToStr(iBounceIncr) + ")";
}

LFilter* BounceFilterCreate(cvsref params, string* errmsg)
{
    float speed = BounceFilter::kDefaultSpeed;
    float bounceAfter = 1;
    float bounceIncr = 0;
    if (! ParamListCheck(params, "bounce", errmsg, 0, 3)) return NULL;
    if (! ParseOptionalParam(&speed,       params, 0, "bounce speed", errmsg)) return NULL;
    if (! ParseOptionalParam(&bounceAfter, params, 1, "bounce after", errmsg)) return NULL;
    if (! ParseOptionalParam(&bounceIncr,  params, 2, "bounce increment", errmsg)) return NULL;
    return new BounceFilter(speed, bounceAfter, bounceIncr);
}

bool BounceFilter::Update() {
  // This effectively updates the offset AFTER the update has been done
  double timeDiff = MilliDiff(L::gTime, L::gStartTime);
  double len = GetCount();
  double doffset = (timeDiff / 1000.0) * len * iSpeed; 
  double bounceLen = len * iBounceAfter * 2 - 1;
  int midPoint = (bounceLen + 1.0) / 2.0;
  int offset = fmod((doffset + .5), bounceLen);
  if (offset >= midPoint) offset = bounceLen - offset;
  SetOffset(offset);
  return iBuffer->Update();
}


DEFINE_LBUFFER_FILTER_TYPE(bounce, BounceFilterCreate, "bounce(speed,after,jitter)",
        "Bounces the display back-and-forth over time. Speed is bounces per second (default is " + FltToStr(RotateFilter::kDefaultSpeed) + 
          "). After is in lengths (default 1). Jitter is in lenghts (default 0).");

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
    MapFilter::InitializeMap();
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

