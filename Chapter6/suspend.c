#include "../inclusions.h"

syscall suspend(pid32 pid)
{
  intmask mask;
  struct procent *prptr;
  pri16 prio;

  mask = disable();
  if (isbadpid(pid) || (pid == NULLPROC)) {
    restore(mask);
    return SYSERR;
  }

  prptr = &proctab[pid];
  if ((prptr->prstate != PR_CURR) && (prptr->prstate != PR_READY)) {
    restore(mask);
    return SYSERR;
  }

  if (prptr->prstate == PR_READY) {
    getitem(pid);
    prptr->prstate = PR_SUSP;
  } else {
    prptr->prstate = PR_SUSP;
    resched();
  }

  prio = prptr->prprio;
  restore(mask);
  return prio;
}
