#include "../inclusions.h"

qid16 readylist;

status ready(pid32 pid)
{
  register struct procent *prptr;

  if (isbadpid(pid))
    return SYSERR;

  prptr = &proctab[pid];
  prptr->prstate = PR_READY;
  insert(pid, readylist, prptr->prprio);
  resched();

  return OK;
}
