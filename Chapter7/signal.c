#include "../inclusions.h"

syscall signal(sid32 sem)
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

  if ((semptr->scount++) < 0) {
    ready(dequeue(semptr->squeue));
  }

  restore(mask);
  return OK;
}
