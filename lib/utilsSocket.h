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
// IP type socket (base class)
//---------------------------------------------

class SocketUDP : public Socket
	{
	public:
		SocketUDP();
		virtual ~SocketUDP() {}

		SocketUDP                   (const SockAddr& sa);
		virtual bool    SetSockAddr (const SockAddr& sa);

		SocketUDP    		        (const IPAddr& ip, int port);
		bool            SetSockAddr	(const IPAddr& ip, int port) {return SetSockAddr(SockAddr(ip, port));}

	protected:
	    SockAddr    iSockAddr;

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
		SocketUDPClient() : SocketUDP() {}
		SocketUDPClient(const SockAddr& sa) : SocketUDP(sa) {}
		SocketUDPClient(const IPAddr& ip, int port) : SocketUDP(ip, port) {}

		bool	Write       (const char* source, int len);

		struct Buffer {const void* ptr; int len; Buffer(const void* ptrArg = NULL, int lenArg = 0) : ptr(ptrArg), len(lenArg) {}};
		bool	MultiWrite  (const Buffer* buffers, int count);

        // For SocketUDPClient, reads must only be done after writes (since writing sets the local port). Use the SocketUDPServer if you're making a server.
		bool    Read        (char* buffer, int buflen, int* bytesRead = NULL);
		bool    HasData     (int timeoutInMS = kInfinite); // Returns true when the socket has data. Returns false if an error or a timeout occurs.

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
		SocketUDPServer() : SocketUDP() {}
		SocketUDPServer(const SockAddr& sa);
        SocketUDPServer(const IPAddr& ip, int port);
		SocketUDPServer(int port);

		virtual bool SetSockAddr(const SockAddr& sa);

        // Reads a UDP packet.
		bool    Read        (char* buffer, int buflen, int* bytesRead = NULL);
		bool    HasData     (int timeoutInMS = kInfinite); // Returns true when the socket has data. Returns false if an error or a timeout occurs.

        // Writes (TBD)
        // These need to write to the address that was most recently read from

	private:
	    SockAddr iLastSockAddr; // The sockaddr of the last packet received

		// Disallow copying
		SocketUDPServer(const SocketUDPServer&);
		const SocketUDPServer& operator=(const SocketUDPServer&);
	};

#endif // UTILS_SOCKET_H
