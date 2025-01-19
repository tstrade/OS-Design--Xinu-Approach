#include "../inclusions.h"

devcall rflputc(struct dentry *devptr, char ch)
{
    struct rflcblk *rfptr;
    rfptr = &rfltab[devptr->dvminor];

    if (rflwrite(devptr, &ch, 1) != 1)
        return SYSERR;

    return OK;
}