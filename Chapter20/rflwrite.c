#include "../inclusions.h"

devcall rflwrite(struct dentry *devptr, char *buff, int32 count)
{
    struct rflcblk *rfptr;
    int32 retval;
    struct rf_msg_wreq msg;
    struct rf_msg_wres resp;
    char *from, *to;
    int i;
    int32 len;

    wait(Rf_data.rf_mutex);

    if ((count <= 0) || (count > RF_DATALEN)) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Verify pseudo-device is in use and mode allows writing */
    rfptr = &rfltab[devptr->dvminor];
    if ((rfptr->rfstate == RF_FREE) || !(rfptr->rfmode & RF_MODE_W)) {
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }

    /* Form write request */
    msg.rf_type = htons(RF_MSG_WREQ);
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

    while ((*to++ = *from++));
    msg.rf_pos = htonl(rfptr->rfpos);
    msg.rf_len = htonl(count);
    for (i = 0; i < count; i++)
        msg.rf_data[i] = *buff++;
    
    while (i < RF_DATALEN)
        msg.rf_data[i++] = NULLCH;

    retval = rfscomm((struct rf_msg_hdr *)&msg, sizeof(struct rf_msg_wreq),
                     (struct rf_msg_hdr *)&resp, sizeof(struct rf_msg_wres));

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

    /* Report results to caller */
    rfptr->rfpos += ntohl(resp.rf_len);
    signal(Rf_data.rf_mutex);
    return ntohl(resp.rf_len);
}