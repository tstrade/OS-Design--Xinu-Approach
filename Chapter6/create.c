#include "../inclusions.h"

local int newpid();

pid32 create(void *funcaddr, uint64 ssize, pri16 priority, char *name, uint32 nargs, ...)
{
  uint32 savsp, *pushsp;
  intmask mask;
  pid32 pid;
  struct procent *prptr;
  int32 i;
  uint32 *a;
  uint32 *saddr;

  mask = disable();
  if (ssize < MINSTK)
    ssize = MINSTK;

  ssize = (uint64)roundmb(ssize);
  if ((priority < 1) || ((pid = newpid()) == SYSERR) || ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR)) {
    restore(mask);
    return SYSERR;
  }

  prcount++;
  prptr = &proctab[pid];

  /* Initialize process table entry for new process */
  prptr->prstate = PR_SUSP;         /* Initial state is suspended */
  prptr->prprio = priority;
  prptr->prstkbase = (char *)saddr;
  prptr->prstklen = ssize;
  prptr->prname[PNMLEN - 1] = NULLCH;
  for (i = 0; i < PNMLEN - 1 && (prptr->prname[i] = name[i]) != NULLCH; i++)
    ;
  prptr->prsem = -1;
  prptr->prparent = (pid32)getpid();
  prptr->prhasmsg = FALSE;

  /* Set up file descriptors */
  prptr->prdesc[0] = 0; // stdin
  prptr->prdesc[1] = 1; // stdout
  prptr->prdesc[2] = 2; // stderr

  /* Initialize stack as if the process was called */
  *saddr = STACKMAGIC;
  savsp = (long)saddr;

  /* Push arguments */
  a = (uint32 *)(&nargs + 1);   /* Start of args */
  a += nargs - 1;               /* Last argument */
  for ( ; nargs > 0; nargs--)   /* Copy args onto new process's stack */
        *--saddr = *a--;
  *--saddr = (long)INITRET;     /* Push return address on stack */

  /* Following entries on the stack must match what ctxsw
     expects a saved process state to contain: return address,
     ebp, interrupt mask, flags, registers, and an old SP
  */
  *--saddr = (long)funcaddr; /* Make stack look like it's halway through a call to ctxsw that
                                "returns" to the new process
                             */

  *--saddr = savsp;          /* This will be register ebp (machine-dependent) for process exit */
  savsp = (long)saddr;       /* Start of frame for ctxsw */
  *--saddr = 0x00000200;     /* New process runs with interrupts enabled */

  *--saddr = 0;      /* %eax */
  *--saddr = 0;      /* %ecx */
  *--saddr = 0;      /* %edx */
  *--saddr = 0;      /* %ebx */
  *--saddr = 0;      /* %esp; value filled in below */
  pushsp = saddr;    /* Remember this location */
  *--saddr = savsp;  /* %ebp (while finishing ctxsw) */
  *--saddr = 0;      /* %esi */
  *--saddr = 0;      /* %edi */

  *pushsp = (unsigned long)(prptr->prstkptr = (char *)saddr);
  restore(mask);
  return pid;
}


local pid32 newpid()
{
  uint32 i;
  static pid32 nextpid = 1;

  for (i = 0; i < NPROC; i++) {
    nextpid %= NPROC;
    if (proctab[nextpid].prstate == PR_FREE)
      return nextpid++;
    else
      nextpid++;
  }

  return (pid32)SYSERR;
}
