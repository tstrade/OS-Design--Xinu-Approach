#include "../inclusions.h"

status lfscreate(did32 disk, ibid32 lfiblks, uint32 dsiz)
{
    uint32 sectors;
    uint32 ibsectors;
    uint32 ibpersector;
    struct lfdir dir;
    uint32 dblks;
    struct lfiblk iblock;
    struct lfdbfree dblock;
    dbid32 dbindex;
    int32 retval;
    int32 i;

    /* Compute total sectors on disk */
    sectors = dsiz / LF_BLKSIZ;

    /* Compute number of sectors comprising i-blocks */
    ibpersector = LF_BLKSIZ / sizeof(struct lfiblk);
    ibsectors = (lfiblks + (ibpersector - 1)) / ibpersector;
    lfiblks = ibsectors * ibpersector;
    if (ibsectors > sectors / 2)
        return SYSERR;

    /* Create an initial directory */
    memset((char *)&dir, NULLCH, sizeof(struct lfdir));
    dir.lfd_nfiles = 0;
    dbindex = (dbid32)(ibsectors + 1);
    dir.lfd_dfree = dbindex;
    dblks = sectors - ibsectors - 1;
    retval = write(disk, (char *)&dir, LF_AREA_DIR);
    if (retval == SYSERR)
        return SYSERR;

    /* Create list of free i-blocks on disk */
    lfibclear(&iblock, 0);
    for (i = 0; i < lfiblks - 1; i++)
    {
        iblock.ib_next = (ibid32)(i + 1);
        lfibput(disk, i, &iblock);
    }
    iblock.ib_next = LF_INULL;
    lfibput(disk, i, &iblock);

    /* Create list of free data blocks on disk */
    memset((char *)&dblock, NULLCH, LF_BLKSIZ);
    for (i = 0; i < dblks - 1; i++)
    {
        dblock.lf_nextdb = dbindex + 1;
        write(disk, (char *)&dblock, dbindex);
        dbindex++;
    }
    dblock.lf_nextdb = LF_DNULL;
    write(disk, (char *)&dblock, dbindex);
    close(disk);
    return OK;
}