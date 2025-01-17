#include "../inclusions.h"

dbid32 lfdballoc(struct lfdbfree *dbuff)
{
    dbid32 dnum;
    int32 retval;

    /* Get ID of first data block on the free list */
    dnum = Lf_data.lf_dir.lfd_dfree;
    if (dnum == LF_INULL)
        panic("out of data blocks");

    retval = read(Lf_data.lf_dskdev, (char *)dbuff, dnum);
    if (retval == SYSERR)
        panic("lfdballoc cannot read disk block\n\r");

    /* Unlink d-block from in-memory directory */
    Lf_data.lf_dir.lfd_dfree = dbuff->lf_nextdb;
    write(Lf_data.lf_dskdev, (char *)&Lf_data.lf_dir, LF_AREA_DIR);
    Lf_data.lf_dirdirty = FALSE;

    memset((char *)dbuff, DFILL, LF_BLKSIZ);
    return dnum;
}