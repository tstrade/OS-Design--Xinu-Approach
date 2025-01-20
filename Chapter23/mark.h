#ifndef MARK_H
#define MARK_H

#include "../xinu_types.h"

#define MAXMARK 20

extern int32 *(marks[]);
extern int32 nmarks;
extern sid32 mkmutex;
typedef int32 memmark[1];

#define notmarked(L) (L[0] < 0 || L[0] >= nmarks || marks[L[0]] != L)

#endif