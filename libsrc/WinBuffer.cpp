// A Windows-based buffer using the SFML portable graphics library
//

#include "Config.h"
#include "WinBuffer.h"
#include "LFramework.h"

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

LBuffer* WinBufferCreate(csref descStr, string* errmsg) {
    WinInfo info(500,100);
    int count = 40;
    if (descStr.empty()) {}
    else if (StrToInt(descStr, &count)) {
        if (count <= 0) {
            *errmsg = "WIndows display type must have a positive could. Count was specified as " + descStr;
            return NULL;
            }
        }
    else {
        // Unparseable
        if (errmsg) *errmsg = "Couldn't parse window display's count paramter: " + descStr;
        return NULL;
        }

    // Return the buffer
    return new WinBuffer(count, info, "Lite Viewer");
}

DEFINE_LBUFFER_DEVICE_TYPE(window, WinBufferCreate, "window[:count]",
        "Outputs to a window. count is the number of big dots across the screen");

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
    sf::Vector2u size = iWindow.getSize();
    float width = 1.0 * size.x / GetCount();
    float diameter = (width < 17.0) ? width * .9 : 15.3;
    float space = width - diameter;
    if (diameter < 1) {diameter = 1; space = 0.0;}

    float x = space * .5;
    float y = .5 * size.y - diameter * .5;
    if (y < diameter * .5) y = .5 * size.y;

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
        x += diameter + space;
        iWindow.draw(shape);
    }
    iWindow.display();
    return true;
}




#endif // HAS_SFML
