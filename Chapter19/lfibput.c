#include "../inclusions.h"

status lfibput(did32 diskdev, ibid32 inum, struct lfiblk *ibuff)
{
    dbid32 diskblock;
    char *from, *to;
    int32 i;
    char dbuff[LF_BLKSIZ];

    /* Compute disk block number and offset of index block */
    diskblock = ib2sect(inum);
    to = dbuff + ib2disp(inum);
    from = (char *)ibuff;

    /* Read disk block */
    if (read(diskdev, dbuff, diskblock) == SYSERR)
        return SYSERR;

    for (i = 0; i < sizeof(struct lfiblk); i++)
        *to++ = *from++;

    write(diskdev, dbuff, diskblock);
    return OK;
}