#ifndef TTY_H
#define TTY_H

#include "../xinu_types.h"
#include "uart.h"
#include "../Chapter14/device.h"

#define TY_OBMINSP    20
#define TY_EBUFLEN    20

#ifndef Ntty
#define Ntty          1
#endif

#ifndef TY_IBUFLEN
#define TY_IBUFLEN    128
#endif

#ifndef TY_OBUFLEN
#define TY_OBUFLEN    64
#endif


#define TY_IMRAW      'R'
#define TY_IMCOOKED   'C'
#define TY_IMCBREAK   'K'
#define TY_OMRAW      'R'

struct ttycblk {
  char   *tyihead;              // Next input char to read
  char   *tyitail;              // Next slot for arriving char
  char   tyibuff[TY_IBUFLEN];   // Input buffer
  sid32  tyisem;                // Input semaphore
  char   *tyohead;              // Next output char to xmit
  char   *tyotail;              // Next slot for outgoing char
  char   tyobuff[TY_OBUFLEN];   // Output buffer
  sid32  tyosem;                // Output semaphore
  char   *tyehead;              // Next echo char to xmit
  char   *tyetail;              // Next slot to deposit echo char
  char   tyebuff[TY_EBUFLEN];   // Echo buffer
  char   tyimode;               // Input mode raw/cbreak/cooked
  bool8  tyiecho;               // Is input echoed?
  bool8  tyieback;              // Do erasing backspace on echo?
  bool8  tyevis;                // Echo control chars as ^X ?
  bool8  tyecrlf;               // Echo CR-LF for newline?
  bool8  tyicrlf;               // Map '\r' to '\n' on input?
  bool8  tyierase;              // Honor erase character?
  char   tyierasec;             // Primary erase character
  char   tyierasec2;            // Alternate erase character
  bool8  tyeof;                 // Honor EOF character?
  char   tyeofch;               // EOF character (usually ^D)
  bool8  tyikill;               // Honor line kill character?
  char   tyikillc;              // Line kill character
  int32  tyicursor;             // Current cursor position
  bool8  tyoflow;               // Honor ostop/ostart?
  bool8  tyoheld;               // Output currently being held?
  char   tyostop;               // Character that stops output
  char   tyostart;              // Character that starts output
  bool8  tyocrlf;               // Output CR/LF for LF ?
  char   tyifullc;              // Char to send when input full
};

extern struct ttycblk ttytab[];

#define TY_BACKSP   '\b'
#define TY_BACKSP2  '\177'
#define TY_BELL     '\07'
#define TY_EOFCH    '\04'
#define TY_BLANK    ' '
#define TY_NEWLINE  '\n'
#define TY_RETURN   '\r'
#define TY_STOPCH   '\023'
#define TY_STRTCH   '\021'
#define TY_KILLCH   '\025'
#define TY_UPARROW  '^'
#define TY_FULLCH   TY_BELL

#define TC_NEXTC    3
#define TC_MODER    4
#define TC_MODEC    5
#define TC_MODEK    6
#define TC_ICHARS   8
#define TC_ECHO     9
#define TC_NOECHO   10

devcall ttyread(struct dentry *devptr, char *buffer, uint32 count);
devcall ttycontrol(struct dentry *devptr, int64 func, int64 arg1, int64 arg2);
devcall ttygetc(struct dentry *devptr);
devcall ttyputc(struct dentry *devptr, char ch);
devcall ttywrite(struct dentry *devptr, char *buffer, uint32 count);
devcall ttyinit(struct dentry *devptr);

void ttykickout(struct uart_csreg *csrptr);
void ttydispatch();
void ttyhandler();
void ttyhandle_in(struct ttycblk *ttyptr, struct uart_csreg *csrptr);
void ttyhandle_out(struct ttycblk *ttyptr, struct uart_csreg *csrptr);




#endif
