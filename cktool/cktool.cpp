#include "utils.h"
#include "Color.h"
#include "cklib.h"
#include "utilsTime.h"
#include "LFramework.h"
#include <iostream>

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------

void Usage(csref msg = "")    {
    if (!msg.empty())
        cerr << ProgramHelp::GetString(kPHprogram) << ": " << msg << endl;
    cerr << ProgramHelp::GetUsage();
    exit(EXIT_FAILURE);
}

void ValidateNumArgs(csref command, int numArgs, int argc, int argp)
    {
        if (argc == argp + numArgs)
            // Everything is correct
            return;
        Usage(command + " expected " + IntToStr(numArgs) + " parameters but got " + IntToStr(argc - 2));
    }

DefProgramHelp(kPHprogram, "cktool");
DefProgramHelp(kPHusage, "Performs various lighting commands: clear, all, set, rotate, rotwash");
DefProgramHelp(kPHadditionalArgs, "command [colorargs...]");
DefProgramHelp(kPHhelp, "command is one of:\n"
    "    clear \n"
    "    all color\n"
    "    rotate color\n"
    "    rotwash color1 color2\n"
    "    set idx color\n"
    "    wash color1 color2\n"
    "  color is \"r,g,b\" or \"HSV(h,s,v)\" or a named color, etc.  All components are scaled from 0.0 to 1.0\n"
    );

int main(int argc, char** argv)
{
    // Parse arguments
    Option::ParseArglist(&argc, argv, Option::kVariable);

    if (argc <= 1) Usage("Missing command");

    int argp = 1;

    string command = argv[argp++];
    string errmsg;
    int idx = -1;  // if -1 all colors should be set
    Color* color;
    Color* color2;
    bool doWash   = false;
    float defaultRotateDelay = 0;

    if (command == "clear")
    {
        ValidateNumArgs(command, 0, argc, argp);
        color = new BLACK;
    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 2, argc, argp);
        idx   = atoi(argv[argp++]);
        color = Color::AllocFromString(argv[argp++], &errmsg);
    }
    else if (command == "all")
    {
        ValidateNumArgs(command, 1, argc, argp);
        color = Color::AllocFromString(argv[argp++], &errmsg);
    }
    else if (command == "rotate")
    {
        ValidateNumArgs(command, 1, argc, argp);
        idx = 0;
        color = Color::AllocFromString(argv[argp++], &errmsg);
        defaultRotateDelay = 50;
    }
    else if (command == "wash")
    {
        ValidateNumArgs(command, 2, argc, argp);
        color = Color::AllocFromString(argv[argp++], &errmsg);
        color2 = color ? Color::AllocFromString(argv[argp++], &errmsg) : NULL;
        doWash = true;
    }
    else if (command == "rotwash")
    {
        ValidateNumArgs(command, 2, argc, argp);
        color = Color::AllocFromString(argv[argp++], &errmsg);
        color2 = color ? Color::AllocFromString(argv[argp++], &errmsg) : NULL;
        doWash = true;
        defaultRotateDelay = 25;
    }
    else
    {
        Usage("Unknown command \"" + command + "\"");
    }

    // Validate the colors
    if (! color) Usage(errmsg);
    if (doWash && !color2) Usage(errmsg);

    // Print summary
    if (L::gVerbose) {
        cout << "Cmd: " << command << "   Color: " << color->ToString();
        if (doWash)
            cout << " Color2: " << color2->ToString();
        if (idx != -1)
            cout << "  Index: " << idx;
        if (L::gRunTime != 0)
            cout << "  Time: " << L::gRunTime << " seconds";
        if (defaultRotateDelay != 0 && L::gRate != 1)
            cout << "  Rate: " << L::gRate;
        cout << endl;
    }

    // Set and update the lights
    if (doWash) {
        HSVColorRange range(*color, *color2);
        int numLights = L::gOutputBuffer->GetCount();;
        float stepSize = 1.0f / numLights;
        for (int i = 0; i < numLights; ++i) {
            HSVColor col = range.GetColor(i * stepSize);
            L::gOutputBuffer->SetColor(i, col);
        }
    } else
    if (idx == -1)
        L::gOutputBuffer->SetAll(*color);
    else
        L::gOutputBuffer->SetColor(idx, *color);
    L::gOutputBuffer->Update();

     // Handle rotation and/or timedelay
    Milli_t duration = L::gRunTime * 1000;
    if (defaultRotateDelay != 0) {
        Milli_t startTime = Milliseconds();
        Milli_t sleepBetween = defaultRotateDelay / L::gRate;
        while (true) {
            L::gOutputBuffer->Rotate();
            L::gOutputBuffer->Update();
            if (L::gOutputBuffer->HasError()) {
                cerr << "Update error: " << L::gOutputBuffer->GetLastError() << endl;
                exit(EXIT_FAILURE);
            }

            SleepMilli(sleepBetween);
            if (duration > 0 && MilliDiff(Milliseconds(),  startTime) > (Milli_t) L::gRunTime * 1000) break;

        }
    } else
        SleepMilli(duration);
    exit(EXIT_SUCCESS);
}
