#include "Lobj.h"
#include "Color.h"
#include "LBuffer.h"
#include <math.h>

bool gAntiAlias = true;

#define MAX_OBJS 10
Lobj gAllLobjs[MAX_OBJS];
short gLobjCount = 0;

void Lobj::Clear() {
  pos = 0;
  color = BLACK;
  speed = 0;
  velocity = 0;
}


void Lobj::Render(LBuffer* buffer) const {
  if (! gAntiAlias) {
    // No anti-aliasing
    short bpos = pos / kPosIncr;
    if (bpos < 0 || bpos >= buffer->GetCount()) return;
    buffer->AddRGB(bpos, color);
  } else {
    float fpos = (float) pos / (float) kPosIncr;
    float floorpos = floor(fpos);
    // High-level clip (avoid aliasing at extreme values)
    if (floorpos <= -1.0 || floorpos >= (float) buffer->GetCount()) return;

    float efrac = fpos - floorpos;
    float bfrac = 1.0 - efrac;
    short bpos = floorpos;
    short epos = bpos + 1;

    RGBColor tcolor;
    if (bfrac > 0 && bpos >= 0 && bpos < buffer->GetCount()) {
      tcolor = color;
      tcolor *= bfrac;
      buffer->AddRGB(bpos, tcolor);
     }

    if (efrac > 0 && epos >= 0 && epos < buffer->GetCount()) {
      tcolor = color;
      tcolor *= efrac;
      buffer->AddRGB(epos, tcolor);
    }
  }
}


// Global Operations
void Lobj::RenderAll(LBuffer* buffer) {
  for (int i = 0; i < gLobjCount; ++i) {
    gAllLobjs[i].Render(buffer);
  }
}

short Lobj::GetNum(void) {
  return gLobjCount;
}

short Lobj::GetMaxNum(void) {
  return MAX_OBJS;
}

Lobj* Lobj::GetNth(short index) {
  if (index >= gLobjCount || index < 0) return 0;
  return gAllLobjs + index;
}

Lobj* Lobj::Alloc(void) {
  if (gLobjCount >= MAX_OBJS) return 0;
  Lobj* retval = &(gAllLobjs[gLobjCount++]);
  retval->Clear();
  return retval;
}

bool Lobj::Free(Lobj* lptr) {
  int index = lptr - gAllLobjs;
  if (index < 0 || index >= gLobjCount) return false;
  for (int i = index; i < gLobjCount-1; ++i)
    gAllLobjs[i] = gAllLobjs[i+1];
  --gLobjCount;
  return true;
}

void Lobj::Map(LobjMapFcn_t mapfcn) {
  for (int i = 0; i < gLobjCount; ++i)
    mapfcn(gAllLobjs + i);
}
