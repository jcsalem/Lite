// Implements various output filters

#include "utils.h"
#include "LBuffer.h"
#include "LFilter.h"
#include "LFramework.h"
#include "utilsParse.h"
#include <cmath>
#include "utilsRandom.h"
#include <iostream>

//-----------------------------------------------------------------------------
// Creating a filter
//-----------------------------------------------------------------------------

static LFilter* CreateError(string* errmsgptr, csref msg) {
    if (errmsgptr) *errmsgptr = msg;
    return NULL;
    }

LFilter* LFilter::Create(csref desc, string* errmsg) {
    size_t argpos = desc.find_first_of(":(");

    // Look up the filter
    string name = desc.substr(0,argpos);
    const LBufferType* type = LBufferType::Find(name);
    if (!type || !type->iIsFilter) return CreateError(errmsg, "No filter named: " + name);

    // Parse parameters (this should be shared with LBuffer::Create)
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
    return type->iFilterCreateFcn(params, errmsg);
}

//-----------------------------------------------------------------------------------------------------
// This section forces the linking of all filters
//-----------------------------------------------------------------------------------------------------


extern void ForceLinkMapFilters();
extern void ForceLinkEffectFilters();

void ForceFilterLinking() {
    ForceLinkMapFilters();
    ForceLinkEffectFilters();
};


