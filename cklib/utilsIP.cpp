// IP Address utilities

#include "utils.h"
#include "utilsIP.h"


//-----------------------------------------------------
// IPAddr
//-----------------------------------------------------

// Converting from an IP address to a string
static const uint32 kBadIP = 0xFFFFFFFFul;

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
    return iIP != kBadIP && iIP != 0;
    }

string IPAddr::GetString() const
    {
    char buffer[25];
    string ret;
    itoa((iIP >> 24) & 0xFF, buffer, 10);
    ret += string(buffer) + ".";
    itoa((iIP >> 16) & 0xFF, buffer, 10);
    ret += string(buffer) + ".";
    itoa((iIP >>  8) & 0xFF, buffer, 10);
    ret += string(buffer) + ".";
    itoa((iIP      ) & 0xFF, buffer, 10);
     return ret;
    }
//-----------------------------------------------------
// SockAddr
//-----------------------------------------------------

void SockAddr::InitStruct()
    {
    memset(&iStruct, 0, sizeof(iStruct));
    iStruct.sin_addr.s_addr = htonl(iAddr.GetIP());
    iStruct.sin_family      = AF_INET;
    iStruct.sin_port        = htons(iPort);
    }

string SockAddr::GetString() const
    {
    char port[20];
    itoa(iPort, port, 10);
    return iAddr.GetString() + ":" + port;
    }

