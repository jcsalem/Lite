// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "utilsOptions.h"
#include "LFramework.h"
#include <iostream>
#include <stdio.h>
#include <math.h>

//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------

typedef enum {kUp, kDown, kQuit, kNop} Key_t;
#ifdef OS_WINDOWS
#include <Windows.h>
HANDLE gConsoleHandle = 0;
void InitializeConsole() {
    gConsoleHandle = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(gConsoleHandle, ENABLE_PROCESSED_INPUT);
}

Key_t GetOneChar() {
    INPUT_RECORD inrec;
    DWORD numread = 0;
    if (! ReadConsoleInput(gConsoleHandle, &inrec, 1, &numread)) return kNop;
    if (numread != 1) return kNop;
    if (inrec.EventType != KEY_EVENT) return kNop;
    if (! inrec.Event.KeyEvent.bKeyDown) return kNop;
    switch (inrec.Event.KeyEvent.wVirtualKeyCode) {
        case VK_UP:         return kUp;
        case VK_DOWN:       return kDown;
        case 'Q':           return kQuit;
        default:            return kUp;
    }
}

#elif defined(OS_LINUX) || defined(OS_MAC)
// For getch
#include <curses.h>
#include "CursesBuffer.h"

void InitializeConsole() {
    MaybeInitializeCurses();
}

Key_t GetOneChar() {
    int c = getch();
    switch (c) {
        case 'q':       return kQuit;
        case 'Q':       return kQuit;
        case KEY_UP:    return kUp;
        case KEY_DOWN:  return kDown;
        default:    return kUp;
    }
}

#endif
//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
float       gWidth = 5;
RGBColor    gColor1 = RED;
RGBColor    gColor2 = GREEN;
RGBColor    gColor3 = BLUE;

RGBColor* GetColorRef(csref name) {
    if (name == "color1") return &gColor1;
    if (name == "color2") return &gColor2;
    if (name == "color3") return &gColor3;
    return &gColor3; // set the least important if we get a bad string
}

string ParseColorCallback(csref name, csref val) {
    RGBColor* colorRef = GetColorRef(name);
    string errmsg;

    if (RGBColor::FromString(val, colorRef, &errmsg))
        return "";
    else
        return "Bad --" + name + " argument: " + val + ". " + errmsg;
}

string DefaultColorCallback(csref name) {
    return GetColorRef(name)->ToString();
}

DefOption(color1, ParseColorCallback, "color", "is the first  color to mix", DefaultColorCallback);
DefOption(color2, ParseColorCallback, "color", "is the second color to mix", DefaultColorCallback);
DefOption(color3, ParseColorCallback, "color", "is the third  color to mix", DefaultColorCallback);

//----------------------------------------------------------------
// Movement code
//----------------------------------------------------------------

Lgroup gObjs;

void MoveOne(Lobj* obj, int incr){
    obj->pos.x += incr;
}


void Loop()
{
    int numLights = L::gOutputBuffer->GetCount();

    Lobj* left = new Lobj();
    Lobj* right = new Lobj();
    left->color     = gColor1;
    right->color    = gColor2;
    left->width     = gWidth;
    right->width    = gWidth;
    left->pos.x     = 0;
    left->pos.y     = 0;
    right->pos.x    = numLights - 1;
    right->pos.y    = 0;
    gObjs.Add(left);
    gObjs.Add(right);

    bool endLoop = false;
    while (!endLoop) {
        // Render
        L::gOutputBuffer->Clear();
        gObjs.RenderAll(L::gOutputBuffer);
        L::gOutputBuffer->Update();

        // Pause for input
        switch (GetOneChar()) {
            case kQuit:
                endLoop = true;
                break;
            case kDown:
                MoveOne(left, -1);
                MoveOne(right, 1);
                break;
            case kUp:
                MoveOne(left, 1);
                MoveOne(right,-1);
                break;
            case kNop:
            default:;
        }

        if (L::gVerbose)
            cout << "L(" << left->pos.x << ", " << left->pos.y << ")  R(" << right ->pos.x << ", " << right->pos.y << ")" << endl;
    }
}

DefProgramHelp(kPHprogram, "testmix");
DefProgramHelp(kPHusage, "Tests the mixing code");

int main(int argc, char** argv)
{
    Option::DeleteOption("rate");
    L::Startup(&argc, argv);
    // Test everything
    // TestLights();
    InitializeConsole();
    Loop();
}
