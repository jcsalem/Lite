// Interface to color kinetics hardware
//
#include "cklib.h"
#include "KiNET.h"
#include "utilsTime.h"
#include "LBuffer.h"
#include <iostream>
#include <stdio.h>

//---------------------------------------------------------------------
// KiNET utilities
//---------------------------------------------------------------------
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
        *buffer++ = citer->rAsChar();
        *buffer++ = citer->gAsChar();
        *buffer++ = citer->bAsChar();
    }
    return bytesNeeded;
}

const char* bufferToString (string* outString, const char* startptr, const char* endptr)
// Reads a null-terminated string from the buffer.  If the buffer ends before the null, then the rest of the string is returned.
// endptr points to the last character in the string + 1
// Returns a point to the next character (or to endptr if that's reached)
{
    for (const char* i = startptr; i != endptr; ++i) {
        if (*i == 0) {
            *outString = string(startptr);
            return (i+1);
        }
    }
    *outString = string(startptr, endptr - startptr);
    return endptr;
}

//---------------------------------------------------------------------
// CKinfo
//---------------------------------------------------------------------
// Info about a CK device

void CKinfo::Clear() {
     ipaddr = numports = serial = universe = 0;
     macaddr.clear();
     info.clear();
     name.clear();
}
bool CKinfo::SetFromPollReply(const char* buffer, int buflen, string* errmsg) {
    Clear();

    if (!buffer || buflen < KiNETpollReply::GetSize() + 2) {
        if (errmsg) *errmsg = "CK poll reply was too short (" + IntToStr(buffer ? buflen : 0) + " bytes)";
        return false;
    }

    KiNETpollReply* pollReply = (KiNETpollReply*) buffer;
    const char* strptr = buffer + KiNETpollReply::GetSize();
    const char* strend = buffer + buflen;

    ipaddr = ntohl(pollReply->ip);
    char macaddrstr[20];
    sprintf(macaddrstr, "%02X:%02X:%02X:%02X:%02X:%02X",
            pollReply->mac[0], pollReply->mac[1], pollReply->mac[2], pollReply->mac[3], pollReply->mac[4], pollReply->mac[5]);
    macaddr     = macaddrstr;
    numports    = pollReply->numPorts; // This is a little uncertain
    serial      = pollReply->serial;
    universe    = pollReply->universe;

    strptr = bufferToString(&info, strptr, strend);
    bufferToString(&name, strptr, strend);

    return true;
}

//---------------------------------------------------------------------
// CKdevice
//---------------------------------------------------------------------

CKdevice::CKdevice(csref devstrArg) : iUniverse(0), iPort(1), iCount(50), iLayout(CK::kNormal)
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
        iCount = atoi(devstr.substr(parenpos + 1, len - parenpos - 2).c_str());
        if (iCount <= 0 || iCount > 256)
        {
            iLastError = "Device count must be between 1 and 256";
            return;
        }
        devstr = devstr.substr(0, parenpos);
        len = parenpos;
    }

    // Port
    size_t atpos = devstr.find('/');
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

string CKdevice::GetDescriptor() const
{
    string r;
    r += iIP.GetString();
    r += "/";
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
            iLastError = "Error opening socket for " + GetDescriptor() + ": " + iSocket.GetLastError();
            cerr << iLastError << endl;
            return false;
        }
    }

    iSocket.Write(buffer, len);
    //cout << "Wrote " << len << " bytes" << endl;
    if (iSocket.HasError())
    {
        iLastError = "Error writing to socket for " + GetDescriptor() + ": " + iSocket.GetLastError();
        cerr << iLastError << endl;
        iSocket.Close(); // Close the socket in the hopes the next write will reopen and fix everything.
        return false;
    }

    return !HasError();
}

//---------------------------------------------------------------------
// CKduffer
//---------------------------------------------------------------------

bool CKbuffer::HasError() const
{
    if (LBuffer::HasError()) return true;
    for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i)
        if (i->HasError()) return true;
    return false;
}

string CKbuffer::GetLastError() const
{
    if (LBuffer::HasError()) return LBuffer::GetLastError();
    for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i)
        if (i->HasError()) return i->GetLastError();
    return string();
}

string CKbuffer::GetDescriptor() const
{
    string str = "ck:";
    for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i)
    {
        if (i != iDevices.begin())
            str += ", ";
        str += i->GetDescriptor();
    }
    return str;
}

bool CKbuffer::PortSync()
{
    const int maxLen = 2048;
    char outbuf[maxLen];

    // Initialize header
    KiNETportOutSync* header = (KiNETportOutSync*) outbuf;
    *header = KiNETportOutSync();
    int len = KiNETportOutSync::GetSize();
    for (size_t i = 0; i < iDevices.size(); ++i)
    {
    if (! iDevices[i].Write(outbuf, len))
        {
            cerr << "Update failed writing to " << iDevices[i].GetIP().GetString() << ": " << iDevices[i].GetLastError() << endl;
        }
    }

    return !HasError();
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
        if (i) SleepMilli(1);
        int len = iDevices[i].GetCount();
        int dataLen = CopyColorsToBuffer(dataPtr, maxLen-hdrLen, bufIter, len, iDevices[i].GetLayout() == CK::kReverse);
        *header = KiNETportOut(); // Initialize header
        //*header = KiNETdmxOut(); // Initialize header
        header->port = iDevices[i].GetPort();
        header->universe = iDevices[i].GetUniverse();
        //cout << "Port is " << iDevices[i].GetPort() << endl;
        header->len = dataLen;
        if (! iDevices[i].Write(outbuf, hdrLen+dataLen))
        {
            cerr << "Update failed on " << iDevices[i].GetIP().GetString() << ": " << iDevices[i].GetLastError() << endl;
        }
        bufIter += len;
    }

    PortSync();

    return !HasError();
    }

//---------------------------------------------------------------------
// CKbuffer: Creating
//---------------------------------------------------------------------

bool CKbuffer::AddDevice(const CKdevice& dev)
    {
    int oldCount = GetCount();
    iDevices.push_back(dev);
    Alloc(oldCount + dev.GetCount());
    return !dev.HasError();
    }

#if 0
bool CKbuffer::AddDevice(csref devstrarg)
{
    bool success = false;
    size_t pos = devstrarg.find(',');
    string devstr = devstrarg.substr(0, pos);
    CKdevice dev(devstr);
    if (dev.HasError())
        iLastError = "Invalid device string: '" + devstrarg + "': " + dev.GetLastError();
    else
        success = AddDevice(dev);
    if (pos != string::npos)
        success = AddDevice(devstrarg.substr(pos+1)) && success;
    return success;
}
#endif

LBuffer* CKbufferCreate(csref descriptorArg, string* errmsg)
{
    string descriptor = descriptorArg;
    CKbuffer* buffer = new CKbuffer();

    while (! descriptor.empty()) {
        size_t pos = descriptor.find(',');
        string devstr = descriptor.substr(0, pos);
        CKdevice dev(devstr);
        if (dev.HasError()) {
            if (errmsg) *errmsg = "Invalid device string: '" + devstr + "': " + dev.GetLastError();
            delete buffer;
            return NULL;
        }
        // Success for this device. Prep for the next one
        buffer->AddDevice(dev);
        if (pos == string::npos) break;
        descriptor = descriptor.substr(pos+1);
    }

    // Check for missing descriptors
    if (buffer->GetCount() == 0) {
        delete buffer;
        if (errmsg) *errmsg = "Couldn't create CKbuffer: zero lights";
        return NULL;
    }
    return buffer;
}

LBuffer* CKbufferAutoCreate(csref descriptorArg, string* errmsg) {
    if (! TrimWhitespace(descriptorArg).empty()) {
        if (errmsg) *errmsg = "The ckauto device type doesn't take any parameters";
        return NULL;
    }

    vector<CKdevice> devices = CKpollForDevices(errmsg);
    if (devices.empty()) {
        if (errmsg) *errmsg = "Didn't detect any ColorKinetics devices.";
        return NULL;
    }


    CKbuffer *ckbuffer = new CKbuffer();
    for (size_t i = 0; i < devices.size(); ++i)
        ckbuffer->AddDevice(devices[i]);

    if (ckbuffer->GetCount() == 0 || ckbuffer->HasError()) {
        if (errmsg) *errmsg = "Couldn't create CKbuffer: " + (ckbuffer->HasError() ? ckbuffer->GetLastError() : string("zero lights"));
        delete ckbuffer;
        return NULL;
    }

    return ckbuffer;
}

// Define the creation functions
DEFINE_LBUFFER_TYPE(ck, CKbufferCreate, "CK:ipaddr/port(size)[,...]",
        "Comma separated list of ColorKinetics devices. Append 'r' to port to reverse order.\n"
        "  Examples: ck:172.16.11.23/1  or  ck:10.5.4.3/1(72),10.5.4.3/2r(72)");

DEFINE_LBUFFER_TYPE(ckauto, CKbufferAutoCreate, "CKAUTO", "Creates a display using all of the local CK devices");

// Dummy function to force linking of this file
void ForceLinkCK() {}

//---------------------------------------------------------------------
// Polling the CK devices on the network
//---------------------------------------------------------------------

vector<CKinfo> CKpollForInfo(string* errmsg, int timeoutInMS) {
    vector<CKinfo> retval;

    // Create the client socket for the polling request
    SockAddr sa(INADDR_BROADCAST, KiNETudpPort); //use broadcast address
    SocketUDPClient sock(sa);
    sock.setsockopt_bool(SOL_SOCKET, SO_BROADCAST, true);

    // Broadcast the poll request
    // Note: QuickPlayPro sends out 4 PORTOUT_SYNC packets before the poll. Not sure if that's important.
    KiNETpoll pollPacket;
    sock.Write((char*) &pollPacket, pollPacket.GetSize());
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error writing CK KiNETpoll packet: " + sock.GetLastError();
        return retval;
    }

    // Wait for a response
    const int buflen = 1000;
    char buffer[buflen];
    int bytesRead = 0;

    while (sock.HasData(timeoutInMS)) {
        if (sock.Read(buffer, buflen, &bytesRead)) {
            CKinfo devInfo;
            string errmsg;
            if (devInfo.SetFromPollReply(buffer, bytesRead, &errmsg))
                retval.push_back(devInfo);
        } else {
            if (errmsg) *errmsg = "Error reading CK KiNETpoll response: " + sock.GetLastError();
            return retval;
        }
    }
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error waiting for poll response: " + sock.GetLastError();
        return retval;
    }
    return retval;
}

vector<CKdevice> CKpollForDevices(string* errmsg, int timeoutInMS) {
    // Get all the info structures
    vector<CKinfo> infos = CKpollForInfo(errmsg, timeoutInMS);
    return CKpollForDevices(infos,errmsg);
}


const int kDefaultCountTimeout = 2500; // timeout in MS
vector<CKdevice> CKpollForDevices(const vector<CKinfo>& infos, string* errmsg) {
    vector<CKdevice> devices;
    const int buflen = 1000;
    char buffer[buflen];
    int bytesRead = 0;

    // For each info, poll for the count of lights
    for (size_t i = 0; i < infos.size(); ++i) {
        CKinfo info = infos[i];
        IPAddr ipaddr(info.ipaddr);

        // Write the GetCount request
        SocketUDPClient sock(ipaddr, KiNETudpPort);
        KiNETgetCount getCountPacket;
        sock.Write((char*) &getCountPacket, getCountPacket.GetSize());
        if (sock.HasError()) {
            if (errmsg) *errmsg = "Error writing CK KiNETpoll packet: " + sock.GetLastError();
            sock.Close();
            continue;
        }

        // Parse the results
        while (sock.HasData(kDefaultCountTimeout) && sock.Read(buffer, buflen, &bytesRead)) {
            if (bytesRead >= KiNETgetCountReply::GetSize()) {
                KiNETgetCountReply* replyPtr = (KiNETgetCountReply*) buffer;
                if (replyPtr->replyType == KiNETgetCountReply::kEnd) break; // Done reading
                if (replyPtr->replyType == KiNETgetCountReply::kData) {
                    // We have the count data
                    KiNETgetCountData* countDataPtr = (KiNETgetCountData*) (buffer + KiNETgetCountReply::GetSize());
                    KiNETgetCountData* countDataEnd = (KiNETgetCountData*) (buffer + bytesRead);
                    while (countDataPtr < countDataEnd) {
                        int universe = info.universe;
                        int port = countDataPtr->portnum;
                        int count = countDataPtr->count;
                        if (port >= 1 && port <= info.numports && count > 0) {
                            CKdevice dev(ipaddr, universe, port, count);
                            devices.push_back(dev);
                        }
                        ++countDataPtr;
                    }
                }
            }
        }
        if (sock.HasError())
            if (errmsg) *errmsg = "Error reading CK KiNETpoll packet: " + sock.GetLastError();

        sock.Close();
    }
    return devices;
}
