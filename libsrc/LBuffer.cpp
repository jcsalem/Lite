#include "utils.h"
#include "LBuffer.h"
#include "Color.h"
#include "Lproc.h"
#include "ComboBuffer.h"
#include "utilsParse.h"
#include "LFilter.h"
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
    string r = "LBuffer(";
    r += IntToStr(GetCount()) + " lights) ";
    r += GetDescriptor();
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

static LBuffer* CreateError(string* errmsgptr, csref msg) {
    if (errmsgptr) *errmsgptr = msg;
    return NULL;
    }

LBuffer* LBuffer::Create(csref descArg, string* errmsg) {
    string desc     = TrimWhitespace(descArg);
    if (desc.empty()) return CreateError(errmsg, "Missing device descriptor");
    if (desc[0] == '[') {
        if (desc[desc.size() - 1] != ']') return CreateError(errmsg, "Device descriptor started with a bracket but didn't end with one: " + descArg);
        return ComboBuffer::Create(desc.substr(1, desc.size()-2), errmsg);
    }

    // If we get here it is a single buffer or a pipeline of filters plus a buffer
    size_t pipepos  = desc.find('|');
    size_t bracketpos = desc.find('[');
    size_t nextpos = string::npos;

    if (pipepos != string::npos)
      nextpos = pipepos + 1;
    
    if (bracketpos != string::npos && (pipepos == string::npos || pipepos > bracketpos)) {
      pipepos = bracketpos;
      nextpos = bracketpos;
    } 

    bool isFilter = (nextpos != string::npos);
    string nextdev  = nextpos != string::npos ? desc.substr(nextpos) : "";
    desc = TrimWhitespace(desc.substr(0, pipepos));
    // Now parse the current filter or device
    size_t argpos = desc.find_first_of(":(");
    string name = desc.substr(0,argpos);
    if (name.empty()) return CreateError(errmsg, "Missing " + string(isFilter ? "filter" : "device") + " name: " + descArg);

    const LBufferType* type = LBufferType::Find(name);
    if (! type) return CreateError(errmsg, "No " + string(isFilter ? "filter" : "device type") + " named: " + name);

    vector<string> params; 
    string paramsString;
    if (argpos != string::npos) {
        paramsString = desc.substr(argpos+1);
        if (desc[argpos] == '(') {
            if (desc[desc.length()-1] != ')') return CreateError(errmsg, "Missing right parenthesis in arguments to " + name);
            paramsString = paramsString.substr(0, paramsString.length() - 1); // remove trailing parenthesis
        }
        // Parse the parameters
        string errmsg2;
        params = ParseParamList(paramsString, name, &errmsg2);
        if (params.empty() && !errmsg2.empty()) return CreateError(errmsg, errmsg2);
    }
    
    if (isFilter) {
        // Expecting a filter
        if (! type->iIsFilter) return CreateError(errmsg, "Device type \"" + desc + "\" found when filter name expected: " + descArg);
        if (nextdev.empty()) return CreateError(errmsg, "Missing device after pipe: " + descArg);
        LFilter* filter = type->iFilterCreateFcn(params, errmsg);
        if (! filter) return NULL;
        // Now get the next component
        LBuffer* buffer = LBuffer::Create(nextdev,errmsg);
        if (! buffer) return NULL;
        filter->SetBuffer(buffer);
        return filter;
    } else {
        // Expecting output device
        if (type->iIsFilter) return CreateError(errmsg, "Filter name \"" + desc + "\" found when device type expected: " + descArg);
        return type->iDeviceCreateFcn(params, errmsg);
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
// This section forces the linking of required devices (otherwise, CKlib, etc. are never loaded!)
//-----------------------------------------------------------------------------------------------------
#include "CKbuffer.h"
#include "CursesBuffer.h"
#include "StripBuffer.h"
#include "WinBuffer.h"
void ForceLinking() {
    ForceLinkCK();
    ForceLinkCurses(); // This actually does nothing on Windows
    ForceLinkStrip(); // This actually does nothing on Windows
    ForceLinkWin();
};
