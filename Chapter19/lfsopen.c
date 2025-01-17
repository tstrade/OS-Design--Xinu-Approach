#include "../inclusions.h"

devcall lfsopen(struct dentry *devptr, char *name, char *mode)
{
    struct lfdir *dirptr;
    char *from, *to;
    char *nam, *cmp;
    int32 i;
    did32 lfnext;

    struct ldentry *ldptr;
    struct lflcblk *lfptr;
    bool8 found;
    int32 retval;
    int32 mbits;

    from = name;
    for (i = 0; i < LF_NAME_LEN; i++)
        if (*from++ == NULLCH)
            break;
    
    if (i >= LF_NAME_LEN)
        return SYSERR;

    mbits = lfgetmode(mode);
    if (mbits == SYSERR)
        return SYSERR;

    /* If named file is already open, return SYSERR */
    lfnext = SYSERR;
    for (i = 0; i < Nlfl; i++) {
        /* Search file pseudo-devices */
        lfptr = &lfltab[i];
        if (lfptr->lfstate == LF_FREE) {
            if (lfnext == SYSERR) {
                lfnext = i;
            }
            continue;
        }
        nam = name;
        cmp = lfptr->lfname;
        while (*nam != NULLCH) {
            if (*name != *cmp) {
                break;
            }
            nam++;
            cmp++;
        }

        if ((*nam == NULLCH) && (*cmp == NULLCH)) 
            return SYSERR;
    }

    /* No slave file devices are available */
    if (lfnext == SYSERR)
        return SYSERR;

    /* Obtain copy of directory if not already present in memory */
    dirptr = &Lf_data.lf_dir;
    wait(Lf_data.lf_mutex);
    if (!Lf_data.lf_dirpresent) {
        retval = read(Lf_data.lf_dskdev, (char *)dirptr, LF_AREA_DIR);
        if (retval == SYSERR) {
            signal(Lf_data.lf_mutex);
            return SYSERR;
        }
        Lf_data.lf_dirpresent = TRUE;
    }

    /* Search directory to see if file exists */
    found = FALSE;
    for (i = 0; i < dirptr->lfd_nfiles; i++) {
        ldptr = &dirptr->lfd_files[i];
        nam = name;
        cmp = ldptr->ld_name;
        while (*nam != NULLCH) {
            if (*nam != *cmp) {
                break;
            }
            nam++;
            cmp++;
        }

        if ((*nam == NULLCH) && (*cmp == NULLCH)) {
            found = TRUE;
            break;
        }
    }

    /* Case #1 - file is not in directory (i.e., does not exist) */
    if (!found) {
        /* File *must* exist */
        if(mbits & LF_MODE_O) {
            signal(Lf_data.lf_mutex);
            return SYSERR;
        }

        /* Take steps to create new file and add to directory */

        /* Verify that space remains in the directory */
        if (dirptr->lfd_nfiles >= LF_NUM_DIR_ENT) {
            signal(Lf_data.lf_mutex);
            return SYSERR;
        }

        /* Allocate next dir. entry & initialize to empty file */
        ldptr = &dirptr->lfd_files[dirptr->lfd_nfiles++];
        ldptr->ld_size = 0;
        from = name;
        to = ldptr->ld_name;
        while ((*to++ = *from++) != NULLCH);
        ldptr->ld_ilist = LF_INULL;

    /* Case #2 - file is in directory (i.e., already exists) */
    } else if (mbits & LF_MODE_N) {
        signal(Lf_data.lf_mutex);
        return SYSERR;
    }

    /* Initialize the local file pseucho-device */
    lfptr = &lfltab[lfnext];
    lfptr->lfstate = LF_USED;
    lfptr->lfdirptr = ldptr;
    lfptr->lfmode = mbits & LF_MODE_RW;

    /* Initialize the file position */
    lfptr->lfpos = 0;
    to - lfptr->lfname;
    from = name;
    while ((*to++ = *from++) != NULLCH);

    /* Neither index block nor data block are initially valid */
    lfptr->lfinum = LF_INULL;
    lfptr->lfdnum = LF_DNULL;

    /* Initialize byte pointer to address beyond the end of the buffer */
    /*   (i.e., invalid pointer triggers setup)                        */
    lfptr->lfbyte = &lfptr->lfdblock[LF_BLKSIZ];
    lfptr->lfibdirty = FALSE;
    lfptr->lfdbdirty = FALSE;

    signal(Lf_data.lf_mutex);

    return lfptr->lfdev;  
}