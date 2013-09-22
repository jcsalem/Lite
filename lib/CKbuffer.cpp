// Interface to color kinetics hardware
//
#include "CKbuffer.h"
#include "KiNET.h"
#include "utilsTime.h"
#include "LBuffer.h"
#include <iostream>
#include <stdio.h>
#include "ComboBuffer.h"

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
int CopyColorsToBuffer(char* buffer, int maxlen, LBuffer::const_iterator citer, int clen)
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
// Info about a CK power supply

void CKinfo::Clear() {
     ipaddr = kinetVersion = serial = universe = 0;
     macaddr.clear();
     info.clear();
     name.clear();
}

bool CKinfo::SetFromDiscoverReply(const char* buffer, int buflen, string* errmsg) {
    Clear();

    if (!buffer || buflen < KiNETdiscoverReply::GetSize() + 2) {
        if (errmsg) *errmsg = "CK discover reply was too short (" + IntToStr(buffer ? buflen : 0) + " bytes)";
        return false;
    }

    KiNETdiscoverReply* reply = (KiNETdiscoverReply*) buffer;
    const char* strptr = buffer + KiNETdiscoverReply::GetSize();
    const char* strend = buffer + buflen;

    ipaddr = ntohl(reply->ip);
    char macaddrstr[20];
    sprintf(macaddrstr, "%02X:%02X:%02X:%02X:%02X:%02X",
            reply->mac[0], reply->mac[1], reply->mac[2], reply->mac[3], reply->mac[4], reply->mac[5]);
    macaddr     = macaddrstr;
    kinetVersion= reply->kinetVersion;
    serial      = reply->serial;
    universe    = reply->universe;

    strptr = bufferToString(&info, strptr, strend);
    bufferToString(&name, strptr, strend);

    return true;
}

bool CKdevice::UpdateKiNetVersion(string* errmsg) {
    SocketUDPClient sock(iIP, KiNETudpPort);
    KiNETdiscover pollPacket;
    sock.Write((char*) &pollPacket, pollPacket.GetSize());
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error writing CK KiNETdiscover packet: " + sock.GetLastError();
        sock.Close();
        return false;
    }

    // Wait for a response
    const int buflen = 1000;
    char buffer[buflen];
    int bytesRead = 0;

    while (sock.HasData(CK::kDefaultPollTimeout)) {
        if (sock.Read(buffer, buflen, &bytesRead)) {
            CKinfo devInfo;
            if (devInfo.SetFromDiscoverReply(buffer, bytesRead, errmsg))
                iKiNetVersion = devInfo.kinetVersion;
            else {
                sock.Close();
                return false;
            }
        }
    }
    sock.Close();
    return true;
}


//---------------------------------------------------------------------
// CK::PortType_t
//---------------------------------------------------------------------
string CK::PortTypeToString(PortType_t type) {
    switch(type) {
        case CK::kChromasic:    return "Chromasic";
        case CK::kChromasicV2:  return "ChromasicV2";
        case CK::kSerial:       return "Serial";
        default:
            return "Unknown-" + IntToStr(type);
    }
}

//---------------------------------------------------------------------
// CKdevice
//---------------------------------------------------------------------

CKdevice::CKdevice(csref devstrArg) : iUniverse(CK::kAnyUniverse), iPort(1), iCount(50), iKiNetVersion(kDefaultKiNetVersion)
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
    size_t slashpos = devstr.find('/');
    if (slashpos == string::npos)
    {
        // No port number, this is a v1 device
        iKiNetVersion = 1;
    } else
    {
        iKiNetVersion = 2;
        if (! StrToUnsigned(devstr.substr(slashpos + 1, len - slashpos - 1), (unsigned*) &iPort)) {
            iLastError = "Device port must be a number";
            return;
        }

        devstr = devstr.substr(0,slashpos);
        len = slashpos;
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
    string r = "ck:";
    r += iIP.GetString();
    if (iKiNetVersion > 1) {
        r += "/";
        r += IntToStr(iPort);
    }
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
    char outbuf[maxLen];

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
    char outbuf[maxLen];

    KiNETdmxOut* header = (KiNETdmxOut*) outbuf;
    int hdrLen = KiNETdmxOut::GetSize();
    char* dataPtr = outbuf + hdrLen;

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
    char outbuf[maxLen];

    KiNETportOut* header = (KiNETportOut*) outbuf;
    int hdrLen = KiNETportOut::GetSize();
    char* dataPtr = outbuf + hdrLen;

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

    PortSync();
    // Force a millisecond delay. Otherwise, if we are going to the next port on the same PDS, we could lose data.
    SleepMilli(1);

    return !HasError();
    }

//---------------------------------------------------------------------
// CKbuffer: Creating
//---------------------------------------------------------------------

LBuffer* CKbufferCreate(csref devstr, string* errmsg)
{
    CKdevice dev(devstr);
    if (dev.HasError()) {
        if (errmsg) *errmsg = "Invalid device string: '" + devstr + "': " + dev.GetLastError();
        return NULL;
    }

    // Check for missing descriptors
    if (dev.GetCount() == 0) {
        if (errmsg) *errmsg = "Couldn't create CKbuffer: zero lights";
        return NULL;
    }
    return new CKbuffer(dev);
}

LBuffer* CKbufferAutoCreate(csref descriptorArg, string* errmsg) {
    if (! TrimWhitespace(descriptorArg).empty()) {
        if (errmsg) *errmsg = "The ckauto device type doesn't take any parameters";
        return NULL;
    }

    vector<CKdevice> devices = CKdiscoverDevices(errmsg);
    if (devices.empty()) {
        if (errmsg) *errmsg = "Didn't detect any ColorKinetics devices.";
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

//---------------------------------------------------------------------
// Polling the CK devices on the network
//---------------------------------------------------------------------

vector<CKinfo> CKdiscoverInfo(string* errmsg, int timeoutInMS) {
    vector<CKinfo> retval;

    // Create the client socket for the polling request
    SockAddr sa(INADDR_BROADCAST, KiNETudpPort); //use broadcast address
    SocketUDPClient sock(sa);
    sock.setsockopt_bool(SOL_SOCKET, SO_BROADCAST, true);

    // Broadcast the poll request
    // Note: QuickPlayPro sends out 4 PORTOUT_SYNC packets before the poll. Not sure if that's important.
    KiNETdiscover pollPacket;
    sock.Write((char*) &pollPacket, pollPacket.GetSize());
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error writing CK KiNETdiscover packet: " + sock.GetLastError();
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
            if (devInfo.SetFromDiscoverReply(buffer, bytesRead, &errmsg))
                retval.push_back(devInfo);
        } else {
            if (errmsg) *errmsg = "Error reading CK KiNETdiscover response: " + sock.GetLastError();
            return retval;
        }
    }
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error waiting for poll response: " + sock.GetLastError();
        return retval;
    }
    return retval;
}

const int kDefaultCountTimeout = 2500; // timeout in MS
bool GetDevicesForInfoV1(const CKinfo& info, vector<CKdevice>* devices, string* errmsg = NULL) {
    const int buflen = 1000;
    char buffer[buflen];
    int bytesRead = 0;
    IPAddr ipaddr(info.ipaddr);
    int universe = info.universe;
    // Write the GetCount request
    SocketUDPClient sock(ipaddr, KiNETudpPort);
    KiNETblinkScan1 getCountPacket;
    sock.Write((char*) &getCountPacket, getCountPacket.GetSize());
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error writing CK getCount1 packet: " + sock.GetLastError();
        sock.Close();
        return false;
    }

    // Parse the results
    if (sock.HasData(kDefaultCountTimeout) && sock.Read(buffer, buflen, &bytesRead)) {
        if (bytesRead >= KiNETblinkScan1CAReply::GetSize()) {
            KiNETblinkScan1CAReply* replyPtr = (KiNETblinkScan1CAReply*) buffer;
            int count = 0;
            for (int i = 0; i < 4; ++i)
                count += replyPtr->counts[i];
            CKdevice dev(ipaddr, universe, 0, count); // v1 devices don't really use the port concept
            dev.SetKiNetVersion(1); // This is a v1 feature
            devices->push_back(dev);
            sock.Close();
            return true;
        }
        else {
            if (errmsg)
                *errmsg = "Error: CK getCount1Reply packet was too short. Got " + IntToStr(bytesRead) + " bytes, but was expecting at least " + IntToStr(KiNETblinkScan1Reply::GetSize()) + " bytes";
            sock.Close();
            return false;
        }
    }

    if (sock.HasError()) {
        if (errmsg)
            *errmsg = "Error reading CK getCount1Reply packet: " + sock.GetLastError();
        sock.Close();
        return false;
    } else {
        sock.Close();
        return true;
    }
}

bool GetDevicesForInfoV2(const CKinfo& info, vector<CKdevice>* devices, string* errmsg = NULL) {
    const int buflen = 1000;
    char buffer[buflen];
    int bytesRead = 0;
    IPAddr ipaddr(info.ipaddr);
    // Write the GetCount request
    SocketUDPClient sock(ipaddr, KiNETudpPort);
    KiNETblinkScan2 getCountPacket;
    sock.Write((char*) &getCountPacket, getCountPacket.GetSize());
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error writing CK blinkScan2 packet: " + sock.GetLastError();
        sock.Close();
        return false;
    }

    // Parse the results
    while (sock.HasData(kDefaultCountTimeout) && sock.Read(buffer, buflen, &bytesRead)) {
        //cout << "BytesRead=" << bytesRead << "  size " << KiNETblinkScan2Reply::GetSize() << endl;
        if (bytesRead >= KiNETblinkScan2Reply::GetSize()) {
            KiNETblinkScan2Reply* replyPtr = (KiNETblinkScan2Reply*) buffer;
            if (replyPtr->replyType == KiNETblinkScan2Reply::kEnd) break; // Done reading
            if (replyPtr->replyType == KiNETblinkScan2Reply::kData) {
                // We have the count data
                KiNETblinkScan2Data* countDataPtr = (KiNETblinkScan2Data*) (buffer + KiNETblinkScan2Reply::GetSize());
                KiNETblinkScan2Data* countDataEnd = (KiNETblinkScan2Data*) (buffer + bytesRead);
                while (countDataPtr < countDataEnd) {
                    int universe = info.universe;
                    int port = countDataPtr->portnum;
                    int count = countDataPtr->count;
                    if (port >= 1 && count > 0) {
                        CKdevice dev(ipaddr, universe, port, count);
                        if (info.kinetVersion == 1 || info.kinetVersion == 2)
                            dev.SetKiNetVersion(info.kinetVersion);
                        devices->push_back(dev);
                    }
                    ++countDataPtr;
                }
            }
        }
    }
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error reading CK blinkScan2Reply packet: " + sock.GetLastError();
        sock.Close();
        return false;
    }

    sock.Close();
    return true;
}

vector<CKdevice> CKdiscoverDevices(string* errmsg, int timeoutInMS) {
    // Get all the info structures
    vector<CKinfo> infos = CKdiscoverInfo(errmsg, timeoutInMS);
    return CKdiscoverDevices(infos,errmsg);
}

vector<CKdevice> CKdiscoverDevices(const vector<CKinfo>& infos, string* errmsg) {
    vector<CKdevice> devices;

    // For each info, poll for the count of lights
    for (size_t i = 0; i < infos.size(); ++i) {
        CKinfo info = infos[i];
        switch (info.kinetVersion)
        {
        case 1:
            // Old protocol
            GetDevicesForInfoV1(info, &devices, errmsg);
            break;
        default:
            // Default to newer protocol
            GetDevicesForInfoV2(info, &devices, errmsg);
        }
    }

    return devices;
}
