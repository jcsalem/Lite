#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include "Lproc.h"
#include "ComboBuffer.h"
#include <vector>

RGBColor LBuffer::kNullColor = BLACK; // note that this is used by functions returning references to colors

void LBufferPhys::Alloc(int count)
{
  iBuffer.resize(count);
}

void LBuffer::SetAll(const Color& color)
{
    RGBColor rgb(color);
    iterator iter = begin();
    iterator endIter = end();
    while (iter != endIter) {
        *iter = rgb;
        ++iter;
    }
}

void LBuffer::SetColor(int idx, const Color& color)
{
    RGBColor rgb(color);
    SetRGB(idx, rgb);
}

string LBuffer::GetDescription() const {
    string r = "LBuffer: ";
    r += IntToStr(GetCount()) + " total lights";
    r += " [" + GetDescriptor() + "]";
    return r;
}

//----------------------------------------------------------------------------------------------------------------
// Functions for creating and managing LBuffers
//----------------------------------------------------------------------------------------------------------------

vector<LBufferType>& GetAllLBufferTypes() {
    static vector<LBufferType> allLBufferTypes;
    return allLBufferTypes;
}

LBufferType::LBufferType(csref name, DeviceFcn_t fcn, csref formatString, csref docString)
  : iName(StrToLower(name)), iIsFilter(false), iDeviceCreateFcn(fcn), iFilterCreateFcn(NULL), iFormatString(formatString), iDocString(docString)
  {
      GetAllLBufferTypes().push_back(*this);
  }

LBufferType::LBufferType(csref name, FilterFcn_t fcn, csref formatString, csref docString, bool ignored)
  : iName(StrToLower(name)), iIsFilter(true), iDeviceCreateFcn(NULL), iFilterCreateFcn(fcn), iFormatString(formatString), iDocString(docString)
  {
      GetAllLBufferTypes().push_back(*this);
  }


const LBufferType* LBufferType::Find(csref nameArg)
{
    string name = StrToLower(nameArg);
    const vector<LBufferType>& allLBufferTypes = GetAllLBufferTypes();
    for (size_t i = 0; i < allLBufferTypes.size(); ++i) {
        if (allLBufferTypes[i].iName == name) return &(allLBufferTypes[i]);
    }
    return NULL;
}

LBuffer* CreateError(string* errmsgptr, csref msg) {
    if (errmsgptr) *errmsgptr = msg;
    return NULL;
    }

LBuffer* LBuffer::Create(csref descArg, string* errmsg) {
    string desc     = TrimWhitespace(descArg);
    if (desc.empty()) return CreateError(errmsg, "Missing device descriptor");
    if (desc[0] == '[') {
        if (desc[desc.size() - 1] != ']') return CreateError(errmsg, "Device descriptor started with a bracket but didn't end with one: " + descArg);
        return ComboBuffer::Create(desc.substr(1, desc.size()-2));
    }

    // If we get here it is a single buffer or a pipeline of filters plus a buffer
    size_t pipepos  = desc.find('|');
    bool isFilter = (pipepos != string::npos);
    string nextdev  = isFilter ? desc.substr(pipepos + 1) : "";

    // Now parse the current filter or device
    desc = desc.substr(0, pipepos);
    size_t colonpos = desc.find(':');
    string name = desc.substr(0,colonpos);
    if (name.empty()) return CreateError(errmsg, "Missing " + string(isFilter ? "filter" : "device") + " name: " + descArg);

    const LBufferType* type = LBufferType::Find(name);
    if (! type) return CreateError(errmsg, "No " + string(isFilter ? "filter" : "device type") + " named: " + name);

    string args = (colonpos == string::npos) ? "" : TrimWhitespace(desc.substr(colonpos+1));
    if (isFilter) {
        if (! type->iIsFilter) return CreateError(errmsg, "Device type found when filter name expected: " + descArg);
        if (nextdev.empty()) return CreateError(errmsg, "Missing device after pipe: " + descArg);
        LBuffer* buffer = LBuffer::Create(nextdev,errmsg);
        if (! buffer) return NULL;
        return type->iFilterCreateFcn(args, buffer, errmsg);
    } else {
        // Output device}
        if (type->iIsFilter) return CreateError(errmsg, "Filter name found when device type expected: " + descArg);
        return type->iDeviceCreateFcn(args, errmsg);
    }
}

string LBufferType::GetDocumentation(bool isFilterType) {
    const vector<LBufferType>& allLBufferTypes = GetAllLBufferTypes();
    string r;
    bool firstTime = true;
    for (size_t i = 0; i < allLBufferTypes.size(); ++i) {
        if (allLBufferTypes[i].iIsFilter == isFilterType) {
            if (!firstTime) r += "\n";
            firstTime = false;
            r += allLBufferTypes[i].iFormatString + " - " + allLBufferTypes[i].iDocString;
        }
    }
    return r;
}

//-----------------------------------------------------------------------------------------------------
// This section forces the linking of required modules (otherwise, CKlib, etc. are never loaded!)
//-----------------------------------------------------------------------------------------------------
#include "CKbuffer.h"
#include "CursesBuffer.h"
#include "StripBuffer.h"
#include "FilterBuffers.h"
#include "WinBuffer.h"
void ForceLinking() {
    ForceLinkCK();
    ForceLinkCurses(); // This actually does nothing on Windows
    ForceLinkStrip(); // This actually does nothing on Windows
    ForceLinkWin();
    ForceLinkFilters();
};
