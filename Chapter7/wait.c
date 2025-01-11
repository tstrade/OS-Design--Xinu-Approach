#include "../inclusions.h"

syscall wait(sid32 sem)
{
  intmask mask;
  struct procent *prptr;
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

  if (--(semptr->scount) < 0) {
    prptr = &proctab[currpid];
    prptr->prstate = PR_WAIT;
    prptr->prsem = sem;
    enqueue(currpid, semptr->squeue);
    resched();
  }

  restore(mask);
  return OK;
}
