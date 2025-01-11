#include "../inclusions.h"

devcall ttyputc(struct dentry *devptr, char ch)
{
  struct ttycblk *typtr;

  typtr = &ttytab[devptr->dvminor];

  /* Handle output CRLF by sending CR first */
  if (ch == TY_NEWLINE && typtr->tyocrlf)
    ttyputc(devptr, TY_RETURN);

  /* Wait for space in queue */
  wait(typtr->tyosem);
  *typtr->tyotail++ = ch;

  /* Wrap around to beginnger of buffer, if needed */
  if (typtr->tyotail >= &typtr->tyobuff[TY_OBUFLEN])
    typtr->tyotail = typtr->tyobuff;

  /* Start output in case device is idle */
  ttykickout((struct uart_csreg *)devptr->dvcsr);

  return OK;
}
