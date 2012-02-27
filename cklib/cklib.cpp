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

CKdevice::CKdevice(csref devstrArg) : iPort(1), iCount(50), iLayout(CK::kNormal)
{
    string devstr = TrimWhitespace(devstrArg);
    size_t len = devstr.length();
    if (len == 0)
    {
        iLastError = "Empty device string";
        return;
    }

    // Count
    size_t parenpos = devstr.find('(');
    if (parenpos != string::npos)
    {
        if (devstr.find(')') != len - 1)
        {
            iLastError = "Device string has mismatched parentheses";
            return;
        }
        iCount = atoi(devstr.substr(parenpos, len - parenpos - 2).c_str());
        if (iCount <= 0 || iCount > 256)
        {
            iLastError = "Device count must be between 1 and 256";
            return;
        }
        devstr = devstr.substr(0, parenpos);
        len = parenpos;
    }

    // Port
    size_t atpos = devstr.find('@');
    if (atpos != string::npos)
    {
        if (! isdigit(devstr[atpos+1]))
        {
            iLastError = "Device port must be a number";
            return;
        }

        if (tolower(devstr[len-1]) == 'r')
        {
            iLayout = CK::kReverse;
        }
        iPort = atoi(devstr.substr(atpos + 1, len - atpos - 1).c_str());
        devstr = devstr.substr(0,atpos);
        len = atpos;
    }

    iIP = IPAddr(devstr);
    if (! iIP.IsValid())
    {
        iLastError = "The IP address was invalid " + devstr;
        return;
    }
}

string CKdevice::GetDescription() const
{
    string r;
    r += "PDS:";
    r += iIP.GetString();
    r += "@";
    r += IntToStr(iPort);
    if (iLayout == CK::kReverse)
        r += "R";
    r += "(";
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
bool CKbuffer::HasError() const
{
    if (!iLastError.empty()) return true;
    for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i)
        if (i->HasError()) return true;
    return false;
}

string CKbuffer::GetLastError() const
{
    if (!iLastError.empty()) return iLastError;
    for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i)
        if (i->HasError()) return i->GetLastError();
    return string();
}

string CKbuffer::GetDescription() const
{
    string str = "CKbuffer: " + IntToStr(GetCount()) + " total lights [";
    for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i)
    {
        if (i != iDevices.begin())
            str += ", ";
        str += i->GetDescription();
    }
    str += "]";
    return str;
}

bool CKbuffer::AddDevice(const CKdevice& dev)
    {
    int oldCount = GetCount();
    iDevices.push_back(dev);
    Alloc(oldCount + dev.GetCount());
    return !dev.HasError();
    }

bool CKbuffer::AddDevice(csref devstr)
{
    CKdevice dev(devstr);
    if (dev.HasError())
    {
        iLastError = "Invalid device string: '" + devstr + "': " + dev.GetLastError();
        return false;
    }
    return AddDevice(dev);
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
        int dataLen = CopyColorsToBuffer(dataPtr, maxLen-hdrLen, bufIter, len, iDevices[i].GetLayout() == CK::kReverse);
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
