#include "../inclusions.h"

syscall th_suspend(tid_typ tid)
{
    struct threadent *thptr;
    struct procent *thparent;
    pri16 prio;

    if (isbadtid(tid))
        return SYSERR;

    thptr = &threadtab[tid];
    thparent = &proctab[currpid];
    if ((thptr->thstate != TH_CURR) && (thptr->thstate != TH_READY))
        return SYSERR;

    if (thptr->thstate == TH_READY) {
        th_getitem(tid);
        thptr->thstate = TH_SUSP;
    } else {
        thptr->thstate = TH_SUSP;
        th_resched();
    }

    prio = thptr->thprio;
    return prio;
}