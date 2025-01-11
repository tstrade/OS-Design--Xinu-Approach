#ifndef UART_H
#define UART_H

#include "../xinu_types.h"
#include "../Chapter7/semaphore.h"
#include "../Chapter14/device.h"

struct uart_csreg
{
  volatile uchar buffer;
  volatile uchar ier;
  volatile uchar iir;
  volatile uchar lcr;
  volatile uchar mcr;
  volatile uchar lsr;
  volatile uchar msr;
  volatile uchar scr;
};

/* Alternative names for control and status registers                    */
#define rbr buffer
#define thr buffer
#define fcr iir
#define dll buffer
#define dlm ier

/* UART Bit flags for control and status registers                       */
/* Interrupt enable bits                                                 */
#define UART_IER_ERBFI  0x01
#define UART_IER_ETBEI  0x02
#define UART_IER_ELSI   0x04
#define UART_IER_EMSI   0x08

/* Interrupt identification masks */
#define UART_IIR_IRQ    0x01
#define UART_IIR_IDMASK 0x0E
#define UART_IIR_MSC    0x00
#define UART_IIR_THRE   0x02
#define UART_IIR_RTO    0x04
#define UART_IIR_RLSI   0x06
#define UART_IIR_RDA    0x0C

/* FIFO control bits */
#define UART_FCR_EFIFO  0x01
#define UART_FCR_RRESET 0x02
#define UART_FCR_TRESET 0x04
#define UART_FCR_TRIG0  0x00
#define UART_FCR_TRIG1  0x40
#define UART_FCR_TRIG2  0x80
#define UART_FCR_TRIG3  0xC0

/* Line control bits */
#define UART_LCR_DLAB   0x80
#define UART_LCR_8N1    0x03

/* Modem control bits */
#define UART_MCR_OUT2   0x08
#define UART_MCR_LOOP   0x10

/* Line status bits */
#define UART_LSR_DR     0x01
#define UART_LSR_THRE   0x20
#define UART_LSR_TEMT   0x40
#define UART_FIFO_LEN   64

/* UART Buffer lengths */
#define UART_IBLEN      1024
#define UART_OBLEN      1024

/* UART 16550 control block */
struct uart
{
  /* Pointers to associated structures */
  struct uart_csreg *csr;
  device            *dev;
  /* Statistical Counts */
  long               cout;
  long               cin;
  long               lserr;
  long               ovrrn;
  long               iirq;
  long               oirq;
  /* UART input fields */
  uchar              iflags;
  sid32              isema;
  ushort             istart;
  ushort             icount;
  uchar              in[UART_IBLEN];
  /* UART output fields */
  uchar              oflags;
  sid32              osema;
  ushort             ostart;
  ushort             ocount;
  uchar              out[UART_OBLEN];
  bool               oidle;
};

extern struct uart uarttab[];

/* UART input flags */
#define UART_IFLAG_NOBLOCK      0x0001
#define UART_IFLAG_ECHO         0x0002

/* UART output flags */
#define UART_OFLAG_NOBLOCK      0x0001

/* uartControl() functions  */
#define UART_IOC_SETIFLAG       0x0010
#define UART_IOC_CLRIFLAG       0x0011
#define UART_IOC_GETIFLAG       0x0012
#define UART_IOC_SETOFLAG       0x0013
#define UART_IOC_CLROFLAG       0x0014
#define UART_IOC_GETOFLAG       0x0015
#define UART_IOC_OUTPUT_IDLE    0x0016

/* Driver functions */

devcall uartinit(device *);
devcall uartread(device *, unsigned char *, int);
devcall uartwrite(device *, unsigned char *, int);
devcall uartgetc(device *);
devcall uartputc(device *, unsigned char);
devcall uartcontrol(device *, int, unsigned char, unsigned char);
interrupt uartintr(void);

#endif
