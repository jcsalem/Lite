// Support for the LSparkle type
// Controls sparkle

#ifndef LSPARKLE_H_INCLUDED
#define LSPARKLE_H_INCLUDED

#include "Lobj.h"
#include "utilsTime.h"

// This describes a sparkly light of some kind
class LSparkle {
  public:
    LSparkle() : startTime(0), attack(0), hold(0), release(0), sleep(0) {}
    Milli_t startTime;
    Milli_t attack;
    Milli_t hold;
    Milli_t release;
    Milli_t sleep;

    RGBColor    ComputeColor(const RGBColor& referenceColor, Milli_t currentTime) const;
    bool        IsOutOfTime(Milli_t currentTime) const {return MilliLT(GetEndTime(), currentTime);}

    typedef enum {kError = -1, kDefault = 0, kSparkle = 1, kFirefly = 2, kSlow = 3} Mode_t;
    static Mode_t StrToMode(csref str);
    static string ModeToStr(Mode_t mode);
    // Returns a random sparkle based on sparkleMode.
    // If inProgress is true is assumes the sparkle is somewhere in the middle of its cycle and sets StartTime accordingly
    static LSparkle MakeRandomSparkle(Milli_t currentTime, Mode_t sparkleMode, float rate, bool inProgress = false);

private:
    Milli_t     GetEndTime() const;
};

class LobjSparkle : public Lobj {
  public:
     // Constructor
    LobjSparkle(Milli_t currentTime = Milliseconds()) : Lobj(currentTime), sparkle() {}
    virtual ~LobjSparkle() {}

    // Variables
    LSparkle    sparkle;

    // Operations
    virtual void Clear() {*this = LobjSparkle();}
    virtual bool IsOutOfTime() const;

    // Allocate function
    static LobjSparkle* Alloc(int,const void*) {return new LobjSparkle();}

    virtual RGBColor GetCurrentColor() const {return sparkle.ComputeColor(color, lastTime);}

};

namespace L {
// Parser support
// This defines the --sparkle option
extern LSparkle::Mode_t gSparkleMode;
// Supports the --sparklerate option
extern float            gSparkleRate;
};




# endif // LSPARKLE_H_INCLUDED

