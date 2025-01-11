#include "../inclusions.h"

void resched()
{
  struct procent *ptold;
  struct procent *ptnew;

  if (Defer.ndefers > 0) {
    Defer.attempt = TRUE;
    return;
  }

  ptold = &proctab[currpid];

  if (ptold->prstate == PR_CURR) {
    if (ptold->prprio > firstkey(readylist))
      return;

    ptold->prstate = PR_READY;
    insert(currpid, readylist, ptold->prprio);
  }

  currpid = dequeue(readylist);
  ptnew = &proctab[currpid];
  ptnew->prstate = PR_CURR;
  preempt = QUANTUM;
  ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
  return;
}

status resched_cntl(int32 defer)
{
  switch (defer) {
  case DEFER_START:
    if (Defer.ndefers++ == 0)
      Defer.attempt = FALSE;
    return OK;

  case DEFER_STOP:
    if (Defer.ndefers <= 0)
      return SYSERR;

    if ((--Defer.ndefers == 0) && Defer.attempt)
      resched();

    return OK;

  default:
    return SYSERR;
  }
}
