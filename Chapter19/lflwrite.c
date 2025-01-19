#include "../inclusions.h"

devcall lflwrite(struct dentry *devptr, char *buff, int32 count)
{
    int32 i;
    if (count < 0)
        return SYSERR;

    /* Iterate and write one byte at a time */
    for (i = 0; i < count; i++)
        if (lflputc(devptr, *buff++) == SYSERR)
            return SYSERR;
        
    return count;
}