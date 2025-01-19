#include "../inclusions.h"

devcall rfsopen(struct dentry *devptr, char *name, char *mode)
{
    struct rflcblk *rfptr;
    struct rf_msg_oreq msg;
    struct rf_msg_ores resp;
    int32 retval;
    int32 len;
    char *nptr;
    char *fptr;
    int32 i;

    wait(Rf_data.rf_mutex);

    /* Search control block array to find a free entry */
    for (i = 0; i < Nrfl; i++) {
        rfptr = &rfltab[i];
        if (rfptr->rfstate == RF_FREE)
            break;
    }
    if (i >= Nrfl) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Copy name into free table slot */
    nptr = name;
    fptr = rfptr->rfname;
    len = 0;
    while ((*fptr++ = *nptr++) != NULLCH) {
        len++;
        if (len >= RF_NAMLEN) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }
    }

    /* Verify that name is non-null */
    if (len == 0) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Parse mode string */
    if ((rfptr->rfmode = rfsgetmode(mode)) == SYSERR) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Form an open request to create a new file or open an old one */
    msg.rf_type = htons(RF_MSG_OREQ);
    msg.rf_status = htons(0);
    msg.rf_seq = 0;
    nptr = msg.rf_name;
    memset(nptr, NULLCH, RF_NAMLEN);

    while ((*nptr++ = *name++) != NULLCH);
    msg.rf_mode = htonl(rfptr->rfmode);

    retval = rfscomm((struct rf_msg_hdr *)&msg, sizeof(struct rf_msg_oreq),
                     (struct rf_msg_hdr *)&resp, sizeof(struct rf_msg_ores));

    if (retval == SYSERR) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    } else if (retval == TIMEOUT) {
        kprintf("Timeout during remote file open\n\r");
        signal(Rf_data.rf_mutex);
        return SYSERR;
    } else if (ntohs(resp.rf_status) != 0) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    rfptr->rfpos = 0;
    rfptr->rfstate = RF_USED;

    signal(Rf_data.rf_mutex);
    return rfptr->rfdev;
}