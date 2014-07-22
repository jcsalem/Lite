// IP Address utilities

#include "utils.h"
#include "utilsIP.h"
#include <sstream>
#include <string.h>
#include <iostream>


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

void ReportInARPCacheError(csref errmsg)
{
  static bool hasReported = false;
  if (hasReported) return;
  cerr << errmsg << endl;
  hasReported = true;
}

// InARPCache
// Returns true if the IP is in the ARP cache
#if defined(OS_LINUX)
bool IPAddr::InARPCache(bool defaultValue) const
{
  // On Linux, check /proc/net/arp
  ifstream pf("/proc/net/arp");
  if (pf.bad()) {
    ReportInARPCacheError("InARPCache: Error opening /proc/net/arp: " + String(strerror(errno)));
    return defaultValue;
  }
  string ipstr = GetString(); 
  while (pf.good()) 
    {
      string line << pf;
      pos = line.find(" ");
      if (pos != string::npos && StrEQ(ipstr, line.substr(0,pos)))
	return true;
    }
  return false;
}
#elif defined(OS_MAC)
// This may work for BSD systems as well.
#include <net/route.h>
#include <netinet/if_ether.h>
#include <sys/sysctl.h>

bool IPAddr::InARPCache(bool defaultValue) const
{
  static bool errorReported = false;
  int mib[6];
  size_t numBytes = 0;
  
  // Compose the sysctl call
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = AF_INET;
  mib[4] = NET_RT_FLAGS;
  mib[5] = RTF_LLINFO;

  if (sysctl(mib, sizeof(mib)/sizeof(int), NULL, &numBytes, NULL, 0) < 0)
    {
      ReportInARPCacheError("InARPCache: Error opening /proc/net/arp: " + string(strerror(errno)));
      return defaultValue;
    }
  // Now we have the desired length.  Allocate a buffer and get the value
  char* buffer = NULL;
  while (true)
    // Need to loop here in case the ARP table grows between calls
    {
      numBytes += 128;  // Allocate a little extra to handle small increases
      char* newBuffer = (char*) realloc(buffer, numBytes);
      if (! newBuffer)
	{
	  ReportInARPCacheError("InARPCache: Failed to allocate " + IntToStr(numBytes) + " for buffer.");
	  if (buffer) free(buffer);
	  return defaultValue;
	}
      buffer = newBuffer;
      if (sysctl(mib, sizeof(mib)/sizeof(int), buffer, &numBytes, NULL, 0) == 0)
	break;
      if (errno != ENOMEM)
	{
	  ReportInARPCacheError("InARPCache: Error opening /proc/net/arp: " + string(strerror(errno)));
	  return defaultValue;
	}
    }
  // Now parse the results and look for a match
  char* endptr = buffer + numBytes;
  uint32 targetAddr = htonl(GetIP());

  while (buffer < endptr)
    {
      struct rt_msghdr*        rtptr   = (struct rt_msghdr*) buffer;
      struct sockaddr_inarp*   arpptr  = (struct sockaddr_inarp*) (buffer + sizeof(rt_msghdr));
      if (arpptr->sin_addr.s_addr == targetAddr)
	return true;
      // Point to next entry
      buffer += rtptr->rtm_msglen;
    }
  return false;
} 
#else
// Default version
bool IPAddr::InARPCache(bool defaultValue) const
{
  return defaultValue;
}
#endif

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

