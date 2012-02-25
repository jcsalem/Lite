#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <string>
#include "../cklib/utils.h"
#include "../cklib/Color.h"
#include "../cklib/cklib.h"

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------

string gIP;
int gNumLights = 512;

//----------------------------------------------------------------------------
// Option Parsing
//----------------------------------------------------------------------------

void Usage(int argc, char** argv)
    {
    string progname;
    if (argc > 0)
        progname = argv[0];
    cerr << "Usage: " << progname << " --ip ipaddr"
            << " command" << endl;
    cerr << "Where:" << endl;
    cerr << "  ipaddr is the power supply IP address" << endl;
    cerr << "  command is one of: " << endl;
    cerr << "    clear " << endl;
    cerr << "    set r g b" << endl;
    exit (EXIT_FAILURE);
    }
struct option longOpts[] =
    {
        {"ip",      required_argument,  0, 'i'},
        {"num",     required_argument,  0, 'n'},
        {"help",    no_argument,        0, 'h'},
        {0,0,0,0}
    };

void ParseArgsCheckArg(const char* param, int argc, char** argv)
{
    if (!optarg)
    {
        cerr << "Missing argument for --" << param << ". Was -" << param << " used instead?" << endl;
        Usage(argc, argv);
    }
}

string ParseArgs(int argc, char** argv)
{
    bool hasError = false;
    optind = 0; // avoid warning

    while (true)
    {
        int optIndex;
        char c = getopt_long (argc, argv, "", longOpts, &optIndex);
        if (c == -1) break; // Done parsing

        switch (c)
        {
        case '?':
            // Error message already issues
            hasError = true;
            break;
        case 'i':
            ParseArgsCheckArg("ip", argc, argv);
            gIP = optarg;
            break;
        case 'n':
            ParseArgsCheckArg("num", argc, argv);
            gNumLights = atoi(optarg);
            break;
        case 'h':
            Usage(argc, argv);
        default:
            cerr << "Internal error - unknown option: " << c << endl;
            hasError = true;
        }
    }
    if (optind == argc || hasError)
        Usage(argc, argv);

    return string(argv[optind++]);
}

void ValidateNumArgs(csref command, int numArgs, int argc, char** argv)
    {
        if (argc == optind + numArgs)
            // Everything is correct
            return;
        cerr << command << " expected " << numArgs << " parameters but got " << argc - optind << endl;
        Usage(argc, argv);
    }

int main(int argc, char** argv)
{
    // Parse arguments
    string command = ParseArgs(argc, argv);
    if (gIP.empty())
    {
        cerr << argv[0] << ": Must specify at least one IP address of a CK device" << endl;
        Usage(argc, argv);
    }

    // Sanity check arguments
    IPAddr ip(gIP);
    if (! ip.IsValid())
    {
        cerr << argv[0] << ": The IP address was invalid (" << gIP << ")" << endl;
        exit(EXIT_FAILURE);
    }
    if (gNumLights <= 0 || gNumLights > 512)
    {
        cerr << argv[0] << ": --num must be between 1 and 512" << endl;
        exit(EXIT_FAILURE);
    }

    // Parse commands
    RGBColor color;
    if (command == "clear")
    {
        ValidateNumArgs(command, 0, argc, argv);
        color = BLACK;

    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 3, argc, argv);
        int r = atoi(argv[optind++]);
        int g = atoi(argv[optind++]);
        int b = atoi(argv[optind++]);
        color = RGBColor(r,g,b);
    }

    cout << "Success: " << command << "  Color: " << color.r << " " << color.g << " " << color.b << endl;
    cout << "IP: " << gIP << "  Num: " << gNumLights << endl;

    // Create CKDevice and Buffer
    CKdevice dev(gIP, gNumLights);
    CKbuffer buffer(dev);
    buffer.SetAll(color);
    buffer.Update();

    if (buffer.HasError())
    {
        cerr << "Error updating CK device: " << buffer.GetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
