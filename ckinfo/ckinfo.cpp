// Display info about current CK devices on the LAN

#include "utils.h"
#include "utilsSocket.h"
#include "CKbuffer.h"
#include "utilsOptions.h"
#include "KiNET.h"
#include <iostream>

void DisplayDevice(const CKinfo& devInfo) {
    cout << "CK Device @ " << IPAddr(devInfo.ipaddr).GetString();
    cout << "  [MAC: " << devInfo.macaddr << "]";
    cout << "  Serial: " << hex << uppercase << devInfo.serial << nouppercase << dec << endl;
    cout << "Universe: " << devInfo.universe << "   KiNet Version: " << devInfo.kinetVersion << endl;
    cout << " Name: " << devInfo.name << endl;
    string info = devInfo.info;
    info = StrReplace(info, "\n", "\n ");
    cout << " " << info;
}

//-----------------------------------------------------------------------------------
// PortInfo (KiNet v2 only)
// Not very useful IMO
//-----------------------------------------------------------------------------------
// Used for the v2 portInfo query
struct CKportInfo {
    CKportInfo() {Clear();};
    CKportInfo(const char* portInfoBufferPtr, const char* portInfoBufferEnd) {SetFromPortInfoReply(portInfoBufferPtr, portInfoBufferEnd);}
    bool SetFromPortInfoReply(const char* portInfoBufferPtr, const char* portInfoBufferEnd, string* errmsg = NULL);
    void Clear() {id = 0; type = CK::kNull;}
    int         id;
    CK::PortType_t  type;
};

// Info about a port on a CK power supply
bool CKportInfo::SetFromPortInfoReply(const char* bufptr, const char* bufend, string* errmsg) {
    Clear();
    if (!bufptr || ! bufend || bufptr + sizeof(KiNETportInfoData) > bufend) {
        if (errmsg) *errmsg = "CK portInfo reply was too short";
        return false;
    }

    KiNETportInfoData* reply = (KiNETportInfoData*) bufptr;
    id = reply->portNum;
    type = (CK::PortType_t) reply->portType;
    return true;
}

vector<CKportInfo> QueryPortInfo(const CKinfo& info, string* errmsg = NULL, int timeoutInMS = CK::kDefaultPollTimeout) {
    vector<CKportInfo> retval;

    // Create the client socket for the polling request
    SocketUDPClient sock(info.ipaddr, KiNETudpPort);
    KiNETportInfo portInfoPacket;
    sock.Write((char*) &portInfoPacket, portInfoPacket.GetSize());
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error writing CK KiNETportInfo packet: " + sock.GetLastError();
        sock.Close();
        return retval;
        }

    // Wait for a response
    const int buflen = 1000;
    char buffer[buflen];
    char* bufptr;
    int bytesRead = 0;

    while (sock.HasData(timeoutInMS)) {
        if (sock.Read(buffer, buflen, &bytesRead)) {
            if (bytesRead < KiNETportInfoReply::GetSize()) {
                if (errmsg) *errmsg = "KiNET portInfo reply was too short: " + IntToStr(bytesRead) + " bytes";
                sock.Close();
                return retval;
            }
            char* bufend = buffer + bytesRead;
            KiNETportInfoReply* reply = (KiNETportInfoReply*) buffer;
            int portsRemaining = reply->numPorts;
            bufptr = buffer + KiNETportInfoReply::GetSize();

            while (portsRemaining-- > 0) {
                CKportInfo portInfo;
                if (! portInfo.SetFromPortInfoReply(bufptr, bufend, errmsg))
                    return retval;
                retval.push_back(portInfo);
                bufptr += sizeof(KiNETportInfoData);
            }
        } else {
            if (errmsg) *errmsg = "Error reading CK KiNETportInfo response: " + sock.GetLastError();
            sock.Close();
            return retval;
        }
    }
    if (sock.HasError()) {
        if (errmsg) *errmsg = "Error waiting for poll response: " + sock.GetLastError();
        sock.Close();
        return retval;
    }
    sock.Close();
    return retval;
}

void DisplayPortInfo(const CKinfo& devInfo) {
    string errmsg;
    vector<CKportInfo> portInfo = QueryPortInfo(devInfo, &errmsg);
    if (! errmsg.empty()) {
        cerr << "QueryPortInfo: " << errmsg << endl;
        return;
    }
    cout << "Ports: ";
    if (portInfo.size() == 0) {
        if (devInfo.kinetVersion == 1)
            cout << "not available for KiNet V1" << endl;
        else
            cout << "None" << endl;
        return;
    }
    for (size_t j = 0; j < portInfo.size(); ++j) {
        if (j != 0) cout << ", ";
        cout << portInfo[j].id << "(" << CK::PortTypeToString(portInfo[j].type) << ")";
    }
    cout << endl;
}

//-----------------------------------------------------------------------------------
// Main function
//-----------------------------------------------------------------------------------

DefProgramHelp(kPHprogram, "ckinfo");
DefProgramHelp(kPHusage, "Displays info about the CK devices on the network.");
bool gVerbose = true;

int main(int argc, char** argv)
{
    // Delete unneeded options
    Option::DeleteOption("rate");
    Option::DeleteOption("color");
    Option::ParseArglist(&argc, argv);

    string errmsg;
    vector<CKinfo> infos = CKdiscoverInfo(&errmsg, 2000);
    for (size_t i = 0; i < infos.size(); ++i) {
        DisplayDevice(infos[i]);
        if (gVerbose)
            DisplayPortInfo(infos[i]);
        cout << endl;
    }

    if (! errmsg.empty()) {
        cerr << "CKdiscoverInfo: " << errmsg << endl;
        return EXIT_FAILURE;
    }

    if (infos.size() == 0 )
        cout << "No CK devices detected" << endl;
    else {
        vector<CKdevice> devices = CKdiscoverDevices(infos, &errmsg);
        cout << "Devices: " << endl;
        for (size_t i = 0; i < devices.size(); ++i) {
            cout << " " << devices[i].GetDescriptor() << endl;
        }
        if (! errmsg.empty()) {
            cerr << "CKdiscoverDevices: " << errmsg << endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
