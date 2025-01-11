#ifndef INCLUSIONS_H
#define INCLUSIONS_H

#include "xinu_types.h"
#include <linux/kernel.h>
#include <string.h>
#include <stdlib.h>

/* ----- Chapter 4 ------ */
#include "Chapter4/queue.h"

/* ----- Chapter 5 ----- */
#include "Chapter5/process.h"

/* ----- Chapter 6 ----- */
#include "Chapter6/pmanage.h"

/* ----- Chapter 7 ----- */
#include "Chapter7/semaphore.h"

/* ----- Chapter 8 ----- */
#include "Chapter8/messages.h"

/* ----- Chapter 9 ----- */
#include "Chapter9/memory.h"

/* ----- Chapter 10 ----- */
#include "Chapter10/bufpool.h"

/* ----- Chapter 11 ----- */
#include "Chapter11/ports.h"

/* ----- Chapter 12 ----- */
#include "Chapter12/interrupts.h"

/* ----- Chapter 13 ----- */
#include "Chapter13/clock.h"

/* ----- Chapter 14 ----- */
#include "Chapter14/conf.h"
#include "Chapter14/device.h"

/* ----- Chapter 15 ----- */
#include "Chapter15/tty.h"
#include "Chapter15/uart.h"

/* ----- Chapter 16 ----- */
#include "Chapter16/quark_eth.h"
#include "Chapter16/ether.h"
#include "Chapter16/net.h"


/* ----- Miscellaneous ----- */
int panic(char *msg);

syscall yield()
{
  intmask mask;

  mask = disable();
  resched();
  restore(mask);
  return OK;
}

#endif
