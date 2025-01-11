#include "../inclusions.h"

void ttyhandler()
{
  struct dentry *devptr;
  struct ttycblk *typtr;
  struct uart_csreg *csrptr;
  /* Interrupt identification */
  byte iir = 0;
  /* Line status */
  // byte lsr = 0;

  /* Get CSR address of the device (assuming CONSOLE here) */
  devptr = (struct dentry *) &devtab[CONSOLE];
  csrptr = (struct uart_csreg *)devptr->dvcsr;

  /* Obtain a pointer to the tty control block */
  typtr = &ttytab[devptr->dvminor];

  /* Decode hardware interrupt request from UART device*/

  /* Check interrupt identification register */
  iir = csrptr->iir;
  if (iir & UART_IIR_IRQ)
    return;

  /* Decode the interrupt cause based upon the value extracted
   * from the UART interrupt identification register. Clear
   * the interrupt source and perform the appropriate handling
   * to coordinate with the upper half of the driver
   */

  iir &= UART_IIR_IDMASK;
  switch (iir) {
  /* Receiver line status interrupt (error) */
  case UART_IIR_RLSI:
    return;

  /* Receiver data available or timed out */
  case UART_IIR_RDA:
  case UART_IIR_RTO:
    resched_cntl(DEFER_START);

    /* While characters available in UARt buffer, call ttyinter_in */
    while ((csrptr->lsr & UART_LSR_DR) != 0)
      ttyhandle_in(typtr, csrptr);

    resched_cntl(DEFER_STOP);
    return;

  /* Transmitter output FIFO is empty (i.e., ready for more data) */
  case UART_IIR_THRE:
    ttyhandle_out(typtr, csrptr);
    return;

  /* Modem status change (simply ignore) */
  case UART_IIR_MSC:
    return;
  }
}
