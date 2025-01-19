#include "../inclusions.h"

devcall lflget(struct dentry *devptr)
{
    struct lflcblk *lfptr;
    struct ldentry *ldptr;
    int32 onebyte;

    /* Obtain exclusive use of the file */
    lfptr = &lfltab[devptr->dvminor];
    wait(lfptr->lfmutex);

    /* If file is not open, return an error */
    if (lfptr->lfstate != LF_USED) {
        signal(lfptr->lfmutex);
        return SYSERR;
    }

    /* Return EOF for any attempt to read beyond the end-of-file */
    ldptr = lfptr->lfdirptr;
    if (lfptr->lfpos >= ldptr->ld_size) {
        signal(lfptr->lfmutex);
        return EOF;
    }

    /* If byte pointer is beyond the current data block, set up */
    /*     a new data block                                     */
    if (lfptr->lfbyte >= &lfptr->lfdblock[LF_BLKSIZ])
        lfsetup(lfptr);

    /* Extract the next byte from block, update file position, and */
    /*    return the byte to the caller                            */
    onebyte = 0xFF & *lfptr->lfbyte++;
    lfptr->lfpos++;
    signal(lfptr->lfmutex);
    return onebyte;
}