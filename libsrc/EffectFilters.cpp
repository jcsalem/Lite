// Effect Filters code

#include "utils.h"
#include "LFramework.h"
#include "EffectFilters.h"
#include "utilsParse.h"
#include "utilsRandom.h"

void ForceLinkEffectFilters() {} // This is referenced in LFilters.cpp to force the link of this file into all binaries

//-----------------------------------------------------------------------------
// PlaneNavigationFilter
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

const int PlaneNavigationFilter::gDefaultWidth = 9;  // Default width of the navigation filter

int PlaneNavigationFilter::GetCount() const {
  int len = LFilter::GetCount();
  return max(len - 2 * iNumPixels, 0);
}

string PlaneNavigationFilter::GetDescriptor() const
{
  return "plane(" + IntToStr(iNumPixels) + ")";
}

bool PlaneNavigationFilter::Update()
 {
   // Force beginning to be red and end to be green
   RGBColor green(GREEN);
   RGBColor red(RED);
   int count = iBuffer->GetCount();
   int stoppos = min(iNumPixels, count);
   for (int i = 0; i < stoppos; ++i)
     iBuffer->SetRGB(i, red);
   stoppos = max(count-iNumPixels,0);
   for (int i = count-1; i >= stoppos;--i)
     iBuffer->SetRGB(i, green);
   return iBuffer->Update();
 }

RGBColor& PlaneNavigationFilter::GetRawRGB(int idx)
{
  int count = iBuffer->GetCount();
  idx += iNumPixels;
  if (idx >= count) return LFilter::GetRawRGB(max(count-1,0)); // Only happens if iNumPixels is too big
  else return LFilter::GetRawRGB(idx);
}

LFilter* PlaneNavigationFilterCreate(cvsref params, string* errmsg)
{
  if (! ParamListCheck(params, "plane", errmsg, 0, 1)) return NULL;

  int numPixels = PlaneNavigationFilter::gDefaultWidth;
  if (! ParseOptionalParam(&numPixels, params, 0, "plane pixel width", errmsg, 0)) return NULL;
  return new PlaneNavigationFilter(numPixels);
}

DEFINE_LBUFFER_FILTER_TYPE(plane, PlaneNavigationFilterCreate, "plane(N)",
        "Adds green and red navigation lights for the last N pixels of either side (default is " + IntToStr(PlaneNavigationFilter::gDefaultWidth) + ")");

//-----------------------------------------------------------------------------
// SparkleFilter
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

// The fraction of pixels that are typically on
const float kDefaultSparkleFraction = 0.05;
// This is std dev of a Sparkle's on/off time (this is normalized to a duration of 1)
const float kDefaultSparkleSigma = 0.3; 
// The duration that a sparkle is on in seconds
const float kDefaultSparkleDuration = 0.02; // Sparkles generally turn on for just a single frame


SparkleFilter::~SparkleFilter() {
  if (iState) delete[] iState; iState = NULL; 
  if (iTimeToChange) delete[] iTimeToChange; iTimeToChange = NULL;
}

string SparkleFilter::GetDescriptor() const
{
  return "sparkle(" + iColor.ToString() + "," + FltToStr(iOnFraction) + "," + FltToStr(iOnDuration) + "," + FltToStr(iSigma) + ")";
}

void SparkleFilter::SetParameters(float onFraction, float onDuration, float sigma)
{
  iOnDuration = onDuration;
  if (onFraction < .001) onFraction = .001;
  iOnFraction = onFraction;
  iOffDuration = iOnDuration / iOnFraction - onDuration;
  iSigma = sigma;
  iPixelsNeedInit = true;
}

Milli_t SparkleFilter::GetSparkleDuration(bool newState)
{
  float avDur = newState ? iOnDuration : iOffDuration;
  float sigma = avDur * iSigma * 2;
  float duration = RandomNormalBounded(avDur, sigma, 0.0, avDur * 100);
  duration = duration * 1000; // convert to milliseconds
  return (Milli_t) duration;
}

void SparkleFilter::InitializeArrays() {
  int     count = GetCount();
  iState  = new bool[count];
  iTimeToChange = new Milli_t[count];
  iPixelsNeedInit = true;
}

SparkleFilter::SparkleFilter() : LFilter(), iColor(WHITE), iPixelsNeedInit(true)
{
  SetParameters(kDefaultSparkleFraction, kDefaultSparkleDuration, kDefaultSparkleSigma);
}

void SparkleFilter::InitializePixels()
{
  int     count = GetCount();
  float   totalDuration = iOnDuration + iOffDuration;
  for (int i = 0; i < count; ++i) {
    bool state = RandomFloat(totalDuration) < iOnDuration;
    iState[i] = state;
    iTimeToChange[i] = L::gStartTime + GetSparkleDuration(state)/2;
  }
  iPixelsNeedInit = false;
}

bool SparkleFilter::Update()
{
  if (iPixelsNeedInit) InitializePixels();

  int     count = GetCount();
  for (int i = 0; i < count; ++i) {
    // Update the states as necessary
    if (L::gTime > iTimeToChange[i])
      {
    	 iState[i] = !iState[i];
	     iTimeToChange[i] = L::gTime + GetSparkleDuration(iState[i]);
      }
    // If the light is sparkled, set it to the sparkle color
    if (iState[i])
      iBuffer->SetRGB(i, iColor);
  }
  return iBuffer->Update();
}

LFilter* SparkleFilterCreate(cvsref params, string* errmsg)
{
  if (! ParamListCheck(params, "sparkle", errmsg, 0, 4)) return NULL;

  // Parse the color
  Color* color = NULL;
  if (! ParseOptionalParam(&color, params, 0, "sparkle color", errmsg)) return NULL;

  // Now get other the parameters;
  float fraction = kDefaultSparkleFraction;
  float duration = kDefaultSparkleDuration;
  float sigma    = kDefaultSparkleSigma;
  
  if (!ParseOptionalParam(&fraction, params, 1, "sparkle fraction", errmsg, 0,  1)) return NULL;
  if (!ParseOptionalParam(&duration, params, 2, "sparkle duration", errmsg, 0, 30)) return NULL;
  if (!ParseOptionalParam(&sigma,    params, 3, "sparkle sigma",    errmsg, 0, 10)) return NULL;
  
  // Create the buffer
  SparkleFilter* newbuf =  new SparkleFilter();
  newbuf->SetParameters(fraction, duration, sigma);
  if (color) {
    newbuf->SetColor(color);
    delete color;
  }
  return newbuf;
}

DEFINE_LBUFFER_FILTER_TYPE(sparkle, SparkleFilterCreate, "sparkle([color],[frac],[ontime],[sigma])",
        "Sparkles the output. Frac is the fraction of time that should be spent sparkling");

//-----------------------------------------------------------------------------
// GradientColorFilter
//-----------------------------------------------------------------------------
// This filter just writes a color wash to iBuffer when Update is called
const RGBColor GradientColorFilter::kDefaultColor(RED);

void GradientColorFilter::SetColors(const Color* color1, const Color* color2) {
  if (color1) iColor1 = color1->AllocateCopy(); else iColor1 = kDefaultColor.AllocateCopy();
  if (color2) iColor2 = color2->AllocateCopy(); else iColor2 = kDefaultColor.AllocateCopy();
}

string GradientColorFilter::GetDescriptor() const {
  return "color_gradient(" + iColor1->ToString() + ", " + iColor2->ToString() + ")";
}

void GradientColorFilter::SetBuffer(LBuffer* buffer) {
  int len = buffer->GetCount();
  iColorBuffer.resize(len);
  HSVColorRange range(*iColor1, *iColor2);
  
  for (int i = 0; i < len; ++i) 
    iColorBuffer[i] = range.GetColor((float) i / (float) len);
  LFilter::SetBuffer(buffer);
}

bool GradientColorFilter::Update() {
  int len = GetCount();
  for (int i = 0; i < len; ++i) 
    iBuffer->SetRGB(i, iColorBuffer[i]);
  return iBuffer->Update();
}

//-----------------------------------------------------------------------------
// SolidColorFilter
//-----------------------------------------------------------------------------
// This filter just writes a color wash to iBuffer when Update is called

const RGBColor SolidColorFilter::kDefaultColor(WHITE);

void SolidColorFilter::SetColor(const Color* color) {
  if (color) iColor = color->AllocateCopy(); 
  else       iColor = kDefaultColor.AllocateCopy();
  }

string SolidColorFilter::GetDescriptor() const {
  return "color_solid(" + iColor->ToString() + ")";
}

bool SolidColorFilter::Update() {
  int len = GetCount();
  for (int i = 0; i < len; ++i) 
    iBuffer->SetRGB(i, *iColor);
  return iBuffer->Update();
}
