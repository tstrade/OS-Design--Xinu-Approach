#include "../inclusions.h"

umsg32 recvtime(int32 maxwait)
{
  intmask mask;
  struct procent *prptr;
  umsg32 msg;

  if (maxwait < 0)
    return SYSERR;

  mask = disable();

  prptr = &proctab[currpid];
  if (prptr->prhasmsg == FALSE) {
    if (insertd(currpid, sleepq, maxwait) == SYSERR) {
      restore(mask);
      return SYSERR;
    }
    prptr->prstate = PR_RECTIM;
    resched();
  }

  if (prptr->prhasmsg) {
    msg = prptr->prmsg;
    prptr->prhasmsg = FALSE;
  } else {
    msg = TIMEOUT;
  }

  restore(mask);
  return msg;
}
