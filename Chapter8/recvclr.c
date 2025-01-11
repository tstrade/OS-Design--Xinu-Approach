#include "../inclusions.h"

umsg32 recvclr()
{
  intmask mask;
  struct procent *prptr;
  umsg32 msg;

  mask = disable();
  prptr = &proctab[currpid];
  if (prptr->prhasmsg == TRUE) {
    msg = prptr->prmsg;
    prptr->prhasmsg = FALSE;
  } else {
    msg = OK;
  }

  restore(mask);
  return msg;
}
