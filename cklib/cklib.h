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
    CKdevice(const IPAddr& ip, int count = 100) : iIP(ip), iCount(count) {}
    bool        HasError()          const {return !iLastError.empty();}
    string      GetLastError()      const {return iLastError;}
    string      GetDescription()    const;
    int         GetCount()          const {return iCount;}

    bool        Write(const char* buffer, int len);

    // Copying (copy everything but socket and error)
    CKdevice(const CKdevice& dev) : iIP(dev.iIP), iCount(dev.iCount) {}
    void operator=(const CKdevice& dev) {iSocket.Close(); iLastError.clear(); iIP = dev.iIP; iCount = dev.iCount;}

private:
    IPAddr          iIP;
    int             iCount;
    SocketUDPClient iSocket;
    string          iLastError;
};

class CKbuffer : public LBuffer
{
public:
    CKbuffer() : LBuffer() {}
    CKbuffer(const CKdevice& dev) : LBuffer() {AddDevice(dev);}
    bool    AddDevice(const CKdevice& dev);
    bool    HasError() const {for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i) if (i->HasError()) return true; return false;}
    string  GetLastError() const {for (DevIter_t i = iDevices.begin(); i != iDevices.end(); ++i) if (i->HasError()) return i->GetLastError(); return string();}
    bool    Update();
private:
    typedef vector<CKdevice>::const_iterator  DevIter_t;
    vector<CKdevice> iDevices;
};

#endif // __CKLIB_H
