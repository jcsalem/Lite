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
string gPortList = "1r,2";
int gLightsPerFixture = 50;

//----------------------------------------------------------------------------
// Option Parsing
//----------------------------------------------------------------------------

void Usage(int argc, char** argv)
    {
    string progname;
    if (argc > 0)
        progname = argv[0];
    cerr << "Usage: " << progname << " --ip ipaddr --port portlist --count count"
            << " command" << endl;
    cerr << "Where:" << endl;
    cerr << "  ipaddr is the power supply IP address (no default)" << endl;
    cerr << "  portlist is a comma separated list of fixture ports. Follow the port number " << endl;
    cerr << "          with an 'r' if you want it reversed. (default is 1r,2)" << endl;
    cerr << "  count is the number of LEDs per port (default is 50)" << endl;
    cerr << "  command is one of: " << endl;
    cerr << "    clear " << endl;
    cerr << "    all r g b" << endl;
    cerr << "    rotate r g b" << endl;
    cerr << "    set idx r g b" << endl;
    exit (EXIT_FAILURE);
    }
struct option longOpts[] =
    {
        {"ip",      required_argument,  0, 'i'},
        {"port",     required_argument,  0, 'p'},
        {"count",     required_argument,  0, 'c'},
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
        case 'p':
            ParseArgsCheckArg("port", argc, argv);
            gPortList = optarg;
            break;
        case 'c':
            ParseArgsCheckArg("count", argc, argv);
            gLightsPerFixture = atoi(optarg);
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

    vector<int> portlist;
    vector<bool> reverselist;
    if (gPortList == "1")       {portlist.push_back(1); reverselist.push_back(false);}
    else if (gPortList == "1r") {portlist.push_back(1); reverselist.push_back(true);}
    else if (gPortList == "2")  {portlist.push_back(2); reverselist.push_back(false);}
    else if (gPortList == "2r") {portlist.push_back(2); reverselist.push_back(true);}
    else if (gPortList == "1,2")
    {
        portlist.push_back(1); reverselist.push_back(false);
        portlist.push_back(2); reverselist.push_back(false);
    }
    else if (gPortList == "1r,2")
    {
        portlist.push_back(1); reverselist.push_back(true);
        portlist.push_back(2); reverselist.push_back(false);
    }
    else if (gPortList == "1,2r")
    {
        portlist.push_back(1); reverselist.push_back(false);
        portlist.push_back(2); reverselist.push_back(true);
    }
    else if (gPortList == "1r,2r")
    {
        portlist.push_back(1); reverselist.push_back(true);
        portlist.push_back(2); reverselist.push_back(true);
    }
    else
    {
        cerr << argv[0] << ": Bad port list.  It should be 1, 2, 1r, 2r, etc." << endl;
        Usage(argc, argv);
    }

    if (gLightsPerFixture <= 0 || gLightsPerFixture > 256)
    {
        cerr << argv[0] << ": --count must be between 1 and 256" << endl;
        exit(EXIT_FAILURE);
    }

    // Parse commands
    int idx = -1;
    RGBColor color;
    if (command == "clear")
    {
        ValidateNumArgs(command, 0, argc, argv);
        color = BLACK;

    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 4, argc, argv);
        idx   = atoi(argv[optind++]);
        int r = atoi(argv[optind++]);
        int g = atoi(argv[optind++]);
        int b = atoi(argv[optind++]);
        color = RGBColor(r,g,b);
    }
    else if (command == "all" || command == "rotate")
    {
        ValidateNumArgs(command, 3, argc, argv);
        int r = atoi(argv[optind++]);
        int g = atoi(argv[optind++]);
        int b = atoi(argv[optind++]);
        color = RGBColor(r,g,b);
    }
    else
    {
        cerr << argv[0] << ": Unknown command \"" << command << "\"" << endl;
        Usage(argc, argv);
    }

    cout << "Success: " << command << "  Color: " << color.r << " " << color.g << " " << color.b << endl;
    cout << "IP: " << gIP << "  Ports: " << gPortList << "  Count: " << gLightsPerFixture << endl;

    // Create CKDevice and Buffer
    CKbuffer buffer;
    for (int i = 0; i < (int) portlist.size(); ++i)
    {
        CKdevice dev(gIP, portlist[i], gLightsPerFixture);
        buffer.AddDevice(dev, reverselist[i]);
    }

    if (command == "rotate")
    {
        while (true)
            for (int i = 0; i < buffer.GetCount(); ++i)
            {
                buffer.Clear();
                buffer.SetRGB(i,color);
                buffer.Update();
                Sleep(50);
            }
    }
    if (idx == -1)
        buffer.SetAll(color);
    else
        buffer.SetRGB(idx, color);
    buffer.Update();

    if (buffer.HasError())
    {
        cerr << "Error updating CK device: " << buffer.GetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
