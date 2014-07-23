#include "utils.h"
#include "Color.h"
#include "utilsTime.h"
#include "LFramework.h"
#include "Lobj.h"
#include <iostream>
#include <cmath>

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------

void ValidateNumArgs(csref command, int numArgs, int argc, int argp)
{
    if (argc == argp + numArgs)
        // Everything is correct
        return;
    L::ErrorExit(command + " expected " + IntToStr(numArgs) + " parameters but got " + IntToStr(argc - 2));
}

DefProgramHelp(kPHprogram, "Lflash");
DefProgramHelp(kPHusage, "Flashes all or some of the lights");
DefProgramHelp(kPHadditionalArgs, "command [colorargs...]");
DefProgramHelp(kPHhelp, "command is one of:\n"
               "    clear \n"
               "    all color\n"
               "    rotate color\n"
               "    rotwash color1 color2\n"
               "    bounce color\n"
               "    set idx color\n"
               "    wash color1 color2\n"
	       "    plane\n"
               "  color is \"r,g,b\" or \"HSV(h,s,v)\" or a named color, etc.  All components are scaled from 0.0 to 1.0"
              );

typedef enum {kStatic, kRotate, kBounce} Mode_t;
Mode_t gMode;
Color* gColor;      // Always set
Color* gColor2;     // Only set if we're doing a color wash

void LflashCallback(Lobj* obj)
{
    // Do the bounary stuff
    Lxy minBound(-.5,0);
    Lxy maxBound(L::gOutputBuffer->GetCount()-.5, 0);
    switch (gMode)
    {
    case kRotate:
        obj->Wrap(minBound, maxBound);
        break;
    case kBounce:
        obj->Bounce(minBound, maxBound);
        break;
    case kStatic:
    default:
        break;
    }
}

#include "utilsIP.h"

int main(int argc, char** argv)
{
    // Initialize and Parse arguments
    L::SetRateMode(L::kRateNonZero);
    Option::DeleteOption("color");
    L::Startup(&argc, argv, Option::kVariable);

    IPAddr ip(argv[1]);
    cout << "InARPCache " << ip.GetString() << " is " << ip.InARPCache() << endl;
    exit(0);


    if (argc <= 1) L::ErrorExit("Missing command");

    int argp = 1;

    string command = argv[argp++];
    string errmsg;
    int idx = -1;  // if -1 all colors should be set

    float speed = 0;

    if (command == "clear")
    {
        ValidateNumArgs(command, 0, argc, argp);
        gColor = new BLACK;
        if (L::gRunTime < 0) L::gRunTime = 0;  // If no run time specified, return immediately
    }
    else if (command == "set")
    {
        ValidateNumArgs(command, 2, argc, argp);
        idx   = atoi(argv[argp++]);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        if (L::gRunTime < 0) L::gRunTime = 0;  // If no run time specified, return immediately
    }
    else if (command == "all")
    {
        ValidateNumArgs(command, 1, argc, argp);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        if (L::gRunTime < 0) L::gRunTime = 0;  // If no run time specified, return immediately
    }
    else if (command == "rotate" || command == "bounce")
    {
        ValidateNumArgs(command, 1, argc, argp);
        idx = 0;
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        speed = 20 * L::gRate;
        gMode = (command == "bounce") ? kBounce : kRotate;
    }
    else if (command == "wash")
    {
        ValidateNumArgs(command, 2, argc, argp);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        gColor2 = gColor ? Color::AllocFromString(argv[argp++], &errmsg) : NULL;
        if (!gColor2) gColor = NULL;
        if (L::gRunTime < 0) L::gRunTime = 0;  // If no run time specified, return immediately
    }
    else if (command == "rotwash")
    {
        ValidateNumArgs(command, 2, argc, argp);
        gColor = Color::AllocFromString(argv[argp++], &errmsg);
        gColor2 = gColor ? Color::AllocFromString(argv[argp++], &errmsg) : NULL;
        if (!gColor2) gColor = NULL;
        speed = 40 * L::gRate;
        gMode = kRotate;
    }
    else if (command == "plane")
    {
        gColor  = new GREEN;
	gColor2 = new RED;
        ValidateNumArgs(command, 0, argc, argp);
        if (L::gRunTime < 0) L::gRunTime = 0;  // If no run time specified, return immediately
    }
    else
    {
        L::ErrorExit("Unknown command \"" + command + "\"");
    }

    // Validate the colors
    if (! gColor) L::ErrorExit(errmsg);

    // Print summary
    if (L::gVerbose)
    {
        cout << "Cmd: " << command << "   Color: " << gColor->ToString();
        if (gColor2)
            cout << " Color2: " << gColor2->ToString();
        if (idx != -1)
            cout << "  Index: " << idx;
        if (L::gRunTime >= 0)
            cout << "  Time: " << L::gRunTime << " seconds";
        if (gMode != kStatic && L::gRate != 1)
            cout << "  Rate: " << L::gRate;
        cout << endl;
    }

    // Allocate objects and set colors
    Lgroup objs;
    if (idx != -1)
    {
        // Just one light
        Lobj* obj = new Lobj();
        obj->pos.x = idx;
        obj->color = *gColor;
        obj->speed.x = speed;
        objs.Add(obj);
    }
    else if (command == "plane")
    {
        // Plane colors (Left side green, Right side red)
        int numLights = L::gOutputBuffer->GetCount();;
        for (int i = 0; i < numLights; ++i)
        {
            Lobj* obj = new Lobj();
            obj->pos.x = i;
	    obj->color = i < numLights/2 ? *gColor : *gColor2;
            objs.Add(obj);
	}
    }        
    else
    {
        // One light for each pixel
        HSVColorRange range;
        if (gColor2) range = HSVColorRange(*gColor, *gColor2);

        int numLights = L::gOutputBuffer->GetCount();;
        for (int i = 0; i < numLights; ++i)
        {
            Lobj* obj = new Lobj();
            obj->pos.x = i;
            if (gColor2)
                obj->color = range.GetColor((float) i / (float) numLights);
            else
                obj->color = *gColor;
            obj->speed.x = speed;
            objs.Add(obj);
        }
    }

    L::Run(objs, LflashCallback);
    L::Cleanup();

    exit(EXIT_SUCCESS);
}
