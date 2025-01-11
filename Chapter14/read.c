#include "../inclusions.h"

syscall read(did32 descrp, char *buffer, uint32 count)
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
  retval = (*devptr->dvread)(devptr, buffer, count);

  restore(mask);
  return retval;
}
