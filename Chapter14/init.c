#include "../inclusions.h"

syscall init(did32 descrp)
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
  retval = (*devptr->dvinit)(devptr);
  restore(mask);
  return retval;
}
