#include "../inclusions.h"

bpid32 mkbufpool(int32 bufsiz, int32 numbufs)
{
  intmask mask;
  bpid32 poolid;
  struct bpentry *bpptr;
  char *buf;

  mask = disable();
  if (bufsiz < BP_MINB || bufsiz > BP_MAXB || numbufs < 1 || numbufs > BP_MAXB || nbpools >= NBPOOLS) {
    restore(mask);
    return (bpid32)SYSERR;
  }

  /* Round request to a multiple of 4 bytes */
  bufsiz = ((bufsiz + 3) & (~3));

  buf = (char *)getmem(numbufs * (bufsiz + sizeof(bpid32)));
  if ((long)buf == SYSERR) {
    restore(mask);
    return (bpid32)SYSERR;
  }

  poolid = nbpools++;
  bpptr = &buftab[poolid];
  bpptr->bpnext = (struct bpentry *)buf;
  bpptr->bpsize = bufsiz;
  if ((bpptr->bpsem = semcreate(numbufs)) == SYSERR) {
    nbpools--;
    restore(mask);
    return (bpid32)SYSERR;
  }

  bufsiz += sizeof(bpid32);
  for (numbufs--; numbufs > 0; numbufs--) {
    bpptr = (struct bpentry *)buf;
    buf += bufsiz;
    bpptr->bpnext = (struct bpentry *)buf;
  }

  bpptr = (struct bpentry *)buf;
  bpptr->bpnext = (struct bpentry *)NULL;
  restore(mask);
  return poolid;
}
