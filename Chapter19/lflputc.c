#include "../inclusions.h"

devcall lflputc(struct dentry *devptr, char ch)
{
    struct lflcblk *lfptr;
    struct ldentry *ldptr;

    lfptr = &lfltab[devptr->dvminor];
    wait(lfptr->lfmutex);

    if (lfptr->lfstate != LF_USED) {
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    /* Return SYSERR for an attempt to skip bytes beyond the byte */
    /*    that is current the end of the file                     */
    ldptr = lfptr->lfdirptr;
    if (lfptr->lfpos >= ldptr->ld_size) {
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    /* If pointer is outside current block, set up new block */
    if (lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ]) 
        lfsetup(lfptr);

    /* If appending a byte to the file, increment the file size.      */
    /*    Note: comparison might be equal, but should not be greater. */
    if (lfptr->lfpos >= ldptr->ld_size) {
        ldptr->ld_size++;
        Lf_data.lf_dirdirty = TRUE;
    }

    /* Place byte in buffer and mark buffer "dirty" */
    *lfptr->lfbyte++ = ch;
    lfptr->lfpos++;
    lfptr->lfdbdirty = TRUE;

    signal(lfptr->lfmutex);
    return OK;
}