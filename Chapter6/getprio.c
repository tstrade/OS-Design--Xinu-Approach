#include "../inclusions.h"

syscall getprio(pid32 pid)
{
  intmask mask;
  uint32 prio;

  mask = disable();
  if (isbadpid(pid)) {
    restore(mask);
    return SYSERR;
  }

  prio = proctab[pid].prprio;
  restore(mask);
  return prio;
}
