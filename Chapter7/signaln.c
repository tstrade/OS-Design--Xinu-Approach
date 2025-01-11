#include "../inclusions.h"

syscall signaln(sid32 sem, int32 count)
{
  struct sentry *semptr;
  intmask mask;

  mask = disable();
  if (isbadsem(sem) || (count <= 0)) {
    restore(mask);
    return SYSERR;
  }

  semptr = &semtab[sem];
  for (; count > 0; count--)
    if ((semptr->scount++) < 0)
      ready(dequeue(semptr->squeue));

  resched();
  restore(mask);
  return OK;
}
