// Support for Lproc
//
#include "utils.h"
#include "Lproc.h"

//---------------------------------------------------------------------------------
// LprocFade
//---------------------------------------------------------------------------------

RGBColor LprocFade::Apply(const RGBColor& rgb, Milli_t currentTime) const {
    bool isFadeIn;
    bool isExponential;

    switch (iType) {
        case kFadeIn:           isFadeIn = true;    isExponential = false; break;
        case kFadeOut:          isFadeIn = false;   isExponential = false; break;
        case kExponentialIn:    isFadeIn = true;    isExponential = true; break;
        case kExponentialOut:   isFadeIn = false;   isExponential = true; break;
        case kNoFade:
        default:
            return rgb;
        }

    float fraction = 1.0 * (currentTime - iStartTime) / (iEndTime - iStartTime);

    if (isFadeIn) {
        if (currentTime < iStartTime)       return BLACK;
        else if (currentTime >= iEndTime)   return rgb;
    } else {
        if (currentTime < iStartTime)       return rgb;
        else if (currentTime < iEndTime)    fraction = 1.0 - fraction;
        else                                return BLACK;
    }

    if (isExponential) fraction = fraction * fraction;

    return rgb * fraction;
}

//---------------------------------------------------------------------------------
// LprocList
//---------------------------------------------------------------------------------


LprocList::~LprocList() {
    for (size_t i = 0; i < iProcList.size(); ++i)
        delete iProcList[i].second;
    iProcList.clear();
}

// Returns a handle to the added proc
int LprocList::AddProc(const Lproc& proc) {
    int handle = iNextHandle++;
    iProcList.push_back(pair<int,Lproc*>(handle, proc.Duplicate()));
    return handle;
}

bool LprocList::ReplaceProc(int handle, const Lproc& proc) {
    for (size_t i = 0; i < iProcList.size(); ++i)
        if (iProcList[i].first == handle) {
            delete iProcList[i].second;
            iProcList[i].second = proc.Duplicate();
            return true;
        }
    return false;
}

bool LprocList::DeleteProc(int handle) {
    for (vector<pair<int,Lproc*> >::iterator i = iProcList.begin(); i != iProcList.end(); ++i)
        if (i->first == handle) {
            delete i->second;
            iProcList.erase(i);
            return true;
        }
    return false;
}

RGBColor LprocList::Apply(const RGBColor& rgbarg, Milli_t currentTime) const {
    RGBColor rgb(rgbarg);
    for (size_t i = 0; i < iProcList.size(); ++i) {
        rgb = iProcList[i].second->Apply(rgb, currentTime);
    }
    return rgb;
}

