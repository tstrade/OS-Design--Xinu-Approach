#include "../inclusions.h"

umsg32 receive()
{
  intmask mask;
  struct procent *prptr;
  umsg32 msg;

  mask = disable();
  prptr = &proctab[currpid];
  if (prptr->prhasmsg == FALSE) {
    prptr->prstate = PR_RECV;
    resched();
  }

  msg = prptr->prmsg;
  prptr->prhasmsg = FALSE;
  restore(mask);
  return msg;
}
