#ifndef PORTS_H
#define PORTS_H

#include "../xinu_types.h"

#define NPORTS   30
#define PT_MSGS  100
#define PT_FREE  1
#define PT_LIMBO 2
#define PT_ALLOC 3

struct ptnode {
  uint32 ptmsg;
  struct ptnode *ptnext;
};

struct ptentry {
  sid32 ptssem;
  sid32 ptrsem;
  uint16 ptstate;
  uint16 ptmaxcnt;
  uint32 ptseq;
  struct ptnode *pthead;
  struct ptnode *pttail;
};

extern struct ptnode *ptfree;
extern struct ptentry porttab[];
extern int32 ptnextid;

#define isbadport(portid) ((portid) < 0 || (portid) >= NPORTS)

syscall ptinit(int32 maxmsgs);
syscall ptcreate(int32 count);
syscall ptsend(int32 portid, umsg32 msg);
uint32 ptrecv(int32 portid);
syscall ptdelete(int32 portid, int32 (*disp)(int32));
syscall ptreset(int32 portid, int32 (*disp)(int32));
void _ptclear(struct ptentry *ptptr, uint16 newstate, int32 (*dispose)(int32));

void outb(int port, char data);

#endif
