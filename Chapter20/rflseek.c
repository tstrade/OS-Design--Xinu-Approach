#include "../inclusions.h"

devcall rflseek(struct dentry *devptr, uint32 pos)
{
    struct rflcblk *rfptr;

    wait(Rf_data.rf_mutex);

    rfptr = &rfltab[devptr->dvminor];
    if (rfptr->rfstate == RF_FREE) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Set new position */
    rfptr->rfpos = pos;
    signal(Rf_data.rf_mutex);
    return OK;
}