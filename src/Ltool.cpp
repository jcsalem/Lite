#include "utils.h"
#include "Color.h"
#include "utilsTime.h"
#include "LFramework.h"
#include "MapFilters.h"
#include "EffectFilters.h"
#include "Lobj.h"
#include "utilsParse.h"
#include <iostream>
#include <cmath>

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------

void ValidateNumArgs(csref command, cvsref params, int minArgs, int maxArgs, bool disallowJustOne = false)
{
  string errmsg;
  if (! ParamListCheck(params, command, &errmsg, minArgs, maxArgs))
    L::ErrorExit(errmsg);
  if (disallowJustOne && params.size() == 1)
    L::ErrorExit(command + " can't have just one argument");
}

DefProgramHelp(kPHprogram, "Ltool");
DefProgramHelp(kPHusage, "Performs various lighting commands: clear, all, set, rotate, bounce, wash, rotwash, bouncewash, plane");
DefProgramHelp(kPHadditionalArgs, "command [colorargs...]");
DefProgramHelp(kPHhelp, "command is one of:\n"
               "    clear \n"
               "    all color\n"
               "    set idx color\n"
               "    rotate color\n"
               "    bounce color\n"
               "    wash color1 color2\n"
               "    rotwash color1 color2\n"
               "    bouncewash color1 color2\n"
  	           "    plane\n"
               "  color is \"r,g,b\" or \"HSV(h,s,v)\" or a named color, etc.  All components are scaled from 0.0 to 1.0\n"
	             "  color arguments are optional and default to white for single color commands and red-to-red for wash commands"
              );

//----------------------------------------------
// Setting up the light pattern
//----------------------------------------------
Color* gColor  = new WHITE;  // Always set
Color* gColor2 = NULL;     // Only set if we're doing a color wash
const int kOneLight  = 0;
const int kAll       = -1;
const int kWash      = -2;
const int kLastLight = -3;
const int kPlane     = -4;
int    gIndex  = kAll;   // Index of the light to set. -1 if all lights are set.

void LtoolCallback(Lgroup* group) {
  int count = L::gOutput.GetCount();
  switch (gIndex) {
    case kAll:
      for (int i = 0; i < L::gOutput.GetCount(); ++i)
        L::gOutput.SetRGB(i, *gColor);
      break;
    case kWash: {
      HSVColorRange range(*gColor, *gColor2);
      WriteColorRangeToSequence(range, L::gOutput.begin(), L::gOutput.end());
      break; 
    }
    case kLastLight: 
      L::gOutput.SetRGB(count-1, *gColor);
      break;
    case kPlane:
      for (int i = 0; i < count; ++i)
        L::gOutput.SetRGB(i, (i < count / 2) ? *gColor : *gColor2);
      break;
    default:
      // Just one pixel
      L::gOutput.SetRGB(gIndex, *gColor);
  }
}

void ToolSetup(csref command, cvsref params) {
  string errmsg;

  // *** Check the number of parameters and set up the colors ***
  if (command == "clear") {
    ValidateNumArgs(command, params, 0, 0);
    gColor = new BLACK;
    gIndex = kAll;
  } 
  else if (command == "set") {
    ValidateNumArgs(command, params, 1, 2);
    if (! ParseRequiredParam(&gIndex, params, 0, "index", &errmsg, 0)) L::ErrorExit(errmsg);
    if (! ParseOptionalParam(&gColor, params, 1, "color", &errmsg)) L::ErrorExit(errmsg);
  }
  else if (command == "all" || command == "rotate" || command == "bounce") {
    ValidateNumArgs(command, params, 0, 1);
    if (! ParseOptionalParam(&gColor, params, 0, "color", &errmsg)) L::ErrorExit(errmsg);
    // If rotating bouncing backwards, start at end
    gIndex = (command == "all") ? kAll : (L::gRate < 0 ? kLastLight : kOneLight);
  }
  else if (command == "wash" || command == "rotwash" || command == "bouncewash") {
    int maxArgs = (command == "bouncewash") ? 4 : 2;
    ValidateNumArgs(command, params, 0, maxArgs, true);
    gColor = new RED; 
    if (! ParseOptionalParam(&gColor, params, 0, "color1", &errmsg)) L::ErrorExit(errmsg);
    gColor2 = gColor;
    if (! ParseOptionalParam(&gColor2, params, 1, "color1", &errmsg)) L::ErrorExit(errmsg);
    gIndex = kWash;
  } 
  else if (command == "plane") {
    ValidateNumArgs(command, params, 0, 0);
    gColor  = new RED;
    gColor2 = new GREEN;
    gIndex = kPlane;
  }
  else 
    L::ErrorExit("Unknown command \"" + command + "\"");

  // *** Now set up any needed movement
  LFilter* movementFilter = NULL;

  if (command == "rotate" || command == "rotwash")
    movementFilter = new RotateFilter(L::gRate * .5);
  else if (command == "bounce")
    movementFilter = new BounceFilter(L::gRate * .5);
  else if (command == "bouncewash") {
    float bounceWidth = 2.0;
    float bounceIncr = 0.0;
    if (! ParseOptionalParam(&bounceWidth, params, 2, "bounce width", &errmsg, 0)) L::ErrorExit(errmsg);
    if (! ParseOptionalParam(&bounceIncr, params, 3, "bounce increment", &errmsg)) L::ErrorExit(errmsg);
    movementFilter = new BounceFilter(L::gRate * .5, bounceWidth, bounceIncr);
  }

  if (movementFilter) 
    L::PrependFilter(movementFilter);
  else if (L::gRunTime < 0) 
    // If no movement and no runtime specified, return immediately
    L::gRunTime = 0; 
}

void OutputSummary(csref command) {
  cout << "Cmd: " << command << "   Color: " << gColor->ToString();
  if (gColor2)
      cout << " Color2: " << gColor2->ToString();
  cout << "  Index: " << gIndex;
  cout << "  Rate: " << L::gRate;
  if (L::gRunTime >= 0)
      cout << "  Time: " << L::gRunTime << " seconds";
  cout << endl;
}


int main(int argc, char** argv)
{
    // Initialize and Parse arguments
    L::SetRateMode(L::kRateNonZero);
    Option::DeleteOption("color");
    L::Startup(&argc, argv, Option::kVariable);

    // Get the Command
    if (argc <= 1) L::ErrorExit("Missing command");
    string command = StrToLower(argv[1]);
    // Get the parameters
    vector<string> params = ParamListFromArgv(argc-2, argv+2);

    // Initialize
    ToolSetup(command, params);
    if (L::gVerbose) OutputSummary(command);

    // Allocate objects and set colors    
    Lgroup group;
    L::Run(group, NULL, LtoolCallback);
    L::Cleanup();

    exit(EXIT_SUCCESS);
}
