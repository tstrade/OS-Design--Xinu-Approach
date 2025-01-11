#include "../inclusions.h"

qid16 newqueue()
{
  static qid16 nextqid = NPROC;
  qid16 qid;

  qid = nextqid;
  if (qid > NQENT)
    return -1;

  nextqid += 2;

  queuetab[queuehead(qid)].qnext = queuetail(qid);
  queuetab[queuehead(qid)].qprev = EMPTY;
  queuetab[queuehead(qid)].qkey = MAXKEY;
  queuetab[queuetail(qid)].qnext = EMPTY;
  queuetab[queuetail(qid)].qprev = queuehead(qid);
  queuetab[queuetail(qid)].qkey = MINKEY;
  return qid;
}
