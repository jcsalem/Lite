// Interface to color kinetics hardware
//
#include "cklib.h"

//---------------------------------------------------------------------
// CKBuffer
//---------------------------------------------------------------------

bool CKbuffer::AddDevice(const CKdevice& dev)
    {
    int oldCount = GetCount();
    iDevices.push_back(dev);
    Alloc(oldCount + dev.GetCount());
    return !dev.HasError();
    }

bool CKbuffer::Update() const
    {
    return !HasError();
    }
