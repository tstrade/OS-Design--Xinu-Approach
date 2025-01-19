#include "../inclusions.h"

status lfsetup(struct lflcblk *lfptr)
{
    dbid32 dnum;
    ibid32 ibnum;
    struct ldentry *ldptr;
    struct lfiblk *ibptr;
    uint32 newoffset;
    int32 dindex;

    wait(Lf_data.lf_mutex);

    /* Get pointers to in-memory directory, file's entry in the */
    /*    directory, and the in-memory index block              */
    ldptr = lfptr->lfdirptr;
    ibptr = &lfptr->lfiblock;

    if (lfptr->lfibdirty || lfptr->lfdbdirty)
        lfflush(lfptr);
    ibnum = lfptr->lfinum;

    /* If there is no index block in memory (e.h., because the file */
    /*    was just opened), either load the first index block of    */
    /*    the file or allocate a new first index block              */
    if (ibnum == LF_INULL)
    {
        /* Check directory entry to see if index block exists */
        ibnum = ldptr->ld_ilist;
        if (ibnum == LF_INULL)
        {
            ibnum = lfiballoc();
            lfibclear(ibptr, 0);
            ldptr->ld_ilist = ibnum;
            lfptr->lfibdirty = TRUE;
        }
        else
        {
            lfibget(Lf_data.lf_dskdev, ibnum, ibptr);
        }
        lfptr->lfinum = ibnum;

        /* Otherwise, if current file position has been moved to an */
        /*    offset before the current index block, start at the   */
        /*    beginning of the index list for the file              */
    }
    else if (lfptr->lfpos < ibptr->ib_offset)
    {
        /* Load initial index block for the file (we know that */
        /*    at least one index block exists)                 */
        ibnum = ldptr->ld_ilist;
        lfibget(Lf_data.lf_dskdev, ibnum, ibptr);
        lfptr->lfinum = ibnum;
    }

    /* At this point, an index block is in memory, but may cover    */
    /*    an offset less than the current file position. Loop until */
    /*    the index block covers the current file position.         */
    while ((lfptr->lfpos & ~LF_IMASK) > ibptr->ib_offset)
    {
        ibnum = ibptr->ib_next;
        if (ibnum == LF_INULL)
        {
            /* Allocate new index block to extend file */
            ibnum = lfiballoc();
            ibptr->ib_next = ibnum;
            lfibput(Lf_data.lf_dskdev, lfptr->lfinum, ibptr);
            lfptr->lfinum = ibnum;
            newoffset = ibptr->ib_offset + LF_IDATA;
            lfibclear(ibptr, newoffset);
            lfptr->lfibdirty = TRUE;
        }
        else
        {
            lfibget(Lf_data.lf_dskdev, ibnum, ibptr);
            lfptr->lfinum = ibnum;
        }
        lfptr->lfdnum = LF_DNULL;
    }

    /* At this point, the index block in lfiblock covers the    */
    /*    current file position (i.e., position lfptr->lfpos).  */
    /*    The next step consists of loading the correct d-block */
    dindex = (lfptr->lfpos & LF_IMASK) >> 9;

    /* If data block index does not match current data block, read */
    /*    the correct data block from disk                         */
    dnum = lfptr->lfiblock.ib_dba[dindex];
    if (dnum == LF_DNULL)
    {
        dnum = lfdballoc((struct lfdbfree *)&lfptr->lfdblock);
        lfptr->lfiblock.ib_dba[dindex] = dnum;
        lfptr->lfibdirty = TRUE;
    }
    else if (dnum != lfptr->lfdnum)
    {
        read(Lf_data.lf_dskdev, (char *)lfptr->lfdblock, dnum);
        lfptr->lfdbdirty = FALSE;
    }
    lfptr->lfdnum = dnum;

    /* Use current file offset to set the pointer to the next byte */
    /*    within the data block                                    */
    lfptr->lfbyte = &lfptr->lfdblock[lfptr->lfpos & LF_DMASK];
    signal(Lf_data.lf_mutex);
    return OK;
}