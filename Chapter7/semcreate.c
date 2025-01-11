#include "../inclusions.h"

local sid32 newsem();

sid32 semcreate(int32 count)
{
  intmask mask;
  sid32 sem;

  mask = disable();
  if (count < 0 || ((sem = newsem()) == SYSERR)) {
    restore(mask);
    return SYSERR;
  }

  semtab[sem].scount = count;
  restore(mask);
  return sem;
}

local sid32 newsem()
{
  static sid32 nextsem = 0;
  sid32 sem;
  int32 i;

  for (i = 0; i < NSEM; i++) {
    sem = nextsem++;
    if (nextsem >= NSEM)
      nextsem = 0;
    if (semtab[sem].sstate == S_FREE) {
      semtab[sem].sstate = S_USED;
      return sem;
    }
  }
  return SYSERR;
}
