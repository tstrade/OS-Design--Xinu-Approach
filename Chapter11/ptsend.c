#include "../inclusions.h"

syscall ptsend(int32 portid, umsg32 msg)
{
  intmask mask;
  struct ptentry *ptptr;
  int32 seq;
  struct ptnode *msgnode;
  struct ptnode *tailnode;

  mask = disable();
  if (isbadport(portid) || (ptptr = &porttab[portid])->ptstate != PT_ALLOC) {
    restore(mask);
    return SYSERR;
  }

  seq = ptptr->ptseq;
  if (wait(ptptr->ptssem) == SYSERR || ptptr->ptstate != PT_ALLOC || ptptr->ptseq != seq) {
    restore(mask);
    return SYSERR;
  }

  if (ptfree == NULL)
    panic("Port system ran out of message nodes");

  msgnode = ptfree;
  ptfree = msgnode->ptnext;
  msgnode->ptnext = NULL;
  msgnode->ptmsg = msg;

  tailnode = ptptr->pttail;
  if (tailnode == NULL) {
    ptptr->pttail = ptptr->pthead = msgnode;
  } else {
    tailnode->ptnext = msgnode;
    ptptr->pttail = msgnode;
  }

  signal(ptptr->ptrsem);
  restore(mask);
  return OK;
}
