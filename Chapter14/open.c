#include "../inclusions.h"

syscall open(did32 descrp, char *name, char *mode)
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
  retval = (*devptr->dvopen)(devptr, name, mode);
  restore(mask);
  return retval;
}
