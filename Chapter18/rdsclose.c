#include "../inclusions.h"

devcall rdsclose(struct dentry *devptr)
{
    struct rdscblk *rdptr;
    struct rdbuff *bptr;
    struct rdbuff *nptr;
    int32 nmoved;

    rdptr = &rdstab[devptr->dvminor];
    if (rdptr->rd_state != RD_OPEN)
        return SYSERR;

    /* Request queue must be empty */
    if (rdptr->rd_rhnext != (struct rdbuff *)&rdptr->rd_rtnext)
        return SYSERR;

    /* Move all buffers from the cache to the free list */
    bptr = rdptr->rd_chnext;
    nmoved = 0;
    while (bptr != (struct rdbuff *)&rdptr->rd_ctnext) {
        nmoved++;

        /* Unlink buffer from cache */
        nptr = bptr->rd_next;
        (bptr->rd_prev)->rd_next = nptr;
        nptr->rd_prev = bptr->rd_prev;

        /* Insert buffer into free list */
        bptr->rd_next = rdptr->rd_free;
        rdptr->rd_free = bptr;
        bptr->rd_status = RD_INVALID;

        bptr = nptr;
    }
    /* Set the state to indeicate the device is closed */
    rdptr->rd_state = RD_FREE;
    return OK;
}