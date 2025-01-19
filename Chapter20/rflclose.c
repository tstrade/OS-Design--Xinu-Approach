#include "../inclusions.h"

devcall rflclose(struct dentry *devptr)
{
    struct rflcblk *rfptr;

    wait(Rf_data.rf_mutex);

    rfptr = &rfltab[devptr->dvminor];
    if (rfptr->rfstate == RF_FREE) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Mark device closed */
    rfptr->rfstate = RF_FREE;
    signal(Rf_data.rf_mutex);
    return OK;
}