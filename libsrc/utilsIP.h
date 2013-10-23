// Standard utilities for dealing with IP addresses

#ifndef _UTILSIP_H
#define _UTILSIP_H

#include "utils.h"

#ifdef WIN32
#include <winsock2.h>
#else
// Standard Mac/Linux
#include <netinet/in.h>
#endif

class IPAddr
    {
    public:
        IPAddr(const char* name)    {Init(name);}
        IPAddr(csref name)          {Init(name.c_str());}
        IPAddr(uint32 ipaddr = 0) : iIP(ipaddr) {}

        bool IsValid() const;
        uint32 GetIP() const {return iIP;}
        string GetString() const;

        bool operator==(const IPAddr& ip) const     {return iIP == ip.iIP;}
        bool operator!=(const IPAddr& ip) const     {return !(*this == ip);}


    private:
        void Init(const char* name);
        uint32 iIP;
    };

class SockAddr
    {
    public:
        SockAddr(const IPAddr& addr, int port) {iAddr = addr; iPort = port; InitStruct();}
        SockAddr() : iAddr(IPAddr()), iPort(0) {InitStruct();}
        SockAddr(const sockaddr* sa, int len);
        SockAddr(const sockaddr_in& saddr);

        // Accessors
        IPAddr                  GetIPAddr()         const   {return iAddr;}
        int                     GetPort()           const   {return iPort;}
		const struct sockaddr*	GetStruct()	    	const   {return (const struct sockaddr*) &iStruct;}
		int						GetStructSize()	    const	{return sizeof(struct sockaddr_in);}
        string                  GetString()         const;
        bool                    IsValid()           const   {return iAddr.IsValid() && iPort != 0;}

        // equalities
		bool operator==(const SockAddr& sa) const {return iAddr == sa.iAddr && iPort == sa.iPort;}
		bool operator!=(const SockAddr& sa) const {return !(*this == sa);}

	private:
	    void InitStruct();
		IPAddr  			iAddr;
		int 				iPort;
		struct sockaddr_in	iStruct;
    };

#endif // utilsIP_H
