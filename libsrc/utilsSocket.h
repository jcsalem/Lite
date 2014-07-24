// Socket routines including UDP support

#ifndef __UTILS_SOCKET_H
#define __UTILS_SOCKET_H

#include "utils.h"
#include "utilsIP.h"

#ifdef WIN32
#include <winsock2.h>
#else
// Standard Mac/Linux
#define SOCKET int
#endif

//----------------------------------------------------------------------------
// Sockets
//----------------------------------------------------------------------------

class Socket;

//---------------------------------------------
// Base class
//---------------------------------------------

class Socket
	{
	public:
		// Close, reset, etc.
		virtual bool	Close();
		virtual void	Reset(); 		// Like close but resets internal values
		virtual			~Socket();

		// sockopt
		bool setsockopt_bool(int level, int optname, bool value);

		// Status
		bool            IsOpen()        const   {return iIsOpen;}
		bool            HasError()      const   {return !iLastError.empty();}
		string			GetLastError()	const	{return iLastError;}
		SOCKET          GetSocket()     const   {return iSocket;}

		// Constants
		static const int kInfinite = -1;

    protected:
		Socket();
		SOCKET		iSocket;
		bool        iIsOpen;
		string		iLastError;

	private:
		// Disallow creation or copying of this type
		Socket(const Socket&);
		void operator=(const Socket&);
	};

//---------------------------------------------
// IP socket (RAW IP type)
//---------------------------------------------

class SocketIP : public Socket
	{
	public:
	    virtual ~SocketIP() {}
	    SocketIP() : Socket() {}
	    SocketIP(const SockAddr& sa) : Socket() {SetSockAddr(sa);}
	    SocketIP(const IPAddr& ip, int port) : Socket() {SetSockAddr(ip, port);}
		
		bool            SetSockAddr	(const IPAddr& ip, int port) {return SetSockAddr(SockAddr(ip, port));}
		virtual bool    SetSockAddr (const SockAddr& sa);
		
		bool	Write       (const char* source, int len);

		struct Buffer {const void* ptr; int len; Buffer(const void* ptrArg = NULL, int lenArg = 0) : ptr(ptrArg), len(lenArg) {}};
		bool	MultiWrite  (const Buffer* buffers, int count);

		// Returns true when the socket has data. Returns false if an error or a timeout occurs.	   
		bool    HasData     (int timeoutInMS = kInfinite); 

		// Discards everything that's in the receive queue
		bool    Discard     ();

        // For SocketUDPClient, reads must only be done after writes (since writing sets the local port). 
		// Use the SocketUDPServer if you're making a server. This binds the local port during initialization
		bool    Read        (char* buffer, int buflen, int* bytesRead = NULL);
		bool    Read        (char* buffer, int buflen, SockAddr* srcAddr, int* bytesRead = NULL); // Returns the receiving socket


	protected:
	    SockAddr        iSockAddr;
		virtual int     GetIPtype() const {return SOCK_RAW;}
		virtual int     GetIPproto() const {return IPPROTO_IP;}

    private:
		// Disallow copying
		SocketIP(const SocketIP&);
		const SocketIP& operator=(const SocketIP&);
	};

//---------------------------------------------
// ICMP type socket
//---------------------------------------------

class SocketICMP : public SocketIP
	{
	public:
     	SocketICMP() : SocketIP() {}
		virtual ~SocketICMP() {}

		SocketICMP (const SockAddr& sa) : SocketIP(sa) {}
		SocketICMP (const IPAddr& ip, int port) : SocketIP(ip, port) {}

	protected:
		virtual int     GetIPtype() const {return SOCK_DGRAM;}
		virtual int     GetIPproto() const {return IPPROTO_ICMP;}

    private:
		// Disallow copying
		SocketICMP(const SocketICMP&);
		const SocketICMP& operator=(const SocketICMP&);
	};

//---------------------------------------------
// UDP type socket (base class)
//---------------------------------------------

class SocketUDP : public SocketIP
	{
	public:
		virtual ~SocketUDP() {}
     	SocketUDP() : SocketIP() {}
		SocketUDP(const SockAddr& sa) : SocketIP(sa) {}
		SocketUDP(const IPAddr& ip, int port) : SocketIP(ip, port) {}

	protected:
		virtual int     GetIPtype() const {return SOCK_DGRAM;}
		virtual int     GetIPproto() const {return IPPROTO_UDP;}

    private:
		// Disallow copying
		SocketUDP(const SocketUDP&);
		const SocketUDP& operator=(const SocketUDP&);
	};

//---------------------------------------------
// SocketUDPClient
//---------------------------------------------

class SocketUDPClient : public SocketUDP
	{
	public:
		virtual ~SocketUDPClient() {}
		SocketUDPClient() : SocketUDP() {}
		SocketUDPClient(const SockAddr& sa) : SocketUDP(sa) {}
		SocketUDPClient(const IPAddr& ip, int port) : SocketUDP(ip, port) {}

	private:
		// Disallow copying
		SocketUDPClient(const SocketUDPClient&);
		const SocketUDPClient& operator=(const SocketUDPClient&);
	};


//---------------------------------------------
// SocketUDPServer
//---------------------------------------------

class SocketUDPServer : public SocketUDP
	{
	public:
		virtual ~SocketUDPServer() {}
		SocketUDPServer() : SocketUDP() {}
		SocketUDPServer(const SockAddr& sa) : SocketUDP(sa) {}
		SocketUDPServer(const IPAddr& ip, int port) : SocketUDP(ip, port) {}
	    SocketUDPServer(int port) : SocketUDP(IPAddr((uint32)INADDR_ANY), port) {}

        // Need to override this because we bind the socket rather than simply open it
		virtual bool SetSockAddr(const SockAddr& sa);

	private:
		// Disallow copying
		SocketUDPServer(const SocketUDPServer&);
		const SocketUDPServer& operator=(const SocketUDPServer&);
	};

#endif // UTILS_SOCKET_H
