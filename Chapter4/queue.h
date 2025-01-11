#ifndef QUEUE_H
#define QUEUE_H

#define NPROC 100 // Example max number of processes
#define NSEM 5 // Example number of lists of semaphores
/*
  Number of entries in the queue, where a boundary occurs at NPROC
  Indexes >= NPROC correspond to the heads and tails of lists
  e.g., the number of semaphore lists is double because an entry is
        is needed for each head and tail of each list
*/
#define NQENT (NPROC + 4 + NSEM + NSEM)
#define EMPTY (-1)
#define MAXKEY 0x7FFFFFFF
#define MINKEY 0x80000000

#include "../xinu_types.h"

struct qentry {
  int qkey;
  qid16 qnext;
  qid16 qprev;
};

extern struct qentry queuetab[];

#define queuehead(q) (q)
#define queuetail(q) ((q) + 1)
#define firstid(q) (queuetab[queuehead(q)].qnext)
#define lastid(q) (queuetab[queuetail(q)].qprev)
#define isempty(q) (firstid(q) >= NPROC)
#define nonempty(q) (firstid(q) < NPROC)
#define firstkey(q) (queuetab[firstid(q)].qkey)
#define lastkey(q) (queuetab[lastif(q)].qkey)

#define isbadqid(x) (((int)(x) < 0) || (int)(x) >= NQENT-1)

pid32 getfirst(qid16 qid);
pid32 getlast(qid16 qid);
pid32 getitem(pid32 pid);
status insert(pid32 pid, qid16 qid, int32 key);
qid16 newqueue();
pid32 enqueue(pid32 pid, qid16 qid);
pid32 dequeue(qid16 qid);

#endif
