// A Windows-based buffer using the SFML portable graphics library
//

#include "Config.h"
#include "WinBuffer.h"
#include "LFramework.h"
#include "utilsParse.h"

// Dummy function to force this file to be linked in.
void ForceLinkWin() {}

#if defined(HAS_SFML) && HAS_SFML
WinBuffer::WinBuffer(int count, const WinInfo& winInfo, csref title) : LBufferPhys(count), iWinInfo(winInfo) {
    sf::ContextSettings cs;
    cs.antialiasingLevel = iWinInfo.antialiaslevel;
    iWindow.create(sf::VideoMode(winInfo.width, winInfo.height, winInfo.depth), title, sf::Style::Default, cs);
}

string  WinBuffer::GetDescriptor() const {
    if (iCreateString.empty()) return "window:(" + IntToStr(GetCount()) + ")";
    else return "window:" + iCreateString;
}

//-----------------------------------------------------------------------------
// Creation function
//-----------------------------------------------------------------------------

LBuffer* WinBufferCreate(cvsref params, string* errmsg) {
    if (! ParamListCheck(params, "window display", errmsg, 0, 2)) return NULL;
    int count = 40;
    bool isVertical = false;

    // Count
    if (params.size() > 0 && !params[0].empty()) 
        if (! ParseParam(&count, params[0], "window count", errmsg, 1)) return NULL;
    // Flags
    if (params.size() > 1 && !params[1].empty() && !StrEQ(params[1],"h")) {
        if (! StrEQ(params[1], "v")) return (LBuffer*) ParamErrmsgSet(errmsg, "window flags", "unknown flag argument", params[1]);
        isVertical = true;
    }

    WinInfo info;
    if (isVertical) info = WinInfo(100,500,true);
    else            info = WinInfo(500,100,false);

    // Return the buffer
    return new WinBuffer(count, info, "Lite Viewer");
}

DEFINE_LBUFFER_DEVICE_TYPE(window, WinBufferCreate, "window(count,flags)",
        "Outputs to a window. count is the number of dots to display (default 40). flags may be 'v' for vertical");

//-----------------------------------------------------------------------------
// Update function
//-----------------------------------------------------------------------------

bool    WinBuffer::Update() {
    if (! iWindow.isOpen() || L::gTerminateNow) {
        iLastError = "Window is Closed";
        L::gTerminateNow = true;
        return false;
    }
    // Handle any events
    sf::Event event;
    while (iWindow.pollEvent(event)) {
        switch (event.type)
        {
        case sf::Event::Closed:
            iWindow.close();
            L::gTerminateNow = true;
            break;
        case sf::Event::Resized: {
            // The view port isn't normally resized when the window is resized.  However, we always want the viewport to match the window
            sf::View newView(sf::FloatRect(0, 0, event.size.width, event.size.height));
            iWindow.setView(newView);
            break;
        }
        default:;
        }
    }
    if (L::gTerminateNow) return true;

    // Fiure out the radius and the space between
    sf::Vector2u winDims = iWindow.getSize();
    int totalRoom = IsVertical() ? winDims.y : winDims.x;
    float width = 1.0 * totalRoom / GetCount();
    float diameter = (width < 17.0) ? width * .9 : 15.3;
    float padding = width - diameter;
    if (diameter < 1) {diameter = 1; padding = 0.0;}

    float x, y;
    if (IsVertical()) {
        x = .5 * winDims.x - diameter * .5;
        if (x < diameter * .5) x = .5 * winDims.x;
        y = padding * .5;
    } else {
        x = padding * .5;
        y = .5 * winDims.y - diameter * .5;
        if (y < diameter * .5) y = .5 * winDims.y;
    }

    // Clear everything
    iWindow.clear(sf::Color(60,60,60));
    // Create the shape
    sf::CircleShape shape(diameter * .5);
    RGBColor rgb;

    for (LBuffer::iterator i = begin(); i < end(); ++i) {
        i->ToRGBColor(&rgb);
        sf::Color color(rgb.rAsChar(), rgb.gAsChar(), rgb.bAsChar());
        shape.setFillColor(color);
        shape.setPosition(x,y);
        if (IsVertical()) 
            y += diameter + padding;
        else
            x += diameter + padding;
        iWindow.draw(shape);
    }
    iWindow.display();
    return true;
}




#endif // HAS_SFML
