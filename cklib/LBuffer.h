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
    // Creating new LBuffers
    // The descriptor is of the format:  <type>:<details> where type defines the type of the LBuffer. Colon is optional if there are reasonable defaults
    static LBuffer* Create(csref descriptor, string* errmsg = NULL);

    // Properties
    int         GetCount(void)      const {return iBuffer.size();}
    bool        InBounds(int coord) const {return coord >= 0 && coord < (int) iBuffer.size();}

    // Reads
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
    bool SetMap(const vector<int>& lmap); // mapping from an integer coordinate to the light's index in the buffer

    // Filters
    //void AttachFilter(const LFilter& filter); // adds a processing filter
    //void DeleteAllFilters();

    // Operations
    void Rotate(int inc = 1);

    // Things to be overridden by the specific output class
    virtual bool    HasError()          const {return !iLastError.empty();}
    virtual string  GetLastError()      const {return iLastError;}
    virtual string  GetDescriptor()     const = 0; // Returns a descriptor of the LBuffer that can be used to recreate the exact LBuffer
    virtual string  GetDescription()    const; // returns a detailed description of the CKbuffer
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

    void        Alloc(int count); // erases the old buffer and map
    int         MapCoord(int coord) const   {if (iMap.size() == 0) return coord; else return iMap[coord];}
    // Functions for writing directly to the buffer
    void        SetRawRGB(int idx, const RGBColor& rgb)   {iBuffer[idx] = rgb;}
    RGBColor    GetRawRGB(int idx)     const {return iBuffer[idx];}

    // Disallow copy construction because it doesn't work reliably for the derived class
    LBuffer(const LBuffer&);
    LBuffer& operator=(const LBuffer&);
    };

// Used to define the derived classes that LBuffer::Create knows how to make
class LBufferType {
    friend class LBuffer;
public:
    typedef LBuffer* (*CreateFcn_t) (csref descriptorArgString, string* errmsg);
    LBufferType(csref name, CreateFcn_t fcn, csref formatString, csref docString);
    static string GetDocumentation();
private:
    string      iName;
    CreateFcn_t iFcn;
    string      iFormatString;
    string      iDocString;
};

#define DEFINE_LBUFFER_TYPE(name, fcn, formatString, docString) \
  LBufferType LBufferType_ ## name(#name, fcn, formatString, docString)

#endif // _LBUFFER_H
