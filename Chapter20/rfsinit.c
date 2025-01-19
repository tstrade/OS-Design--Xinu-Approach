#include "../inclusions.h"

devcall rfsinit(struct dentry devptr)
{
    Rf_data.rf_seq = 1;

    if (dot2zip(RF_SERVER_IP, &Rf_data.rf_ser_ip) == SYSERR)
        panic("invalid IP address for remote file server");

    Rf_data.rf_ser_port = RF_SERVER_PORT;
    Rf_data.rf_loc_port = RF_LOC_PORT;

    if ((Rf_data.rf_mutex = semcreate(1)) == SYSERR)
        panic("Cannot create remote file system semaphore");

    Rf_data.rf_registered = FALSE;
    return OK;
}
