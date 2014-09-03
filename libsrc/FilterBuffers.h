// An LBuffer variant built using ncurses
//

#ifndef FILTERBUFFER_H_INCLUDED
#define FILTERBUFFER_H_INCLUDED

#include "Config.h"
#include "LBuffer.h"

//-----------------------------------------------------------------------------
// Abstract LFilter type
//-----------------------------------------------------------------------------
// This is just a special case of LBuffer
// Any class that is used as a filter should use this
class LFilter: public LBuffer
{
public:
    LFilter(LBuffer* buffer) : LBuffer(), iBuffer(buffer) {}
    virtual ~LFilter() {}

    virtual int     GetCount() const {return iBuffer->GetCount();}
    virtual bool    Update() {return iBuffer->Update();}
    virtual void    Clear() {iBuffer->Clear();}
    // Note that the derived class requires GetDescriptor


protected:
    virtual RGBColor& GetRawRGB(int idx) {return iBuffer->GetRawRGB(idx);}
    LBuffer* iBuffer;
};
  

//-----------------------------------------------------------------------------
// Abstract LMapFilter type
//-----------------------------------------------------------------------------
// Building block class.  Remaps locations in the buffer using a map.
class LMapFilter : public LFilter
{
public:
    LMapFilter(LBuffer* buffer);
    virtual ~LMapFilter() {}
    // Note that the derived class requires GetDescriptor

protected:
    virtual RGBColor&   GetRawRGB(int idx) {return iBuffer->GetRawRGB(iMap[idx]);}
    vector<int> iMap;
};

//-----------------------------------------------------------------------------
// Filters that rotate the output
//-----------------------------------------------------------------------------
class LShiftFilter : public LMapFilter
{
public:
    LShiftFilter(LBuffer* buffer, int offset = 0) : LMapFilter(buffer) {SetOffset(offset);}
    virtual ~LShiftFilter() {}
    virtual string GetDescriptor() const;
    void SetOffset(int offset);
    int GetOffset() const {return iOffset;}

private:
    int iOffset;
};

class LRotateFilter : public LShiftFilter
{
public:
    LRotateFilter(LBuffer* buffer, float speed = 1.0, float bounceAfter = 0.0) : LShiftFilter(buffer,0), iSpeed(speed), iBounceAfter(bounceAfter) {}
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

// This function is defined only so LFramework can reference it and force the FilterBuffers.cpp to be linked in.
void ForceLinkFilters();

#endif // !COMBOBUFFER_H_INCLUDED

