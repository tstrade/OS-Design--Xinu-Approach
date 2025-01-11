#include "../inclusions.h"

syscall semcount(sid32 sem)
{
    if (isbadsem(sem))
    {
        return SYSERR;
    }

    return (semtab[sem].scount);
}
