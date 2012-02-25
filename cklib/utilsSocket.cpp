// Socket utilities
// TODO:
//  For multi-thread, make net initialize code and SocketErrorCode reentrant

#include "utilsSocket.h"

//----------------------------------------------------------------------------
// Platform specific
//----------------------------------------------------------------------------

#ifdef WIN32
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

string SocketErrorString()
	{
    return ErrorCodeString();
	}

static bool DoNetInit(string* errmsg = NULL)
    {}

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

//--------------------------------------------------------------------------
// SocketUDPClient
//--------------------------------------------------------------------------

SocketUDPClient::SocketUDPClient()
	: Socket()
	{
	}

SocketUDPClient::SocketUDPClient (const IPAddr& addr, int port)
	: Socket()
	{
	SetSockAddr(addr, port);
	}

SocketUDPClient::SocketUDPClient(const SockAddr& sa)
	: Socket()
	{
	SetSockAddr(sa);
	}

bool SocketUDPClient::SetSockAddr(const IPAddr& addr, int port)
	{
	return SetSockAddr(SockAddr(addr, port));
	}

bool SocketUDPClient::SetSockAddr(const SockAddr& sa)
	{
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
// Writing
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
	if (count < 0 || count > kSocketMaxWriteBuffers )
		{
		iLastError = "Socket::MultiWrite had too many or too few buffers (" + IntToStr(count) + ". Max is " + IntToStr(kSocketMaxWriteBuffers) + ")";
		return false;
		}

	// Build up the WSABUF list
	WSABUF bufs[kSocketMaxWriteBuffers];

	int bufsIdx = 0;
	unsigned long totalBytes = 0;
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
	unsigned long bytesWritten;
	if (WSASendTo(iSocket, bufs, bufsIdx, &bytesWritten, 0, iSockAddr.GetStruct(), iSockAddr.GetStructSize(), NULL, NULL) == SOCKET_ERROR)
        {
        // Unknown error
        iLastError = "Socket error trying to write " + IntToStr(totalBytes) + " bytes: " + SocketErrorString()
            + " (" + IntToStr(bytesWritten) + " bytes written)";
        return false;
        }


	if (bytesWritten == 0)
		{
		iLastError = "Zero bytes written. Socket closed?";
		return false;
		}

    if (bytesWritten != totalBytes)
		{
		iLastError = "Partial write.  Only " + IntToStr(bytesWritten) + " out of " + IntToStr(totalBytes);
		return false;
		}
	return true;
	}

