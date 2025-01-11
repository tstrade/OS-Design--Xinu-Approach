#include "../inclusions.h"

/* Placeholder macros - first arg of outb() indicates valid 8-bit port ID */
#define CLOCK0 0xFF
#define CLKCNTL 0x00

uint32 clktime;
uint32 ctrl1000 = 0;
qid16 sleepq;
uint32 preempt;

void clkinit_galileo()
{
  uint16 intv;

  /* Allocate queue to hold delta list */
  sleepq = newqueue();

  /* Initialize globals */
  preempt = QUANTUM;
  clktime = 0;

  /* Set interrupt vector for clock to invoke clkdisp */
  set_evec(IRQBASE, (uint64)clkdisp);

  /* Set hardware clock: timer 0, 16-bit counter, rate, generator mode, and counter runs in binary */
  outb(CLKCNTL, 0x34);

  /* Set clock rate to 1.190 MHz - 1 ms interrupt rate */
  intv = 1193;

  /* Must rate LSB first, then MSB */
  outb(CLOCK0, (char)(0xFF & intv));
  outb(CLOCK0, (char)(0xFF & (intv >> 8)));

  return;
}

void clkinit_beaglebone()
{
  volatile struct am335x_timer1ms *csrptr = (volatile struct am335x_timer1ms *)AM335X_TIMER1MS_ADDR;
  volatile uint32 *clkctrl = (volatile uint32 *)AM335X_TIMER1MS_CLKCTRL_ADDR;

  *clkctrl = AM335X_TIMER1MS_CLKCTRL_EN;
  while ((*clkctrl) != 0x2);

  /* Reset timer module */
  csrptr->tiocp_cfg |= AM335X_TIMER1MS_TIOCP_CFG_SOFTRESET;

  /* Wait until reset is complete */
  while ((csrptr->tistat & AM335X_TIMER1MS_TISTAT_RESETDONE) == 0);

  /* Set interrupt vector for clock to invoke clkint */
  set_evec(AM335X_TIMER1MS_IRQ, (uint64)clkhandler);

  /* Allocate queue to hold delta list */
  sleepq = newqueue();

  /* Initialize globals */
  preempt = QUANTUM;
  clktime = 0;

  /* Follow values calculated for 1 ms tick rate */
  csrptr->tpir = 1000000;
  csrptr->tnir = 0;
  csrptr->tldr = 0xFFFFFFFF - 26000;

  /* Set timer to auto reload */
  csrptr->tclr = AM335X_TIMER1MS_TCLR_AR;

  /* Start the timer */
  csrptr->tclr |= AM335X_TIMER1MS_TCLR_ST;

  /* Enable overflow interrupt which interrupt every 1 ms */
  csrptr->tier = AM335X_TIMER1MS_TIER_OVF_IT_ENA;

  /* Kickstart the timer */
  csrptr->ttgr = 1;

  return;
}
