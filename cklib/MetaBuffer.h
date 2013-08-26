// An LBuffer variant built using ncurses
//

#ifndef METABUFFER_H_INCLUDED
#define METABUFFER_H_INCLUDED

#include "utilsPrecomp.h"
#include "LBuffer.h"

class ComboBuffer : public LBuffer
{
public:
    // Used by LBuffer::Create
    static LBuffer* Create(csref descString, string* errmsg = NULL);
    static vector<string> ParseDeviceList(csref descStr, string* errmsg = NULL);

    // Constructors/Destructors
    ComboBuffer() : LBuffer(), iCount(0) {}
    ComboBuffer(LBuffer* buffer);
    ComboBuffer(const vector<LBuffer*>& buffers);
    virtual ~ComboBuffer();  // Calls delete on each of the nested buffers

    virtual int     GetCount()      const {return iCount;}
    virtual string  GetDescriptor() const;
    virtual bool    Update();

protected:
    virtual RGBColor&   GetRawRGB(int idx);
    void AddBuffer(LBuffer*);
    void AddBuffers(const vector<LBuffer*>& buffers);

private:
    int              iCount;
    vector<int>      iCounts;
    vector<LBuffer*> iBuffers;
};

// Flips the order of lights in the buffer
class ReverseBuffer : public LBuffer
{
public:
    ReverseBuffer(LBuffer* buffer) : LBuffer(), iBuffer(buffer) {}
    virtual ~ReverseBuffer() {}

    virtual int     GetCount() const {return iBuffer->GetCount();}
    virtual string  GetDescriptor() const {return "flip:" + iBuffer->GetDescriptor();}
    virtual bool    Update() {return iBuffer->Update();}

protected:
    virtual RGBColor&   GetRawRGB(int idx) {idx = iBuffer->GetCount() - idx - 1; return iBuffer->GetRawRGB(idx);}

private:
    LBuffer* iBuffer;
};

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkMeta();

#endif // !METABUFFER_H_INCLUDED
