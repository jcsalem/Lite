// Implements various virtual buffers

#include "utilsPrecomp.h"

// Dummy function to force this file to be linked in.
void ForceLinkMeta() {}

#include "utils.h"
#include "MetaBuffer.h"

//-----------------------------------------------------------------------------
// ComboBuffer
//-----------------------------------------------------------------------------

#if 0
string ComboBuffer::GetDescriptor() const {
    return "curses:" + IntToStr(GetCount());
}

bool    Update();

virtual RGBColor&   GetRawRGB(int idx);
void AddBuffer(LBuffer*);
#endif

//-----------------------------------------------------------------------------
// ReverseBuffer
//-----------------------------------------------------------------------------

LBuffer* ReverseBufferCreate(csref descStr, string* errmsg) {
    if (TrimWhitespace(descStr).empty()) {
        if (errmsg) *errmsg = "Missing device specification to 'flip' type.";
        return NULL;
    }
    LBuffer* buffer = LBuffer::Create(descStr, errmsg);
    if (! buffer) return NULL;
    return new ReverseBuffer(buffer);
}

DEFINE_LBUFFER_TYPE(flip, ReverseBufferCreate, "flip:<device spec>",
        "Outputs to the device described by <device spec>, but flips the result.");

