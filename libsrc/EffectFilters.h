// Effect LFilter definitions

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

//-----------------------------------------------------------------------------
// ColorArrayFilter
//-----------------------------------------------------------------------------
// Abstract base class for any kind of filter that simply sets an array of pixels.
class ColorArrayFilter : public LFilter
{
public:
  ColorArrayFilter() : LFilter() {}
  virtual ~ColorArrayFilter() {}
  virtual void SetBuffer(LBuffer* buffer) {LFilter::SetBuffer(buffer); iColorBuffer.resize(buffer->GetCount());}
  virtual void InitializeBuffer() = 0; // every inheriting class must implement this
  virtual bool Update();
private:
  vector<RGBColor> iColorBuffer;
};

//-----------------------------------------------------------------------------
// GradientColorFilter
//-----------------------------------------------------------------------------

// This filter just writes a color wash to iBuffer when Update is called
class GradientColorFilter : public LFilter
{
public:
  static const RGBColor kDefaultColor;
  GradientColorFilter(const Color* color1 = &kDefaultColor, const Color* color2 = &kDefaultColor) : LFilter() {SetColors(color1, color2);}
  virtual ~GradientColorFilter() {delete iColor1; delete iColor2;}
  virtual void SetColors(const Color* color1, const Color* color2);
  virtual void SetBuffer(LBuffer* buffer);
  virtual string GetDescriptor() const;
  virtual bool Update();
private:
  const Color*  iColor1;
  const Color*  iColor2;
  vector<RGBColor> iColorBuffer;
};

//-----------------------------------------------------------------------------
// SolidColorFilter
//-----------------------------------------------------------------------------

// This filter just writes a solid color to iBuffer when Update is called
class SolidColorFilter : public LFilter
{
public:
  static const RGBColor kDefaultColor;
  SolidColorFilter(const Color* color = &kDefaultColor) : LFilter() {SetColor(color);}
  virtual ~SolidColorFilter() {if (iColor) delete iColor;}
  virtual void SetColor(const Color* color);
  virtual string GetDescriptor() const;
  virtual bool Update();
private:
  const Color* iColor;
};

#endif // EFFECTFILTERS_H_INCLUDED
