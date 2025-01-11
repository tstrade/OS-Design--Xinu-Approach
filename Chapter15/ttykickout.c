#include "../inclusions.h"

void ttykickout(struct uart_csreg *csrptr)
{
  /* Force UART hardware to generate an output interrupt */
  csrptr->ier = UART_IER_ERBFI | UART_IER_ETBEI;
  return;
}
