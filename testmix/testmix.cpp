// Top level firefly code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "cklib.h"
#include "Lobj.h"
#include "utilsRandom.h"
#include "stdOptions.h"
#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <math.h>

//----------------------------------------------------------------
// Utilities
//----------------------------------------------------------------
RGBColor    gColor1 = RED;
RGBColor    gColor2 = GREEN;
RGBColor    gColor3 = BLUE;
float       gWidth = 5;

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

#elif defined(OS_LINUX)
// For getch
#include <curses.h>

void InitializeConsole() {}
Key_t GetOneChar() {
    char c = getch();
    switch (c) {
        case 'q': return kQuit;
        case 'Q': return kQuit;
        case '\0': return kNop;
        default:    return kUp;
    }
}

#endif
//----------------------------------------------------------------
// Argument Parsing
//----------------------------------------------------------------
void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << CK::kStdOptionsArgs << "" << endl;
    cerr << "Where:" << endl;
    cerr << CK::kStdOptionsArgsDoc << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"color1",  required_argument,  0, '1'},
        {"color2",  required_argument,  0, '2'},
        {"color3",  required_argument,  0, '3'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int* argc, char** argv)
{
    // Parse stamdard options
    string errmsg;
    bool success = CK::StdOptionsParse(argc, argv, &errmsg);
    if (! success)
        Usage(progname, errmsg);

    // Parse remaining
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (*argc, argv, "", longOpts, &optIndex);
        if (c == -1) break; // Done parsing
        switch (c)
            {
            case 'h':
                Usage(progname);
            case '1':
                if (! RGBColor::FromString(optarg, &gColor1, &errmsg)) Usage(progname, "Bad --color1 argument: " + string(optarg) + errmsg);
                break;
            case '2':
                if (! RGBColor::FromString(optarg, &gColor2, &errmsg)) Usage(progname, "Bad --color2 argument: " + errmsg);
                break;
            case '3':
                if (! RGBColor::FromString(optarg, &gColor3, &errmsg)) Usage(progname, "Bad --color3 argument: " + errmsg);
                break;
            default:
                cerr << "Internal error - unknown option: " << c << endl;
                Usage(progname);
            }
    }
}

Lgroup gObjs;

void MoveOne(LobjBase* obj, int incr){
    obj->pos.x += incr;
}


void Loop()
{
    int numLights = CK::gOutputBuffer->GetCount();

    LobjBase* left = new LobjBase();
    LobjBase* right = new LobjBase();
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
        CK::gOutputBuffer->Clear();
        gObjs.RenderAll(CK::gOutputBuffer);
        CK::gOutputBuffer->Update();

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

    }
}

int main(int argc, char** argv)
{
    const char* progname = "ckunknown";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

    // Parse arguments
    ParseArgs(progname, &argc, argv);

    // Parse command
    if (optind != argc)
    {
        cerr << "Too many arguments." << endl;
        Usage(progname);
    }

    // Test everything
    // TestLights();
    InitializeConsole();
    Loop();
}
