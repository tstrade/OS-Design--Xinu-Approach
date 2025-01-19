#include "../inclusions.h"

devcall rfscontrol(struct dentry *devptr, int64 func, int64 arg1, int64 arg2)
{
    int32 len;
    struct rf_msg_sreq msg;
    struct rf_msg_sres resp;
    struct rflcblk *rfptr;
    char *to, *from;
    int32 retval;

    wait(Rf_data.rf_mutex);

    rfptr = &rfltab[devptr->dvminor];
    from = rfptr->rfname;
    to = msg.rf_name;
    to = msg.rf_name;
    len = 0;
    memset(to, NULLCH, RF_NAMLEN);
    while ((*to++ = *from++)) {
        len++;
        if (len >= (RF_NAMLEN - 1)) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }
    }

    switch (func) {
    case RFS_CTL_DEL:
        if (rfsnmsg(RF_MSG_DREQ, (char *)arg1) == SYSERR) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }
        break;

    case RFS_CTL_TRUNC:
        if (rfsndmsg(RF_MSG_TREQ, (char *)arg1) == SYSERR) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }
        break;

    case RFS_CTL_MKDIR:
        if (rfsndmsg(RF_MSG_MREQ, (char *)arg1) == SYSERR) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }
        break;

    case RFS_CTL_RMDIR:
        if (rfsndmsg(RF_MSG_XREQ, (char *)arg1) == SYSERR) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        }

    case RFS_CTL_SIZE:
        msg.rf_type = htons(RF_MSG_SREQ);
        msg.rf_status;
        msg.rf_seq = 0;

        retval = rfscomm((struct rf_msg_hdr *)&msg, sizeof(struct rf_msg_sreq),
                         (struct rf_msg_hdr *)&resp, sizeof(struct rf_msg_sres));

        if ((retval == SYSERR) || (retval == TIMEOUT)) {
            signal(Rf_data.rf_mutex);
            return SYSERR;
        } else {
            signal(Rf_data.rf_mutex);
            return ntohl(resp.rf_size);
        }

    default:
        kprintf("rfscontrol: function %d no valid\n", func);
        signal(Rf_data.rf_mutex);
        return SYSERR;
    }
    signal(Rf_data.rf_mutex);
    return OK;
}