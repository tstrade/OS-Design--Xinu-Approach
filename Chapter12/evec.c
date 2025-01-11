#include "../inclusions.h"

void irq_dispatch()
{
  struct intc_csreg *csrptr = (struct intc_csreg *)0x48200000;
  uint32 xnum;
  interrupt (*handler)(uint32);

  xnum = csrptr->sir_irq & 0x7F;

  if (intc_vector[xnum]) {
    handler = intc_vector[xnum];
    handler(xnum);
  }

  csrptr->control |= (INTC_CONTROL_NEWIRQAGR);
}
