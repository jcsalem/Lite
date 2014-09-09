// An LBuffer variant built using ncurses
//

#ifndef MAPFILTERS_H_INCLUDED
#define MAPFILTERS_H_INCLUDED

#include "Config.h"
#include "LBuffer.h"
#include "LFilter.h"

//-----------------------------------------------------------------------------
// Abstract MapFilter type
//-----------------------------------------------------------------------------
// Building block class.  Remaps locations in the buffer using a map.
class MapFilter : public LFilter
{
public:
    MapFilter(LBuffer* buffer = NULL) : LFilter(buffer) {if (buffer) {AllocateMap(); InitializeMap();}}
    virtual ~MapFilter() {}
    virtual int GetCount() const {return iBuffer ? iMap.size() : 0;}
    virtual void SetBuffer(LBuffer* buffer) {LFilter::SetBuffer(buffer); AllocateMap(); InitializeMap();}
    virtual void InitializeMap();
    // Note that the derived class requires GetDescriptor

protected:
    virtual RGBColor&   GetRawRGB(int idx) {return iBuffer->GetRawRGB(iMap[idx]);}
    vector<int> iMap;
private:
    void AllocateMap() {if (iBuffer) iMap.resize(iBuffer->GetCount());}
};

//-----------------------------------------------------------------------------
// Static Shift/Rotate
//-----------------------------------------------------------------------------

class ShiftFilter : public MapFilter
{
public:
    ShiftFilter(int offset = 0) : iOffset(offset), MapFilter() {} // Note that it's important to set iOffset first, so it will be set for the call to InitializeMap
    virtual ~ShiftFilter() {}
    virtual string GetDescriptor() const;
    void SetOffset(int offset) {iOffset = offset; InitializeMap();}
    int GetOffset() const {return iOffset;}
    virtual void InitializeMap();

private:
    int iOffset;
};

//-----------------------------------------------------------------------------
// RotateFilter -- Rotates the output an amount that changes over time
//-----------------------------------------------------------------------------

class RotateFilter : public ShiftFilter
{
public:
    static const float kDefaultSpeed; 
    RotateFilter(float speed = kDefaultSpeed) : ShiftFilter(0), iSpeed(speed) {}
    virtual ~RotateFilter() {}
    virtual string GetDescriptor() const;
    virtual bool Update();
    void  SetSpeed(float speed) {iSpeed = speed;}
    float GetSpeed() const {return iSpeed;}

private:
    float iSpeed;    // Speed of rotation in full buffer lengths per second
};

//-----------------------------------------------------------------------------
// BounceFilter -- Rotates the output an amount that changes over time
//-----------------------------------------------------------------------------

class BounceFilter : public ShiftFilter
{
public:
    static const float kDefaultSpeed; 
    BounceFilter(float speed = kDefaultSpeed, float bounceAfter = 1.0, float bounceIncr = 0.0) 
        : ShiftFilter(0), iSpeed(speed), iBounceAfter(bounceAfter), iBounceIncr(bounceIncr) {}
    virtual ~BounceFilter() {}
    virtual string GetDescriptor() const;
    virtual bool Update();
    void  SetSpeed(float speed) {iSpeed = speed;}
    float GetSpeed() const {return iSpeed;}
    void  SetBounceAfter(float bounceAfter) {iBounceAfter = bounceAfter;}
    void  SetBounceIncr(float bounceIncr) {iBounceIncr = bounceIncr;}

private:
    float iSpeed;       // Speed of rotation in full buffer lengths per second
    float iBounceAfter; // Number of rotations after which we should rotate back the other way.  Set to 0 for no bounce or 1 for bouncing after one full rotation
    float iBounceIncr;  // Amount the bounce should get off kilter. In units 
};

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

//-----------------------------------------------------------------------------
// RandomizedBuffer
//-----------------------------------------------------------------------------
// Randomizes the order of a buffer.
class RandomizedBuffer : public MapFilter
{
public:
    RandomizedBuffer() : MapFilter() {}
    virtual ~RandomizedBuffer() {}
    virtual string  GetDescriptor() const {return "random";}

    virtual void InitializeMap();

};

//-----------------------------------------------------------------------------
// SkipBuffer
//-----------------------------------------------------------------------------
// Interleaves the pixels (e.g., skips every other).
class SkipBuffer : public MapFilter
{
public:
    SkipBuffer(int skipnum = 2) : MapFilter(), iSkip(skipnum) {}
    virtual ~SkipBuffer() {}
    virtual int GetCount() {int count = MapFilter::GetCount(); return count - (count % max(iSkip,1));}
    virtual string  GetDescriptor() const {return "skip:" + IntToStr(iSkip);}
    virtual void InitializeMap();
  private:
    int iSkip;
};


#endif 

