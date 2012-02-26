// CK's Kinet protocol definitions

#ifndef _KINET_H_
#define _KINET_H_

#include "utils.h"

const uint16 KiNETudpPort   = 6038;

const uint32 KiNETmagic     = 0x4adc0104;
//#define KINET_MORE_MAGIC			0xdeadbeef
//#define KINET_DEEP_MAGIC			0xc001d00d
//#define KINET_MAGIC_HASH			0x69000420
const uint16 KiNETversion   = 0x0001;  // Some commands are 0x0002

const uint16 KiNetFlagSendAcks = 0x0001;  // This is the only flag I know.

struct KiNETheader
{
    KiNETheader() {BaseInit();}
    KiNETheader(uint16 typeArg) {BaseInit(); type = typeArg;}
    uint32  magic;
    uint16  version;
    uint16  type;
    uint32  seqnum;
    void BaseInit() {magic = KiNETmagic; version = KiNETversion; type = 0; seqnum = 0;}
};

// Normal PortOut command - Includes length
struct KiNETportOut : public KiNETheader
    {
    KiNETportOut() : KiNETheader(0x0108) {universe = 0; port = 1; padding = 0; flags = 0; len = 0; startcode = 0;}
    static int GetSize();
    uint32  universe;
    uint8   port;       // Not sure how this is used
    uint8   padding;    // zero
    uint16  flags;
    uint16  len;        // Payload length
    uint16  startcode;  // Not sure how this is used. Leave as zero
    };

// Original PortOut command - send this header followed by a fixed length string of bytes (512 x 3)
// Note that there is some disagreement on these headers.  Kinet.py has them somewhat different and in a form that doesn't make sense to me.
struct KiNETdmxOut : public KiNETheader
    {
    KiNETdmxOut() : KiNETheader(0x0101) {port = 1; padding = 0; flags = 0; timer = 0xFFFFFFFF; universe = 0;}
    static int GetSize();
    uint8   port;    // Not sure how this is used
    uint8   padding;    // zero
    uint16  flags; // zero
    uint32  timer;   // Not sure how this is used.  Examples have it at 0 or -1
    uint8   universe;// Not sure how this is used. Leave as zero
    };

// Not sure what these are.  I think they are either flags or start codes
#define KINET_PORTOUT_PAYLOAD_16BIT 0x0002
#define KINET_PORTOUT_PAYLOAD_SYNCWAIT 0x0004

#if 0
// Other packets.
#define KTYPE_SET 0x0103
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
