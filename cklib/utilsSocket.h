// Socket routines including UDP support

#ifndef __UTILS_SOCKET_H
#define __UTILS_SOCKET_H

#include "utils.h"
#include "utilsIP.h"

#ifdef WIN32
#include <winsock2.h>
#else
#define SOCKET int
#endif

//----------------------------------------------------------------------------
// Sockets
//----------------------------------------------------------------------------

class Socket;

// constants
static const int kSocketTimeoutNever = -1;
static const int kSocketTimeoutError = -999;

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

		// Status
		bool            IsOpen() const   {return iIsOpen;}
		bool            HasError()  const   {return !iLastError.empty();}
		string			GetLastError()	const	{return iLastError;}

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
// Client class
//---------------------------------------------

class SocketUDPClient : public Socket
	{
	public:
		SocketUDPClient();

		SocketUDPClient     (const SockAddr& sa);
		bool    SetSockAddr (const SockAddr& sa);

		SocketUDPClient		(const IPAddr& ip, int port);
		bool    SetSockAddr	(const IPAddr& ip, int port);

		bool	Write       (const char* source, int len);

		struct Buffer {const void* ptr; int len; Buffer(const void* ptrArg = NULL, int lenArg = 0) : ptr(ptrArg), len(lenArg) {}};
		bool	MultiWrite  (const Buffer* buffers, int count);

	private:
	    SockAddr    iSockAddr;

		// Disallow copying
		SocketUDPClient(const SocketUDPClient&);
		const SocketUDPClient& operator=(const SocketUDPClient&);
	};

#endif // UTILS_SOCKET_H
