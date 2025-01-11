#include "../inclusions.h"

#define MAXSECONDS 4294967     /* Max seconds per 32-bit msec */

syscall sleep(uint32 delay)
{
  if (delay > MAXSECONDS)
    return SYSERR;

  sleepms(CLKTICKS_PER_SEC * delay);
  return OK;
}

syscall sleepms(uint32 delay)
{
  intmask mask;

  mask = disable();
  if (delay == 0) {
    yield();
    restore(mask);
    return OK;
  }

  if (insertd(currpid, sleepq, delay) == SYSERR) {
    restore(mask);
    return SYSERR;
  }

  proctab[currpid].prstate = PR_SLEEP;
  resched();
  restore(mask);
  return OK;
}
