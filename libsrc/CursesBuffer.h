// An LBuffer variant built using ncurses
//

#ifndef CURSESBUFFER_H_INCLUDED
#define CURSESBUFFER_H_INCLUDED

#include "Config.h"

#ifndef OS_WINDOWS
#include <curses.h>
#include "LBuffer.h"

class CursesBuffer : public LBufferPhys
{
public:
    CursesBuffer(int width) : LBufferPhys(width) {}
    virtual ~CursesBuffer() {}

    virtual string  GetDescriptor() const;
    virtual bool    Update();

private:
    // Don't allow copying
    CursesBuffer(const CursesBuffer&);
    CursesBuffer& operator=(const CursesBuffer&);
};

// General curses support function
// Used by code that wants to make direct curses calls (e.g., testmix)
void MaybeInitializeCurses();

// Calls addchstr for the given string
int Curses_AddString(csref str);

#endif    // !OS_WINDOWS

// This function is defined only so LFramework can reference it and force it to be linked in. Otherwise, CKBuffer is never linked in!
void ForceLinkCurses();

#endif // !CURSESBUFFER_H_INCLUDED
