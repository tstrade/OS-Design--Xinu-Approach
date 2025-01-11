#ifndef PMANAGE_H
#define PMANAGE_H

#include "../xinu_types.h"

/*
 * Delay units are in microseconds.
 */
#define	DELAY(n)                                	\
{                                               	\
	volatile long N = 2*n;				\
							\
	while(N > 0) {					\
		N--;					\
	}						\
}

/*
 * Delay units are in milli-seconds.
 */
#define	MDELAY(n)					\
{							\
	register long i;				\
							\
	for (i=n;i>0;i--) {				\
		DELAY(1000);				\
	}						\
}

pid32 getpid();
syscall suspend(pid32 pid);
pri16 resume(pid32 pid);
void xdone();
syscall kill(pid32 pid);
void userret();
pid32 create(void *funcaddr, uint64 ssize, pri16 priority, char *name, uint32 nargs, ...);
syscall getprio(pid32 pid);
pri16 chprio(pid32 pid, pri16 newprio);

#endif
