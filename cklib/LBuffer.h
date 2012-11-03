// Light buffer

#ifndef _LBUFFER_H
#define _LBUFFER_H
#include "utils.h"
#include "Color.h"
#include <vector>

class LBuffer
    {
  public:
    void        Alloc(int count);
    int         GetCount(void)      const {return iBuffer.size();}
    RGBColor    GetRGB(int idx)     const {if (idx >= 0 && idx < (int) iBuffer.size()) return iBuffer[idx]; else return BLACK;}
    // Writes
    void Clear(void) {SetAll(BLACK);}
    void SetRGB(int idx, const RGBColor& rgb)   {if (idx >= 0 && idx < (int) iBuffer.size()) iBuffer[idx] = rgb;}
    void AddRGB(int idx, const RGBColor& rgb)   {SetRGB(idx, GetRGB(idx) + rgb);}
    void SetColor(int idx, const Color& color);
    void SetAll(const Color& color);

    typedef vector<RGBColor>::const_iterator const_iterator;
    typedef vector<RGBColor>::iterator       iterator;
    const_iterator  begin() const           {return iBuffer.begin();}
    const_iterator  end()   const           {return iBuffer.end();}
    iterator        begin()                 {return iBuffer.begin();}
    iterator        end()                   {return iBuffer.end();}

  protected:
    LBuffer(int count = 0) : iBuffer(count) {}
    vector<RGBColor>    iBuffer;
    };



#endif // _LBUFFER_H
