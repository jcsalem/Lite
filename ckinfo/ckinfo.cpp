// Display info about current CK devices on the LAN

#include "../cklib/utils.h"
#include "../cklib/utilsSocket.h"
#include "../cklib/cklib.h"
#include <iostream>
#include <getopt.h>

//----------------------------------------------------------------------------
// Option Parsing
//----------------------------------------------------------------------------

void Usage(const char* progname, csref msg = "")
    {
    if (! msg.empty()) cerr << msg << endl;
    cerr << "Usage: " << progname << endl;
    exit (EXIT_FAILURE);
    }

struct option longOpts[] =
    {
        {"help",    no_argument,        0, 'h'},
        {0,0,0,0}
    };


void ParseArgs(const char* progname, int* argc, char** argv)
{
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
            default:
                cerr << "Internal error - unknown option: " << c << endl;
                Usage(progname);
            }
    }
}

void DisplayDevice(const CKinfo& devInfo) {
    cout << "CK Device @ " << IPAddr(devInfo.ipaddr).GetString() << "  [MAC: " << devInfo.macaddr << "]"  << "   Serial: " << devInfo.serial << endl;
    cout << "Universe: " << devInfo.universe << " NumPorts: " << devInfo.numports<< endl;
    cout << " Name: " << devInfo.name << endl;
    string info = devInfo.info;
    info = strReplace(info, "\n", "\n ");
    cout << " " << info << endl;
}

int main(int argc, char** argv)
{
    const char* progname = "ckinfo";
    if (argc > 0 && argv != NULL && argv[0] != NULL)
        progname = argv[0];

    // Parse arguments
    ParseArgs(progname, &argc, argv);
    // Check remaining arguments
    if (optind != argc)
        // Error if any more arguments
        Usage(progname);

    string errmsg;
    vector<CKinfo> infos = CKpollForInfo(&errmsg, 2000);
    for (size_t i = 0; i < infos.size(); ++i) {
        DisplayDevice(infos[i]);
    }
    if (! errmsg.empty()) {
        cerr << "CKpollForInfo: " << errmsg << endl;
        exit(EXIT_FAILURE);
    }

    if (infos.size() == 0 )
        cout << "No CK devices detected" << endl;
    else {
        vector<CKdevice> devices = CKpollForDevices(infos, &errmsg);
        cout << "Devices: " << endl;
        for (size_t i = 0; i < devices.size(); ++i) {
            cout << " " << devices[i].GetDescription() << endl;
        }
        if (! errmsg.empty()) {
            cerr << "CKpollForDevices: " << errmsg << endl;
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}

// TODO
// Clean up socket code
//   Add SocketUDP type (that creates the right data gram socket type)
//   Support Read for client UDP
//   Implement HasData
// Using HasData and timeout, allow ckinfo to terminate
//
