// Object data structure
#ifndef _LOBJ_H
#define _LOBJ_H

#include "utils.h"
#include "Color.h"
#include "LBuffer.h"

extern bool gAntiAlias; // 1 to enable

class Lobj {
  public:
    // Globals
    static const short kPosIncr = 64;

    // Constructor
    Lobj() : pos(0), color(BLACK), speed(0) {}
    // Variables
    short    pos;
    RGBColor maxColor;
    RGBColor color;
    short    speed;
    short    velocity;

    // Dimming data
    unsigned long startAttack;
    unsigned long startHold;
    unsigned long startRelease;
    unsigned long startSleep;
    unsigned long startNext;
    //enum {kAttack, kSustain, kRelease, kSleep} mode;

    // Standard operations
    void Render(LBuffer* buffer) const;
    void Clear();

    // Global operations
    static short  GetNum(void); // number of Lobjs
    static short  GetMaxNum(void);  // maximum number of Lobjs
    static Lobj*  GetNth(short index);
    static Lobj*  Alloc(void);
    static bool Free(Lobj*);
    static void RenderAll(LBuffer* buffer);
    typedef void (*LobjMapFcn_t)(Lobj*);
    static void Map(LobjMapFcn_t mapfcn);
};


#endif
