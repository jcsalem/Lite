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

class CKdevice
{
public:
    CKdevice(const IPAddr& ip, int port = 1, int count = 50) : iIP(ip), iPort(port), iCount(count) {}
    bool        HasError()          const {return !iLastError.empty();}
    string      GetLastError()      const {return iLastError;}
    string      GetDescription()    const;
    IPAddr      GetIP()             const {return iIP;}
    int         GetPort()           const {return iPort;}
    int         GetCount()          const {return iCount;}

    bool        Write(const char* buffer, int len);

    // Copying (copy everything but socket and error)
    CKdevice(const CKdevice& dev) : iIP(dev.iIP), iPort(dev.iPort), iCount(dev.iCount) {}
    void operator=(const CKdevice& dev) {iSocket.Close(); iLastError.clear(); iIP = dev.iIP; iPort = dev.iPort; iCount = dev.iCount;}

private:
    IPAddr          iIP;
    int             iPort; // This is the fixure port NOT the udp port which is hard-coded
    int             iCount;
    SocketUDPClient iSocket;
    string          iLastError;
};

class CKbuffer : public LBuffer
{
public:
    CKbuffer() : LBuffer() {}
    CKbuffer(const CKdevice& dev, bool reverseOrder = false) : LBuffer() {AddDevice(dev, reverseOrder);}
    bool    AddDevice(const CKdevice& dev, bool reverseOrder = false);
    bool    HasError() const {for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i) if (i->HasError()) return true; return false;}
    string  GetLastError() const {for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i) if (i->HasError()) return i->GetLastError(); return string();}
    bool    Update();
private:
    typedef vector<CKdevice>::const_iterator  DevIter_t;
    vector<CKdevice>    iDevices;
    vector<bool>        iReverseFlag; // if true order of the lights is reversed
};

#endif // __CKLIB_H
