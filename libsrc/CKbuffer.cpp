// Interface to color kinetics hardware
//
#include "CKbuffer.h"
#include "KiNET.h"
#include "utilsTime.h"
#include "LBuffer.h"
#include <iostream>
#include <stdio.h>
#include "ComboBuffer.h"
#include "utilsParse.h"

//---------------------------------------------------------------------
// KiNET utilities
//---------------------------------------------------------------------
// Returns the number of bytes copied or a negative number if there wasn't enough room.
int CopyColorsToBuffer(unsigned char* buffer, int maxlen, LBuffer::const_iterator citer, int clen)
{
    int bytesNeeded = clen * 3;
    if (bytesNeeded > maxlen) return -bytesNeeded;

    for (int i = 0; i < clen; ++i)
    {
        if (i != 0) ++citer;
        *buffer++ = citer->rAsChar();
        *buffer++ = citer->gAsChar();
        *buffer++ = citer->bAsChar();
    }
    return bytesNeeded;
}

//---------------------------------------------------------------------
// CKduffer
//---------------------------------------------------------------------

CKbuffer::CKbuffer(const CKdevice& dev) : LBufferPhys(), iDevice(dev)
{
    if (dev.GetCount() == 0) {
        iLastError = "Zero length ColorKinetics device";
    }

    Alloc(dev.GetCount());
}


bool CKbuffer::HasError() const
{
    if (LBuffer::HasError()) return true;
    if (iDevice.HasError()) return true;
    return false;
}

string CKbuffer::GetLastError() const
{
    if (LBuffer::HasError()) return LBuffer::GetLastError();
    if (iDevice.HasError()) return iDevice.GetLastError();
    return string();
}

string CKbuffer::GetDescriptor() const
{
    return "ck:" + iDevice.GetDescriptor();
}

bool CKbuffer::PortSync()
{
    const int maxLen = 2048;
    unsigned char outbuf[maxLen];

    // Initialize header
    KiNETportOutSync* header = (KiNETportOutSync*) outbuf;
    *header = KiNETportOutSync();
    int len = KiNETportOutSync::GetSize();
    if (! iDevice.Write(outbuf, len))
        cerr << "PortSync failed writing to " << iDevice.GetIP().GetString() << ": " << iDevice.GetLastError() << endl;

    return !HasError();
}

void UpdateOneCKv1(CKdevice& device, LBuffer::const_iterator bufIter)
    {
    const int maxLen = 2048;
    unsigned char outbuf[maxLen];

    KiNETdmxOut* header = (KiNETdmxOut*) outbuf;
    int hdrLen = KiNETdmxOut::GetSize();
    unsigned char* dataPtr = outbuf + hdrLen;

    int len = device.GetCount();
    int dataLen = CopyColorsToBuffer(dataPtr, maxLen-hdrLen, bufIter, len);
    int paddedLen = 512;
    // Fill in rest with zeros
    for (int i = dataLen; i < paddedLen; ++i)
        *(dataPtr+i) = 0;

    *header = KiNETdmxOut(); // Initialize header
    header->universe = device.GetUniverse();
    if (! device.Write(outbuf, hdrLen+paddedLen))
      {
       cerr << "Update failed on " << device.GetIP().GetString() << ": " << device.GetLastError() << endl;
      }
    }

void UpdateOneCKv2(CKdevice& device, LBuffer::const_iterator bufIter)
    {
    const int maxLen = 2048;
    unsigned char outbuf[maxLen];

    KiNETportOut* header = (KiNETportOut*) outbuf;
    int hdrLen = KiNETportOut::GetSize();
    unsigned char* dataPtr = outbuf + hdrLen;

    int len = device.GetCount();
    int dataLen = CopyColorsToBuffer(dataPtr, maxLen-hdrLen, bufIter, len);
    *header = KiNETportOut(); // Initialize header
    header->port = device.GetPort();
    header->universe = device.GetUniverse();
    //cout << "Port is " << device.GetPort() << endl;
    header->len = dataLen;
    if (! device.Write(outbuf, hdrLen+dataLen))
      {
       cerr << "Update failed on " << device.GetIP().GetString() << ": " << device.GetLastError() << endl;
      }
    }

bool CKbuffer::Update()
    {
    const_iterator bufIter = const_cast<const CKbuffer*>(this)->begin();
    switch (iDevice.GetKiNetVersion())
        {
        case 2:
            UpdateOneCKv2(iDevice, bufIter);
            break;
        default:
            UpdateOneCKv1(iDevice, bufIter);
            break;
        }
    // Not using sync. To enable, I think there is a flag that must be set with the PortOut command.
    // PortSync();
    return !HasError();
    }

//---------------------------------------------------------------------
// CKbuffer: Creating
//---------------------------------------------------------------------

LBuffer* CKbufferCreate(const vector<string>& params, string* errmsg)
{
    if (! ParamListCheck(params, "CK display buffer", errmsg, 1, 1)) return NULL;
    CKdevice dev(params[0]);
    if (dev.HasError()) {
        if (errmsg) *errmsg = "Invalid device string: '" + params[0] + "': " + dev.GetLastError();
        return NULL;
    }

    // Check for missing descriptors
    if (dev.GetCount() == 0) {
        if (errmsg) *errmsg = "Couldn't create CKbuffer: zero lights";
        return NULL;
    }

    // Prime the UDP connection
    dev.InitializeUDPConnection();

    return new CKbuffer(dev);
}

LBuffer* CKbufferAutoCreate(const vector<string>& params, string* errmsg) {
    if (! ParamListCheck(params, "ckauto display buffer", errmsg, 0, 0)) return NULL;

    vector<CKdevice> devices = CKdiscoverDevices(errmsg);
    if (devices.empty()) {
        if (errmsg) *errmsg = "Didn't detect any ColorKinetics devices: " + *errmsg;
        return NULL;
    }

    // Create the different CKbuffers
    vector<LBuffer*> ckbuffers;
    for (size_t i = 0; i < devices.size(); ++i)
        ckbuffers.push_back(new CKbuffer(devices[i]));

    // Check for errors
    bool hasError = false;
    for (size_t i = 0; i < ckbuffers.size(); ++i) {
        if (ckbuffers[i]->HasError()) {
            if (errmsg) *errmsg = "Couldn't create CKbuffer: " + ckbuffers[i]->GetLastError();
            hasError = true;
            break;
        }
    }

    if (hasError) {
        for (size_t i = 0; i < ckbuffers.size(); ++i)
            delete ckbuffers[i];
        return NULL;
    }

    // No error, return the buffer
    if (ckbuffers.size() == 1)
        return ckbuffers[0];
    else
        return new ComboBuffer(ckbuffers);
}

// Define the creation functions
DEFINE_LBUFFER_DEVICE_TYPE(ck, CKbufferCreate, "CK:ipaddr/port(size)[,...]",
        "ColorKinetics device. If port is missing, assumes a KiNet V1 device.\n"
        "  Examples: ck:172.16.11.23/1  or  ck:10.5.4.3/1(72) or ck:172.16.11.54(21)");

DEFINE_LBUFFER_DEVICE_TYPE(ckauto, CKbufferAutoCreate, "CKAUTO", "Creates a display using all of the local CK devices");

// Dummy function to force linking of this file
void ForceLinkCK() {}
