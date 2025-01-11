#include "../inclusions.h"

syscall ptdelete(int32 portid, int32 (*disp)(int32))
{
  intmask mask;
  struct ptentry *ptptr;

  mask = disable();
  if (isbadport(portid) || (ptptr = &porttab[portid])->ptstate != PT_ALLOC) {
    restore(mask);
    return SYSERR;
  }

  _ptclear(ptptr, PT_FREE, disp);
  ptnextid = portid;
  restore(mask);
  return OK;
}
