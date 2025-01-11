#include "../inclusions.h"

void wakeup()
{
  resched_cntl(DEFER_START);
  while (nonempty(sleepq) && (firstkey(sleepq) <= 0))
    ready(dequeue(sleepq));

  resched_cntl(DEFER_STOP);
  return;
}
