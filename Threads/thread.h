#ifndef THREAD_H
#define THREAD_H

#include "../xinu_types.h"
#include "../Chapter5/process.h"

#define NTHREAD 100

#define TH_FREE     PR_FREE
#define TH_CURR     PR_CURR
#define TH_READY    PR_READY
#define TH_RECV     PR_RECV
#define TH_SLEEP    PR_SLEEP
#define TH_SUSP     PR_SUSP
#define TH_WAIT     PR_WAIT
#define TH_RECTIM   PR_RECTIM

/* Threads within the same process share the entire 
    address space, meaning they share all the code, 
    data, and resources (like open files) of that process, 
    allowing for efficient communication between them; 
    however, each thread has its own separate stack 
    and registers to manage its execution context 
    independently
*/

struct threadent {
    uint16 thstate;
    pri16  thprio;
    char   *thstkptr;
    char   *thstkbase;
    uint32 thstklen;
    sid32  thsem;
};

extern struct threadent threadtab[];
extern int32 thcount;
extern tid_typ currtid;

struct th_defer {
    int16 ndefers;
    int16 attempt;
};

extern struct th_defer ThreadDefer;
extern int16 th_preempt;

void th_resched();
status th_resched_cntl(int32 defer);
status th_ready(tid_typ tid);
void th_ctxsw(char **old_thstkptr, char **new_thstkptr);

tid_typ gettid();
syscall th_suspend(tid_typ tid);
syscall th_kill(tid_typ tid);
tid_typ th_create(void *payload, uint64 ssize, pri16 priority, uint32 nargs, ...);
syscall th_getprio(tid_typ tid);
pri16 th_chprio(tid_typ tid, pri16 newprio);

tid_typ th_enqueue(tid_typ tid, qid16 qid);
tid_typ th_dequeue(qid16 qid);
status th_insert(tid_typ tid, qid16 qid, int32 key);
tid_typ th_getfirst(qid16 qid);
tid_typ th_getlast(qid16 qid);
tid_typ th_getitem(tid_typ tid);

#define TH_MINSTK MINSTK / NTHREAD

void th_freestk(char **stkptr, uint32 thstklen);

void th_unsleep(tid_typ tid);


#define isbadtid(x) (((tid_typ)(x) < 0) || \
                    ((tid_typ)(x) >= NTHREAD) || \
                    (threadtab[(x)].thstate == TH_FREE))
                

#endif