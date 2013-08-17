// An LBuffer variant built using nstrip
//

#ifndef STRIPBUFFER_H_INCLUDED
#define STRIPBUFFER_H_INCLUDED

#include "utilsPrecomp.h"

#ifndef OS_WINDOWS
#include "LBuffer.h"
// // Currently for the WS2801-based LED strips from SparkFun

namespace Strip
{
// Strip Type
//typedef enum {
//    kWS2801        = 1
//    } Type_t;

// The options for each strip
enum {
    kOptNone = 0,
    kOptReverse = 1,   // Reverses the order of the
    kOptFlipColor = 2
    };

typedef int Options_t;
};

class StripBuffer : public LBuffer
{
public:
    StripBuffer(int count = 0) : LBuffer(count)) {iOptions = Strip::kOptNone;}
    virtual ~StripBuffer() {}

    virtual void    SetOptions(int options) {iOptions = options;}
    virtual string  GetDescriptor() const {return "strip:" + (iCreateString.empty() ? "unknown" : iCreateString); }
    virtual bool    Update() {iLastError = "Attempted to update invalid strip."; return false;}
    void            SetCreateString(csref str) {iCreateString = str;}

private:
    string              iCreateString;
    Strip::Options_t    iOptions;
    // Don't allow copying
    StripBuffer(const StripBuffer&);
    StripBuffer& operator=(const StripBuffer&);
};

class StripBufferWS2801 : public StripBuffer
{
public:
    StripBufferWS2801(int count, int SDIgpio, int CLKgpio) : StripBuffer(count), iSDIgpio(SDIgpio), iCLKgpio(CLKgpio) {}
    virtual ~StripBufferWS2801();

    virtual bool    Update();
private:
    int iSDIgpio;
    int iCLKgpio;
    void MaybeInitializeStrip();
    // Don't allow copying
    StripBuffer(const StripBuffer&);
    StripBuffer& operator=(const StripBuffer&);
};
#endif    // !OS_WINDOWS

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkStrip();

#endif // !STRIPBUFFER_H_INCLUDED
