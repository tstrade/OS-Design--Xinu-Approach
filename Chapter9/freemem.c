#include "../inclusions.h"

syscall freemem(char *blkaddr, uint64 nbytes)
{
  intmask mask;
  struct memblk *next, *prev, *block;
  uint64 top;

  mask = disable();
  if ((nbytes == 0) || ((uint64)blkaddr < (uint64)minheap) || ((uint64)blkaddr > (uint64)maxheap)) {
    restore(mask);
    return SYSERR;
  }

  nbytes = (uint64)roundmb(nbytes);
  block = (struct memblk *)blkaddr;

  prev = &memlist;
  next = memlist.mnext;
  while ((next != NULL) && (next < block)) {
    prev = next;
    next = next->mnext;
  }

  if (prev == &memlist)
    top = (uint64)NULL;
  else
    top = (uint64)prev + prev->mlength;

  if (((prev != &memlist) && (uint64)block < top) || ((next != NULL) && (uint64)block + nbytes > (uint64)next)) {
    restore(mask);
    return SYSERR;
  }

  memlist.mlength += nbytes;

  if (top == (uint64)block) {
    prev->mlength += nbytes;
    block = prev;
  } else {
    block->mnext = next;
    block->mlength = nbytes;
    prev->mnext = block;
  }

  restore(mask);
  return OK;
 }
