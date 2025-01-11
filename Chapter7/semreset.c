#include "../inclusions.h"

syscall semreset(sid32 sem, int32 count)
{
  intmask mask;
  struct sentry *semptr;
  qid16 semqueue;
  pid32 pid;

  mask = disable();
  if (count < 0 || isbadsem(sem) || semtab[sem].sstate == S_FREE) {
    restore(mask);
    return SYSERR;
  }

  semptr = &semtab[sem];
  semqueue = semptr->squeue;

  resched_cntl(DEFER_START);
  while ((pid = getfirst(semqueue)) != EMPTY)
    ready(pid);
  semptr->scount = count;
  resched_cntl(DEFER_STOP);
  restore(mask);
  return OK;
}
