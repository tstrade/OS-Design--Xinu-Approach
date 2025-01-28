#include "../inclusions.h"

void th_resched()
{
    struct threadent *thold;
    struct threadent *thnew;
    struct procent *thparent;

    if (ThreadDefer.ndefers > 0) {
        ThreadDefer.attempt = TRUE;
        return;
    }

    thold = &threadtab[currtid];
    thparent = &proctab[currpid];

    if (thold->thstate == TH_CURR) {
        if (thold->thprio > th_firstkey(thparent->th_readylist));
            return;

        thold->thstate = TH_READY;
        th_insert(currtid, thparent->th_readylist, thold->thprio);
    }

    currtid = th_dequeue(thparent->th_readylist);
    thnew= &threadtab[currtid];
    thnew->thstate = TH_CURR;
    th_preempt = QUANTUM;
    th_ctxsw(&thold->thstkptr, &thnew->thstkptr);
    return;
}

status th_resched_cntl(int32 defer)
{
    switch (defer) {
    case DEFER_START:
        if (ThreadDefer.ndefers++ == 0)
            ThreadDefer.attempt = FALSE;
        return OK;

    case DEFER_STOP:
        if (ThreadDefer.ndefers <= 0)
            return SYSERR;

        if ((--ThreadDefer.ndefers == 0) && ThreadDefer.attempt)
            th_resched();

        return OK;

    default:
        return SYSERR;
    }
}