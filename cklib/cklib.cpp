// Interface to color kinetics hardware
//
#include "cklib.h"
#include "KiNET.h"
#include <iostream>

//---------------------------------------------------------------------
// KiNET utilities
//---------------------------------------------------------------------
int KiNETportOut::GetSize()
{
    return sizeof (KiNETportOut);
}

int KiNETdmxOut::GetSize()
{
    // Need to work around C++'s need to pad bytes
    int len = sizeof (KiNETdmxOut);
    if ((len & 0x3) == 0) return len - 3;
    if ((len & 0x1) == 0) return len - 1;
    return len;
}

// Returns the number of bytes copied or a negative number if there wasn't enough room.
int CopyColorsToBuffer(char* buffer, int maxlen, vector<RGBColor>::const_iterator citer, int clen, bool reverseOrder)
{
    int bytesNeeded = clen * 3;
    if (bytesNeeded > maxlen) return -bytesNeeded;

    // We reverse the list while copying because that better fits the PDS pattern which has high numbered lights further from the  PDS
    int incr = 1;
    if (reverseOrder)
    {
        citer += clen - 1;
        incr = -1;
    }
    for (int i = 0; i < clen; ++i)
    {
        if (i != 0) citer += incr;
        *buffer++ = citer->r;
        *buffer++ = citer->g;
        *buffer++ = citer->b;
    }
    return bytesNeeded;
}

//---------------------------------------------------------------------
// CKdevice
//---------------------------------------------------------------------

string CKdevice::GetDescription() const
{
    string r;
    r += "CK PDS @ ";
    r += iIP.GetString();
    r += " Port ";
    r += IntToStr(iPort);
    r += " (";
    r += IntToStr(iCount);
    r += ")";
    return r;
}

bool CKdevice::Write(const char* buffer, int len)
{
    if (! iSocket.IsOpen())
    {
        SockAddr sa(iIP, KiNETudpPort);
        iSocket.SetSockAddr(sa);
        if (iSocket.HasError() || !iSocket.IsOpen())
        {
            iLastError = "Error opening socket for " + GetDescription() + ": " + iSocket.GetLastError();
            cerr << iLastError << endl;
            return false;
        }
    }

    iSocket.Write(buffer, len);
    //cout << "Wrote " << len << " bytes" << endl;
    if (iSocket.HasError())
    {
        iLastError = "Error writing to socket for " + GetDescription() + ": " + iSocket.GetLastError();
        cerr << iLastError << endl;
        return false;
    }

    return !HasError();
}

//---------------------------------------------------------------------
// CKduffer
//---------------------------------------------------------------------

bool CKbuffer::AddDevice(const CKdevice& dev, bool reverseOrder)
    {
    int oldCount = GetCount();
    iDevices.push_back(dev);
    iReverseFlag.push_back(reverseOrder);
    Alloc(oldCount + dev.GetCount());
    return !dev.HasError();
    }

bool CKbuffer::Update()
    {
    const int maxLen = 2048;
    char outbuf[maxLen];

    KiNETportOut* header = (KiNETportOut*) outbuf;
    int hdrLen = KiNETportOut::GetSize();
   // KiNETdmxOut* header = (KiNETdmxOut*) outbuf;
   // int hdrLen = KiNETdmxOut::GetSize();
    char* dataPtr = outbuf + hdrLen;

    int numDevs = iDevices.size();
    const_iterator bufIter = iBuffer.begin();
    for (int i = 0; i < numDevs; ++i)
    {
        // Force a millisecond delay. Otherwise, if we are going to the next port on the same PDS, we could lose data.
        if (i) Sleep(1);
        int len = iDevices[i].GetCount();
        int dataLen = CopyColorsToBuffer(dataPtr, maxLen-hdrLen, bufIter, len, iReverseFlag[i]);
        *header = KiNETportOut(); // Initialize header
        //*header = KiNETdmxOut(); // Initialize header
        header->port = iDevices[i].GetPort();
        //cout << "Port is " << iDevices[i].GetPort() << endl;
        header->len = dataLen;
        if (! iDevices[i].Write(outbuf, hdrLen+dataLen))
        {
            cerr << "Update failed on " << iDevices[i].GetDescription() << ": " << iDevices[i].GetLastError() << endl;
        }
        bufIter += len;
    }

    return !HasError();
    }
