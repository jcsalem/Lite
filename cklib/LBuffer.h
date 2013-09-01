// Light buffer

#ifndef _LBUFFER_H
#define _LBUFFER_H
#include "utils.h"
#include "Color.h"
#include <vector>
#include "utilsTime.h"

// ToDo
// Move mapping to special class. Remove from base class entirely

class LBuffer
    {
    friend class LBufferIter;
    friend class LBufferConstIter;
    friend class ComboBuffer;
    friend class ReverseBuffer;
  public:
    // Creating new LBuffers
    // The descriptor is of the format:  <type>:<details> where type defines the type of the LBuffer. Colon is optional if there are reasonable defaults
    static LBuffer* Create(csref descriptor, string* errmsg = NULL);
    static RGBColor kNullColor;

    // Properties
    virtual int GetCount(void)          const = 0; // Must be defined
    bool        InBounds(int coord)     const {return coord >= 0 && coord < GetCount();}

    // Reads
    RGBColor    GetRGB(int coord)       const {if (InBounds(coord)) return const_cast<LBuffer*>(this)->GetRawRGB(MapCoord(coord)); else return BLACK;}

    // Writes
    void Clear(void) {SetAll(BLACK);}
    void SetColor(int coord, const Color& color);
    void SetAll(const Color& color);
    void SetRGB(int coord, const RGBColor& rgb)   {if (InBounds(coord)) GetRawRGB(MapCoord(coord)) = rgb;}
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

    virtual ~LBuffer() {/*DeleteAllFilters();*/}

  protected:
    LBuffer() {}

    string              iLastError;
    vector<int>         iMap;

    int                 MapCoord(int coord) const   {if (iMap.size() == 0) return coord; else return iMap[coord];}

        // Buffer access functions. Idx assumed to be in bounds.
    virtual RGBColor&   GetRawRGB(int idx) = 0;
    RGBColor&           GetRGBRef(int coord)             {if (InBounds(coord)) return GetRawRGB(MapCoord(coord)); else return kNullColor;}

    // Disallow copy construction because it doesn't work reliably for the derived class
    LBuffer(const LBuffer&);
    LBuffer& operator=(const LBuffer&);

  public:
    // Iterators
    class iterator
    {
        public:
            iterator(LBuffer* buffer, int coord) : iBuffer(buffer), iCoord(coord) {}
            RGBColor&       operator*   ()    const {return iBuffer->GetRGBRef(iCoord);}
            RGBColor*       operator->  ()    const {return &(iBuffer->GetRGBRef(iCoord));}
            iterator&       operator++  ()          {++iCoord; return *this;}
            iterator&       operator--  ()          {--iCoord; return *this;}
            iterator        operator++  (int)       {iterator r = *this; iCoord++; return r;}
            iterator        operator--  (int)       {iterator r = *this; iCoord--; return r;}
            iterator&       operator+=  (int incr)  {iCoord+=incr; return *this;}
            iterator&       operator-=  (int incr)  {iCoord-=incr; return *this;}
            const iterator  operator+   (int incr)  {return iterator(iBuffer, iCoord+incr);}
            const iterator  operator-   (int incr)  {return iterator(iBuffer, iCoord-incr);}
            bool            operator==  (const iterator& iter) const {return iBuffer == iter.iBuffer && iCoord == iter.iCoord;}
            bool            operator!=  (const iterator& iter) const {return !(*this == iter);}
            bool            operator<   (const iterator& iter) const {return iCoord < iter.iCoord;}
            bool            operator>   (const iterator& iter) const {return iCoord > iter.iCoord;}
            bool            operator<=  (const iterator& iter) const {return iCoord <= iter.iCoord;}
            bool            operator>=  (const iterator& iter) const {return iCoord >= iter.iCoord;}
        private:
            LBuffer*    iBuffer;
            int         iCoord;
    };
    iterator        begin()         {return iterator(this, 0);}
    iterator        end()           {return iterator(this, GetCount());}

    class const_iterator
    {
        public:
            const_iterator(const LBuffer* buffer, int coord) : iBuffer(buffer), iCoord(coord) {}
            const RGBColor& operator*   ()    const {return const_cast<LBuffer*>(iBuffer)->GetRGBRef(iCoord);}
            const RGBColor* operator->  ()    const {return &(**this);}
            const_iterator&      operator++  ()          {iCoord++; return *this;}
            const_iterator&      operator--  ()          {iCoord--; return *this;}
            const_iterator       operator++  (int)       {const_iterator r = *this; iCoord++; return r;}
            const_iterator       operator--  (int)       {const_iterator r = *this; iCoord--; return r;}
            const_iterator&      operator+=  (int incr)  {iCoord+=incr; return *this;}
            const_iterator&      operator-=  (int incr)  {iCoord-=incr; return *this;}
            const const_iterator operator+   (int incr)  {return const_iterator(iBuffer, iCoord+incr);}
            const const_iterator operator-   (int incr)  {return const_iterator(iBuffer, iCoord-incr);}
            bool            operator==  (const const_iterator& iter) const {return iBuffer == iter.iBuffer && iCoord == iter.iCoord;}
            bool            operator!=  (const const_iterator& iter) const {return !(*this == iter);}
            bool            operator<   (const const_iterator& iter) const {return iCoord < iter.iCoord;}
            bool            operator>   (const const_iterator& iter) const {return iCoord > iter.iCoord;}
            bool            operator<=  (const const_iterator& iter) const {return iCoord <= iter.iCoord;}
            bool            operator>=  (const const_iterator& iter) const {return iCoord >= iter.iCoord;}
        private:
            const LBuffer*  iBuffer;
            int             iCoord;
    };
    const_iterator   begin() const   {return const_iterator(this, 0);}
    const_iterator   end()   const   {return const_iterator(this, GetCount());}


}; // End LBuffer definitions


class LBufferPhys : public LBuffer
    {
  public:
    virtual         ~LBufferPhys() {}
    // Properties
    int             GetCount(void)      const   {return iBuffer.size();}
    virtual bool    Update() = 0;              // Updates the actual device based on the buffer contents.  Must be supplied for all derived types.

  protected:
    LBufferPhys(int count = 0) : iBuffer(count) {}

    vector<RGBColor>    iBuffer;

    //$$$ Temporary  this needs to go away to be replaced by the ComboBuffer
    void        Alloc(int count); // erases the old buffer and map

    // Functions for writing directly to the buffer
    virtual RGBColor& GetRawRGB(int idx)        {return iBuffer[idx];}

    // Disallow copy construction because it doesn't work reliably for the derived class
    LBufferPhys(const LBuffer&);
    LBufferPhys& operator=(const LBuffer&);
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
