#ifndef _UTILSFILE_H
#define _UTILSFILE_H

#include "utils.h"
#include <ios>

class File
{
public:
    static const int kModeBinary = 1;
    static const int kModeText = 0;
    File(csref name, int mode = 0) {Init(name, mode);}
    void Init(csref name, int mode = 0);
    bool ReadToString(string* str);
    // Accessors
    string  GetName     () const {return iName;}
    bool    HasError    () const {return !iLastError.empty();}
    string  GetLastError() const {return iLastError;}

private:
    string iName;
    int    iMode;
    string iLastError;
    ios_base::openmode GetOpenMode(bool isWriteMode);
};


#endif // _UTILSFILE_H
