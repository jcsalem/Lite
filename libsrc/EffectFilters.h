// Effect Filter definitions

#ifndef EFFECTFILTERS_H_INCLUDED
#define EFFECTFILTERS_H_INCLUDED

#include "LFilter.h"

//-----------------------------------------------------------------------------
// PlaneNavigationFilter
//-----------------------------------------------------------------------------
// Makes one end red and the other end green. Hides those portions of iBuffer

class PlaneNavigationFilter : public LFilter
{
public:
  static const int gDefaultWidth;  // Default width of the navigation filter

  PlaneNavigationFilter(int pixelWidth = gDefaultWidth) 
    : LFilter(), iNumPixels(pixelWidth) {}
  virtual ~PlaneNavigationFilter() {}

  virtual int     GetCount() const;
  virtual string  GetDescriptor() const;
  virtual bool    Update();

protected:
  virtual RGBColor&   GetRawRGB(int idx);

private:
  int 	     iNumPixels;
};

//-----------------------------------------------------------------------------
// SparkleFilter
//-----------------------------------------------------------------------------
// Introduces sparkles into the rendering pipeline

class SparkleFilter : public LFilter
{
public:
  SparkleFilter();
  virtual ~SparkleFilter();

  virtual void    SetBuffer(LBuffer* buffer) {LFilter::SetBuffer(buffer); InitializeArrays();}
  virtual string  GetDescriptor() const;
  virtual bool    Update();
  void SetColor(Color* color) {color->ToRGBColor(&iColor);}
  void SetParameters(float fraction, float onDuration, float sigma);

private:
  RGBColor   iColor;          // Should eventually be a ColorMode
  float      iOnFraction;       // Fraction of pixels that are sparkling at any one time
  float      iOnDuration;     // Time that a pixel should be on
  float      iOffDuration;    // Time between a single pixel's sparkle
  float      iSigma;          // Std. deviation of the on/off time (scale by duration)
  bool       iPixelsNeedInit; // true when iState needs initialization
  bool*      iState;
  Milli_t*   iTimeToChange;
  Milli_t    GetSparkleDuration(bool newState);
  void       InitializeArrays();
  void       InitializePixels();
};

#endif // EFFECTFILTERS_H_INCLUDED