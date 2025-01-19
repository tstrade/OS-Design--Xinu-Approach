#include "../inclusions.h"

devcall rflgetc(struct dentry *devptr)
{
    char ch;
    int32 retval;

    retval = rflread(devptr, &ch, 1);
    if (retval != 1)
        return SYSERR;

    return (devcall)ch;
}