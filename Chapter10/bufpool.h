#ifndef BUFPOOL_H
#define BUFPOOL_H

#include "../xinu_types.h"

#ifndef NBPOOLS
#define NBPOOLS 20
#endif

#ifndef BP_MAXB
#define BP_MAXB 8192
#endif

#ifndef BP_MINB
#define BP_MINB 8
#endif

struct bpentry {
  struct bpentry *bpnext;
  sid32 bpsem;
  uint32 bpsize;
};

extern struct bpentry buftab[];
extern bpid32 nbpools;

char *getbuf(bpid32 poolid);
syscall freebuf(char *bufaddr);
bpid32 mkbufpool(int32 bufsiz, int32 numbufs);
status bufinit();

#endif
