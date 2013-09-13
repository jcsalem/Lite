// Implements various virtual buffers

#include "utilsPrecomp.h"

// Dummy function to force this file to be linked in.
void ForceLinkMeta() {}

#include "utils.h"
#include "MetaBuffer.h"
#include <algorithm>

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

vector<string> ComboError(string* errmsgptr, csref msg) {
    if (errmsgptr) *errmsgptr = msg;
    vector<string> nullReturn;
    return nullReturn;
}

// Parses a description string into a list of devices. Handles nested square braces. No escaping (yet)
vector<string> ParseDeviceList(csref descStr, string* errmsg) {
    vector<string> descStrings;
    int startPos = 0;
    int bracketDepth = 0;
    bool lookingForComma = false;

    for (size_t i = 0; i < descStr.size(); ++i) {
        switch (descStr[i])
        {
        case '[':
            ++bracketDepth;
            if (lookingForComma) return ComboError(errmsg, "Missing comma after ']'");
            break;
        case ']':
            if (bracketDepth == 0) return ComboError(errmsg, "Mismatched square brackets: Too many right brackets.");
            if (lookingForComma)   return ComboError(errmsg, "Misplaced ']', expected a comma");
            --bracketDepth;
            if (bracketDepth == 0) lookingForComma = true;
            break;
        case ',':
            if (bracketDepth == 0) {
                descStrings.push_back(TrimWhitespace(descStr.substr(startPos, i - startPos)));
                startPos = i+1;
                lookingForComma = false;
            }
        case ' ':
        case '\t':
            break;
        default:
            if (lookingForComma) return ComboError(errmsg, "Expected a comma after the right bracket.");
        }
    }
    if (bracketDepth != 0) return ComboError(errmsg, "Mismatched square brackets: Too many left brackets.");
    descStrings.push_back(TrimWhitespace(descStr.substr(startPos, descStr.size() - startPos)));
    return descStrings;
}

LBuffer* ComboBuffer::Create(csref descStr, string* errmsg) {
    vector<string> descStrings = ParseDeviceList(descStr, errmsg);
    if (descStr.size() == 0) {
        if (errmsg && errmsg->empty()) *errmsg = "Empty list of devices.";
        return NULL;
    }
//    if (descStr.size() == 1 && collapse) return LBuffer::Create(descStrings[0], errmsg);

    ComboBuffer* combo = new ComboBuffer();
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

// Used by CreateOutputBuffer to collapse a ComboBuffer with a single buffer
LBuffer* ComboBuffer::PopLastBuffer()
{
    if (iBuffers.size() == 0) return NULL;
    iCount -= iCounts.back();
    iCounts.pop_back();
    LBuffer* buffer = iBuffers.back();
    iBuffers.pop_back();
    return buffer;
}


// Only used for documentation string
DEFINE_LBUFFER_TYPE(combo_internal, ComboBuffer::Create, "[<device spec>,<device spec>,...]",
        "Combines all devices end-to-end into a single combo device.");

//-----------------------------------------------------------------------------
// Utility Fcn
//-----------------------------------------------------------------------------

LBuffer* SafeCreateBuffer(csref typeStr, csref descStr, string* errmsg){
    if (descStr.empty()) {
        if (errmsg) *errmsg = "Missing device specification to '" + typeStr + "' type.";
        return NULL;
    }
    return LBuffer::Create(descStr, errmsg);
}

//-----------------------------------------------------------------------------
// ReverseBuffer
//-----------------------------------------------------------------------------

LBuffer* ReverseBufferCreate(csref descStr, string* errmsg)
{
    LBuffer* buffer = SafeCreateBuffer("flip", descStr, errmsg);
    if (! buffer) return NULL;
    return new ReverseBuffer(buffer);
}

DEFINE_LBUFFER_TYPE(flip, ReverseBufferCreate, "flip:<device spec>",
        "Outputs to the device described by <device spec>, but flips the result.");

//-----------------------------------------------------------------------------
// LBufferMap -- Abstract class for any type that uses a map
//-----------------------------------------------------------------------------

LBufferMap::LBufferMap(LBuffer* buffer) : LBuffer(), iBuffer(buffer)
{
    int len = iBuffer->GetCount();
    iMap = vector<int>(len);
    for (int i = 0; i < len; ++i) {
        iMap[i] = i;
    }
}

//-----------------------------------------------------------------------------
// RandomizedBuffer
//-----------------------------------------------------------------------------
void RandomizedBuffer::RandomizeMap()
{
    random_shuffle(iMap.begin(), iMap.end());
}


LBuffer* RandomizedBufferCreate(csref descStr, string* errmsg)
{
    LBuffer* buffer = SafeCreateBuffer("random", descStr, errmsg);
    if (! buffer) return NULL;
    return new RandomizedBuffer(buffer);
}

DEFINE_LBUFFER_TYPE(random, RandomizedBufferCreate, "random:<device spec>",
                    "Randomizes the order of pixels in the device.");

//-----------------------------------------------------------------------------
// Skip2Buffer
//-----------------------------------------------------------------------------
Skip2Buffer::Skip2Buffer(LBuffer* buffer) : LBufferMap(buffer)
{
    int len = buffer->GetCount();
    int idx = 0;
    for (int i = 0; i < len; i += 2)
        iMap[idx++] = i;
    for (int i = 1; i < len; i += 2)
        iMap[idx++] = i;
}


LBuffer* Skip2BufferCreate(csref descStr, string* errmsg)
{
    LBuffer* buffer = SafeCreateBuffer("skip2", descStr, errmsg);
    if (! buffer) return NULL;
    return new Skip2Buffer(buffer);
}

DEFINE_LBUFFER_TYPE(skip2, Skip2BufferCreate, "skip2:<device spec>",
                    "Interleaves the order of pixels.");
