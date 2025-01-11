#include "../inclusions.h"

void ttyhandle_out(struct ttycblk *typtr, struct uart_csreg *csrptr)
{
  uint32 ochars;
  uint32 avail;
  uint32 uspace;
  byte ier = 0;

  /* If output is current held, simply ignore the call */
  if (typtr->tyoheld)
    return;

  /* If echo and output queues empty, turn of interrupts */
  if ((typtr->tyehead == typtr->tyetail) && (semcount(typtr->tyosem) >= TY_OBUFLEN)) {
    ier = csrptr->ier;
    csrptr->ier = ier & ~UART_IER_ETBEI;
    return;
  }

  /* Initialize uspace to the size of the transmit FIFO */
  uspace = UART_FIFO_LEN;

  /* While onboard FIFO is not fill and the echo queue is
   * nonempty, xmit characters from the echo queue
   */

  while ((uspace > 0) && typtr->tyehead != typtr->tyetail) {
    csrptr->buffer = *typtr->tyehead++;

    if (typtr->tyehead >= &typtr->tyebuff[TY_EBUFLEN])
      typtr->tyehead = typtr->tyebuff;

    uspace--;
  }

  /* While onboard FIFO is not fill and the output queue is
   * nonempty, transmit chars from the output queue
   */

  ochars = 0;
  avail = TY_OBUFLEN - semcount(typtr->tyosem);
  while ((uspace > 0) && (avail > 0)) {
    csrptr->buffer = *typtr->tyohead++;

    if (typtr->tyohead >= &typtr->tyobuff[TY_OBUFLEN])
      typtr->tyohead = typtr->tyobuff;

    avail--;
    uspace--;
    ochars++;
  }

  if (ochars > 0)
    signaln(typtr->tyosem, ochars);

  return;
}
