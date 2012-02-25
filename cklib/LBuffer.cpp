#include "LBuffer.h"
#include "Color.h"

void LBuffer::Alloc(int count)
{
    RGBColor* newBuffer = new RGBColor[count];

    if (iBuffer)
    {
        for (int i = 0; i < iCount; ++i)
            newBuffer[i]= iBuffer[i];
        delete []iBuffer;
    }
    iCount = count;
    iBuffer = newBuffer;
}
