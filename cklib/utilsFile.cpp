#include "utilsFile.h"
#include <fstream>

ios_base::openmode File::GetOpenMode(bool isWriteMode)
{
    ios_base::openmode mode;
    if (isWriteMode)
        mode |= ios_base::out;
    else
        mode |= ios_base::in;
    if (iMode & kModeBinary)
        mode |= ios_base::binary;
    return mode;
}

bool File::ReadToString(string* str)
{
    if (! str)
    {
        iLastError = "NULL string buffer";
        return false;
    }
    ifstream fstr(iName.c_str(), GetOpenMode(false));
    if (fstr.fail())
    {
        iLastError = "Error opening " + iName;
        return false;
    }
    fstr.seekg(0, ios::end);
    size_t length = fstr.tellg();
    fstr.seekg(0, ios::beg);
    char* buffer = new char[length + 1];
    fstr.read(buffer, length);
    buffer[length] = '\0';
    fstr.close();
    str->assign(buffer, length);
    return true;
}
