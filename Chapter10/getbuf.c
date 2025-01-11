#include "../inclusions.h"

char *getbuf(bpid32 poolid)
{
  intmask mask;
  struct bpentry *bpptr;
  struct bpentry *bufptr;

  mask = disable();
  if (poolid < 0 || poolid >= nbpools) {
    restore(mask);
    return (char *)SYSERR;
  }

  bpptr = &buftab[poolid];
  /* Wait for pool to have > 0 buffers and allocate a buffer */
  wait(bpptr->bpsem);
  bufptr = bpptr->bpnext;
  /* Unlink buffer from pool */
  bpptr->bpnext = bufptr->bpnext;
  /* Record pool ID in first four bytes of buffer and skip */
  *(bpid32 *)bufptr = poolid;
  bufptr = (struct bpentry *)(sizeof(bpid32) + (char *)bufptr);
  restore(mask);
  return (char *)bufptr;
}
