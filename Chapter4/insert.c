#include "../inclusions.h"

status insert(pid32 pid, qid16 qid, int32 key)
{
  int16 curr, prev;

  if (isbadqid(qid) || isbadpid(pid))
    return -1;

  curr = firstid(qid);
  while (queuetab[curr].qkey >= key)
    curr = queuetab[curr].qnext;

  prev = queuetab[curr].qprev;
  queuetab[pid].qnext = curr;
  queuetab[pid].qprev = prev;
  queuetab[pid].qkey = key;
  queuetab[prev].qnext = pid;
  queuetab[curr].qprev = pid;
  return 0;
}
