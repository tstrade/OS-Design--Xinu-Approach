#include "../inclusions.h"

void lfibclear(struct lfiblk *ibptr, int32 offset)
{
    int32 i;

    ibptr->ib_offset = offset;
    for (i = 0; i < LF_IBLEN; i++)
        ibptr->ib_dba[i] = LF_DNULL;

    ibptr->ib_next = LF_INULL;
    return;
}