#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "../xinu_types.h"

#ifndef NSEM
#define NSEM 120
#endif

#define S_FREE 0
#define S_USED 1

struct sentry
{
    byte sstate;
    int32 scount;
    qid16 squeue;
};

extern struct sentry semtab[];

#define isbadsem(s) ((int32)(s) < 0 || (s) >= NSEM)

syscall wait(sid32 sem);
syscall signal(sid32 sem);
sid32 semcreate(int32 count);
syscall semdelete(sid32 sem);
syscall semreset(sid32 sem, int32 count);
syscall semcount(sid32 sem);
syscall signaln(sid32 sem, int32 count);

#endif
