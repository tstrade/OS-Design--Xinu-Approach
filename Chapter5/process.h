#ifndef PROCESS_H
#define PROCESS_H

#include "../xinu_types.h"

#ifndef NPROC
#define NPROC 8
#endif

#define PR_FREE    0
#define PR_CURR    1
#define PR_READY   2
#define PR_RECV    3
#define PR_SLEEP   4
#define PR_SUSP    5
#define PR_WAIT    6
#define PR_RECTIM  7

#define PNMLEN     16
#define NULLPROC   0

#define INITSTK    65536
#define INITPRIO   20
#define INITRET    userret

#define isbadpid(x) (((pid32)(x) < 0) || \
		     ((pid32)(x) >= NPROC) || \
		     (proctab[(x)].prstate == PR_FREE))

#define NDESC      5

struct procent {
  uint16 prstate;
  pri16 prprio;
  char *prstkptr;
  char *prstkbase;
  uint32 prstklen;
  char prname[PNMLEN];
  sid32 prsem;
  pid32 prparent;
  umsg32 prmsg;
  bool8 prhasmsg;
  int16 prdesc[NDESC];
  qid16 th_readylist;
};

#define STACKMAGIC 0x0A0AAAA9
#define DEFER_START 1
#define DEFER_STOP 0

extern struct procent proctab[];
extern int32 prcount;
extern pid32 currpid;

struct defer {
  int16 ndefers;
  int16 attempt;
};

extern struct defer Defer;

void resched();
status resched_cntl(int32 defer);
status ready(pid32 pid);
void ctxsw(char **old_prstkptr, char **new_prstkptr);

#endif
