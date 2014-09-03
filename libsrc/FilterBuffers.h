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
// A Filter that rotates the output
//-----------------------------------------------------------------------------
// Building block class.  Remaps locations in the buffer using a map.
class LRotateFilter : public LMapFilter
{
public:
    LRotateFilter(LBuffer* buffer, int offset = 0) : LMapFilter(buffer) {SetOffset(offset);}
    virtual ~LRotateFilter() {}
    virtual string GetDescriptor() const;
    void SetOffset(int offset);
    int GetOffset() const {return iOffset;}

private:
    int iOffset;
};


// This function is defined only so LFramework can reference it and force the FilterBuffers.cpp to be linked in.
void ForceLinkFilters();

#endif // !COMBOBUFFER_H_INCLUDED

