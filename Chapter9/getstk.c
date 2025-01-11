#include "../inclusions.h"

char *getstk(uint64 nbytes)
{
  intmask mask;
  struct memblk *prev, *curr;
  struct memblk *fits, *fitsprev;

  mask = disable();
  if (nbytes == 0) {
    restore(mask);
    return (char *)SYSERR;
  }

  nbytes = (uint64)roundmb(nbytes);

  prev = &memlist;
  curr = memlist.mnext;
  fits = NULL;
  fitsprev = NULL;

  while (curr != NULL) {
    if (curr->mlength >= nbytes) {
      fits = curr;
      fitsprev = prev;
    }
    prev = curr;
    curr = curr->mnext;
  }

  if (fits == NULL) {
    restore(mask);
    return (char *)SYSERR;
  }

  if (nbytes == fits->mlength) {
    fitsprev->mnext = fits->mnext;
  } else {
    fits->mlength -= nbytes;
    fits = (struct memblk *)((uint64)fits + fits->mlength);
  }

  memlist.mlength -= nbytes;
  restore(mask);
  return (char *)((uint64)fits + nbytes - sizeof(uint64));
}
