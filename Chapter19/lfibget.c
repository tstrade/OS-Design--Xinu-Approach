#include "../inclusions.h"

void lfibget(did32 diskdev, ibid32 inum, struct lfiblk *ibuff)
{
    char *from, *to; 
    int32 i;
    char dbuff[LF_BLKSIZ];

    read(diskdev, dbuff, ib2sect(inum));

    from = dbuff + ib2disp(inum);
    to = (char *)ibuff;
    for (i = 0; i < sizeof(struct lfiblk); i++)
        *to++ = *from++;

    return;
}