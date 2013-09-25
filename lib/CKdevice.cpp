// ColorKinetics Device Support. KiNET protocol support

#include "CKdevice.h"
#include "KiNET.h"
#include "utilsTime.h"
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
