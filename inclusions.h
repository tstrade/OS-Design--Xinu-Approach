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

/* ----- Chapter 17 ----- */
#include "Chapter17/arp.h"
#include "Chapter17/net.h"
#include "Chapter17/ip.h"
#include "Chapter17/udp.h"
#include "Chapter17/dhcp.h"

/* ----- Chapter 18 ----- */
#include "Chapter18/rdisksys.h"

/* ----- Chapter 19 ----- */
#include "Chapter19/lfilesys.h"

/* ----- Chapter 20 ----- */
#include "Chapter20/rfilesys.h"

/* ----- Chapter 21 ----- */

/* ----- Chapter 22 ----- */

/* ----- Chapter 23 ----- */

/* ----- Chapter 24 ----- */

/* ----- Chapter 25 ----- */

/* ----- Chapter 26 ----- */

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

#define F_MODE_R 0x01  /* Bit to grant read access	*/
#define F_MODE_W 0x02  /* Bit to grant write access	*/
#define F_MODE_RW 0x03 /* Mask for read and write bits	*/
#define F_MODE_N 0x04  /* Bit for "new" mode		*/
#define F_MODE_O 0x08  /* Bit for "old" mode		*/
                       /* Note: new and old mode bits 	*/
                       /*  are mutually exclusive	*/
#define F_MODE_NO 0x0C /* Mask for "n" and "o" bits	*/

/* Control functions for files */

#define F_CTL_DEL 1   /* Delete a file		*/
#define F_CTL_TRUNC 2 /* Truncate a file		*/
#define F_CTL_MKDIR 3 /* make a directory		*/
#define F_CTL_RMDIR 4 /* remove a directory		*/
#define F_CTL_SIZE 5  /* Obtain the size of a file	*/

#endif
