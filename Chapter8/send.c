#include "../inclusions.h"

syscall send(pid32 pid, umsg32 msg)
{
  intmask mask;
  struct procent *prptr;

  mask = disable();
  if (isbadpid(pid)) {
    restore(mask);
    return SYSERR;
  }

  prptr = &proctab[pid];
  if ((prptr->prstate == PR_FREE) || prptr->prhasmsg) {
    restore(mask);
    return SYSERR;
  }

  prptr->prmsg = msg;
  prptr->prhasmsg = TRUE;

  if (prptr->prstate == PR_RECV) {
    ready(pid);
  } else if (prptr->prstate == PR_RECTIM) {
    unsleep(pid);
    ready(pid);
  }

  restore(mask);
  return OK;
}
