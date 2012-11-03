#include "../cklib/utils.h"
#include "../cklib/Color.h"
#include "../cklib/cklib.h"
#include <iostream>
#include <getopt.h>

// The CK light buffer
CKbuffer gCKbuffer;
// Time in seconds before existing
int gRunTime = 0;

//----------------------------------------------------------------------------
// Option Parsing
//----------------------------------------------------------------------------

void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << CKbuffer::kArglistArgs
            << " command" << endl;
    cerr << "Where:" << endl;
    cerr << CKbuffer::kArglistDoc << endl;
    cerr << "  command is one of: " << endl;
    cerr << "    clear " << endl;
    cerr << "    all color" << endl;
    cerr << "    rotate color" << endl;
    cerr << "    set idx color" << endl;
    cerr << " <color> is \"r,g,b\" or \"HSV(h,s,v)\" or etc.  All components are scaled from 0.0 to 1.0" << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"time",    required_argument,  0, 't'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int* argc, char** argv)
{
    // Parse device arguments
    bool success = CKbuffer::CreateFromArglist(&gCKbuffer, argc, argv);
    if (! success)
        Usage(progname, gCKbuffer.GetLastError());
    if (gCKbuffer.GetCount() == 0)
        Usage(progname, "You must supply at least one --pds argument.");


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
            case 't':
                gRunTime = atoi(optarg);
                if (gRunTime <= 0)
                    Usage(progname, "--time argument must be positive. Was " + string(optarg));
                break;
            default:
                cerr << "Internal error - unknown option: " << c << endl;
                Usage(progname);
            }
    }
}

void ValidateNumArgs(csref command, int numArgs, const char* progname, int argc, char** argv)
    {
        if (argc == optind + numArgs)
            // Everything is correct
            return;
        cerr << command << " expected " << numArgs << " parameters but got " << argc - optind << endl;
        Usage(progname);
    }

int main(int argc, char** argv)
{
    const char* progname = "cktool";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

    // Parse arguments
    ParseArgs(progname, &argc, argv);

    // Parse command
    if (optind == argc)
        // No command argument
        Usage(progname);
    string command = argv[optind++];
    string errmsg;
    int idx = -1;  // if -1 all colors should be set
    Color* color;
    Color* color2;
    bool doWash   = false;
    bool doRotate = false;

    if (command == "clear")
    {
        ValidateNumArgs(command, 0, progname, argc, argv);
        color = new BLACK;
    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 2, progname, argc, argv);
        idx   = atoi(argv[optind++]);
        color = Color::AllocFromString(argv[optind++], &errmsg);
    }
    else if (command == "all")
    {
        ValidateNumArgs(command, 1, progname, argc, argv);
        color = Color::AllocFromString(argv[optind++], &errmsg);
    }
    else if (command == "rotate")
    {
        ValidateNumArgs(command, 1, progname, argc, argv);
        color = Color::AllocFromString(argv[optind++], &errmsg);
        doRotate = true;
    }
    else if (command == "wash")
    {
        ValidateNumArgs(command, 2, progname, argc, argv);
        color = Color::AllocFromString(argv[optind++], &errmsg);
        color2 = color ? Color::AllocFromString(argv[optind++], &errmsg) : NULL;
        doWash = true;
    }
    else if (command == "rotwash")
    {
        ValidateNumArgs(command, 2, progname, argc, argv);
        color = Color::AllocFromString(argv[optind++], &errmsg);
        color2 = color ? Color::AllocFromString(argv[optind++], &errmsg) : NULL;
        doWash = true;
        doRotate = true;
    }
    else
    {
        cerr << argv[0] << ": Unknown command \"" << command << "\"" << endl;
        Usage(progname);
    }

    // Validate the colors
    if (! color) Usage(progname, errmsg);
    if (doWash && !color2) Usage(progname, errmsg);

    // Print summary
    // $$$$ Add time, specify color in type-specific format, specify wash color
    RGBColor rgb(*color);

    cout << "Running: " << command << "  Color: " << rgb.r << " " << rgb.g << " " << rgb.b << endl;
    cout << gCKbuffer.GetDescription() << endl;

    // Set and update the lights
    if (doWash) {
        // $$$$
    } else
    if (idx == -1)
        gCKbuffer.SetAll(*color);
    else
        gCKbuffer.SetColor(idx, *color);
    gCKbuffer.Update();

    if (gCKbuffer.HasError())
    {
        cerr << "Error updating CK device: " << gCKbuffer.GetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    // Handle rotation and/or timedelay
    if (doRotate) {
        // $$$
    } else
        Sleep(gRunTime * 1000);

    exit(EXIT_SUCCESS);
}
