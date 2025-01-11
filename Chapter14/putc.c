#include "../inclusions.h"

syscall putc(did32 descrp, char ch)
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
  retval = (*devptr->dvputc)(devptr, ch);
  restore(mask);
  return retval;
}
