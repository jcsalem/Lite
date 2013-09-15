// Object data structure
#ifndef _LOBJ_H
#define _LOBJ_H

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "LBuffer.h"

extern bool gAntiAlias; // 1 to enable
class LprocList; //fwd decl

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
    const Lxy operator-(const Lxy& a) const {Lxy r = *this; r -= a; return r;}
    const Lxy operator*(float s)      const {Lxy r = *this; r *= s; return r;}
    const Lxy operator/(float s)      const {Lxy r = *this; r /= s; return r;}
};

class Lobj {
  public:
     // Constructor
    Lobj(Milli_t currentTime = Milliseconds()) : /*initColor(BLACK), initWidth(0), */
        lastTime(currentTime), nextTime(currentTime), color(BLACK), width(0) {}
    virtual ~Lobj() {}

    // These variables are set at the beginning
    Lxy         initSpeed;  // Initial speed of the object
//    RGBColor    initColor;  // Initial color of the object
//    Lxy         initPos;
//    float       initWidth;

    // These are current as of nextTime
    Milli_t     lastTime;   // Last update time
    Milli_t     nextTime;   // During the update loop, it's equal to currentTime
    Lxy         speed;      // Speed in positions per second
    Lxy         pos;        // Updated by standard operations
    RGBColor    color;      // Updated by UpdateColor
    float       width;      // Generally not changed

    // These are during rendering
    RGBColor    renderColor;// Update sets this equal to color

    // These are the standard operations that get called in order
    virtual void UpdatePrepare(Milli_t currentTime) {nextTime = currentTime;}
    virtual void UpdateMove();                          // Moves the object
    virtual void UpdateColor();                         // Updates color and renderColor
    virtual void UpdateProcs(const LprocList& procs);   // Usually just updates renderColor
    virtual void UpdateRender(LBuffer* output);         // Displays the object using renderColor
    virtual void UpdateDone() {lastTime = nextTime;}
    // This calls all of the functions above in order
    virtual void Update(Milli_t currentTime, const LprocList& procs, LBuffer* output);

    // Other operations that can be overloaded
    virtual void Wrap           (const Lxy& minBound, const Lxy& maxBound);
    virtual void Bounce         (const Lxy& minBound, const Lxy& maxBound);
    virtual bool IsOutOfBounds  (const Lxy& minBound, const Lxy& maxBound) const;
//    virtual bool IsOutOfTime    ()   const {return false;}

    virtual void Clear() {*this = Lobj();}

    // Function types
    typedef void   (*MapFcn_t)      (Lobj* obj, const void* info);
    typedef Lobj*  (*AllocFcn_t)    (int idx,   const void* info);
    typedef bool   (*FreeIfFcn_t)   (Lobj* obj, const void* info);  // Returns tree if item should be freed

    // Info
    virtual string GetTypeName() const {return "Lobj";}
    virtual string GetDescription(bool verbose = false) const;
};

class Lgroup {
public:
    Lgroup() {}
    ~Lgroup() {FreeAll();}
    // Access
    Lobj*   Get(int idx) const {if (idx < 0 || (size_t) idx >= iObjs.size()) return NULL; else return iObjs[idx];}
    int         GetCount() const {return iObjs.size();}

    // Allocation and deallocation
    void Add(Lobj* obj);
    void Add(int num, Lobj::AllocFcn_t fcn, const void* info);
    bool Free(Lobj* obj);
    void FreeAll();
    void FreeIf(Lobj::FreeIfFcn_t fcn, const void* info);
    void FreeIfOutOfBounds(Lxy MinBound, Lxy maxBound) const;

    // Common functions
    void RenderAll(Milli_t currentTime, LBuffer* buffer) const;
    void RenderAll(Milli_t currentTime, const LprocList& procs, LBuffer* buffer) const;

//    void MoveAll    (Milli_t newTime) const;
//    void WrapAll    (const Lxy& MinBound, const Lxy& maxBound) const;
//    void BounceAll  (const Lxy& MinBound, const Lxy& maxBound) const;

    string GetDescription(bool verbose = false) const;

    // Mapping
    void Map(Lobj::MapFcn_t mapfcn, const void* info = NULL);
    typedef vector<Lobj*>::const_iterator   const_iterator;
    typedef vector<Lobj*>::iterator         iterator;
    const_iterator  begin() const       {return iObjs.begin();}
    const_iterator  end()   const       {return iObjs.end();}
    iterator        begin()             {return iObjs.begin();}
    iterator        end()               {return iObjs.end();}
    Lobj&       operator[](int i)   {return *(iObjs[i]);}
    const Lobj& operator[](int i) const {return *(iObjs[i]);}

private:
    vector<Lobj*> iObjs;
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
