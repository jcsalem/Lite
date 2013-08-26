// Implements various virtual buffers

#include "utilsPrecomp.h"

// Dummy function to force this file to be linked in.
void ForceLinkMeta() {}

#include "utils.h"
#include "MetaBuffer.h"

//-----------------------------------------------------------------------------
// ComboBuffer
//-----------------------------------------------------------------------------

string ComboBuffer::GetDescriptor() const {
    string s = "[";
    for (vector<LBuffer*>::const_iterator i = iBuffers.begin(); i != iBuffers.end(); ++i) {
        if (i != iBuffers.begin()) s+= ",";
        s += (*i)->GetDescriptor();
    }

    s += "]";
    return s;
}

bool ComboBuffer::Update() {
    bool success = true;
    for (vector<LBuffer*>::const_iterator i = iBuffers.begin(); i != iBuffers.end(); ++i)
        success = (*i)->Update() && success;
    return success;
}

RGBColor& ComboBuffer::GetRawRGB(int idx) {
    for (size_t i = 0; i < iBuffers.size(); ++i) {
        if (idx < iCounts[i])
            return iBuffers[i]->GetRawRGB(idx);
        else
            idx = idx - iCounts[i];
    }
    return LBuffer::kNullColor;
}

ComboBuffer::~ComboBuffer() {
    for (vector<LBuffer*>::iterator i = iBuffers.begin(); i != iBuffers.end(); ++i)
        delete *i;
}

ComboBuffer::ComboBuffer(LBuffer* buffer) : LBuffer(), iCount(0)  {
    AddBuffer(buffer);
}

ComboBuffer::ComboBuffer(const vector<LBuffer*>& buffers) : LBuffer(), iCount(0) {
    AddBuffers(buffers);
}

void ComboBuffer::AddBuffers(const vector<LBuffer*>& buffers)
{
    int numBuffers = buffers.size() + iBuffers.size();
    iBuffers.reserve(numBuffers);
    iCounts.reserve(numBuffers);
    for (size_t i = 0; i < buffers.size(); ++i)
        AddBuffer(buffers[i]);
}

void ComboBuffer::AddBuffer(LBuffer* buffer)
{
    int count = buffer->GetCount();
    iBuffers.push_back(buffer);
    iCounts.push_back(count);
    iCount += count;
}

// Parses a description string into a list of devices. Handles nested square braces. No escaping (yet)
vector<string> ComboBuffer::ParseDeviceList(csref descStr, string* errmsg)
 {
    vector<string> nullReturn;
    vector<string> descStrings;
    int startPos = 0;
    int bracketDepth = 0;

    for (size_t i = 0; i < descStr.size(); ++i) {
        switch (descStr[i])
        {
        case '[':
            ++bracketDepth;
            break;
        case ']':
            if (bracketDepth == 0) {
                if (errmsg) *errmsg = "Mismatched square brackets.";
                return nullReturn;
            }
            --bracketDepth;
            break;
        // $$$$$$ NEEDS LOTS OF WORK
        }

    }
    string lastStr = TrimWhitespace(descStr.substr(startPos, descStr.size() - startPos));
    if (! lastStr.empty())
        descStrings.push_back(lastStr);
    return descStrings;
 }


LBuffer* ComboBuffer::Create(csref descStr, string* errmsg) {
    vector<string> descStrings = ParseDeviceList(descStr, errmsg);
    if (descStr.size() == 0) {
        if (errmsg && errmsg->empty()) *errmsg = "Empty list of devices.";
        return NULL;
    }
    ComboBuffer* combo = new ComboBuffer();
    vector<LBuffer*> buffers;
    for (vector<string>::const_iterator i = descStrings.begin(); i != descStrings.end(); ++i) {
        LBuffer* buffer = LBuffer::Create(*i, errmsg);
        if (!buffer) {
            delete combo;
            return NULL;
        } else {
            combo->AddBuffer(buffer);
        }
    }
    return combo;
}

DEFINE_LBUFFER_TYPE(combo, ComboBuffer::Create, "[<device spec>,<device spec>,...]",
        "Combines all devices end-to-end into a single device.");

//-----------------------------------------------------------------------------
// ReverseBuffer
//-----------------------------------------------------------------------------

LBuffer* ReverseBufferCreate(csref descStr, string* errmsg) {
    if (descStr.empty()) {
        if (errmsg) *errmsg = "Missing device specification to 'flip' type.";
        return NULL;
    }
    LBuffer* buffer = LBuffer::Create(descStr, errmsg);
    if (! buffer) return NULL;
    return new ReverseBuffer(buffer);
}

DEFINE_LBUFFER_TYPE(flip, ReverseBufferCreate, "flip:<device spec>",
        "Outputs to the device described by <device spec>, but flips the result.");

