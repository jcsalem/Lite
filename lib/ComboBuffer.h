// An LBuffer variant built using ncurses
//

#ifndef COMBOBUFFER_H_INCLUDED
#define COMBOBUFFER_H_INCLUDED

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

#endif // !COMBOBUFFER_H_INCLUDED
