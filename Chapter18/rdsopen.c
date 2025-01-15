#include "../inclusions.h"

devcall rdsopen(struct dentry *devptr, char *diskid, char *mode)
{
    struct rdscblk *rdptr;
    struct rd_msg_oreq msg;
    struct rd_msg_ores resp;
    int32 retval;
    int32 len;
    char *idto;
    char *idfrom;

    rdptr = &rdstab[devptr->dvminor];

    if (rdptr->rd_state != RD_FREE)
        return SYSERR;
    rdptr->rd_state = RD_PEND;

    /* Copy disk ID into free table slot */
    idto = rdptr->rd_id;
    idfrom = diskid;
    len = 0;
    while ((*idto++ = *idfrom++) != NULLCH) {
        len++;
        if (len >= RD_IDLEN)
            return SYSERR;
    }

    if (len == 0)
        return SYSERR;

    /* Hand-craft an open request message to be sent to the server */
    msg.rd_type = htons(RD_MSG_OREQ);
    msg.rd_status = htons(0);
    msg.rd_seq = 0;
    idto = msg.rd_id;
    memset(idto, NULLCH, RD_IDLEN);

    idfrom = diskid;
    while ((*idto++ = *idfrom++) != NULLCH);
    
    retval = rdscomm((struct rd_msg_hdr *)&msg, sizeof(struct rd_msg_oreq), 
                     (struct rd_msg_hdr *)&resp, sizeof(struct rd_msg_ores), rdptr);

    /* Check response */
    if (retval == SYSERR) {
        rdptr->rd_state = RD_FREE;
        return SYSERR;
    } else if (retval == TIMEOUT) {
        kprintf("Timeout during remote file open\n\r");
        rdptr->rd_state = RD_FREE;
        return SYSERR;
    } else if (ntohs(resp.rd_status) != 0) {
        rdptr->rd_state = RD_FREE;
        return SYSERR;
    }

    rdptr->rd_state = RD_OPEN;
    return devptr->dvnum;
}