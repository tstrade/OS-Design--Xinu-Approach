#include "../inclusions.h"

struct rdscblk rdstab[Nrds];

devcall rdsinit(struct dentry *devptr)
{
    struct rdscblk *rdptr;
    struct rdbuff *bptr;
    struct rdbuff *pptr;
    struct rdbuff *buffend;
    uint32 size;

    rdptr = &rdstab[devptr->dvminor];

    rdptr->rd_state = RD_FREE;
    rdptr->rd_id[0] = NULLCH;
    rdptr->rd_seq = 1;

    rdptr->rd_rhnext = (struct rdbuff *)&rdptr->rd_rtnext;
    rdptr->rd_rhprev = (struct rdbuff *)NULL;
    rdptr->rd_rtnext = (struct rdbuff *)NULL;
    rdptr->rd_rtprev = (struct rdbuff *)&rdptr->rd_rhnext;

    rdptr->rd_chnext = (struct rdbuff *)&rdptr->rd_ctnext;
    rdptr->rd_chprev = (struct rdbuff *)NULL;
    rdptr->rd_ctnext = (struct rdbuff *)NULL;
    rdptr->rd_ctprev = (struct rdbuff *)&rdptr->rd_chnext;

    size = sizeof(struct rdbuff) * RD_BUFFS;

    bptr = (struct rdbuff *)getmem(size);
    rdptr->rd_free = bptr;

    if ((int32)bptr == SYSERR)
        panic("Cannot allocate memory for remote disk buffers");

    buffend = (struct rdbuff *)((char *)bptr + size);
    /* Walk through memory */
    while (bptr < buffend) {
        pptr = bptr;
        bptr = (struct rdbuff *)(sizeof(struct rdbuff) + (char *)bptr);
        pptr->rd_status - RD_INVALID;
        pptr->rd_next = bptr;
    }
    pptr->rd_next = (struct rdbuff *)NULL;

    /* Create the request list and available buffer semaphroes */
    rdptr->rd_availsem = semcreate(RD_BUFFS);
    rdptr->rd_reqsem = semcreate(0);

    /* Set the server IP address, server port, and local port */
    if (dot2ip(RD_SERVER_IP, &rdptr->rd_ser_ip) == SYSERR)
        panic("Invalid IP address for remote disk server");

    /* Set the port numbers */
    rdptr->rd_ser_port = RD_SERVER_PORT;
    rdptr->rd_loc_port = RD_LOC_PORT + devptr->dvminor;

    /* Specifiy that the server port is not yet registered */
    rdptr->rd_registered = FALSE;

    /* Create a communication process */
    rdptr->rd_comproc = create(rdsprocess, RD_STACK, RD_PRIO, "rdsproc", 1, rdptr);
    if (rdptr->rd_comproc == SYSERR)
        panic("Cannot create remote disk server process");
    resume(rdptr->rd_comproc);
    
    return OK;
}
