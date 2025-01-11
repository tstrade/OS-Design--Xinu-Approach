#include "../inclusions.h"

syscall freebuf(char *bufaddr)
{
  intmask mask;
  struct bpentry *bpptr;
  bpid32 poolid;

  mask = disable();
  /* Extract pool ID from integer prior to buffer address */
  bufaddr -= sizeof(bpid32);
  poolid = *(bpid32 *)bufaddr;
  if (poolid < 0 || poolid >= nbpools) {
    restore(mask);
    return SYSERR;
  }

  /* Get address of correct pool entry in table */
  bpptr = &buftab[poolid];
  /* Insert buffer into list and signal semaphore */
  ((struct bpentry *)bufaddr)->bpnext = bpptr->bpnext;
  signal(bpptr->bpsem);
  restore(mask);
  return OK;
}
