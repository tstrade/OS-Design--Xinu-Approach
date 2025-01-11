#include "../inclusions.h"

struct bpentry buftab[NBPOOLS];
bpid32 nbpools;

status bufinit()
{
  nbpools = 0;
  return OK;
}
