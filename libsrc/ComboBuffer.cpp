// Implements various virtual buffers
#include "utils.h"
#include "ComboBuffer.h"
#include "utilsParse.h"

// Dummy function to force this file to be linked in.
void ForceLinkMeta() {}

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

#if 0
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
    int parenDepth = 0;
    bool lookingForComma = false;
 
    for (size_t i = 0; i < descStr.size(); ++i) {
        bool checkLookingForComma = true;
        switch (descStr[i])
        {
        case '[':
            ++bracketDepth;
            if (parenDepth > 0) return ComboError(errmsg, "Brackets cannot appear within parentheses");
            break;
        case ']':
            if (bracketDepth == 0) return ComboError(errmsg, "Mismatched square brackets: Too many right brackets.");
            if (parenDepth > 0)    return ComboError(errmsg, "Brackets cannot appear within parentheses");
            --bracketDepth;
            if (bracketDepth == 0) {
                checkLookingForComma = lookingForComma;
                lookingForComma = true;
            }
            break;
        case '(':
            ++parenDepth;
            break;
        case ')':
            if (parenDepth == 0) return ComboError(errmsg, "Mismatched parentheses: Too many right parentheses");
            --parenDepth;
            break;
        case ',':
            if (bracketDepth == 0 && parenDepth == 0) {
                descStrings.push_back(TrimWhitespace(descStr.substr(startPos, i - startPos)));
                startPos = i+1;
                lookingForComma = false;
            }
            break;
        case ' ':
        case '\t':
            checkLookingForComma = false;
            break;
        }
        if (checkLookingForComma && lookingForComma) return ComboError(errmsg, "Expected a comma after ']'");
    }
    if (bracketDepth != 0) return ComboError(errmsg, "Mismatched square brackets: Too many left brackets.");
    if (parenDepth != 0) return ComboError(errmsg, "Mismatched parentheses: Too many left parentheses.");
    descStrings.push_back(TrimWhitespace(descStr.substr(startPos, descStr.size() - startPos)));
    return descStrings;
}
#endif

LBuffer* ComboBuffer::Create(const vector<string>& descStrings, string* errmsg) {
    if (descStrings.size() == 0) {
        if (errmsg) *errmsg = "Empty list of devices.";
        return NULL;
    }

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

LBuffer* ComboBuffer::Create(csref descStr, string* errmsg) {
    vector<string> descStrings = ParseParamList(descStr, "devices", errmsg);
    if (descStrings.size() == 0) {
        if (errmsg && errmsg->empty()) *errmsg = "Empty list of devices.";
        return NULL;
    }
    return ComboBuffer::Create(descStrings, errmsg);
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
DEFINE_LBUFFER_DEVICE_TYPE(combo_internal, ComboBuffer::Create, "[deviceInfo,deviceInfo,...]",
        "Combines all devices end-to-end into a single combo device.");

