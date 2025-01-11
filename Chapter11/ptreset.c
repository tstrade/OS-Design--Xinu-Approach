#include "../inclusions.h"

syscall ptreset(int32 portid, int32 (*disp)(int32))
{
  intmask mask;
  struct ptentry *ptptr;

  mask = disable();
  if (isbadport(portid) || (ptptr = &porttab[portid])->ptstate != PT_ALLOC) {
    restore(mask);
    return SYSERR;
  }

  _ptclear(ptptr, PT_ALLOC, disp);
  restore(mask);
  return OK;
}
