// Generic Light Processor routines

#ifndef _lfilter_h
#define _lfilter_h

#include "utilsTime.h"
#include "Color.h"
#include <vector>

class Lproc {
public:
    Lproc() {}
    virtual ~Lproc() {}

    // These are the key functions that each Lproc must provide
    virtual Lproc* Duplicate() const = 0; // Makes a copy of the Lproc
    virtual RGBColor Apply(const RGBColor& rgb, Milli_t currentTime) const = 0;
};

class LprocFade : public Lproc {
public:
    typedef enum {kNoFade = 0, kFadeIn = 1, kFadeOut = 2, kExponentialIn = 3, kExponentialOut = 4} Type_t;
    LprocFade(Type_t fadeType, Milli_t startTime, Milli_t endTime) : Lproc(), iType(fadeType), iStartTime(startTime), iEndTime(endTime) {}
    virtual ~LprocFade() {}

    virtual Lproc* Duplicate() const {return new LprocFade(*this);}
    virtual RGBColor Apply(const RGBColor& rgb, Milli_t currentTime) const;
private:
    Type_t  iType;
    Milli_t iStartTime;
    Milli_t iEndTime;
};

class LprocFadeIn : public LprocFade {
public:
    LprocFadeIn(bool isExponential, Milli_t startTime, Milli_t endTime) : LprocFade(isExponential ? LprocFade::kExponentialIn : LprocFade::kFadeIn, startTime, endTime) {}
    virtual ~LprocFadeIn() {}

    virtual Lproc* Duplicate() const {return new LprocFadeIn(*this);}
};

class LprocFadeOut : public LprocFade {
public:
    LprocFadeOut(bool isExponential, Milli_t startTime, Milli_t endTime) : LprocFade(isExponential ? LprocFade::kExponentialOut : LprocFade::kFadeOut, startTime, endTime) {}
    virtual ~LprocFadeOut() {}

    virtual Lproc* Duplicate() const {return new LprocFadeOut(*this);}
};

class LprocList {
public:
    LprocList() : iNextHandle(1) {}
    ~LprocList();

    int AddProc(const Lproc& proc);
    bool ReplaceProc(int handle, const Lproc& proc);
    bool DeleteProc(int handle);

    RGBColor Apply(const RGBColor& rgb, Milli_t currentTime) const;

private:
    int iNextHandle;
    vector<pair<int,Lproc*> > iProcList;
};

#endif
