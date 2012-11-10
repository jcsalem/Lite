// Display info about current CK devices on the LAN

#include "../cklib/utils.h"
#include "../cklib/utilsSocket.h"
#include "../cklib/KiNET.h"
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

const char* bufferToString (string* outString, const char* startptr, const char* endptr)
// Reads a null-terminated string from the buffer.  If the buffer ends before the null, then the rest of the string is returned.
// endptr points to the last character in the string + 1
// Returns a point to the next character (or to endptr if that's reached)
{
    for (const char* i = startptr; i != endptr; ++i) {
        if (*i == 0) {
            *outString = string(startptr);
            return (i+1);
        }
    }
    *outString = string(startptr, endptr - startptr);
    return endptr;
}

bool DisplayDevice(const char* buffer, int len) {
    if (len < KiNETpollReply::GetSize() + 2) {
        cerr << "Response to CK poll was too short (" << len << " bytes)" << endl;
        return false;
    }
    KiNETpollReply* pollReply = (KiNETpollReply*) buffer;
    const char* strptr = buffer + KiNETpollReply::GetSize();
    const char* strend = buffer + len;

    IPAddr ip(pollReply->ip);
    string macAddr;
    for (int i = 0; i < 6; ++i) {
        if (i != 0) macAddr += ":";
        macAddr += IntToHex(pollReply->mac[i]);
    }
    int numPorts    = pollReply->numPorts; // This is a little uncertain
    int serial      = pollReply->serial;
    int universe    = pollReply->universe;

    string devInfo, name;
    strptr = bufferToString(&devInfo, strptr, strend);
    bufferToString(&name, strptr, strend);
    devInfo = strReplace(devInfo, "\n", "\n ");

    // Now display it
    cout << "CK Device @ " << ip.GetString() << "  [MAC: " << macAddr << "]" << endl;
    cout << "Universe: " << universe << " NumPorts: " << numPorts << "   Serial: " << serial << endl;
    cout << " Name: " << name << endl;
    cout << " " << devInfo << endl;
    return true;
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

    // Create the client socket for the polling request
    SockAddr outSA(INADDR_BROADCAST, KiNETudpPort); //use broadcast address
    SocketUDPClient outsock(outSA);
    outsock.setsockopt_bool(SOL_SOCKET, SO_BROADCAST, true);

    // Broadcast the poll request
    KiNETpoll pollPacket;
    outsock.Write((char*) &pollPacket, pollPacket.GetSize());
    if (outsock.HasError()) {
        cerr << "Error writing polling packet: " << outsock.GetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    // Wait for a response
    const int buflen = 1000;
    char buffer[buflen];
    int bytesRead = 0;

    while (outsock.Read(buffer, buflen, &bytesRead)) {
        DisplayDevice(buffer, bytesRead);
    }
    {
        cerr << "Error reading poll response: " << outsock.GetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    outsock.Close();

    exit(EXIT_SUCCESS);
}

// TODO
// Clean up socket code
//   Add SocketUDP type (that creates the right data gram socket type)
//   Support Read for client UDP
//   Implement HasData
// Using HasData and timeout, allow ckinfo to terminate
//
