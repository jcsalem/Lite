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
};

// String representation of a CK device
//   IP@port(count)
//  Port may be followed by r or R if it should be reversed.
//  Port defaults to 1.
//  Count defaulta to 50.
// Examples:
//  172.24.22.51
//  172.24.22.51@1
//  172.24.22.51@1r
//  172.24.22.51@2R(25)
//  172.24.22.51(25)


class CKdevice
{
public:
    CKdevice(const IPAddr& ip, int port = 1, int count = 50) : iIP(ip), iPort(port), iCount(count), iLayout(CK::kNormal) {}
    CKdevice(csref devstr);
    bool        HasError()          const {return !iLastError.empty();}
    string      GetLastError()      const {return iLastError;}
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
private:
    typedef vector<CKdevice>::const_iterator  DevIter_t;
    vector<CKdevice>    iDevices;
    string              iLastError;
};

#endif // __CKLIB_H
