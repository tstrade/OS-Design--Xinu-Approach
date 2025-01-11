#include "../inclusions.h"

void clkhandler()
{
  static uint32 count1000 = CLKTICKS_PER_SEC;

  if ((--count1000) <= 0) {
    clktime++;
    count1000 = CLKTICKS_PER_SEC;
  }

  if (!isempty(sleepq))
    if ((--queuetab[firstid(sleepq)].qkey) <= 0)
      wakeup();

  if ((--preempt) <= 0) {
    preempt = QUANTUM;
    resched();
  }
}
