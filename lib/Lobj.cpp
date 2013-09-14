#include "Lobj.h"
#include "Color.h"
#include "LBuffer.h"
#include "Lproc.h"
#include <math.h>
#include <iostream>

bool gAntiAlias = true;

#define MAX_OBJS 100
LobjOld gAllLobjOlds[MAX_OBJS];
short gLobjOldCount = 0;

void LobjOld::Clear() {
  pos = 0;
  color = BLACK;
  speed = 0;
  velocity = 0;
}


void LobjOld::Render(LBuffer* buffer) const {
  if (! gAntiAlias) {
    // No anti-aliasing
    int bpos = pos + .5;
    if (bpos < 0 || bpos >= buffer->GetCount()) return;
    buffer->AddRGB(bpos, color);
  } else {
    float floorpos = floor(pos);
    // High-level clip (avoid aliasing at extreme values)
    if (floorpos <= -1.0 || floorpos >= (float) buffer->GetCount()) return;

    float efrac = pos - floorpos;
    float bfrac = 1.0 - efrac;
    short bpos = floorpos;
    short epos = bpos + 1;

    RGBColor tcolor;
    if (bfrac > 0 && bpos >= 0 && bpos < buffer->GetCount()) {
      tcolor = color;
      tcolor *= bfrac;
      buffer->AddRGB(bpos, tcolor);
     }

    if (efrac > 0 && epos >= 0 && epos < buffer->GetCount()) {
      tcolor = color;
      tcolor *= efrac;
      buffer->AddRGB(epos, tcolor);
    }
  }
}


// Global Operations
void LobjOld::RenderAll(LBuffer* buffer) {
  for (int i = 0; i < gLobjOldCount; ++i) {
    gAllLobjOlds[i].Render(buffer);
  }
}

short LobjOld::GetNum(void) {
  return gLobjOldCount;
}

short LobjOld::GetMaxNum(void) {
  return MAX_OBJS;
}

LobjOld* LobjOld::GetNth(short index) {
  if (index >= gLobjOldCount || index < 0) return 0;
  return gAllLobjOlds + index;
}

LobjOld* LobjOld::Alloc(void) {
  if (gLobjOldCount >= MAX_OBJS) return 0;
  LobjOld* retval = &(gAllLobjOlds[gLobjOldCount++]);
  retval->Clear();
  return retval;
}

bool LobjOld::Free(LobjOld* lptr) {
  int index = lptr - gAllLobjOlds;
  if (index < 0 || index >= gLobjOldCount) return false;
  for (int i = index; i < gLobjOldCount-1; ++i)
    gAllLobjOlds[i] = gAllLobjOlds[i+1];
  --gLobjOldCount;
  return true;
}

void LobjOld::Map(LobjOldMapFcn_t mapfcn) {
  for (int i = 0; i < gLobjOldCount; ++i)
    mapfcn(gAllLobjOlds + i);
}

//----------------------------------------------------------------------
// Rendering
//----------------------------------------------------------------------

namespace { // These are internal to just this file
// Just used for default values
LprocList gDummyFilterList;

void ObjRender(const Lobj* obj, LBuffer* buffer, const LprocList& filters) {
    // Only supports 1D rendering at the moment
    // Position is the middle of the object

    RGBColor rgb = obj->GetCurrentColor();
    rgb = filters.Apply(rgb, obj->lastTime);
    float   width   = obj->width;
    Lxy     pos     = obj->pos;
    //cout << "Render: " << rgb.ToString() << " at " << pos.x << "," << pos.y << endl;

    if (width == 0 || !gAntiAlias) {
        // Round to nearest pixel to beginning of object
        int ipos = pos.x - width/2.0 + .5;
        int iwidth = width == 0? 1 : (width + .5);
        for (int i = 0; i < iwidth; ++i, ++ipos)
            buffer->AddRGB(ipos, rgb);
    } else {
    // Non-zero width AND anti-aliasing.
    float left = pos.x - width / 2.0 + .5;  // The .5 insures that a sparkle of width 1 displays as a single pixel
    float lfloor = floor(left);
    float lfrac = left - lfloor;
    int lpos = lfloor;
    float right = pos.x + width / 2.0 + .5;
    float rfloor = floor(right);
    float rfrac = 1.0 - (right - rfloor);
    int rpos = rfloor;
    if (lpos == rpos) lfrac *= rfrac;  // May need more thought

    // Write out first pixel
    buffer->AddRGB(lpos, rgb * lfrac);
    // Write out middle pixels
    for (int ipos = lpos + 1; ipos < rpos; ++ipos)
        buffer->AddRGB(ipos, rgb);
    // Write out last pixel
    if (lpos != rpos)
        buffer->AddRGB(rpos, rgb * rfrac);
    }
}

}; // Local namespace

//----------------------------------------------------------------------
// Lobj Operations
//----------------------------------------------------------------------

void Lobj::Move(Milli_t newTime) {
    float timeDiff = MilliDiff(newTime, lastTime);
    pos += speed * timeDiff/1000;
    lastTime = newTime;
}

void Lobj::Wrap(const Lxy& minBound, const Lxy& maxBound) {
    Lxy range = maxBound - minBound;

    float newX = fmod(pos.x - minBound.x, range.x) + minBound.x;
    float newY = fmod(pos.y - minBound.y, range.y) + minBound.y;
    //cout << "Wrap: " << pos.x << " to " << minBound.x << "," << maxBound.x << "  Result: " << newX << endl;
    pos = Lxy(newX, newY);
}

void Lobj::Bounce(const Lxy& minBound, const Lxy& maxBound) {
    if      (pos.x <  minBound.x) {speed.x = -speed.x; pos.x = 2 * minBound.x - pos.x;}
    else if (pos.x >= maxBound.x) {speed.x = -speed.x; pos.x = 2 * maxBound.x - pos.x;}
    if      (pos.y <  minBound.y) {speed.y = -speed.y; pos.y = 2 * minBound.y - pos.y;}
    else if (pos.y >= maxBound.y) {speed.y = -speed.y; pos.y = 2 * maxBound.y - pos.y;}
}

bool Lobj::IsOutOfBounds(const Lxy& minBound, const Lxy& maxBound) const {
    return pos.x < minBound.x && pos.x > maxBound.x && pos.y < minBound.y && pos.y > maxBound.y;
}

string Lobj::GetDescription(bool verbose) const {
    string retval = "[" + DblToStr(pos.x) + "," + DblToStr(pos.y) + "," + color.ToString() + "]";
    return retval;
}

//----------------------------------------------------------------------
// Group functions
//----------------------------------------------------------------------


void Lgroup::Add(Lobj* obj) {
    if (obj) iObjs.push_back(obj);
}

void Lgroup::Add(int num, Lobj::AllocFcn_t fcn, const void* info) {
    for (int i = 0; i < num; ++i)
        Add(fcn(i, info));
}

bool Lgroup::Free(Lobj* obj) {
    for (iterator i = begin(); i != end(); ++i) {
        if (*i == obj) {
            iObjs.erase(i);
            delete obj;
            return true;
        }
    }
    return false;
}

void Lgroup::FreeAll() {
    for (iterator i = begin(); i != end(); ++i)
        delete *i;
    iObjs.clear();
}

void Lgroup::FreeIf(Lobj::FreeIfFcn_t fcn, const void* info) {
    for (iterator i = begin(); i != end();) {
        if (fcn(*i, info)) {
            delete *i;
            iObjs.erase(i);
        } else
            ++i;
    }
}

//void Lgroup::FreeIfOutOfBounds(Lxy MinBound, Lxy maxBound) const;

// Common functions

void Lgroup::RenderAll(LBuffer* buffer) const {
    RenderAll(buffer, gDummyFilterList);
}

void Lgroup::RenderAll(LBuffer* buffer, const LprocList& filters) const {
    for (const_iterator i = begin(); i != end(); ++i)
        ObjRender(*i, buffer, filters);
}

void Lgroup::MoveAll(Milli_t newTime) const {
    for (const_iterator i = begin(); i != end(); ++i)
        (*i)->Move(newTime);
}

void Lgroup::WrapAll(const Lxy& minBound, const Lxy& maxBound) const {
    for (const_iterator i = begin(); i != end(); ++i)
        (*i)->Wrap(minBound, maxBound);
}

void Lgroup::BounceAll(const Lxy& minBound, const Lxy& maxBound) const {
    for (const_iterator i = begin(); i != end(); ++i)
        (*i)->Bounce(minBound, maxBound);
}

// Mapping
void Lgroup::Map(Lobj::MapFcn_t mapfcn, const void* info) {
    for (const_iterator i = begin(); i != end(); ++i)
        mapfcn(*i, info);
}

string Lgroup::GetDescription(bool verbose) const {
    string retval;
    retval += IntToStr(GetCount()) + " objects";
    if (verbose) {
        for (const_iterator i = begin(); i != end(); ++i) {
            retval += (i == begin() ? ":" : ",");
            retval += (*i)->GetDescription(verbose);
        }
    }
    return retval;
}
