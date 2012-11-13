#include "../cklib/utils.h"
#include "../cklib/Color.h"
#include "../cklib/cklib.h"
#include "../cklib/utilsTime.h"
#include "../cklib/stdOptions.h"
#include <iostream>
#include <getopt.h>

float gRotateRate   = 1;    // Relative rotation rate

//----------------------------------------------------------------------------
// Option Parsing
//----------------------------------------------------------------------------

void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << CK::kStdOptionsArgs
            << " [--rate rateval]"
            << " command args" << endl;
    cerr << "Where:" << endl;
    cerr << CK::kStdOptionsArgsDoc << endl;
    cerr << "  numsecs is the time the command should run for in seconds " << endl;
    cerr << "  rateval is the relative speed for the rotate and rotwash commands (default value is 1.0)" << endl;
    cerr << "  command is one of: " << endl;
    cerr << "    clear " << endl;
    cerr << "    all color" << endl;
    cerr << "    rotate color" << endl;
    cerr << "    rotwash color1 color2" << endl;
    cerr << "    set idx color" << endl;
    cerr << "    wash color1 color2" << endl;
    cerr << "  color is \"r,g,b\" or \"HSV(h,s,v)\" or a named color, etc.  All components are scaled from 0.0 to 1.0" << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {"rate",    required_argument,  0, 'r'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int* argc, char** argv)
{
    // Parse stamdard options
    string errmsg;
    bool success = CK::StdOptionsParse(argc, argv, &errmsg);
    if (! success)
        Usage(progname, errmsg);

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
                break;
            case 'r':
                gRotateRate = atof(optarg);
                if (gRotateRate <= 0)
                    Usage(progname, "--rate argument must be positive. Was " + string(optarg));
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
    float defaultRotateDelay = 0;

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
        idx = 0;
        color = Color::AllocFromString(argv[optind++], &errmsg);
        defaultRotateDelay = 50;
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
        defaultRotateDelay = 25;
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
    if (CK::gVerbose) {
        cout << "Cmd: " << command << "   Color: " << color->ToString();
        if (doWash)
            cout << " Color2: " << color2->ToString();
        if (idx != -1)
            cout << "  Index: " << idx;
        if (CK::gRunTime != 0)
            cout << "  Time: " << CK::gRunTime << " seconds";
        if (defaultRotateDelay != 0 && gRotateRate != 1)
            cout << "  Rate: " << gRotateRate;
        cout << endl;
    }

    // Set and update the lights
    if (doWash) {
        HSVColorRange range(*color, *color2);
        int numLights = CK::gOutputBuffer->GetCount();;
        float stepSize = 1.0f / numLights;
        for (int i = 0; i < numLights; ++i) {
            HSVColor col = range.GetColor(i * stepSize);
            CK::gOutputBuffer->SetColor(i, col);
        }
    } else
    if (idx == -1)
        CK::gOutputBuffer->SetAll(*color);
    else
        CK::gOutputBuffer->SetColor(idx, *color);
    CK::gOutputBuffer->Update();

     // Handle rotation and/or timedelay
    Milli_t duration = CK::gRunTime * 1000;
    if (defaultRotateDelay != 0) {
        Milli_t startTime = Milliseconds();
        Milli_t sleepBetween = defaultRotateDelay / gRotateRate;
        while (true) {
            CK::gOutputBuffer->Rotate();
            CK::gOutputBuffer->Update();
            if (CK::gOutputBuffer->HasError()) {
                cerr << "Update error: " << CK::gOutputBuffer->GetLastError() << endl;
                exit(EXIT_FAILURE);
            }

            SleepMilli(sleepBetween);
            if (duration > 0 && MillisecondsDiff(Milliseconds(),  startTime) > (Milli_t) CK::gRunTime * 1000) break;

        }
    } else
        SleepMilli(duration);
    exit(EXIT_SUCCESS);
}
