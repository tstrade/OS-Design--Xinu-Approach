#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "../xinu_types.h"

/* Place holder macro and struct - not listed in textbook */
#define INTC_CONTROL_NEWIRQAGR 1

#ifndef IRQBASE
#define	IRQBASE 32 /* base ivec for IRQ0                  */
#endif

#define IRQ0     0 /* programmable interval timer         */
#define IRQ1     1 /* keyboard controller                 */
#define IRQ2     2 /* unknown                             */
#define IRQ3     3 /* serial port 2                       */
#define IRQ4     4 /* serial port 1                       */
#define IRQ5     5 /* hard disk controller                */
#define IRQ6     6 /* floppy disk controller              */
#define IRQ7     7 /* parallel port / spurious interrupts */

#define	IGDT_TASK	 5	/* task gate IDT descriptor       */
#define	IGDT_INTR	14	/* interrupt gate IDT descriptor  */
#define	IGDT_TRAPG	15	/* Trap Gate                      */

#define	NID  48

struct intc_csreg {
  uint32 sir_irq;
  uint32 control;
};

#ifndef __ASSEMBLER__

typedef interrupt (*interrupt_handler_t)(uint32);
extern interrupt_handler_t intc_vector[];

intmask disable();
intmask restore(intmask mask);
void enable();
void dispatch(int, int *);

void halt();
void pause();

intmask enable_irq(intmask);
intmask disable_irq(intmask);

syscall irq_except();
void init_evec(void);
void set_evec(int, int);
void set_handler(int, interrupt (*)(void));

#endif

void irq_dispatch();


#endif
