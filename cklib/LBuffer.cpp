#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include "LFilter.h"
#include <vector>
#include <algorithm>

void LBuffer::Alloc(int count)
{
    iBuffer.resize(count);
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
    incr = incr % GetCount();
    if (incr < 0) incr = GetCount() + incr;
    rotate(iBuffer.begin(), iBuffer.begin() + incr, iBuffer.end());
}

// Default info about LBuffer
string LBuffer::GetPath() const {
    return "unknown";
}

string LBuffer::GetDescription() const {
    return "LBuffer::" + GetPath();
}
