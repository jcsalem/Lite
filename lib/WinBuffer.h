// A flavor of LBuffer that displays on a window based on the cross-platform SFML toolkit

#ifndef WINBUFFER_H_INCLUDED

#include "Config.h"

#ifdef HAS_SFML


#endif // HAS_SFML

// This function is defined only so LFramework can reference it and force it to be linked in.
void ForceLinkWin();

#endif // _WINBUFFER_H
