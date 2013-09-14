// An LBuffer variant built using ncurses
//

#ifndef METABUFFER_H_INCLUDED
#define METABUFFER_H_INCLUDED

#include "utilsPrecomp.h"
#include "LBuffer.h"

class ComboBuffer : public LBuffer
{
    friend LBuffer* CreateOutputBuffer(csref descString, string* errmsg);
public:
    // Used by LBuffer::Create
    // It is an error to have a combo with no devices.
    static LBuffer* Create(csref descString, string* errmsg = NULL);

    // Constructors/Destructors
    ComboBuffer() : LBuffer(), iCount(0) {}
    ComboBuffer(LBuffer* buffer);
    ComboBuffer(const vector<LBuffer*>& buffers);
    virtual ~ComboBuffer();  // Calls delete on each of the nested buffers

    virtual int     GetCount()      const {return iCount;}
    virtual string  GetDescriptor() const;
    virtual bool    Update();

    // Used by L::CreateOutputBuffer
    int GetNumBuffers() const {return iBuffers.size();}
    LBuffer* PopLastBuffer();

protected:
    virtual RGBColor&   GetRawRGB(int idx);
    void AddBuffer(LBuffer*);
    void AddBuffers(const vector<LBuffer*>& buffers);

private:
    int              iCount;
    vector<int>      iCounts;
    vector<LBuffer*> iBuffers;
};

// Building block class.  Remaps locations in the buffer using a map.
class LBufferMap : public LBuffer
{
public:
    LBufferMap(LBuffer* buffer);
    virtual ~LBufferMap() {}

    virtual int     GetCount() const {return iBuffer->GetCount();}
    virtual bool    Update() {return iBuffer->Update();}
    // Note that the derived class requires GetDescriptor

protected:
    virtual RGBColor&   GetRawRGB(int idx) {return iBuffer->GetRawRGB(iMap[idx]);}
    LBuffer*    iBuffer;
    vector<int> iMap;
};

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkMeta();

#endif // !METABUFFER_H_INCLUDED
