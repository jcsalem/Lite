// An LBuffer variant built using ncurses
//

#ifndef LFILTER_H_INCLUDED
#define LFILTER_H_INCLUDED

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
    LFilter(LBuffer* buffer = NULL) : LBuffer(), iBuffer(buffer) {}
    virtual ~LFilter() {}

    // Static creation function
    static LFilter* Create(csref desc, string* errmsg);

    // Set the buffer that is the target of this filter instance
    virtual void    SetBuffer(LBuffer* buffer) {iBuffer = buffer;}
    LBuffer* GetBuffer() const {return iBuffer;}

    // Required definitions from LBuffer
    virtual int     GetCount() const {return iBuffer ? iBuffer->GetCount() : 0;}
    virtual bool    Update() {return iBuffer ? iBuffer->Update() : false;}
    virtual void    Clear() {if (iBuffer) iBuffer->Clear();}
    virtual string  GetDescription() const {return GetDescriptor() + "|" + (iBuffer ? iBuffer->GetDescription() : "(empty)");}
    // Note that the derived class requires GetDescriptor

protected:
    virtual RGBColor& GetRawRGB(int idx) {return iBuffer->GetRawRGB(idx);} // Don't need to check if iBuffer exists on this one
    LBuffer* iBuffer;
};

#endif // !LFILTER_H_INCLUDED

