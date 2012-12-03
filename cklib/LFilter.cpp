// Support for LFilter
//
#include "utils.h"
#include "LFilter.h"

//---------------------------------------------------------------------------------
// LFilterFade
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
// LFilterList
//---------------------------------------------------------------------------------


LFilterList::~LFilterList() {
    for (size_t i = 0; i < iFilters.size(); ++i)
        delete iFilters[i].second;
    iFilters.clear();
}

#if 0
int AddFilter(const LFilter& filter);
    bool ReplaceFilter(int handle, const LFilter& filter);
    bool DeleteFilter(int handle);
    RGBColor Apply(const RGBColor& rgb, Milli_t currentTime);

#endif
