// An LBuffer variant built using ncurses
//

#ifndef FILTERBUFFER_H_INCLUDED
#define FILTERBUFFER_H_INCLUDED

#include "utilsPrecomp.h"
#include "LBuffer.h"

// Building block class.  Remaps locations in the buffer using a map.
class LBufferMap : public LBuffer
{
public:
    LBufferMap(LBuffer* buffer);
    virtual ~LBufferMap() {}

    virtual int     GetCount() const {return iBuffer->GetCount();}
    virtual bool    Update() {return iBuffer->Update();}
    virtual void    Clear() {iBuffer->Clear();}
    // Note that the derived class requires GetDescriptor

protected:
    virtual RGBColor&   GetRawRGB(int idx) {return iBuffer->GetRawRGB(iMap[idx]);}
    LBuffer*    iBuffer;
    vector<int> iMap;
};

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkFilters();

#endif // !COMBOBUFFER_H_INCLUDED

