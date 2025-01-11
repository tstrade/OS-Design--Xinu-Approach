#include "../inclusions.h"

pri16 chprio(pid32 pid, pri16 newprio)
{
  intmask mask;
  struct procent *prptr;
  pri16 oldprio;

  mask = disable();
  if (isbadpid(pid) || newprio <= 0) {
    restore(mask);
    return (pri16)SYSERR;
  }

  prptr = &proctab[pid];
  oldprio = prptr->prprio;
  prptr->prprio = newprio;
  restore(mask);
  return oldprio;
}
