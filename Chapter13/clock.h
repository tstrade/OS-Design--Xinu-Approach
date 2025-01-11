#ifndef CLOCK_H
#define CLOCK_H

#include "../xinu_types.h"

#define CLKTICKS_PER_SEC  1000
#define CLOCK0 0xFF
#define CLKCNTL 0x00

extern qid16 sleepq;
extern uint32 clktime;
extern uint32 preempt;

status insertd(pid32 pid, qid16 q, int32 key);
syscall sleep(uint32 delay);
syscall sleepms(uint32 delay);
umsg32 recvtime(int32 maxwait);
status unsleep(pid32 pid);
void wakeup();


/* --------------------- icu.h placeholder ---------------------------- */

#define ICU1 0x20    /* address of primary interrupt control unit */
#define ICU2 0xA0    /* address of slave interrupt control unit   */

#define OCR1 ICU1     /* Operation Command Register for ICU1      */
#define OCR2 ICU2     /* Operation Command Register for ICU2      */
#define IMR1 (ICU1+1) /* Interrupt Mask Register for ICU1         */
#define IMR2 (ICU2+1) /* Interrupt Mask Register for ICU2         */

#define EOI 0x20     /* end-of-interrupt signal                   */

/* ---------------------- end placeholder ----------------------------- */


void clkdisp();
void clkhandler();
void clkinit_galileo();
void clkinit_beaglebone();

struct am335x_timer1ms {
  uint32 tidr;
  uint32 res1[3];
  uint32 tiocp_cfg;
  uint32 tistat;
  uint32 tisr;
  uint32 tier;
  uint32 twer;
  uint32 tclr;
  uint32 tcrr;
  uint32 tldr;
  uint32 ttgr;
  uint32 twps;
  uint32 tmar;
  uint32 tcar1;
  uint32 tsicr;
  uint32 tcar2;
  uint32 tpir;
  uint32 tnir;
  uint32 tcvr;
  uint32 tocr;
  uint32 towr;
};

#define AM335X_TIMER1MS_ADDR 0x44E31000
#define AM335X_TIMER1MS_IRQ 67
#define AM335X_TIMER1MS_TIOCP_CFG_SOFTRESET 0x00000002
#define AM335X_TIMER1MS_TISTAT_RESETDONE 0x00000001
#define AM335X_TIMER1MS_TISR_MAT_IT_FLAG 0x00000001
#define AM335X_TIMER1MS_TISR_OVF_IT_FLAG 0x00000002
#define AM335X_TIMER1MS_TISR_TCAR_IT_FLAG 0x00000004
#define AM335X_TIMER1MS_TIER_MAT_IT_ENA 0x00000001
#define AM335X_TIMER1MS_TIER_OVF_IT_ENA 0x00000002
#define AM335X_TIMER1MS_TIER_TCAR_IT_ENA 0x00000004
#define AM335X_TIMER1MS_TCLR_ST 0x00000001
#define AM335X_TIMER1MS_TCLR_AR 0x00000002
#define AM335X_TIMER1MS_CLKCTRL_ADDR 0x44E004C4
#define AM335X_TIMER1MS_CLKCTRL_EN 0x00000002

#endif
