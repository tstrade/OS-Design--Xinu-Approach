#include "../inclusions.h"

pid32 getfirst(qid16 qid)
{
  pid32 head;

  if (isempty(qid))
    return EMPTY;

  head = queuehead(qid);
  return getitem(queuetab[head].qnext);
}

pid32 getlast(qid16 qid)
{
  pid32 tail;

  if (isempty(qid))
    return EMPTY;

  tail = queuetail(qid);
  return getitem(queuetab[tail].qprev);
}

pid32 getitem(pid32 pid)
{
  pid32 prev, next;

  next = queuetab[pid].qnext;
  prev = queuetab[pid].qprev;
  queuetab[prev].qnext = next;
  queuetab[next].qprev = prev;
  return pid;
}
