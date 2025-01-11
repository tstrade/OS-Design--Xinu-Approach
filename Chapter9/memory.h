#ifndef MEMORY_H
#define MEMORY_H

#include "../xinu_types.h"

#define PAGE_SIZE 4096

#define roundmb(x) (char *)((7 + (uint64)(x)) & (~7))
#define truncmb(x) (char *)((uint64(x)) & (~7))

#define freestk(p,len) freemem((char *)((uint64)(p) \
                                        - ((uint64)roundmb(len)) \
                                        + (uint64)sizeof(uint64)), \
                               (uint64)roundmb(len))

struct memblk {
  struct memblk *mnext;
  uint32 mlength;
};

extern struct memblk memlist;
extern void *minheap;
extern void *maxheap;

extern int text;
extern int etext;
extern int data;
extern int edata;
extern int bss;
extern int ebss;
extern int end;

char *getmem(uint64 nbytes);
char *getstk(uint64 nbytes);
syscall freemem(char *blkaddr, uint64 nbytes);

#endif
