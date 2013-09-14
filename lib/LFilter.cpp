// Support for LFilter
//
#include "utils.h"
#include "LFilter.h"

//---------------------------------------------------------------------------------
// LFilterFade
//---------------------------------------------------------------------------------

RGBColor LFilterFade::Apply(const RGBColor& rgb, Milli_t currentTime) const {
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
// LFilterList
//---------------------------------------------------------------------------------


LFilterList::~LFilterList() {
    for (size_t i = 0; i < iFilters.size(); ++i)
        delete iFilters[i].second;
    iFilters.clear();
}

// Returns a handle to the added filter
int LFilterList::AddFilter(const LFilter& filter) {
    int handle = iNextHandle++;
    iFilters.push_back(pair<int,LFilter*>(handle, filter.Duplicate()));
    return handle;
}

bool LFilterList::ReplaceFilter(int handle, const LFilter& filter) {
    for (size_t i = 0; i < iFilters.size(); ++i)
        if (iFilters[i].first == handle) {
            delete iFilters[i].second;
            iFilters[i].second = filter.Duplicate();
            return true;
        }
    return false;
}

bool LFilterList::DeleteFilter(int handle) {
    for (vector<pair<int,LFilter*> >::iterator i = iFilters.begin(); i != iFilters.end(); ++i)
        if (i->first == handle) {
            delete i->second;
            iFilters.erase(i);
            return true;
        }
    return false;
}

RGBColor LFilterList::Apply(const RGBColor& rgbarg, Milli_t currentTime) const {
    RGBColor rgb(rgbarg);
    for (size_t i = 0; i < iFilters.size(); ++i) {
        rgb = iFilters[i].second->Apply(rgb, currentTime);
    }
    return rgb;
}

