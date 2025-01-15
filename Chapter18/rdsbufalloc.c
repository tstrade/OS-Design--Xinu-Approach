#include "../inclusions.h"

struct rdbuff *rdsbufalloc(struct rdscblk *rdptr)
{
    struct rdbuff *bptr;
    struct rdbuff *pptr;
    struct rdbuff *nptr;

    wait(rdptr->rd_availsem);

    /* If free list contains a buffer, extract it */
    if (bptr != (struct rdbuff *)NULL) {
        rdptr->rd_free = bptr->rd_next;
        return bptr;
    }

    /* Extract oldest item in cache that has ref count zero (at */
    /*   least one such entry must exit because the semaphore   */
    /*   had a nonzero count)                                   */
    bptr = rdptr->rd_ctprev;
    while (bptr != (struct rdbuff *)&rdptr->rd_chnext) {
        if (bptr->rd_refcnt <= 0) {
            /* Remove from cache and return to caller */
            pptr = bptr->rd_prev;
            nptr = bptr->rd_next;
            pptr->rd_next = nptr;
            nptr->rd_prev = pptr;
            return bptr;
        }
        bptr = bptr->rd_prev;
    }
    panic("Remote disk cannot find an available buffer");
    return (struct rdbuff *)SYSERR;
}