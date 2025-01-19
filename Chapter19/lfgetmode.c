#include "../inclusions.h"

int32 lfgetmode(char *mode)
{
    int32 mbits;
    char ch;

    mbits = 0;

    /* Mode string specifies                        */
    /*      r - read                                */
    /*      w - write                               */
    /*      o - old (existing file)                 */
    /*      n - new (create new file)               */

    while ((ch = *mode++) != NULLCH) {
        switch (ch) {
        case 'r':
            if (mbits & LF_MODE_R)
                return SYSERR;
            mbits |= LF_MODE_R;
            continue;
        case 'w':
            if (mbits & LF_MODE_W)
                return SYSERR;
            mbits |= LF_MODE_W;
            continue;
        case 'o':
            if (mbits & LF_MODE_O || mbits & LF_MODE_N)
                return SYSERR;
            mbits |= LF_MODE_O;
            break;    
        case 'n':
            if (mbits & LF_MODE_O || mbits & LF_MODE_N)
                return SYSERR;
            mbits |= LF_MODE_N;
            break;
        default:
            return SYSERR;
        }
    }

    /* If neither read or write specified, allow both */
    if ((mbits & LF_MODE_RW) == 0)
        mbits |= LF_MODE_RW;

    return mbits;
}