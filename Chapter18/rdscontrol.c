#include "../inclusions.h"

devcall rdscontrol(struct dentry *devptr, int64 func, int64 arg1, int64 arg2)
{
    struct rdscblk *rdptr;
    struct rdbuff *bptr;
    struct rdbuff *pptr;
    struct rd_msg_dreq msg;
    struct rd_msg_dres resp;
    char *to, *from;
    int32 retval;

    rdptr = &rdstab[devptr->dvminor];
    if (rdptr->rd_state != RD_OPEN)
        return SYSERR;

    switch (func) {
    /* Synchronize writes */
    case RDS_CTL_SYNC:
        /* Allocate a buffer to use for the request list */
        bptr = rdsbufalloc(rdptr);
        if (bptr == (struct rdbuff *)SYSERR)
            return SYSERR;

        /* Form a sync request */
        bptr->rd_op = RD_OP_SYNC;
        bptr->rd_refcnt = 1;
        bptr->rd_blknum = 0;
        bptr->rd_status = RD_INVALID;
        bptr->rd_pid = getpid();

        /* Insert new request into list just before tail */
        pptr = rdptr->rd_rtprev;
        rdptr->rd_rtprev = bptr;
        bptr->rd_next = pptr->rd_next;
        bptr->rd_prev = pptr;
        pptr->rd_next = bptr;

        /* Prepare to wait until item is processed */
        recvclr();
        resume(rdptr->rd_comproc);

        /* Block to wait for message */
        bptr = (struct rdbuff *)receive();
        break;

    /* Delete the remote disk (entirely remove it) */
    case RDS_CTL_DEL:
        /* Handcraft a message for the server that requests */
        /*   deleting the disk with the specified ID        */
        msg.rd_type = htons(RD_MSG_DREQ);
        msg.rd_status = htons(0);
        msg.rd_seq = 0;
        to - msg.rd_id;
        memset(to, NULLCH, RD_IDLEN);
        from = rdptr->rd_id;
        while ((*to++ = *from++) != NULLCH);

        /* Send message and receive response */
        retval = rdscomm((struct rd_msg_hdr *)&msg, sizeof(struct rd_msg_dreq),
                         (struct rd_msg_hdr *)&resp, sizeof(struct rd_msg_dres),
                         rdptr);

        /* Check response */
        if (retval == SYSERR) {
            return SYSERR;
        } else if (retval == TIMEOUT) {
            kprintf("Timeout during remote file delete\n\r");
            return SYSERR;
        } else if (ntohs(resp.rd_status) != 0) {
            return SYSERR;
        }

        return rdsclose(devptr);

    default:
        kprintf("rfsControl: function %d not valid\n\r", func);
        return SYSERR;
    }
    return OK;
}