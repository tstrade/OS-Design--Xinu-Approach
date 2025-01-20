#include "../inclusions.h"

devcall namopen(struct dentry *devptr, char *name, char *mode)
{
    char newname[NM_MAXLEN];
    did32 newdev;

    /* Use namespace to map name to new name and new descriptor */
    newdev = nammap(name, newname, devptr->dvnum);
    if (newdev == SYSERR)
        return SYSERR;

    return open(newdev, newname, mode);
}