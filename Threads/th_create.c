#include "../inclusions.h"

local int newtid();

tid_typ th_create(void *payload, uint64 ssize, pri16 priority, uint32 nargs, ...)
{
    uint32 savsp, *pushsp;
    tid_typ tid;
    struct threadent *thptr;
    int32 i;
    uint32 *a;
    uint32 *saddr;

    if (ssize < TH_MINSTK)
        ssize = TH_MINSTK;

    ssize = (uint64)roundmb(ssize);
    if ((priority < 1) || ((tid = newtid()) == SYSERR) 
        || ((saddr = (uint32 *)th_getstk(ssize)) == (uint32 *)SYSERR)) {
        return SYSERR;
    }
}