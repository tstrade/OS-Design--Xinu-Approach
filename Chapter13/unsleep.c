#include "../inclusions.h"

status unsleep(pid32 pid)
{
  intmask mask;
  struct procent *prptr;
  pid32 pidnext;

  mask = disable();
  if (isbadpid(pid)) {
    restore(mask);
    return SYSERR;
  }

  prptr = &proctab[pid];
  if ((prptr->prstate != PR_SLEEP) && (prptr->prstate != PR_RECTIM)) {
    restore(mask);
    return SYSERR;
  }

  pidnext = queuetab[pid].qnext;
  if (pidnext < NPROC)
    queuetab[pidnext].qkey += queuetab[pid].qkey;

  getitem(pid);
  restore(mask);
  return OK;
}
