#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include <vector>

void LBuffer::Alloc(int count)
{
    iBuffer.resize(count);
}

void LBuffer::SetAll(const RGBColor& rgb)
{
    int len = iBuffer.size();
    for (int i = 0; i < len; ++i)
        iBuffer[i] = rgb;
}
