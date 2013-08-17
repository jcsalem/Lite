// CK's Kinet protocol definitions
// Sources of info on the KiNet protocol:
//   http://code.google.com/p/open-lighting/
//   kinet.py

#ifndef _KINET_H_
#define _KINET_H_

#include "utils.h"
#include "string.h" // for memset

// Protocol is apparently little endian (even though network byte order is typically big endian).
// This code assume little endian and isn't endian-aware. So it will work on x86 and default ARM architectures.

const uint16 KiNETudpPort   = 6038;

const uint32 KiNETmagic     = 0x4adc0104;
const uint32 KiNETmagic2    = 0xdeadbeef;
//#define KINET_MORE_MAGIC			0xdeadbeef
//#define KINET_DEEP_MAGIC			0xc001d00d
//#define KINET_MAGIC_HASH			0x69000420
const uint16 KiNETversion   = 0x0001;  // Some commands are 0x0002

const uint16 KiNetFlagSendAcks = 0x0001;  // This is the only flag I know.

// KiNet packet types
typedef enum {
  KTYPE_DISCOVER            = 0x0001,   // KiNet v1 and v2
  KTYPE_DISCOVER_REPLY      = 0x0002,   // KiNet v1 and v2
  KTYPE_SET_IP              = 0x0003,   // KiNet v1 and v2
  KTYPE_SET_UNIVERSE        = 0x0005,   // KiNet v1 and v2
  KTYPE_SET_NAME            = 0x0006,   // KiNet v1 and v2
  KTYPE_PORT_INFO           = 0x000A,   // KiNet v2 only
  KTYPE_PORT_INFO_REPLY     = 0x000B,
  KTYPE_DMXOUT              = 0x0101,   // KiNet v1 (limitations with v2)
  KTYPE_PORTOUT             = 0x0108,   // KiNet v2 only
  KTYPE_PORTOUT_SYNC        = 0x0109,   // KiNet v2 only
  KTYPE_GET_COUNT1          = 0x0201,   // seems to be a discovery packet (KiNet v1 only?)
  // KTYPE_??               = 0x0203,   // get dmx address?
  KTYPE_GET_COUNT1_REPLY    = 0x0206,   // ?? (KiNet v1 only?)
  KTYPE_GET_COUNT2          = 0x0207,   // KiNet v2 only
  KTYPE_GET_COUNT2_REPLY    = 0x0208    // Returns the number of ports and lights on each port (KiNet v2 only)
} KTYPE_t;

struct KiNETheader
{
    KiNETheader() {BaseInit();}
    KiNETheader(KTYPE_t typeArg) {BaseInit(); type = typeArg;}
    uint32  magic;
    uint16  version;
    uint16  type;
    uint32  seqnum;  // Apparently not implemented in most supplies.  Leave at zero.
    void BaseInit() {magic = KiNETmagic; version = KiNETversion; type = 0; seqnum = 0;}
};

// Poll for what KiNet power supplies are out there
struct KiNETdiscover : public KiNETheader
    {
    static const uint32 kDiscoverTempIP = 0x8988870a;  // 10.135.136.137
    KiNETdiscover() : KiNETheader(KTYPE_DISCOVER) {tempIP = kDiscoverTempIP;}
    static int GetSize() {return sizeof(KiNETdiscover);}
    uint32  tempIP; // The IP address to respond from. Not very important.
    };

// The reply to the above discovery packet
struct KiNETdiscoverReply : public KiNETheader
{
    KiNETdiscoverReply() : KiNETheader(KTYPE_DISCOVER_REPLY) {char* ptr = (char*) this + sizeof(KiNETheader); size_t len = sizeof(KiNETdiscoverReply) - sizeof(KiNETheader); memset(ptr, 0, len);}
    static int GetSize() {return sizeof(KiNETdiscoverReply);}
    uint32    ip;           // The power supply's IP address. In network order
    uint8     mac[6];       // The power supply's MAC address
    uint16    kinetVersion; // Version of KiNet supported by this power supply
    uint32    serial;       // The power supply's serial #
    uint32    universe;
    // This is followed by:
    // 1) PDS info string: a null terminated string with several lines of text.  Example:
    //       M:Color Kinetics Incorporated\nD:PDS-60 Combo\n#:SFT-000114-00\nR:05\n\0
    // 2) PDS name: a null terminated string
};

// Change a PDS's IP (old protocol)
struct KiNETsetIP : public KiNETheader
{
    KiNETsetIP() : KiNETheader(KTYPE_SET_IP) {magic2 = KiNETmagic2; magic3 = 0x6705;}
    static int GetSize() {return sizeof(KiNETsetIP);}
    uint32  magic2;
    uint8   hw_address[6];  // The MAC address to match
    uint16  magic3;  // 05 67  (not sure if this is required)
    uint32  new_ip;
};

// Change a PDS's Universe
struct KiNETsetUniverse : public KiNETheader
{
    KiNETsetUniverse() : KiNETheader(KTYPE_SET_UNIVERSE) {magic2 = KiNETmagic2;}
    static int GetSize() {return sizeof(KiNETsetUniverse);}
    uint32  magic2;  // ef be ad de
    uint32  universe;
};

// Change a PDS's Name
struct KiNETsetName : public KiNETheader
{
    KiNETsetName() : KiNETheader(KTYPE_SET_NAME) {magic2 = KiNETmagic2;}
    void SetName(csref newName);
    static int GetSize() {return sizeof(KiNETsetName);} // May not be right since it may be padded with an extra byte
    uint32  magic2;
    uint8   new_name[31];  // Null terminated.
};

// Poll for what KiNet power supplies are out there
struct KiNETportInfo : public KiNETheader
    {
    KiNETportInfo() : KiNETheader(KTYPE_PORT_INFO) {zero = 0;}
    static int GetSize() {return sizeof(KiNETportInfo);}
    uint32  zero; // Not sure if this is used for anything
    };

// The reply to the above discovery packet
struct KiNETportInfoReply : public KiNETheader
{
    KiNETportInfoReply() : KiNETheader(KTYPE_PORT_INFO_REPLY) {char* ptr = (char*) this + sizeof(KiNETheader); size_t len = sizeof(KiNETdiscoverReply) - sizeof(KiNETheader); memset(ptr, 0, len);}
    static int GetSize() {return sizeof(KiNETportInfoReply);}
    uint32 numPorts;     // The number of KiNETportInfoData structures
    // This is followed by KiNETportInfoData structures
};

struct KiNETportInfoData {
    uint8   portNum;
    uint8   portType;   // 0x02 -> Chromasic, 0x01 -> Serial, 0x00 -> Null?
    uint16  length;     // number of bytes following this subheader.  Usually 4 and must be a multiple of 4
    uint32  unused;
};

// Normal PortOut command - Includes length
struct KiNETportOut : public KiNETheader
    {
    KiNETportOut() : KiNETheader(KTYPE_PORTOUT) {universe = 0; port = 1; padding = 0; flags = 0; len = 0; startcode = 0;}
    static int GetSize() {return sizeof(KiNETportOut);}
    uint32  universe;   // zero?
    uint8   port;       // Port number.  Generally starts at 1.
    uint8   padding;    // zero
    uint16  flags;
    uint16  len;        // Payload length
    uint16  startcode;  // Not sure how this is used. Leave as zero
    // Flags
    static const uint16 kFlag16bit      = 2;     // Presumably sets 16 bit per channel rather than 8
    static const uint16 kFlagSyncWait   = 4;     // Presumably prevents updates until the PortoutSync command is sent
    };

// PortOut sync command
struct KiNETportOutSync : public KiNETheader
    {
    KiNETportOutSync() : KiNETheader(KTYPE_PORTOUT_SYNC) {padding = 0;}
    static int GetSize() {return sizeof(KiNETportOutSync);}
    uint32  padding; // unused
    };

// V1 version PortOut command - send this header followed by a fixed length string of bytes (512 x 3)
// Note that there is some disagreement on these headers.  Kinet.py has them somewhat different and in a form that doesn't make sense to me.
// Don't know how the universe is specified.
// For V1 gear, both ports are combined as one
struct KiNETdmxOut : public KiNETheader
    {
    KiNETdmxOut() : KiNETheader(KTYPE_DMXOUT) {padding = 0; flags = 0; timer = 0xFFFFFFFF; dmxStartCode = 0;}
    static int GetSize(); // Not simple since it needs to remove the extra C++ padding
    uint16  padding;   // Not sure how this is used. It doesn't seem to really be a port number?
    uint16  flags;      // zero
    uint32  timer;      // Not sure how this is used.  Examples have it at 0 or -1
    uint8   dmxStartCode;
    };

// Poll for what KiNet devices are out there
struct KiNETgetCount1 : public KiNETheader
    {
    KiNETgetCount1() : KiNETheader(KTYPE_GET_COUNT1) {ipaddr = 0; unknown1 = 0x728D; unknown2 = 0x1001;}
    static int GetSize() {return sizeof(KiNETgetCount1);}
    uint32  ipaddr;  // Don't know if this is required to be set
    uint16  unknown1;
    uint16  unknown2; // This could be a receive buffer length?
    };

// The reply to the above poll packet (this is sent by the PDS)
struct KiNETgetCount1Reply : public KiNETheader
{
    KiNETgetCount1Reply() : KiNETheader(KTYPE_GET_COUNT1_REPLY) {}
    static int GetSize() {return sizeof(KiNETgetCount1Reply);}
    uint8   counts[4];  // count of nodes on ports 1-4
};

// Poll for what KiNet devices are out there
struct KiNETgetCount2 : public KiNETheader
    {
    KiNETgetCount2() : KiNETheader(KTYPE_GET_COUNT2) {zero = 0; unknown = 0x1001;}
    static int GetSize() {return sizeof(KiNETgetCount2);}
    uint16  zero;
    uint16  unknown; // This could be a receive buffer length?
    };

// The reply to the above poll packet (this is sent by the PDS)
struct KiNETgetCount2Reply : public KiNETheader
{
    KiNETgetCount2Reply() : KiNETheader(KTYPE_GET_COUNT2_REPLY) {char* ptr = (char*) this + sizeof(KiNETheader); size_t len = sizeof(KiNETdiscoverReply) - sizeof(KiNETheader); memset(ptr, 0, len);}
    static int GetSize() {return sizeof(KiNETgetCount2Reply);}
    typedef enum {kStart = 0x0101, kEnd = 0x0001, kData = 0x0102 } replyType_t;
    uint16  replyType;
    uint16  zero;
    // For kStart and kEnd this is followed by a uint32 zero
    // For kData this is followed by the next structure for each port
};

struct KiNETgetCount2Data {
    uint8   portnum;
    uint8   porttype;   // 0x02 -> Chromasic, 0x01 -> Serial, 0x00 -> Null?
    uint8   length;     // number of bytes following this subheader.  Usually 4 and must be a multiple of 4
    uint8   zero;       // padding
    uint32  count;
};

// Get Address command (not sure how it's used)
//struct KiNETgetAddress : public KiNETheader
//{
//    KiNETgetAddress() : KiNETheader(KTYPE_??) {}
//    static int GetSize() {return sizeof(KiNETgetAddress);}
//    uint32  serial;
//    uint32  something;  // 41 00 12 00
//};

#if 0

// Other packets.
#define KTYPE_SET_IP 0x0103
typedef struct {
  KiNET_Hdr hdr;
  unsigned short var;
  unsigned short pad;
  unsigned long val;
} KiNET_Set;

typedef struct {
  KiNET_Hdr hdr;
  unsigned long status;
} KiNET_Ack;

#define KTYPE_KDM_SER9OUT 0xfb02
typedef struct {
  KiNET_Hdr hdr;
  unsigned char port;
  unsigned char flags;
  unsigned short timerVal;
  unsigned long uni;
  // need to add serial data here
} KiNET_KDM_Ser9Out;

#define KTYPE_KDM_TICKER 0xfb03
typedef struct {
  KiNET_Hdr hdr;
  unsigned short tickval;
  unsigned short pad;
  unsigned long uni;
} KiNET_KDM_Ticker;

#define KTYPE_KDM_DMXSTROBE 0xfb04
typedef struct {
  KiNET_Hdr hdr;
  unsigned char duration;
  unsigned char flags;
  unsigned short interval;
} KiNET_KDM_DMXStrobe;
#endif // 0

#endif // _KINET_H_
