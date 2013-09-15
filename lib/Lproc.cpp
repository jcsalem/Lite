// Support for Lproc
//
#include "utils.h"
#include "Lproc.h"
#include "Lobj.h"

//---------------------------------------------------------------------------------
// LprocFade
//---------------------------------------------------------------------------------

void LprocFade::Apply(Lobj* obj) const {
    bool isFadeIn;
    bool isExponential;

    switch (iType) {
        case kFadeIn:           isFadeIn = true;    isExponential = false; break;
        case kFadeOut:          isFadeIn = false;   isExponential = false; break;
        case kExponentialIn:    isFadeIn = true;    isExponential = true; break;
        case kExponentialOut:   isFadeIn = false;   isExponential = true; break;
        case kNoFade:
        default:
            return;
        }

    float currentTime = obj->lastTime;
    float fraction;

    if (isFadeIn) {
        if (currentTime < iStartTime)       fraction = 0.0;
        else if (currentTime >= iEndTime)   return; // fraction = 1.0;
        else fraction = 1.0 * (currentTime - iStartTime) / (iEndTime - iStartTime);
    } else {
        if (currentTime < iStartTime)       return; // fraction = 1.0;
        else if (currentTime >= iEndTime)   fraction = 0.0;
        else fraction = 1.0 * (iEndTime - currentTime) / (iEndTime - iStartTime);
    }

    if (isExponential) fraction = fraction * fraction;

    obj->renderColor = obj->renderColor * fraction;
}

//---------------------------------------------------------------------------------
// LprocList
//---------------------------------------------------------------------------------


LprocList::~LprocList() {
    for (size_t i = 0; i < iProcList.size(); ++i)
        delete iProcList[i];
    iProcList.clear();
}

// Returns a handle to the added proc
void LprocList::AddProc(const Lproc& proc) {
    iProcList.push_back(proc.Duplicate());
}

// Returns a handle to the added proc
void LprocList::PrependProc(const Lproc& proc) {
    iProcList.insert(iProcList.begin(), proc.Duplicate());
}


void LprocList::Apply(Lobj* obj) const {
    for (size_t i = 0; i < iProcList.size(); ++i) {
        iProcList[i]->Apply(obj);
    }
}

/* OBSOLETE
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


*/ // No longer used
