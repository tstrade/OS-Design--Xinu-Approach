#include "../inclusions.h"

struct ttycblk ttytab[Ntty];

devcall ttyinit(struct dentry *devptr)
{
  struct ttycblk *typtr;
  struct uart_csreg *uptr;

  typtr = &ttytab[devptr->dvminor];

  typtr->tyihead = typtr->tyitail = &typtr->tyibuff[0];
  typtr->tyisem = semcreate(0);

  typtr->tyohead = typtr->tyotail = &typtr->tyobuff[0];
  typtr->tyosem = semcreate(TY_OBUFLEN);

  typtr->tyimode = TY_IMCOOKED;
  typtr->tyiecho = TRUE;
  typtr->tyieback = TRUE;
  typtr->tyevis = TRUE;
  typtr->tyecrlf = TRUE;
  typtr->tyicrlf = TRUE;
  typtr->tyierase = TRUE;
  typtr->tyierasec = TY_BACKSP;
  typtr->tyierasec2 = TY_BACKSP2;
  typtr->tyeofch = TY_EOFCH;
  typtr->tyikill = TRUE;
  typtr->tyikillc = TY_KILLCH;
  typtr->tyicursor = 0;
  typtr->tyoflow = TRUE;
  typtr->tyoheld = FALSE;
  typtr->tyostop = TY_STOPCH;
  typtr->tyostart = TY_STRTCH;
  typtr->tyocrlf = TRUE;
  typtr->tyifullc = TY_FULLCH;

  /* Initialize the UART */
  uptr = (struct uart_csreg *)devptr->dvcsr;

  /* Se baud rate */
  uptr->lcr = UART_LCR_DLAB;
  uptr->dlm = 0x00;
  uptr->dll = 0x18;

  /* 8 bit character, No Parity, 1 Stop */
  uptr->lcr = UART_LCR_8N1;
  /* Disable FIFO for now */
  uptr->fcr = 0x00;

  /* Register interrupt dispatcher for the tty device */
  set_evec(devptr->dvirq, (uint64)devptr->dvintr);

  /* Enable interrupts on the device: reset the transmit and
   * receive FIFOS, and set the interrupt trigger level
   */
  uptr->fcr = UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRESET | UART_FCR_TRIG2;

  /* Start the device */
  ttykickout(uptr);
  return OK;
}
