#include "../inclusions.h"

syscall control(did32 descrp, int64 func, int64 arg1, int64 arg2)
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
  retval = (*devptr->dvcntl)(devptr, func, arg1, arg2);
  restore(mask);
  return retval;
}
