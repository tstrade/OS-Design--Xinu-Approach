#include "../inclusions.h"

syscall kill(pid32 pid)
{
  intmask mask;
  struct procent *prptr;
  int32 i;

  mask = disable();
  if (isbadpid(pid) || (pid == NULLPROC) || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
    restore(mask);
    return SYSERR;
  }

  if (--prcount <= 1)
    xdone();

  send(prptr->prparent, pid);
  for (i = 0; i < 3; i++)
    close(prptr->prdesc[i]);
  freestk(prptr->prstkbase, prptr->prstklen);


  switch (prptr->prstate) {
  case PR_CURR:
    prptr->prstate = PR_FREE;
    resched();
  case PR_SLEEP:
  case PR_RECTIM:
    unsleep(pid);
    prptr->prstate = PR_FREE;
    break;
  case PR_WAIT:
    semtab[prptr->prsem].scount++;
  case PR_READY:
    getitem(pid);
    /* Fall through */
  default:
    prptr->prstate = PR_FREE;
  }

  restore(mask);
  return OK;
}
