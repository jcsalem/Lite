// An LBuffer variant built for LED strips like the Sparkfun WS2801
//

#ifndef STRIPBUFFER_H_INCLUDED
#define STRIPBUFFER_H_INCLUDED

#include "utilsPrecomp.h"

#ifndef OS_WINDOWS
//#ifdef __arm__
#include "LBuffer.h"
// // Currently for the Raspberry Pi and the WS2801-based LED strips from SparkFun

// Generic strip type
class StripBuffer : public LBufferPhys
{
public:
    StripBuffer(int count = 0) : LBufferPhys(count), iColorFlip(false) {}
    virtual ~StripBuffer() {}

    virtual string  GetDescriptor() const {return (iCreateString.empty() ? "unknownstriptype" : iCreateString); }
    virtual bool    Update() {iLastError = "Attempted to update invalid strip."; return false;}
    void            SetCreateString(csref str)  {iCreateString = str;}
    void            SetColorFlip(bool val)      {iColorFlip = val;}
    bool            GetColorFlip() const        {return iColorFlip;}

private:
    string              iCreateString;
    bool                iColorFlip;
    // Don't allow copying
    StripBuffer(const StripBuffer&);
    StripBuffer& operator=(const StripBuffer&);
};

class StripBufferWS2801 : public StripBuffer
{
public:
    StripBufferWS2801(int count, int SDIgpio, int CLKgpio) : StripBuffer(count), iSDIgpio(SDIgpio), iCLKgpio(CLKgpio) {}
    virtual ~StripBufferWS2801() {}

    virtual bool    Update();
private:
    int iSDIgpio;
    int iCLKgpio;
    // Don't allow copying
    StripBufferWS2801(const StripBufferWS2801&);
    StripBufferWS2801& operator=(const StripBufferWS2801&);
};
#endif    // __arm__

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkStrip();

#endif // !STRIPBUFFER_H_INCLUDED
