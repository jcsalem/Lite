// Implements an LBuffer variant using the curses console library

#include "CursesBuffer.h"

// Dummy function to force this file to be linked in.
void ForceLinkCurses() {}

#ifndef OS_WINDOWS
#include "utils.h"
#include "Color.h"
#include <curses.h>
#include <iostream>

//-----------------------------------------------------------------------------
// Utility functions
//-----------------------------------------------------------------------------

string CursesBuffer::GetDescriptor() const {
    return "console:" + IntToStr(GetCount());
}

void MaybeInitializeCurses() {
    static bool firstTime = true;
    if (firstTime) {
        setlocale(LC_ALL, "");
        initscr();              // Initialize curses
        cbreak();               // Raw input mode (needed for getch to work)
        noecho();               // Don't echo input
        keypad(stdscr, TRUE);   // Make sure that arraow keys are supported
        curs_set(0);             // Invisible cursor
        firstTime = false;

#if 0
        move(5,0); refresh();
        cout << "Screen size: " << getmaxx(stdscr) << "," << getmaxy(stdscr) << "  Origin: " << getbegx(stdscr) << "," << getbegy(stdscr)  << endl;
        cout << "After!!!" << endl;
        move(0,0); addch('0');
        move(1,0); addch('1');
        move(2,0); addch('2');
        move(3,0); addch('3');
        move(4,0); addch('4');
        refresh();
#endif
    }
}

//-----------------------------------------------------------------------------
// Creation function
//-----------------------------------------------------------------------------

LBuffer* CursesBufferCreate(csref descStr, string* errmsg) {
    int width = 72;
    if (descStr.empty() || StrEQ(descStr, "auto")) {
        MaybeInitializeCurses();
        width = getmaxx(stdscr) - getbegx(stdscr);
        if (width < 2) width = 10;
        }
    else if (StrToInt(descStr, &width)) {
        if (width <= 0) {
            *errmsg = "Console display type must have a positive width. Width was specified as " + descStr;
            return NULL;
            }
        }
    else {
        // Unparseable
        if (errmsg) *errmsg = "Couldn't parse console display's size paramter: " + descStr;
        return NULL;
        }

    // Return the buffer
    return new CursesBuffer(width);
}

DEFINE_LBUFFER_DEVICE_TYPE(console, CursesBufferCreate, "console[:size]",
        "Outputs to the console. No size defaults to screen width.\n"
        "  Examples: console or console:50");

//-----------------------------------------------------------------------------
// Update function
//-----------------------------------------------------------------------------

// Testing note:
// For some reason on the mac, I'm getting terminal windows that don't display the first two lines.  That's why we start displaying at line two
const int gBegYoffset = 2;

chtype RGBColorToCursesChar(const RGBColor& color) {
    float value = (color.r + color.g + color.b) / 3;

    if      (value <= 0)    return ' ';
    else if (value <= 0.1)   return ACS_BULLET;
    else if (value <= 0.35)  return 'o';
    else if (value <= 1.0)  return ACS_DIAMOND;
    else                    return ACS_CKBOARD;
}


bool CursesBuffer::Update() {
    MaybeInitializeCurses();
    move(gBegYoffset, 0);
    for (iterator i = begin(); i != end(); ++i) {
        addch(RGBColorToCursesChar(*i));
    }
    // Move the cursor out of the way and
    move(0,0);
    refresh();
    return true;
}

#endif