// Socket utilities
// TODO:
//  For multi-thread, make net initialize code and SocketErrorCode reentrant

#include "utilsSocket.h"
#include <string.h>

//----------------------------------------------------------------------------
// Platform specific
//----------------------------------------------------------------------------

#ifdef WIN32
// Linux compatibility
typedef int socklen_t;

string SocketErrorString()
	{
	return ErrorCodeString(WSAGetLastError());
	}

bool DoNetInit(string* errmsg = NULL)
	{
	WSADATA wsaData;
	int majorVer = 2;
	int minorVer = 0;

	WORD ver = MAKEWORD(majorVer, minorVer); /* Version 2.0 required okay */

	int status = WSAStartup (ver, &wsaData);

	if (status != 0)
		{
		if (errmsg) *errmsg = "Failed to initialize Windows networking (code " + IntToStr(status) + ")";
		return false;
		}

	/* Confirm that the WinSock DLL supports the requested version.*/
	if (wsaData.wVersion != ver)
		{
		WSACleanup();
		if (errmsg)
			*errmsg = "Failed to initialize Windows networking: Version " +
				IntToStr(majorVer) + "." + IntToStr(minorVer) + " was not supported.";
		return false;
		}
    return true;
	}

// Not used.  Not sure if this is a problem.
void NetCleanup()
	{
	WSACleanup();
	}

#else // !WIN32

#include "unistd.h"
#include <sys/socket.h>
#include <netinet/in.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

string SocketErrorString()
	{
    return ErrorCodeString();
	}

static bool DoNetInit(string* errmsg = NULL) {
    return true;
}

#endif

// Generic initialization code
namespace {
    bool gNetIsInitialized = false;
};

static bool NetInitIfNeeded()
    {
    if (gNetIsInitialized)
        return true;
    if (!DoNetInit())
        return false;
    gNetIsInitialized = true;
    return true;
    }

//--------------------------------------------------
// Open/Close
//--------------------------------------------------

Socket::Socket()
	{
	iSocket	= INVALID_SOCKET;
	iIsOpen = false;
	Reset();
	}

Socket::~Socket()
	{
	Reset();
	}

bool Socket::Close()
	{
#ifdef WIN32
	int status  = closesocket(iSocket);
#else
	int status  = close(iSocket);
#endif
	iSocket	= INVALID_SOCKET;
	iIsOpen = false;
    if (status == SOCKET_ERROR)
		{
		iLastError = "Error closing socket: " + SocketErrorString();
		return false;
		}
	return true;
	}

void Socket::Reset()
	{
	if (iIsOpen)
        Close();
	iSocket			= INVALID_SOCKET;
	iLastError = "";
	}

bool Socket::setsockopt_bool(int level, int optname, bool value) {
    int bvalue = value ? 1 : 0;
    int retval = setsockopt(iSocket, level, optname, (char*) &bvalue, sizeof (int));
    if (retval == 0) return true;
    iLastError = "setsockopt error: " + SocketErrorString();
    return false;
}

//--------------------------------------------------------------------------
// SocketUDP
//--------------------------------------------------------------------------

SocketUDP::SocketUDP() : Socket() {}

SocketUDP::SocketUDP (const IPAddr& addr, int port)
	: Socket() {
	SetSockAddr(addr, port);
}

SocketUDP::SocketUDP(const SockAddr& sa)
	: Socket() {
	SetSockAddr(sa);
}

bool SocketUDP::SetSockAddr(const SockAddr& sa) {
	if (! sa.IsValid())
		{
		if (iIsOpen) Close();
		iLastError = "Invalid socket address: " + sa.GetString();
        return false;
		}

    // Set address
    iSockAddr = sa;

	// Create socket (if needed)
	if (! iIsOpen)
        {
        NetInitIfNeeded();
        iSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (iSocket == INVALID_SOCKET)
            {
            iLastError = "Error opening socket: " + SocketErrorString();
            return false;
            }
        iIsOpen = true;
        }
    return true;
}

//--------------------------------------------------
// SocketUDPClient
//--------------------------------------------------

bool SocketUDPClient::Write(const char* source, int len)
	{
	if (! source && len > 0)
		{
		iLastError = "Socket::Write was not given a source string";
		return false;
		}

    Buffer buf;
    buf.ptr = source;
    buf.len = len;

	return MultiWrite(&buf, 1);
	}

const int kSocketMaxWriteBuffers = 16;

bool SocketUDPClient::MultiWrite(const Buffer* buffers, int count)
	{
    if (! iIsOpen)
        {
        iLastError = "Socket wasn't open";
        return false;
        }
	if (count <= 0 || count > kSocketMaxWriteBuffers )
		{
		iLastError = "Socket::MultiWrite had too many or too few buffers (" + IntToStr(count) + ". Max is " + IntToStr(kSocketMaxWriteBuffers) + ")";
		return false;
		}

    // How many bytes were planned to write and how many were actually written
	unsigned long totalBytes = 0;


#ifdef WIN32
	unsigned long bytesWritten = 0;
	// Build up the WSABUF list
	WSABUF bufs[kSocketMaxWriteBuffers];

	int bufsIdx = 0;
	for (int i = 0; i < count; ++i)
		{
		if (buffers[i].len > 0)
			{
			bufs[bufsIdx].len = buffers[i].len;
			bufs[bufsIdx].buf = (char *)(buffers[i].ptr);
			++bufsIdx;
			totalBytes += buffers[i].len;
			}
		}

	// Write the data
	if (WSASendTo(iSocket, bufs, bufsIdx, &bytesWritten, 0, iSockAddr.GetStruct(), iSockAddr.GetStructSize(), NULL, NULL) == SOCKET_ERROR)
        {
        // Unknown error
        iLastError = "Socket error trying to write " + IntToStr(totalBytes) + " bytes: " + SocketErrorString()
            + " (" + IntToStr(bytesWritten) + " bytes written)";
        return false;
        }
#else // WIN32
    // Mac and Linux version
    // Prepare the buffer
	long bytesWritten = 0;
    void* bufptr;
    if (count == 1) {
        bufptr = const_cast<void*>(buffers[0].ptr);
        totalBytes = buffers[0].len;
    } else {
        // multiple buffers that we need to combine
        for (int i = 0; i < count; ++i)
            totalBytes += buffers[i].len;
        bufptr = malloc(totalBytes);
        char* ptr = (char*) bufptr;
        for (int i = 0; i < count; ++i) {
            memcpy(ptr, buffers[i].ptr, buffers[i].len);
            ptr += buffers[i].len;
        }
    }

    // Write the buffer
    bytesWritten = sendto(iSocket, bufptr, totalBytes, 0, iSockAddr.GetStruct(), iSockAddr.GetStructSize());
    if (count != 1) free(bufptr); // Free the temp buffer (if needed)

    // Test for error
    if (bytesWritten < 0) {
        iLastError = "Socket error trying to write " + IntToStr(totalBytes) + " bytes: " + SocketErrorString()
            + " (" + IntToStr(bytesWritten) + " bytes written)";
        return false;
    }

#endif // Mac/Linux

	if (bytesWritten == 0)
		{
		iLastError = "Zero bytes written. Socket closed?";
		return false;
		}

    if ((unsigned long) bytesWritten != totalBytes)
		{
		iLastError = "Partial write.  Only " + IntToStr(bytesWritten) + " out of " + IntToStr(totalBytes);
		return false;
		}
	return true;
	}

bool SocketUDPClient::Read(char* buffer, int buflen, int* bytesRead) {
    if (! iIsOpen)
        {
        iLastError = "Socket wasn't open";
        return false;
        }
    int numRead = recv(iSocket, buffer, buflen, 0);
    if (numRead == SOCKET_ERROR) {
        if (bytesRead) *bytesRead = 0;
        iLastError = "Error reading from socket: " + SocketErrorString();
        return false;
    }
    if (bytesRead) *bytesRead = numRead;
    return true;
}

bool SocketUDPClient::HasData(int timeoutInMS) {
    if (! iIsOpen)
        {
        iLastError = "Socket wasn't open";
        return false;
        }
    // Setup fd_set
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(iSocket, &fdset);
    // Set up timeval
    struct timeval tv;
    struct timeval* tvarg = &tv;
    if (timeoutInMS == kInfinite)
        tvarg = NULL;
    else {
        tv.tv_sec = timeoutInMS / 1000;
        tv.tv_usec = (timeoutInMS % 1000) * 1000;
    }

    int status = select(iSocket+1, &fdset, NULL, NULL, tvarg);
    switch (status) {
        case SOCKET_ERROR:
            iLastError = "Error waiting for data to be available: " + SocketErrorString();
            return false;
        case 0: // Timeout
            return false;
        default: // data ready to be received
            return true;
    }
}

//--------------------------------------------------
// SocketUDPServer
//--------------------------------------------------

SocketUDPServer::SocketUDPServer(const SockAddr& sa) : SocketUDP() {
    SetSockAddr(sa);
}
SocketUDPServer::SocketUDPServer(const IPAddr& ip, int port) : SocketUDP() {
    SetSockAddr(SockAddr(ip, port));
}
SocketUDPServer::SocketUDPServer(int port) : SocketUDP() {
    SetSockAddr(SockAddr(IPAddr((uint32)INADDR_ANY), port));
}

bool SocketUDPServer::SetSockAddr(const SockAddr& sa) {
    if (! SocketUDP::SetSockAddr(sa))
        return false;
    if (bind(iSocket, iSockAddr.GetStruct(), iSockAddr.GetStructSize())) {
        iLastError = "Error binding socket to " + iSockAddr.GetString() + ": " + SocketErrorString();
        return false;
    } else
        return true;
}

bool SocketUDPServer::Read(char* buffer, int buflen, int* bytesRead) {
    if (! iIsOpen)
        {
        iLastError = "Socket wasn't open";
        return false;
        }
    struct sockaddr_in rsa;
    socklen_t rsa_len = sizeof(rsa);
    memset(&rsa, 0, rsa_len);
    int numRead = recvfrom(iSocket, buffer, buflen, 0, (sockaddr*) &rsa, &rsa_len);
    if (numRead == SOCKET_ERROR) {
        if (bytesRead) *bytesRead = 0;
        iLastError = "Error reading from socket: " + SocketErrorString();
        return false;
    }
    if (bytesRead) *bytesRead = numRead;
    iLastSockAddr = SockAddr(rsa);
    if (iLastSockAddr.IsValid())
        return true;
    else {
        iLastError = "recvfrom didn't retrieve a valid sockaddr";
        return false;
    }
}

