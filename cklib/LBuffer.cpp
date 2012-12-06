#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include "LFilter.h"
#include <vector>
#include <algorithm>

void LBuffer::Alloc(int count)
{
    iBuffer.resize(count);
    ClearMap();
}

void LBuffer::SetAll(const Color& color)
{
    RGBColor rgb(color);
    int len = iBuffer.size();
    for (int i = 0; i < len; ++i)
        iBuffer[i] = rgb;
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
        int coord = (i + incr) % GetCount();
        if (coord < 0) coord = GetCount() - coord;
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

// Default info about LBuffer
string LBuffer::GetPath() const {
    return "unknown";
}

string LBuffer::GetDescription() const {
    return "LBuffer::" + GetPath();
}

