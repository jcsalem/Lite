#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include <vector>

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
