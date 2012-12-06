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
    void        Alloc(int count); // erases the old map
    int         GetCount(void)      const {return iBuffer.size();}
    bool        InBounds(int coord) const {return coord >= 0 && coord < (int) iBuffer.size();}
    RGBColor    GetRGB(int coord)   const {if (InBounds(coord)) return GetRawRGB(MapCoord(coord)); else return BLACK;}

    // Writes
    void Clear(void) {SetAll(BLACK);}
    void SetColor(int coord, const Color& color);
    void SetAll(const Color& color);
    void SetRGB(int coord, const RGBColor& rgb)   {if (InBounds(coord)) SetRawRGB(MapCoord(coord), rgb);}
    void AddRGB(int coord, const RGBColor& rgb)   {SetRGB(coord, GetRGB(coord) + rgb);}

    // Mapping functions
    void RandomizeMap();
    void ClearMap();  // 1 to 1 mapping
    bool SetMap(const vector<int>& lmap); // mapping from coordinate to the light's index on the device

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
    vector<int>         iMap;

    int MapCoord(int coord) const   {if (iMap.size() == 0) return coord; else return iMap[coord];}
    // Functions for writing directly to the buffer
    void        SetRawRGB(int idx, const RGBColor& rgb)   {iBuffer[idx] = rgb;}
    RGBColor    GetRawRGB(int idx)     const {return iBuffer[idx];}

    // Disallow copy construction because it doesn't work reliably for the derived class
    LBuffer(const LBuffer&);
    LBuffer& operator=(const LBuffer&);
    };

#endif // _LBUFFER_H
