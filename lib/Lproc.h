// Generic Light Processor routines

#ifndef _lfilter_h
#define _lfilter_h

#include "utilsTime.h"
#include "Color.h"
#include <vector>

class Lobj; // fwd decl

class Lproc {
public:
    Lproc() {}
    virtual ~Lproc() {}

    // These are the key functions that each Lproc must provide
    virtual Lproc* Duplicate() const = 0; // Makes a copy of the Lproc
    virtual void Apply(Lobj* obj) const = 0;
};

class LprocFcn : public Lproc {
public:
    typedef void (*Callback_t) (Lobj* obj);
    LprocFcn(Callback_t fcn) : iFcn(fcn) {}
    virtual ~LprocFcn() {}
    virtual LprocFcn* Duplicate() const {return new LprocFcn(*this);}
    virtual void Apply(Lobj* obj) const {if (iFcn) iFcn(obj);}

private:
    Callback_t iFcn;
};

class LprocDim : public Lproc {
public:
    LprocDim(float fraction) : Lproc(), iFraction(fraction) {}
    virtual ~LprocDim() {}

    virtual Lproc* Duplicate() const {return new LprocDim(*this);}
    virtual void Apply(Lobj* obj) const;
private:
    float iFraction;
};

class LprocFade : public Lproc {
public:
    typedef enum {kNoFade = 0, kFadeIn = 1, kFadeOut = 2, kExponentialIn = 3, kExponentialOut = 4} Type_t;
    LprocFade(Type_t fadeType, Milli_t startTime, Milli_t endTime) : Lproc(), iType(fadeType), iStartTime(startTime), iEndTime(endTime) {}
    virtual ~LprocFade() {}

    virtual Lproc* Duplicate() const {return new LprocFade(*this);}
    virtual void Apply(Lobj* obj) const;
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
    LprocList() {}
    ~LprocList();

    void    AddProc(const Lproc& proc);
    void    PrependProc(const Lproc& proc);
    size_t  NumProcs() const {return iProcList.size();}
    Lproc*  operator[](size_t idx) const {return iProcList[idx];}
    void Apply(Lobj* obj) const;

//    bool ReplaceProc(int handle, const Lproc& proc);
//    bool DeleteProc(int handle);

private:
    vector<Lproc*> iProcList;
//    int iNextHandle;
//    vector<pair<int,Lproc*> > iProcList;
};

#endif
