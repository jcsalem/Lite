// Random number functions

#ifndef _UTILSRANDOM_H
#define _UTILSRANDOM_H

#include "utils.h"
#ifdef __AVR__
// Arduino already has this function
#include "WProgram.h"
#define Random random
#else
int32 Random (int32 min, int32 max);
int32 Random (int32 max);
#endif

#endif // _UTILSRANDOM_H
