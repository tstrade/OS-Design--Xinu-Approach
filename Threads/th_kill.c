#include "../inclusions.h"

syscall th_kill(tid_typ tid)
{
    struct threadent *thptr;
    int32 i;

    if (isbadtid(tid) || ((thptr = &threadtab[tid])->thstate == TH_FREE))
        return SYSERR;

    if (--thcount <= 1)
        resched();

    send(currpid, tid);
    th_freestk(thptr->thstkbase, thptr->thstklen);

    switch (thptr->thstate) {
    case TH_CURR:
        thptr->thstate = TH_FREE;
        th_resched();

    case TH_SLEEP:
    case TH_RECTIM:
        th_unsleep(tid);
        thptr->thstate = TH_FREE;
        break;

    case TH_WAIT:
        semtab[thptr->thsem].scount++;

    case TH_READY:
        th_getitem(tid);

    default:
        thptr->thstate = TH_FREE;
    }
    
    return OK;
}