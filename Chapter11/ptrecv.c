#include "../inclusions.h"

uint32 ptrecv(int32 portid)
{
  intmask mask;
  struct ptentry *ptptr;
  int32 seq;
  umsg32 msg;
  struct ptnode *msgnode;

  mask = disable();
  if (isbadport(portid) || (ptptr = &porttab[portid])->ptstate != PT_ALLOC) {
    restore(mask);
    return (uint32)SYSERR;
  }

  seq = ptptr->ptseq;
  if (wait(ptptr->ptrsem) == SYSERR || ptptr->ptstate != PT_ALLOC || ptptr->ptseq != seq) {
    restore(mask);
    return (uint32)SYSERR;
  }

  msgnode = ptptr->pthead;
  msg = msgnode->ptmsg;
  if (ptptr->pthead == ptptr->pttail)
    ptptr->pthead = ptptr->pttail = NULL;
  else
    ptptr->pthead = msgnode->ptnext;

  msgnode->ptnext = ptfree;
  ptfree = msgnode;
  signal(ptptr->ptssem);
  restore(mask);
  return msg;
}
