// Object data structure
#ifndef _LOBJ_H
#define _LOBJ_H

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "LBuffer.h"

extern bool gAntiAlias; // 1 to enable
class LFilterList; //fwd decl

// Type definitions
class LobjBase; // forward decl
namespace Lobj {
    typedef void       (*MapFcn_t)      (LobjBase* obj, const void* info);
    typedef LobjBase*  (*AllocFcn_t)    (int idx,       const void* info);
    typedef bool       (*FreeIfFcn_t)   (LobjBase* obj, const void* info);  // Returns tree if item should be freed
};

// Class for holding XY coordinates
// Used for coordinates and speed
struct Lxy {
    Lxy() : x(0), y(0) {}
    Lxy(float xx, float yy) : x(xx), y(yy) {}

    float x;
    float y;

    Lxy& operator+=(const Lxy& a)    {x += a.x; y += a.y; return *this;}
    Lxy& operator-=(const Lxy& a)    {x -= a.x; y -= a.y; return *this;}
    Lxy& operator*=(float s)         {x *= s; y *= s; return *this;}
    Lxy& operator/=(float s)         {x /= s; y /= s; return *this;}

    const Lxy operator+(const Lxy& a) const {Lxy r = *this; r += a; return r;}
    const Lxy operator-(const Lxy& a) const {Lxy r = *this; r += a; return r;}
    const Lxy operator*(float s)      const {Lxy r = *this; r *= s; return r;}
    const Lxy operator/(float s)      const {Lxy r = *this; r /= s; return r;}
};

class Lgroup {
public:
    Lgroup() {}
    ~Lgroup() {FreeAll();}
    // Access
    LobjBase*   Get(int idx) const {if (idx < 0 || (size_t) idx >= iObjs.size()) return NULL; else return iObjs[idx];}
    int         GetCount() const {return iObjs.size();}

    // Allocation and deallocation
    void Add(LobjBase* obj);
    void Add(int num, Lobj::AllocFcn_t fcn, const void* info);
    bool Free(LobjBase* obj);
    void FreeAll();
    void FreeIf(Lobj::FreeIfFcn_t fcn, const void* info);
    void FreeIfOutOfBounds(Lxy MinBound, Lxy maxBound) const;

    // Common functions
    void RenderAll(LBuffer* buffer) const;
    void RenderAll(LBuffer* buffer, const LFilterList& filters) const;

    void MoveAll(Milli_t newTime) const;
    void WrapAll(const Lxy& MinBound, const Lxy& maxBound) const;
    string GetDescription(bool verbose = false) const;

    // Mapping
    void Map(Lobj::MapFcn_t mapfcn, const void* info = NULL);
    typedef vector<LobjBase*>::const_iterator   const_iterator;
    typedef vector<LobjBase*>::iterator         iterator;
    const_iterator   begin() const   {return iObjs.begin();}
    const_iterator   end()   const   {return iObjs.end();}
    iterator         begin()         {return iObjs.begin();}
    iterator         end()           {return iObjs.end();}

private:
    vector<LobjBase*> iObjs;
};

class LobjBase {
  public:
     // Constructor
    LobjBase(Milli_t currentTime = Milliseconds()) : width(0), color(BLACK), lastTime(currentTime) {}
    virtual ~LobjBase() {}

    // Instance variables
    Lxy         pos;
    float       width;
    RGBColor    color;
    Lxy         speed;      // Speed in positions per second
    Lxy         initSpeed;
    Milli_t     lastTime;

    // Standard operations
    virtual void Move           (Milli_t currentTime);
    virtual void Wrap           (const Lxy& minBound, const Lxy& maxBound);
    virtual bool IsOutOfBounds  (const Lxy& minBound, const Lxy& maxBound) const;
    virtual bool IsOutOfTime    ()   const {return false;}

    virtual void Clear() {*this = LobjBase();}

    // Returns the color as of lastTime
    virtual RGBColor GetCurrentColor() const {return color;}

    //virtual void Render(LBuffer* buffer, const LFilterList& filters) const;

    //void Render(LBuffer* buffer, const RGBColor& color) const;

    virtual string GetDescription(bool verbose = false) const;
};

// This describes a sparkly light of some kind
class Lsparkle {
  public:
    Lsparkle() : startTime(0), attack(0), hold(0), release(0), sleep(0) {}
    Milli_t startTime;
    Milli_t attack;
    Milli_t hold;
    Milli_t release;
    Milli_t sleep;

    RGBColor    ComputeColor(const RGBColor& referenceColor, Milli_t currentTime) const;
    bool        IsOutOfTime(Milli_t currentTime) const {return MilliLT(GetEndTime(), currentTime);}
    Milli_t     GetEndTime() const;
};

class LobjSparkle : public LobjBase {
  public:
     // Constructor
    LobjSparkle(Milli_t currentTime = Milliseconds()) : LobjBase(currentTime), sparkle() {}
    virtual ~LobjSparkle() {}

    // Variables
    Lsparkle    sparkle;

    // Operations
    virtual void Clear() {*this = LobjSparkle();}
    virtual bool IsOutOfTime() const;

    // Allocate function
    static LobjSparkle* Alloc(int,const void*) {return new LobjSparkle();}

    virtual RGBColor GetCurrentColor() const {return sparkle.ComputeColor(color, lastTime);}

};

// Obsolete

class LobjOld {
  public:
     // Constructor
    LobjOld() : pos(0), color(BLACK), speed(0) {}
    // Variables
    float    pos;
    RGBColor maxColor;
    RGBColor color;
    float    speed;     // maximum speed
    float    velocity;  // current velocity

    // Dimming data
    Milli_t startAttack;
    Milli_t startHold;
    Milli_t startRelease;
    Milli_t startSleep;
    Milli_t startNext;
    //enum {kAttack, kSustain, kRelease, kSleep} mode;

    // Standard operations
    void Render(LBuffer* buffer) const;
    void Clear();

    // Global operations
    static short  GetNum(void); // number of Lobjs
    static short  GetMaxNum(void);  // maximum number of Lobjs
    static LobjOld*  GetNth(short index);
    static LobjOld*  Alloc(void);
    static bool Free(LobjOld*);
    static void RenderAll(LBuffer* buffer);
    typedef void (*LobjOldMapFcn_t)(LobjOld*);
    static void Map(LobjOldMapFcn_t mapfcn);
};


#endif
