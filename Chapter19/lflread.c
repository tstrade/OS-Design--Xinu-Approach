#include "../inclusions.h"

devcall lflread(struct dentry *devptr, char *buff, int32 count)
{
    uint32 numread;
    int32 nxtbyte;
    if (count < 0)
        return SYSERR;

    /* Iterate and use lflgetc to read individual bytes */
    for (numread = 0; numread < count; numread++) {
        nxtbyte = lflgetc(devptr);
        if (nxtbyte == SYSERR) {
            return SYSERR;
        } else if (nxtbyte == EOF) {
            if (numread == 0) {
                return EOF;
            } else {
                return numread;
            }
        } else {
            *buff++ = (char)(0xFF & nxtbyte);
        }
    }
    return numread;
}