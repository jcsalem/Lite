// Light buffer

#ifndef _LBUFFER_H
#define _LBUFFER_H
#include "utils.h"
#include "Color.h"
#include <vector>
#include "utilsTime.h"

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

    // Filters
    //void AttachFilter(const LFilter& filter); // adds a processing filter
    //void DeleteAllFilters();

    // Operations
    void Rotate(int inc = 1);

    // Things to be overridden by the specific output class
    virtual bool    HasError()          const {return !iLastError.empty();}
    virtual string  GetLastError()      const {return iLastError;}
    virtual string  GetPath()           const; // Returns the path describing the device
    virtual string  GetDescription()    const; // A description of the device
    virtual bool    Update() = 0;              // Updates the actual device based on the buffer contents.  Must be supplied for all derived types.

    typedef vector<RGBColor>::const_iterator const_iterator;
    typedef vector<RGBColor>::iterator       iterator;
    const_iterator  begin() const           {return iBuffer.begin();}
    const_iterator  end()   const           {return iBuffer.end();}
    iterator        begin()                 {return iBuffer.begin();}
    iterator        end()                   {return iBuffer.end();}

    virtual ~LBuffer() {/*DeleteAllFilters();*/}

  protected:
    LBuffer(int count = 0) : iBuffer(count) {}
    vector<RGBColor>    iBuffer;
    string              iLastError;
    //vector<LFilter*>    iFilters;

    // Disallow copy construction because it doesn't work reliably for the derived class
    LBuffer(const LBuffer&);
    LBuffer& operator=(const LBuffer&);
    };

#endif // _LBUFFER_H
