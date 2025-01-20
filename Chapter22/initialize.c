#include "../inclusions.h"
#include "start.s"

extern void start();
extern void *_end;

/* Function prototypes */
extern void main();
extern void xdone();
static void sysinit();
extern void meminit();

/* Declarations of major kernel variables */
struct procent proctab[NPROC];
struct sentry semtab[NSEM];
struct memblk memlist;

/* Active system status */
int prcount;
pid32 currpid;

/* --------------------------------------------------------------------------------------
 *    nulluser() - Initialize the system and become the null process
 *
 *  Note: execution begins here after the C run-time environment has been established.
 *    Interrupts are intially DISABLED, and must eventually be enabled explicitly. The
 *    code turns itself into the null process after initialization. Because it must 
 *    always remain ready to execute, the null process cannot execute code that might
 *    cause it to be suspended, wait for a semaphore, put to sleep, or exit. In
 *    particular, the code must not perform I/O except for polled versions such
 *    as kprintf.
 * --------------------------------------------------------------------------------------
 */
void nulluser()
{
    struct memblk *memptr;
    uint32 free_mem;

    sysinit();

    kprintf("\n\r%s\n\n\r", VERSION);

    /* Output Xinu memory layout */
    free_mem = 0;
    for (memptr = memlist.mnext; memptr != NULL; memptr = memptr->mnext)
        free_mem += memptr->mlength;
    kprintf("%10d bytes of free memory. Free list:\n", free_mem);

    for (memptr = memlist.mnext; memptr != NULL; memptr = memptr->mnext)
        kprintf("        [0x%08X to 0x%08X]\r\n", 
            (uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);

    kprintf("%10d bytes of Xinu code.\n", (uint32)&etext - (uint32)&text);
    kprintf("        [0x%08X to 0x%08X]\r\n", (uint32)&text, (uint32)&etext - 1);
    kprintf("%10d bytes of data.\n", (uint32)&ebss - (uint32)&data);
    kprintf("        [0x%08X to 0x%08X]\r\n", (uint32)&data, (uint32)&ebss - 1);

    /* Enable interrupts */
    enable();

    /* Create process to execute function main() */
    resume(create((void *)main, INITSTK, INITPRIO, "Main process", 0, NULL));

    /* Become the Null process (i.e., guarantee that the CPU has      */
    /*    something to run when no other process is ready to execute) */
    while (TRUE);
}

/* ---------------------------------------------------------------------------------
 *    sysinit() - Initialize all Xinu data structures and devices
 * ---------------------------------------------------------------------------------
 */
static void sysinit()
{
    int32 i;
    struct procent *prptr;
    struct sentry *semptr;

    /* Platform Specific Initialization */
    platinit();

    /* Initialize interrupt vectors */
    initevec();

    /* Initialize free memory list */
    meminit();

    /* Initialize system variables */
    prcount = 1;
    Defer.ndefers = 0;
    
    for (i = 0; i < NPROC; i++) {
        prptr = &proctab[i];
        prptr->prstate = PR_FREE;
        prptr->prname[0] = NULLCH;
        prptr->prstkbase = NULL;
        prptr->prprio = 0;
    }

    prptr = &proctab[NULLPROC];
    prptr->prstate = PR_CURR;
    prptr->prprio = 0;
    strncpy(prptr->prname, "prnull", 7);
    prptr->prstkbase = getstk(NULLSTK);
    prptr->prstklen = NULLSTK;
    prptr->prstkptr = 0;
    currpid = NULLPROC;

    for (i = 0; i < NSEM; i++) {
        semptr = &semtab[i];
        semptr->sstate = S_FREE;
        semptr->scount = 0;
        semptr->squeue = newqueue();
    }

    bufinit();
    readylist = newqueue();

    clkinit();
    for (i = 0; i < NDEVS; i++)
        init(i);

    return;
}

int32 stop(char *s)
{
    kprintf("%s\n", s);
    kprintf("looping... press reset\n");
    while (1);
}

int32 delay(int n)
{
    DELAY(n);
    return OK;
}