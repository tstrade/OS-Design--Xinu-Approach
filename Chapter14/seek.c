#include "../inclusions.h"

syscall seek(did32 descrp, uint32 pos)
{
  intmask mask;
  struct dentry *devptr;
  int32 retval;

  mask = disable();
  if (isbaddev(descrp)) {
    restore(mask);
    return SYSERR;
  }

  devptr = (struct dentry *) &devtab[descrp];
  retval = (*devptr->dvseek)(devptr, pos);
  restore(mask);
  return retval;
}
