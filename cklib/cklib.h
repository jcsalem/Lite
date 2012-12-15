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
    // Mapping of the lights into the CKbuffer;
    typedef enum {kNormal, kReverse} Layout_t;
    const int kDefaultPollTimeout =  250; // default timeout in MS
};

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

// Getting information about the connected CK devices

// This class parses the pollReply info into an easy-to-access info structure
struct CKinfo {
    CKinfo() {Clear();}
    CKinfo(const char* pollReplyBuffer, int replyLength) {SetFromPollReply(pollReplyBuffer, replyLength);}
    bool SetFromPollReply(const char* pollReplyBuffer, int replyLength, string* errmsg = NULL);
    void Clear();
    uint32  ipaddr;
    string  macaddr;
    uint16  numports;
    uint32  serial;
    uint32  universe;
    string  info;
    string  name;
};

class CKdevice
{
public:
    CKdevice(const IPAddr& ip, int universe = 0, int port = 1, int count = 50) : iIP(ip), iUniverse(universe), iPort(port), iCount(count), iLayout(CK::kNormal) {}
    CKdevice(csref devstr);
    bool        HasError()          const {return !iLastError.empty();}
    string      GetLastError()      const {return iLastError;}
    string      GetPath()           const;
    string      GetDescription()    const;
    bool        Update();

    // Specific to this device
    IPAddr      GetIP()             const {return iIP;}
    int         GetPort()           const {return iPort;}
    int         GetUniverse()       const {return iUniverse;}
    int         GetCount()          const {return iCount;}
    CK::Layout_t GetLayout()        const {return iLayout;}
    void        SetLayout(CK::Layout_t layout)  {iLayout = layout;}

    // Writes a KiNET UDP packet to this device
    bool            Write(const char* buffer, int len);

    // Copying (copy everything but socket and error)
    CKdevice(const CKdevice& dev) : iIP(dev.iIP), iUniverse(dev.iUniverse), iPort(dev.iPort), iCount(dev.iCount), iLayout(dev.iLayout) {}
    CKdevice& operator=(const CKdevice& dev) {iSocket.Close(); iLastError.clear(); iIP = dev.iIP; iPort = dev.iPort; iUniverse = dev.iUniverse; iCount = dev.iCount; iLayout = dev.iLayout; return *this;}
private:
    // These describe the device
    IPAddr          iIP;
    int             iUniverse;  // The device's universe setting
    int             iPort;      // This is the fixure port NOT the udp port which is hard-coded
    int             iCount;     // Number of lights controlled
    CK::Layout_t    iLayout;   //
    // Used to manage communication
    SocketUDPClient iSocket;
    string          iLastError;
};

vector<CKinfo>      CKpollForInfo   (string* errmsg = NULL, int timeoutInMS = CK::kDefaultPollTimeout);
vector<CKdevice>    CKpollForDevices(string* errmsg = NULL, int timeoutInMS = CK::kDefaultPollTimeout);
vector<CKdevice>    CKpollForDevices(const vector<CKinfo>& infos, string* errmsg = NULL);

class CKxmldoc; // fwd decl of generic xml doc
class CKbuffer : public LBuffer
{
public:
    CKbuffer() : LBuffer() {}
    CKbuffer(const CKdevice& dev) : LBuffer() {AddDevice(dev);}
    virtual ~CKbuffer() {}
    bool    AddDevice(const CKdevice& dev);
    bool    AddDevice(csref desc); // May add multiple devices if desc is a comma-separated list of devices

    virtual bool    HasError()       const;
    virtual string  GetLastError()   const;
    virtual string  GetDescription() const;
    virtual bool    Update();
    virtual bool    PortSync();

    // Alternative creation methods
    static bool    CreateFromArglist(CKbuffer* buffer, int* argc, char** argv);
    static bool    CreateFromXML(CKbuffer* buffer, const CKxmldoc& xmldoc);

private:
    typedef vector<CKdevice>::const_iterator  DevIter_t;
    vector<CKdevice>    iDevices;
    // Don't allow copying
    CKbuffer(const CKbuffer&);
    CKbuffer& operator=(const CKbuffer&);
};

#endif // __CKLIB_H
