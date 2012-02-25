// Light buffer

#ifndef _LBUFFER_H
#define _LBUFFER_H
#include "utils.h"
#include "Color.h"
class LBuffer
    {
  public:
    void        Alloc(int count);
    int         GetCount(void)      const {return iCount;}
    RGBColor    GetRGB(int idx)     const {if (idx >= 0 && idx < iCount) return iBuffer[idx]; else return BLACK;}
    // Writes
    void Clear(void) {SetAll(BLACK);}
    void SetRGB(int idx, const RGBColor& rgb)   {if (idx >= 0 && idx < iCount) iBuffer[idx] = rgb;}
    void AddRGB(int idx, const RGBColor& rgb)   {SetRGB(idx, GetRGB(idx) + rgb);}
    void SetAll(const RGBColor& rgb)            {for (int i = 0; i < iCount; ++i) SetRGB(i, rgb);}
  protected:
    LBuffer(int count = 0) : iCount(0), iBuffer(NULL) {Alloc(count);}
  private:
    int         iCount;
    RGBColor    *iBuffer;
    };



#endif // _LBUFFER_H
