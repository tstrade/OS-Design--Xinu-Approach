#include "../inclusions.h"

char *getmem(uint64 nbytes)
{
  intmask mask;
  struct memblk *prev, *curr, *leftover;

  mask = disable();
  if (nbytes == 0) {
    restore(mask);
    return (char *)SYSERR;
  }

  nbytes = (uint64)roundmb(nbytes);

  prev = &memlist;
  curr = memlist.mnext;
  while (curr != NULL) {

    if (curr->mlength == nbytes) {
      prev->mnext = curr->mnext;
      memlist.mlength -= nbytes;
      restore(mask);
      return (char *)(curr);

    } else if (curr->mlength > nbytes) {
      leftover = (struct memblk *)((uint64) curr + nbytes);
      prev->mnext = leftover;
      leftover->mnext = curr->mnext;
      leftover->mlength = curr->mlength - nbytes;
      memlist.mlength -= nbytes;
      restore(mask);
      return (char *)(curr);

    } else {
      prev = curr;
      curr = curr->mnext;
    }
  }

  restore(mask);
  return (char *)SYSERR;
}
