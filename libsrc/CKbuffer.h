// LBuffers for ColorKinetics devices

#ifndef CKBUFFER_H_INCLUDED
#define CKBUFFER_H_INCLUDED

#include "utils.h"
#include "LBuffer.h"
#include "CKdevice.h"

class CKbuffer : public LBufferPhys
{
public:
    //CKbuffer() : LBufferPhys() {}
    CKbuffer(const CKdevice& dev);
    virtual ~CKbuffer() {}
    //bool    AddDevice(const CKdevice& dev);

    virtual bool    HasError()       const;
    virtual string  GetLastError()   const;
    virtual string  GetDescriptor()  const;
    virtual bool    Update();
    virtual bool    PortSync();

    // Alternative creation methods
//    static bool    CreateFromArglist(CKbuffer* buffer, int* argc, char** argv);
//    static bool    CreateFromXML(CKbuffer* buffer, const CKxmldoc& xmldoc);

private:
    CKdevice iDevice;
    // Don't allow copying
    CKbuffer(const CKbuffer&);
    CKbuffer& operator=(const CKbuffer&);
};

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkCK();

#endif // CKBUFFER_H_INCLUDED
