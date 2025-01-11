#include "../inclusions.h"

void _ptclear(struct ptentry *ptptr, uint16 newstate, int32 (*dispose)(int32))
{
  struct ptnode *walk;

  ptptr->ptstate = PT_LIMBO;

  ptptr->ptseq++;
  walk = ptptr->pthead;

  if (walk != NULL) {
    for ( ; walk != NULL; walk = walk->ptnext)
      (*dispose)(walk->ptmsg);

    (ptptr->pttail)->ptnext = ptfree;
    ptfree = ptptr->pthead;
  }

  if (newstate == PT_ALLOC) {
    ptptr->pttail = ptptr->pthead = NULL;
    semreset(ptptr->ptssem, ptptr->ptmaxcnt);
    semreset(ptptr->ptrsem, 0);
  } else {
    semdelete(ptptr->ptssem);
    semdelete(ptptr->ptrsem);
  }

  ptptr->ptstate = newstate;
  return;
}
