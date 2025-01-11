#include "../inclusions.h"

syscall ptcreate(int32 count)
{
  intmask mask;
  int32 i;
  int32 ptnum;
  struct ptentry *ptptr;

  mask = disable();
  if (count < 0) {
    restore(mask);
    return SYSERR;
  }

  for (i = 0; i < NPORTS; i++) {
    ptnum = ptnextid;
    if (++ptnextid >= NPORTS)
      ptnextid = 0;

    ptptr = &porttab[ptnum];
    if (ptptr->ptstate == PT_FREE) {
      ptptr->ptstate = PT_ALLOC;
      ptptr->ptssem = semcreate(count);
      ptptr->ptrsem = semcreate(0);
      ptptr->pthead = ptptr->pttail = NULL;
      ptptr->ptseq++;
      ptptr->ptmaxcnt = count;
      restore(mask);
      return ptnum;
    }
  }

  restore(mask);
  return SYSERR;
}
