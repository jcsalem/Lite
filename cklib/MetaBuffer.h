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

// Randomizes the order of a buffer.
class RandomizedBuffer : public LBufferMap
{
public:
    RandomizedBuffer(LBuffer* buffer) : LBufferMap(buffer) {RandomizeMap();}
    virtual ~RandomizedBuffer() {}
    virtual string  GetDescriptor() const {return "random:" + iBuffer->GetDescriptor();}

    void RandomizeMap();

};

// Interleaves the pixel (e.g., skips every other).  It would be nice to make this more generic (e.g., skip3, skip4, etc.)
class Skip2Buffer : public LBufferMap
{
public:
    Skip2Buffer(LBuffer* buffer);
    virtual ~Skip2Buffer() {}
    virtual string  GetDescriptor() const {return "skip2:" + iBuffer->GetDescriptor();}
};

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkMeta();

#endif // !METABUFFER_H_INCLUDED
