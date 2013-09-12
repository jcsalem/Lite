// ColorKinetics C++ Library utilities
// Attempts to be cross platform
//

#ifndef __CKLIB_H
#define __CKLIB_H

#include "utils.h"
#include "Color.h"
#include "LBuffer.h"
#include "utilsIP.h"
#include "utilsSocket.h"
#include <vector>

namespace CK
{
    typedef enum {kNull = 0, kSerial = 1, kChromasic = 2, kChromasicV2 = 3} PortType_t;
    string PortTypeToString(PortType_t type);
    const int kAnyUniverse = -1;  // This means to output to any universe on the device
    const int kDefaultPollTimeout =  250; // default timeout in MS
};

const int kDefaultKiNetVersion = 2; // This is the default KiNet version to communicate to an older device. Some old 24v supplies don't support v2

// String representation of a CK device
//   IP/port(count)
//  Port may be followed by r or R if it should be reversed.
//  Port defaults to 1.
//  Count defaulta to 50.
// Examples:
//  172.24.22.51
//  172.24.22.51/1
//  172.24.22.51/1r
//  172.24.22.51/2R(25)
//  172.24.22.51(25)

class CKdevice
{
public:
    CKdevice(const IPAddr& ip, int universe = CK::kAnyUniverse, int port = 1, int count = 50) :
        iIP(ip), iUniverse(universe), iPort(port), iCount(count), iKiNetVersion(kDefaultKiNetVersion) {}
    CKdevice(csref devstr);
    bool        HasError()          const {return !iLastError.empty();}
    string      GetLastError()      const {return iLastError;}
    string      GetDescriptor()     const;
    bool        Update()            const;

    // Specific to this device
    IPAddr      GetIP()             const {return iIP;}
    int         GetPort()           const {return iPort;}
    int         GetUniverse()       const {return iUniverse;}
    int         GetCount()          const {return iCount;}
    int         GetKiNetVersion()   const {return iKiNetVersion;}

    void        SetKiNetVersion(int version)    {iKiNetVersion = version;}
    bool        UpdateKiNetVersion(string* errmsg = NULL);           // Ask the light what version it is

    // Writes a KiNET UDP packet to this device
    bool        Write(const char* buffer, int len);

    // Copying (copy everything but socket and error)
    CKdevice(const CKdevice& dev) : iIP(dev.iIP), iUniverse(dev.iUniverse), iPort(dev.iPort), iCount(dev.iCount), iKiNetVersion(dev.iKiNetVersion) {}
    CKdevice& operator=(const CKdevice& dev) {iSocket.Close(); iLastError.clear(); iIP = dev.iIP; iPort = dev.iPort; iUniverse = dev.iUniverse; iCount = dev.iCount; iKiNetVersion = dev.iKiNetVersion; return *this;}
private:
    // These describe the device
    IPAddr          iIP;
    int             iUniverse;  // The device's universe setting
    int             iPort;      // This is the fixure port NOT the udp port which is hard-coded
    int             iCount;     // Number of lights controlled
    int             iKiNetVersion;
    // Used to manage communication
    SocketUDPClient iSocket;
    string          iLastError;
};

// Getting information about the connected CK devices

// This class parses the discoverReply info into an easy-to-access info structure
struct CKinfo {
    CKinfo() {Clear();}
    CKinfo(const char* discoverReplyBuffer, int replyLength) {SetFromDiscoverReply(discoverReplyBuffer, replyLength);}
    bool SetFromDiscoverReply(const char* discoverReplyBuffer, int replyLength, string* errmsg = NULL);
    void Clear();
    uint32  ipaddr;
    string  macaddr;
    uint16  kinetVersion;
    uint32  serial;
    uint32  universe;
    string  info;
    string  name;
};

vector<CKdevice>    CKdiscoverDevices(string* errmsg = NULL, int timeoutInMS = CK::kDefaultPollTimeout);
vector<CKdevice>    CKdiscoverDevices(const vector<CKinfo>& infos, string* errmsg = NULL);
vector<CKinfo>      CKdiscoverInfo   (string* errmsg = NULL, int timeoutInMS = CK::kDefaultPollTimeout);

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

#endif // __CKLIB_H
