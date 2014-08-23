// A flavor of LBuffer that displays on a window based on the cross-platform SFML toolkit

#ifndef WINBUFFER_H_INCLUDED

#include "Config.h"

#if defined(HAS_SFML) && HAS_SFML
#include <SFML/Graphics.hpp>
#include "LBuffer.h"

struct WinInfo
{
    WinInfo(unsigned int w, unsigned int h, bool isVert = false) : width(w),height(h),depth(32),antialiaslevel(8),isVertical(isVert) {}
    WinInfo() {*this = WinInfo(0,0);}
    unsigned int        width;
    unsigned int        height;
    unsigned int        depth;
    unsigned int        antialiaslevel;
    bool                isVertical;
};

class WinBuffer : public LBufferPhys
{
public:
    WinBuffer(int count, const WinInfo& winInfo, csref title = "Lite");
    virtual ~WinBuffer() {iWindow.close();}

    void        SetTitle(csref title) {iWindow.setTitle(title);}

    virtual string  GetDescriptor() const;
    virtual bool    Update();
    void            SetCreateString(csref str)  {iCreateString = str;}
    bool            IsVertical() const {return iWinInfo.isVertical;}

private:
    string              iCreateString;
    WinInfo             iWinInfo;  // Initial setup
    sf::RenderWindow    iWindow;

    // Don't allow copying
    WinBuffer(const WinBuffer&);
    WinBuffer& operator=(const WinBuffer&);
};

#endif // HAS_SFML

// This function is defined only so LFramework can reference it and force it to be linked in.
void ForceLinkWin();

#endif // _WINBUFFER_H
