#include "../inclusions.h"

devcall rflread(struct dentry *devptr, char *buff, int32 count)
{
    struct rflcblk *rfptr;
    int32 retval;
    struct rf_msg_rreq msg;
    struct rf_msg_rres resp;
    int32 i;
    char *from, *to;
    int32 len;

    wait(Rf_data.rf_mutex);

    if ((count <= 0) || (count > RF_DATALEN)) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Verify pseudo-device is in use */
    rfptr = &rfltab[devptr->dvminor];
    if (rfptr->rfstate == RF_FREE) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Verify pseudo-device allows reading */
    if ((rfptr->rfmode & RF_MODE_R) == 0) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Form read request */
    msg.rf_type = htons(RF_MSG_RREQ);
    msg.rf_status = htons(0);
    msg.rf_seq = 0;
    from = rfptr->rfname;
    to = msg.rf_name;
    memset(to, NULLCH, RF_NAMLEN);
    len = 0;
    while ((*to++ = *from++)) {
        if (++len >= RF_NAMLEN) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }
    }
    msg.rf_pos = htonl(rfptr->rfpos);
    msg.rf_len = htonl(count);

    retval = rfscomm((struct rf_msg_hdr *)&msg, sizeof(struct rf_msg_rreq),
                     (struct rf_msg_hdr *)&resp, sizeof(struct rf_msg_rres));

    if (retval == SYSERR) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    } else if (retval == TIMEOUT) {
        kprintf("Timeout during remote file read\n");
        signal(Rf_data.rf_mutex);
        return SYSERR;
    } else if (ntohs(resp.rf_status) != 0) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Copy data to application buffer and update file position */
    for (i = 0; i < htonl(resp.rf_len); i++)
        *buff++ = resp.rf_data[i];
    rfptr->rfpos += htonl(resp.rf_len);

    signal(Rf_data.rf_mutex);
    return htonl(resp.rf_len);
}