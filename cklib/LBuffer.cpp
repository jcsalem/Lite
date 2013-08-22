#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include "LFilter.h"
#include <vector>
#include <algorithm>
#include <iostream>

RGBColor LBuffer::kNullColor = BLACK; // note that this is used by functions returning references to colors

void LBufferPhys::Alloc(int count)
{
  iBuffer.resize(count);
    ClearMap();
}

void LBuffer::SetAll(const Color& color)
{
    RGBColor rgb(color);
    iterator iter = begin();
    iterator endIter = end();
    while (iter != endIter) {
        *iter = rgb;
        ++iter;
    }
}

void LBuffer::SetColor(int idx, const Color& color)
{
    RGBColor rgb(color);
    SetRGB(idx, rgb);
}

void LBuffer::Rotate(int incr) {
    if (GetCount() == 0 || incr == 0) return;
    vector<RGBColor> newBuffer(GetCount());
    for (int i = 0; i < GetCount(); ++i) {
        int coord = (i - incr) % GetCount();
        if (coord < 0) coord = GetCount() + coord;
        newBuffer[i] = GetRGB(coord);
    }
    for (int i = 0; i < GetCount(); ++i) {
        SetRGB(i, newBuffer[i]);
    }
}

// Map stuff
void LBuffer::RandomizeMap() {
    vector<int> lmap(GetCount());
    for (int i = 0; i < GetCount(); ++i)
        lmap[i] = i;
    random_shuffle(lmap.begin(), lmap.end());
    iMap = lmap;
    }

void LBuffer::ClearMap() {
// Just use a 1 to 1 mapping
    iMap.clear();
    }

bool LBuffer::SetMap(const vector<int>& lmap) {
    // mapping from coordinate to the light's index on the device
    if ((int) lmap.size() != GetCount()) return false;
    for (int i = 0; i < GetCount(); ++i) {
        if (! InBounds(lmap[i])) return false;
    }
    iMap = lmap;
    return true;
}

string LBuffer::GetDescription() const {
    string r = "LBuffer: ";
    r += IntToStr(GetCount()) + " total lights";
    if (iMap.size() > 0) r += " with custom mapping";
    r += " [" + GetDescriptor() + "]";
    return r;
}

//----------------------------------------------------------------------------------------------------------------
// Functions for creating and managing LBuffers
//----------------------------------------------------------------------------------------------------------------

vector<LBufferType>& GetAllLBufferTypes() {
    static vector<LBufferType> allLBufferTypes;
    return allLBufferTypes;
}

LBufferType::LBufferType(csref name, CreateFcn_t fcn, csref formatString, csref docString)
  : iName(StrToLower(name)), iFcn(fcn), iFormatString(formatString), iDocString(docString)
  {
      GetAllLBufferTypes().push_back(*this);
  }

LBuffer* LBuffer::Create(csref descriptor, string* errmsg) {
    string name     = TrimWhitespace(descriptor);
    size_t colonpos = name.find(':');
    string desc     = colonpos == string::npos ? "" : TrimWhitespace(name.substr(colonpos+1));
    name = StrToLower(name.substr(0,colonpos));
    if (name.empty()) {
        if (errmsg) *errmsg = "Missing device descriptor: " + descriptor;
        return NULL;
    }

    const vector<LBufferType>& allLBufferTypes = GetAllLBufferTypes();

    for (size_t i = 0; i < allLBufferTypes.size(); ++i) {
        if (allLBufferTypes[i].iName == name)
            return allLBufferTypes[i].iFcn(desc, errmsg);
        }
    // Unknown LBuffer type
    if (errmsg) *errmsg = "Unknown device type: \"" + name + "\"";
    return NULL;

}

string LBufferType::GetDocumentation() {
    const vector<LBufferType>& allLBufferTypes = GetAllLBufferTypes();
    string r;
    for (size_t i = 0; i < allLBufferTypes.size(); ++i) {
        r += allLBufferTypes[i].iFormatString + " - " + allLBufferTypes[i].iDocString + "\n";
        }
    return r;
}

//-----------------------------------------------------------------------------------------------------
// This section forces the linking of required modules (otherwise, CKlib, etc. are never loaded!)
//-----------------------------------------------------------------------------------------------------
#include "cklib.h"
#include "CursesBuffer.h"
#include "MetaBuffer.h"
void ForceLinking() {
    ForceLinkCK();
    ForceLinkCurses(); // This actually does nothing on Windows
    ForceLinkMeta();
};
