#include "../inclusions.h"

devcall rflinit(struct dentry *devptr)
{
    struct rflcblk *rflptr;
    int32 i;
    rflptr = &rfltab[devptr->dvminor];

    rflptr->rfstate = RF_FREE;
    rflptr->rfdev = devptr->dvnum;
    for (i = 0; i < RF_NAMLEN; i++)
        rflptr->rfname[i] = NULLCH;

    rflptr->rfpos = rflptr->rfmode = 0;
    return OK;
}