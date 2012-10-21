#include "../cklib/utils.h"
#include "../cklib/Color.h"
#include "../cklib/cklib.h"
#include <iostream>
#include <getopt.h>

// The CK light buffer
CKbuffer gCKbuffer;

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
    cerr << "    all r g b" << endl;
    cerr << "    rotate r g b" << endl;
    cerr << "    set idx r g b" << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
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
    int idx = -1;
    RGBColor color;
    if (command == "clear")
    {
        ValidateNumArgs(command, 0, progname, argc, argv);
        color = BLACK;

    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 4, progname, argc, argv);
        idx   = atoi(argv[optind++]);
        int r = atoi(argv[optind++]);
        int g = atoi(argv[optind++]);
        int b = atoi(argv[optind++]);
        color = RGBColor(r,g,b);
    }
    else if (command == "all" || command == "rotate")
    {
        ValidateNumArgs(command, 3, progname, argc, argv);
        int r = atoi(argv[optind++]);
        int g = atoi(argv[optind++]);
        int b = atoi(argv[optind++]);
        color = RGBColor(r,g,b);
    }
    else
    {
        cerr << argv[0] << ": Unknown command \"" << command << "\"" << endl;
        Usage(progname);
    }

    cout << "Running: " << command << "  Color: " << color.r << " " << color.g << " " << color.b << endl;
    cout << gCKbuffer.GetDescription() << endl;

    // Execute the command
    if (command == "rotate")
    {
        while (true)
            for (int i = 0; i < gCKbuffer.GetCount(); ++i)
            {
                gCKbuffer.Clear();
                gCKbuffer.SetRGB(i,color);
                gCKbuffer.Update();
                Sleep(50);
            }
    }
    // Everything other than rotate
    if (idx == -1)
        gCKbuffer.SetAll(color);
    else
        gCKbuffer.SetRGB(idx, color);
    gCKbuffer.Update();

    if (gCKbuffer.HasError())
    {
        cerr << "Error updating CK device: " << gCKbuffer.GetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
