#include "utils.h"
#include "Color.h"
#include "cklib.h"
#include "utilsTime.h"
#include "LFramework.h"
#include <iostream>
#include <cmath>

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
    "    bounce color\n"
    "    set idx color\n"
    "    wash color1 color2\n"
    "  color is \"r,g,b\" or \"HSV(h,s,v)\" or a named color, etc.  All components are scaled from 0.0 to 1.0\n"
    );

typedef enum {kStatic, kRotate, kBounce} Mode_t;
Mode_t gMode;
Color* gColor;      // Always set
Color* gColor2;     // Only set if we're doing a color wash

int main(int argc, char** argv)
{
    // Initialize and Parse arguments
    L::SetRateMode(L::kRateNonZero);
    L::Startup(&argc, argv, Option::kVariable);

    if (argc <= 1) Usage("Missing command");

    int argp = 1;

    string command = argv[argp++];
    string errmsg;
    int idx = -1;  // if -1 all colors should be set

    float defaultRotateDelay = 0;

    if (command == "clear")
    {
        ValidateNumArgs(command, 0, argc, argp);
        gColor = new BLACK;
    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 2, argc, argp);
        idx   = atoi(argv[argp++]);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
    }
    else if (command == "all")
    {
        ValidateNumArgs(command, 1, argc, argp);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
    }
    else if (command == "rotate" || command == "bounce")
    {
        ValidateNumArgs(command, 1, argc, argp);
        idx = 0;
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        defaultRotateDelay = 50;
        gMode = (command == "bounce") ? kBounce : kRotate;
    }
    else if (command == "wash")
    {
        ValidateNumArgs(command, 2, argc, argp);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        gColor2 = gColor ? Color::AllocFromString(argv[argp++], &errmsg) : NULL;
        if (!gColor2) gColor = NULL;
    }
    else if (command == "rotwash")
    {
        ValidateNumArgs(command, 2, argc, argp);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        gColor2 = gColor ? Color::AllocFromString(argv[argp++], &errmsg) : NULL;
        if (!gColor2) gColor = NULL;
        gMode = kRotate;
        defaultRotateDelay = 25;
    }
    else
    {
        Usage("Unknown command \"" + command + "\"");
    }

    // Validate the colors
    if (! gColor) Usage(errmsg);

    // Print summary
    if (L::gVerbose) {
        cout << "Cmd: " << command << "   Color: " << gColor->ToString();
        if (doWash)
            cout << " Color2: " << gColor2->ToString();
        if (idx != -1)
            cout << "  Index: " << idx;
        if (L::gRunTime >= 0)
            cout << "  Time: " << L::gRunTime << " seconds";
        if (defaultRotateDelay != 0 && L::gRate != 1)
            cout << "  Rate: " << L::gRate;
        cout << endl;
    }

    // Set and update the lights
    if (gColor2) {
        HSVColorRange range(*gColor, *gColor2);
        int numLights = L::gOutputBuffer->GetCount();;
        float stepSize = 1.0f / numLights;
        for (int i = 0; i < numLights; ++i) {
            HSVColor col = range.GetColor(i * stepSize);
            L::gOutputBuffer->SetColor(i, col);
        }
    } else
    if (idx == -1)
        L::gOutputBuffer->SetAll(*gColor);
    else
        L::gOutputBuffer->SetColor(idx, *gColor);
    L::gOutputBuffer->Update();

     // Handle rotation and/or timedelay
    if (defaultRotateDelay != 0) {
        Milli_t duration = L::gRunTime * 1000;
        Milli_t startTime = Milliseconds();
        Milli_t sleepBetween = defaultRotateDelay / fabs(L::gRate);
        int width = L::gOutputBuffer->GetCount();
        int count = 0;
        int direction = L::gRate < 0 ? -1 : 1;
        while (true) {
            if (bounce && count != 0 && width > 1 && (count % (width-1)) == 0) direction = -direction;
            ++count;
            L::gOutputBuffer->Rotate(direction);
            L::gOutputBuffer->Update();
            if (L::gOutputBuffer->HasError()) {
                cerr << "Update error: " << L::gOutputBuffer->GetLastError() << endl;
                exit(EXIT_FAILURE);
            }

            SleepMilli(sleepBetween);
            if (duration >= 0 && MilliDiff(Milliseconds(),  startTime) >= (Milli_t) L::gRunTime * 1000) break;

        }
    } else if (L::gRunTime > 0)
        // If it's not rotate mode and run time wasn't set, end immediately
        SleepMilli(L::gRunTime * 1000);

    exit(EXIT_SUCCESS);
}
