#include "../inclusions.h"

ibid32 lfiballoc()
{
    ibid32 ibnum;
    struct lfiblk iblock;

    ibnum = Lf_data.lf_dir.lfd_ifree;
    if (ibnum == LF_INULL)
        panic("out of index blocks");

    lfibget(Lf_data.lf_dskdev, ibnum, &iblock);

    /* Unlink index block from the directory free list */
    Lf_data.lf_dir.lfd_ifree = iblock.ib_next;

    /* write a copy of the directory to disk after the change */
    write(Lf_data.lf_dskdev, (char *)&Lf_data.lf_dir, LF_AREA_DIR);
    Lf_data.lf_dirdirty = FALSE;

    return ibnum;
}