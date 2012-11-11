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
    extern const string kPDSinfoHelp;  // A help string describing the PDSInfo
    const int kDefaultPollTimeout =  250; // default timeout in MS
}

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
    CKdevice(const IPAddr& ip, int port = 1, int count = 50) : iIP(ip), iPort(port), iCount(count), iLayout(CK::kNormal) {}
    CKdevice(csref devstr);
    bool        HasError()          const {return !iLastError.empty();}
    string      GetLastError()      const {return iLastError;}
    string      GetPath()           const;
    string      GetDescription()    const;
    IPAddr      GetIP()             const {return iIP;}
    int         GetPort()           const {return iPort;}
    int         GetCount()          const {return iCount;}
    CK::Layout_t GetLayout()        const {return iLayout;}
    void        SetLayout(CK::Layout_t layout)  {iLayout = layout;}

    bool        Write(const char* buffer, int len);

    // Copying (copy everything but socket and error)
    CKdevice(const CKdevice& dev) : iIP(dev.iIP), iPort(dev.iPort), iCount(dev.iCount), iLayout(dev.iLayout) {}
    void operator=(const CKdevice& dev) {iSocket.Close(); iLastError.clear(); iIP = dev.iIP; iPort = dev.iPort; iCount = dev.iCount; iLayout = dev.iLayout;}

private:
    // These describe the device
    IPAddr          iIP;
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
    bool    AddDevice(const CKdevice& dev);
    bool    AddDevice(csref desc);
    bool    HasError() const;
    string  GetLastError() const;
    string  GetDescription() const;
    bool    Update();

    // Alternative creation methods
    static bool    CreateFromArglist(CKbuffer* buffer, int* argc, char** argv);
    static const char*    kArglistArgs;
    static const char*    kArglistDoc;
    static bool    CreateFromXML(CKbuffer* buffer, const CKxmldoc& xmldoc);

private:
    typedef vector<CKdevice>::const_iterator  DevIter_t;
    vector<CKdevice>    iDevices;
    string              iLastError;
};

#endif // __CKLIB_H
