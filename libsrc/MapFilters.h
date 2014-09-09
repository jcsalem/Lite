// An LBuffer variant built using ncurses
//

#ifndef MAPFILTERS_H_INCLUDED
#define MAPFILTERS_H_INCLUDED

#include "Config.h"
#include "LBuffer.h"
#include "LFilter.h"

//-----------------------------------------------------------------------------
// Abstract LMapFilter type
//-----------------------------------------------------------------------------
// Building block class.  Remaps locations in the buffer using a map.
class LMapFilter : public LFilter
{
public:
    LMapFilter(LBuffer* buffer = NULL) : LFilter(buffer) {if (buffer) {AllocateMap(); InitializeMap();}}
    virtual ~LMapFilter() {}
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

//-----------------------------------------------------------------------------
// LRotateFilter -- Rotates the output an amount that changes over time
//-----------------------------------------------------------------------------

class LRotateFilter : public LShiftFilter
{
public:
    static const float gDefaultSpeed; 
    LRotateFilter(float speed = gDefaultSpeed, float bounceAfter = 0.0) : LShiftFilter(0), iSpeed(speed), iBounceAfter(bounceAfter) {}
    virtual ~LRotateFilter() {}
    virtual string GetDescriptor() const;
    virtual bool Update();
    void  SetSpeed(float speed) {iSpeed = speed;}
    float GetSpeed() const {return iSpeed;}
    void  EnableBounce() {iBounceAfter = 1.0;}
    void  SetBounceAfter(float bounceAfter) {iBounceAfter = bounceAfter;}

private:
    float iSpeed;    // Speed of rotation in full buffer lengths per second
    float iBounceAfter; // Number of rotations after which we should rotate back the other way.  Set to 0 for no bound or 1 for bouncing after one full rotation
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
class RandomizedBuffer : public LMapFilter
{
public:
    RandomizedBuffer() : LMapFilter() {}
    virtual ~RandomizedBuffer() {}
    virtual string  GetDescriptor() const {return "random";}

    virtual void InitializeMap();

};

//-----------------------------------------------------------------------------
// SkipBuffer
//-----------------------------------------------------------------------------
// Interleaves the pixels (e.g., skips every other).
class SkipBuffer : public LMapFilter
{
public:
    SkipBuffer(int skipnum = 2) : LMapFilter(), iSkip(skipnum) {}
    virtual ~SkipBuffer() {}
    virtual int GetCount() {int count = LMapFilter::GetCount(); return count - (count % max(iSkip,1));}
    virtual string  GetDescriptor() const {return "skip:" + IntToStr(iSkip);}
    virtual void InitializeMap();
  private:
    int iSkip;
};


#endif 

