#include "../inclusions.h"

devcall ttygetc(struct dentry *devptr)
{
  char ch;
  struct ttycblk *typtr;

  typtr = &ttytab[devptr->dvminor];

  wait(typtr->tyisem);
  ch = *typtr->tyihead++;

  if (typtr->tyihead >= &typtr->tyibuff[TY_IBUFLEN])
    typtr->tyihead = typtr->tyibuff;

  if ((typtr->tyimode == TY_IMCOOKED) && (typtr->tyeof) && (ch == typtr->tyeofch))
    return (devcall)EOF;

  return (devcall)ch;
}
