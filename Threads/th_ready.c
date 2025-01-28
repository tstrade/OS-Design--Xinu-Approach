#include "../inclusions.h"

status th_ready(tid_typ tid)
{
    register struct threadent *thptr;
    struct procent *thparent;

    if (isbadtid(tid))
        return SYSERR;

    thptr = &threadtab[tid];    
    thparent = &proctab[currpid];
    thptr->thstate = TH_READY;
    th_insert(tid, thparent->th_readylist, thptr->thprio);
    th_resched();

    return OK;
}