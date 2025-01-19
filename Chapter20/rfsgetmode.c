#include "../inclusions.h"

int32 rfsgetmode(char *mode)
{
    int32 mbits;
    char ch;
    mbits = 0;

    while ((ch = *mode++) != NULLCH) {
        switch (ch) {
        case 'r':
            if (mbits & RF_MODE_R)
                return SYSERR;
            mbits |= RF_MODE_R;
            continue;

        case 'w':
            if (mbits & RF_MODE_W)
                return SYSERR;
            mbits |= RF_MODE_W;
            continue;

        case 'o':
            if (mbits & RF_MODE_O || mbits & RF_MODE_N)
                return SYSERR;
            mbits |= RF_MODE_O;
            break;

        case 'n':
            if (mbits & RF_MODE_O || mbits & RF_MODE_N)
                return SYSERR;
            mbits |= RF_MODE_N;
            break;

        default:
            return SYSERR;
        }
    }

    /* If neither read nor write specified, allow both */
    if ((mbits & RF_MODE_RW) == 0)
        mbits |= RF_MODE_RW;

    return mbits;
}