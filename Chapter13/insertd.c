#include "../inclusions.h"

status insertd(pid32 pid, qid16 q, int32 key)
{
  int32 next;
  int32 prev;

  if (isbadqid(q) || isbadpid(pid))
    return SYSERR;

  prev = queuehead(q);
  next = queuetab[queuehead(q)].qnext;
  while ((next != queuetail(q)) && (queuetab[next].qkey <= key)) {
    key -= queuetab[next].qkey;
    prev = next;
    next = queuetab[next].qnext;
  }

  queuetab[pid].qnext = next;
  queuetab[pid].qprev = prev;
  queuetab[pid].qkey = key;
  queuetab[prev].qnext = pid;
  queuetab[next].qprev = pid;

  if (next != queuetail(q))
    queuetab[next].qkey -= key;

  return OK;
}
