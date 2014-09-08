// Top level Starry Night code

#include "utils.h"
#include "utilsTime.h"
#include "Color.h"
#include "Lobj.h"
#include "LFramework.h"
#include <iostream>
#include <fstream>

DefProgramHelp(kPHprogram, "Lpov");
DefProgramHelp(kPHusage, "Displays an image via \"persistance of vision\"");
DefProgramHelp(kPHadditionalArgs, "rgbfilename width");
DefProgramHelp(kPHhelp, "file must be in a raw RGB format.");

float gPovDefaultSliceDurationMS = 20;  // duration of each output line (milliseconds)
float gPovDefaultOnFraction = .5; // Fraction of time the pixel is on during each cycle.

//----------------------------------------------------------------
// Option definitions
//----------------------------------------------------------------
// OnFraction: Fraction of the time per frame that the pixels are on 
float gPovOnFraction      = gPovDefaultOnFraction;

string DensityCallback(csref name, csref val) {
     if (! StrToFlt(val, &gPovOnFraction))
        return "--density wasn't a number: " + val;
    if (gPovOnFraction <= 0 || gPovOnFraction > 1)
        return "--density argument must be between 0 and 1. Was " + val;
    return "";
}

string DensityDefaultCallback(csref name) {
    return FltToStr(gPovDefaultOnFraction);
}

DefOption(density, DensityCallback, "density", "The fraction of time the lights are on from 0 to 1.", DensityDefaultCallback);

// Show image
bool gShowImage = false;
string ShowImageCallback(csref name, csref val) {
  gShowImage = true;
  return "";
}
DefOptionBool(showimage, ShowImageCallback, "If set, print an ASCII version of the image.");

//----------------------------------------------------------------
// Image Code
//----------------------------------------------------------------

class Limage {
  public:
    Limage(int w, int h) {Init(); SetSize(w,h);}
    Limage() {Init();}
    virtual ~Limage() {FreeStorage();}
    // Accessors and initialization
    int GetWidth() const {return iWidth;}
    int GetHeight() const {return iHeight;}
    void SetSize(int w, int h) {if (iWidth == w && iHeight == h) return; FreeStorage(); iWidth = w; iHeight = h;}
    void Allocate(int w, int h) {SetSize(w,h); AllocateIfNeeded();}
    
    RGBColor GetPixel(int x, int y) const;
    void SetPixel(int x, int y, const RGBColor& color);
    //void CopyToBuffer(unsigned char*[3] rgbarray);
    bool ReadFromFileRGB(csref filename, int width, string* errmsg = NULL);
    void Clear();
    string ToString() const;

  private:
    int iWidth;
    int iHeight;
    RGBColor *iBuffer;
      
    void AllocateIfNeeded() {if (! iBuffer) {iBuffer = new RGBColor[iWidth * iHeight]; Clear();}}
    void FreeStorage() {if (iBuffer) delete[] iBuffer;}
    void Init() {iWidth = iHeight = 0; iBuffer = NULL;}

    // Disable copying (for now)
    Limage(const Limage&); 
    Limage& operator=(const Limage&);
};


bool Limage::ReadFromFileRGB(csref filename, int width, string* errmsg)
{
  ifstream in(filename.c_str(), ios::binary);
  if (in.fail()) {
    if (errmsg) *errmsg = "Error opening \"" + filename + "\": " + ErrorCodeString();
    return false;
  }
  in.seekg(0, ios::end);
  long len = in.tellg();
  in.seekg(0, ios::beg);

  if ( len % (width * 3) != 0)
    {
      if (errmsg) *errmsg = "Error reading \"" + filename + "\": Filesize was not a multiple of the width * 3. Filesize is " + IntToStr(len) + ", width * 3 is " + IntToStr(width *3);
      in.close();
      return false;
    }  
  int height = len / (width * 3);
  SetSize(width, height);
  AllocateIfNeeded();
  int numPixels = width * height;
  unsigned char (*tempbuf)[3] = new unsigned char[numPixels][3];
  in.read((char*) tempbuf, numPixels * 3);
  if (in.bad())
      {
        if (errmsg) *errmsg = "Error reading \"" + filename + "\": " + ErrorCodeString();
        delete[] tempbuf;
        in.close();
        return false;
      }
  in.close();
  for (int i = 0; i < numPixels; ++i)
    iBuffer[i] = RGBColor(tempbuf[i][0] / 255.0, tempbuf[i][1] / 255.0, tempbuf[i][2] / 255.0);

  delete[] tempbuf;
  return true;
}

string Limage::ToString() const
  {
    string ret;
    if (iWidth == 0 || iHeight == 0) return ret;
    int numPixels = iHeight * iWidth;
    int len = iHeight * (iWidth + 1);
    ret.reserve(len);
    for (int y = 0; y < iHeight; ++y) {
      for (int x = 0; x < iWidth; ++x) {
        RGBColor rgb(GetPixel(x, y));
        float colorTotal = rgb.r + rgb.g + rgb.b;
        if (colorTotal == 0) ret += " ";
        else if (colorTotal < .25) ret += ".";
        else ret += "*";
      }
      ret += "\n";
    }
    return ret;
  }

RGBColor Limage::GetPixel(int x, int y) const
{
  if (!iBuffer || x < 0 || x >= iWidth || y < 0 || y >= iHeight) return BLACK;
  return iBuffer[x + y*iWidth];
}
void Limage::SetPixel(int x, int y, const RGBColor& color)
{
  if (!iBuffer || x < 0 || x >= iWidth || y < 0 || y >= iHeight) return;
  iBuffer[x+y*iWidth] = color;
}

void Limage::Clear()
  {
    if (! iBuffer) return;
    for (int i = 0; i < iWidth * iHeight; ++i)
      iBuffer[i] = BLACK;
  }

//----------------------------------------------------------------
// Initializing and running the lights
//----------------------------------------------------------------

class LpovGroup : public Lgroup
{
  // Global configuration
  public:
    LpovGroup() : Lgroup(), Image(), FramesPerCycle(10), NumOnFrames(1), FrameCount(0), ImageRow(0) {}
    Limage Image;
    int FramesPerCycle;
    int NumOnFrames;
    int FrameCount;
    int ImageRow;
    static void Callback(Lgroup* group);
};

void LpovGroup::Callback(Lgroup* group)
{
  LpovGroup* povGroup = dynamic_cast<LpovGroup*>(group); 

  if (povGroup->FrameCount++ >= povGroup->NumOnFrames)
    {
      if (povGroup->FrameCount >= povGroup->FramesPerCycle)
      {
        povGroup->FrameCount = 0;
      }
      return;
    }
    int w = L::gOutput.GetCount();
    if (w > povGroup->Image.GetWidth()) w = povGroup->Image.GetWidth();
    for (int x = 0; x < w; ++x)
        L::gOutput.SetColor(x, povGroup->Image.GetPixel(x,povGroup->ImageRow));
    // Setup for next frame
    if (povGroup->FrameCount == povGroup->NumOnFrames) 
      povGroup->ImageRow = (povGroup->ImageRow + 1) % povGroup->Image.GetHeight();
  }

//----------------------------------------------------------------
// Main functions
//----------------------------------------------------------------

int main(int argc, char** argv)
{
    Option::DeleteOption("color");
    L::gRate = gPovDefaultSliceDurationMS;
    L::SetRateDoc("How often to change the slice of the image display. In ms");

    // Parse arguments
    L::Startup(&argc, argv, 2, 2);
    string filename = argv[1];
    string widthStr = argv[2];
    int width;
    if (! StrToInt(widthStr, &width) && width > 0)
        L::ErrorExit("width must be a positive number");

    // Compute number of frames to flash
    if (L::gRate < L::gFrameDuration) 
      L::gFrameDuration = L::gRate + .5;
    if (L::gFrameDuration == 0) L::gFrameDuration = 1;
    int totalFrames = L::gRate / L::gFrameDuration + .5;
    //if (totalFrames < 2) totalFrames = 2;
    int onFrames = totalFrames * gPovOnFraction + .5;
    if (onFrames < 1) onFrames = 1;
    else if (onFrames >= totalFrames) onFrames = totalFrames;

    // Initialize the group
    LpovGroup group;
    group.FramesPerCycle = totalFrames;
    group.NumOnFrames = onFrames;

    // Read file
    string errmsg;
    if (! group.Image.ReadFromFileRGB(filename, width, &errmsg))
        L::ErrorExit("While reading image, " + errmsg);
    if (gShowImage)
        cout << group.Image.ToString() << endl;

    if (L::gVerbose)
        {
        cout << "Image Slice Duration (--rate) = " << L::gRate << "   L:gFrameDuration = " << L::gFrameDuration << endl;
        cout << "Frames per Slice= " << group.FramesPerCycle 
             << " of which pixels are lit for " << group.NumOnFrames << " frames." << endl;
        cout << "Image: " << filename <<  " Size: " << group.Image.GetWidth() << "x" << group.Image.GetHeight() << endl;
      }

    // Perform
    L::Run(group, (L::ObjCallback_t) NULL, LpovGroup::Callback);
    L::Cleanup();
    exit(EXIT_SUCCESS);
}
