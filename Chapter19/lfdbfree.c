#include "../inclusions.h"

status lfdbfree(did32 diskdev, dbid32 dnum)
{
    struct lfdir *dirptr;
    struct lfdbfree buf;

    dirptr = &Lf_data.lf_dir;
    buf.lf_nextdb = dirptr->lfd_dfree;
    dirptr->lfd_dfree = dnum;
    write(diskdev, (char *)&buf, dnum);
    write(diskdev, (char *)dirptr, LF_AREA_DIR);

    return OK;
}