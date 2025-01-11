#include "../inclusions.h"

struct qentry queuetab[NQENT];

pid32 enqueue(pid32 pid, qid16 qid)
{
  qid16 tail, prev;

  if (isbadqid(qid) || isbadpid(pid))
    return -1;

  tail = queuetail(qid);
  prev = queuetab[tail].qprev;

  queuetab[pid].qnext = tail;
  queuetab[pid].qprev = prev;
  queuetab[prev].qnext = pid;
  queuetab[tail].qprev = pid;
  return pid;
}

pid32 dequeue(qid16 qid)
{
  pid32 pid;

  if (isbadqid(qid)) {
    return -1;
  } else if (isempty(qid)) {
    return EMPTY;
  }

  pid = getfirst(qid);
  queuetab[pid].qprev = EMPTY;
  queuetab[pid].qnext = EMPTY;
  return pid;
}
