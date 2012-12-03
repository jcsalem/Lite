// Generic Light Filter routines

#ifndef _lfilter_h
#define _lfilter_h

#include "utilsTime.h"
#include "Color.h"
#include <vector>

class LFilter {
public:
    LFilter();
    virtual ~LFilter() {}

    // These are the key functions that each filter must provide
    virtual LFilter* Duplicate() const = 0; // Makes a copy of the filter
    virtual RGBColor Apply(const RGBColor& rgb, Milli_t currentTime) = 0;
};

class LFilterFade : public LFilter {
public:
    typedef enum {kNoFade = 0, kFadeIn = 1, kFadeOut = 2, kExponentialIn = 3, kExponentialOut = 4} Type_t;
    LFilterFade(Type_t fadeType, Milli_t startTime, Milli_t endTime) : LFilter(), iType(fadeType), iStartTime(startTime), iEndTime(endTime) {}
    virtual ~LFilterFade() {}

    virtual LFilter* Duplicate() const {return new LFilterFade(*this);}
    virtual RGBColor Apply(const RGBColor& rgb, Milli_t currentTime);
private:
    bool    iType;
    Milli_t iStartTime;
    Milli_t iEndTime;
};

class LFilterFadeIn : public LFilterFade {
public:
    LFilterFadeIn(bool isExponential, Milli_t startTime, Milli_t endTime) : LFilterFade(isExponential ? LFilterFade::kExponentialIn : LFilterFade::kFadeIn, startTime, endTime) {}
    virtual ~LFilterFadeIn() {}

    virtual LFilter* Duplicate() const {return new LFilterFadeIn(*this);}
};

class LFilterFadeOut : public LFilterFade {
public:
    LFilterFadeOut(bool isExponential, Milli_t startTime, Milli_t endTime) : LFilterFade(isExponential ? LFilterFade::kExponentialOut : LFilterFade::kFadeOut, startTime, endTime) {}
    virtual ~LFilterFadeOut() {}

    virtual LFilter* Duplicate() const {return new LFilterFadeOut(*this);}
};

class LFilterList {
public:
    LFilterList() : iNextHandle(1) {}
    ~LFilterList();

    int AddFilter(const LFilter& filter);
    bool ReplaceFilter(int handle, const LFilter& filter);
    bool DeleteFilter(int handle);
    RGBColor Apply(const RGBColor& rgb, Milli_t currentTime);

private:
    int iNextHandle;
    vector<pair<int,LFilter*> > iFilters;
};

#endif
