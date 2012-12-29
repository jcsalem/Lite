// Display info about current CK devices on the LAN

#include "utils.h"
#include "utilsSocket.h"
#include "cklib.h"
#include "utilsOptions.h"
#include <iostream>

void DisplayDevice(const CKinfo& devInfo) {
    cout << "CK Device @ " << IPAddr(devInfo.ipaddr).GetString() << "  [MAC: " << devInfo.macaddr << "]"  << "   Serial: " << devInfo.serial << endl;
    cout << "Universe: " << devInfo.universe << " NumPorts: " << devInfo.numports<< endl;
    cout << " Name: " << devInfo.name << endl;
    string info = devInfo.info;
    info = StrReplace(info, "\n", "\n ");
    cout << " " << info << endl;
}

DefProgramHelp(kPHprogram, "ckinfo");
DefProgramHelp(kPHusage, "Displays info about the CK devices on the network.");

int main(int argc, char** argv)
{
    // Delete unneeded options
    Option::DeleteOption("rate");
    Option::DeleteOption("color");
    Option::ParseArglist(&argc, argv);

    string errmsg;
    vector<CKinfo> infos = CKpollForInfo(&errmsg, 2000);
    for (size_t i = 0; i < infos.size(); ++i) {
        DisplayDevice(infos[i]);
    }
    if (! errmsg.empty()) {
        cerr << "CKpollForInfo: " << errmsg << endl;
        return EXIT_FAILURE;
    }

    if (infos.size() == 0 )
        cout << "No CK devices detected" << endl;
    else {
        vector<CKdevice> devices = CKpollForDevices(infos, &errmsg);
        cout << "Devices: " << endl;
        for (size_t i = 0; i < devices.size(); ++i) {
            cout << " " << devices[i].GetDescriptor() << endl;
        }
        if (! errmsg.empty()) {
            cerr << "CKpollForDevices: " << errmsg << endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
