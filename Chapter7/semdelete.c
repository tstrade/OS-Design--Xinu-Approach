#include "../inclusions.h"

syscall semdelete(sid32 sem)
{
  intmask mask;
  struct sentry *semptr;

  mask = disable();
  if (isbadsem(sem)) {
    restore(mask);
    return SYSERR;
  }

  semptr = &semtab[sem];
  if (semptr->sstate == S_FREE) {
    restore(mask);
    return SYSERR;
  }

  resched_cntl(DEFER_START);
  while (semptr->scount++ < 0) {
    ready(getfirst(semptr->squeue));
  }
  resched_cntl(DEFER_STOP);

  restore(mask);
  return OK;
}
