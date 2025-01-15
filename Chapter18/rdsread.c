#include "../inclusions.h"

devcall rdsread(struct dentry *devptr, char *buff, int32 blk)
{
    struct rdscblk *rdptr;
    struct rdbuff *bptr;
    struct rdbuff *nptr;
    struct rdbuff *pptr;
    struct rdbuff *cptr;

    rdptr = &rdstab[devptr->dvminor];
    if (rdptr->rd_state != RD_OPEN)
        return SYSERR;

    /* Search cache for specified block */
    bptr = rdptr->rd_chnext;
    while (bptr != (struct rdbuff *)&rdptr->rd_ctnext) {
        if (bptr->rd_blknum == blk) {
            if (bptr->rd_status == RD_VALID) {
                break;
            }
            memcpy(buff, bptr->rd_block, RD_BLKSIZ);
            return OK;
        }
        bptr = bptr->rd_next;
    }

    /* Search request list for most recent occurence of block */
    bptr = rdptr->rd_rtprev; 
    while (bptr != (struct rdbuff *)&rdptr->rd_rhnext) {
        if (bptr->rd_blknum == blk) {
            /* If most recent request for block is write, copy data */
            if (bptr->rd_op == RD_OP_WRITE) {
                memcpy(buff, bptr->rd_block, RD_BLKSIZ);
                return OK;
            }
            break;
        }
        bptr = bptr->rd_prev;
    }

    /* Allocate a buffer and add read request to tail of req. queue */
    bptr = rdsbufalloc(rdptr);
    bptr->rd_op = RD_OP_READ;
    bptr->rd_refcnt = 1;
    bptr->rd_blknum = blk;
    bptr->rd_status = RD_INVALID;
    bptr->rd_pid = getpid();

    /* Insert new request into list just before tail */
    pptr = rdptr->rd_rtprev;
    rdptr->rd_rtprev = bptr;
    bptr->rd_next = pptr->rd_next;
    bptr->rd_prev = pptr;
    pptr->rd_next = bptr;

    /* Prepare to receive message when read completes */
    recvclr();

    signal(rdptr->rd_reqsem);

    bptr = (struct rdbuff *)receive();
    if (bptr == (struct rdbuff *)SYSERR)
        return SYSERR;

    memcpy(buff, bptr->rd_block, RD_BLKSIZ);
    bptr->rd_refcnt--;
    if (bptr->rd_refcnt <= 0) {
        /* Look for previous item in cache with the same block */
        /*   number to see if this item was only being kept    */
        /*   until pending read completed                      */
        cptr = rdptr->rd_chnext;
        while (cptr != bptr) {
            if (cptr->rd_blknum == blk) {
                /* Unlink from cache */
                pptr = bptr->rd_prev;
                nptr = bptr->rd_next;
                pptr->rd_next = nptr;
                nptr->rd_prev = pptr;

                bptr->rd_next = rdptr->rd_free;
                rdptr->rd_free = bptr;
            }
        }
    }
    return OK;
}