#include "../inclusions.h"

devcall lflclose(struct dentry *devptr)
{
    struct lflcblk *lfptr;

    /* Obtain exclusive use of the file */
    lfptr = &lfltab[devptr->dvminor];
    wait(lfptr->lfmutex);

    /* If file not open, return an error */
    if (lfptr->lfstate != LF_USED) {
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    /* Write index or data blocks to disk if they have changed */
    if (Lf_data.lf_dirdirty || lfptr->lfdbdirty || lfptr->lfibdirty)
        lfflush(lfptr);

    /* Set device state to FREE and return */
    lfptr->lfstate = LF_FREE;
    signal(lfptr->lfmutex);
    return OK;
}