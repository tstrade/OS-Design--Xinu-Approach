#include "../inclusions.h"

devcall ttycontrol(struct dentry *devptr, int64 func, int64 arg1, int64 arg2)
{
  struct ttycblk *typtr;
  char ch;

  if (arg1 == arg2) {} // Just to get rid of stupid unused arg. error

  typtr = &ttytab[devptr->dvminor];

  switch (func) {
  case TC_NEXTC:
    wait(typtr->tyisem);
    ch = *typtr->tyitail;
    signal(typtr->tyisem);
    return (devcall)ch;

  case TC_MODER:
    typtr->tyimode = TY_IMRAW;
    return (devcall)OK;

  case TC_MODEC:
    typtr->tyimode = TY_IMCOOKED;
    return (devcall)OK;

  case TC_MODEK:
    typtr->tyimode = TY_IMCBREAK;
    return (devcall)OK;

  case TC_ICHARS:
    return (semcount(typtr->tyisem));

  case TC_ECHO:
    typtr->tyiecho = TRUE;
    return (devcall)OK;

  case TC_NOECHO:
    typtr->tyiecho = FALSE;
    return (devcall)OK;

  default:
    return (devcall)SYSERR;
  }
}
