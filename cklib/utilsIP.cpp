// IP Address utilities

#include "utils.h"
#include "utilsIP.h"
#include <sstream>


//-----------------------------------------------------
// IPAddr
//-----------------------------------------------------

// Converting from an IP address to a string
static const uint32 kBadIP = 0xFFFFFFFEul;

static uint32 IPFromString(const char* str)
	{
    if (! str) return kBadIP;
    uint32 ip = 0;
    char c = *str++;

    // Trim leading whitespace
    while (c != 0 && IsWhitespace(c))
        c = *str++;

    int octet_count = 0;
    int octet = 0;
    while (c != 0) // Loop to end of string
        {
        if (c >= '0' && c <= '9')
            octet = octet * 10 + c - '0';
        else if (c >= 'A' && c <= 'F')
            octet = octet * 10 + c - 'A';
        else if (c >= 'a' && c <= 'f')
            octet = octet * 10 + c - 'a';
        else if (c == '.')
            {
            ip = (ip << 8) + octet;
            ++ octet_count;
            octet = 0;
            }
        else if (IsWhitespace(c))
            break;
        else
            return kBadIP;
        c = *str++;
        }
    if (octet_count != 3)
        return kBadIP;
    ip = (ip << 8) + octet;

   // Skip training whitespace
    while (c != 0)
        {
        c = *str++;
        if (!IsWhitespace(c))
            return kBadIP;
        }
    return ip;
	}

void IPAddr::Init(const char* addr)
    {
    iIP = IPFromString(addr);
    }

bool IPAddr::IsValid() const
    {
    return iIP != kBadIP;
    }

string IPAddr::GetString() const
    {
    stringstream ss;
    ss << ((iIP >> 24) & 0xFF) << "." << ((iIP >> 16) & 0xFF) << "." << ((iIP >> 8) & 0xFF) << "." << (iIP & 0xFF);
    return ss.str();
    }

//-----------------------------------------------------
// SockAddr
//-----------------------------------------------------


SockAddr::SockAddr(const sockaddr* sa, int len) {
    if (sa && sa->sa_family == AF_INET && len >= (int) sizeof(sockaddr_in))
        *this = SockAddr(*((sockaddr_in*) sa));
    else
        *this = SockAddr(kBadIP, 0);
    }

#ifdef WIN32
// Win32
#define SINADDR_TO_IP(_sa_addr) (_sa_addr.S_un.S_addr)
#else
// Mac and Linux
#define SINADDR_TO_IP(_sa_addr) (_sa_addr.s_addr)
#endif

SockAddr::SockAddr(const sockaddr_in& sa) {
    if (sa.sin_family == AF_INET) {
        iAddr = IPAddr(ntohl(SINADDR_TO_IP(sa.sin_addr)));
        iPort = ntohs(sa.sin_port);
        iStruct = sa;
    } else {
        *this = SockAddr(kBadIP, 0);
    }
}

void SockAddr::InitStruct()
    {
    memset(&iStruct, 0, sizeof(iStruct));
    iStruct.sin_family      = AF_INET;
    iStruct.sin_addr.s_addr = htonl(iAddr.GetIP());
    iStruct.sin_port        = htons(iPort);
    }

string SockAddr::GetString() const
    {
    stringstream ss;
    ss << iAddr.GetString() << ":" << iPort;
    return ss.str();
    }

