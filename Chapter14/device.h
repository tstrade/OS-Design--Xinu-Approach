#ifndef DEVICE_H
#define DEVICE_H

#include "conf.h"

#define isbaddev(f)  ( !(0 <= (f) && (f) < NDEVS) )

syscall read(did32 descrp, char *buffer, uint32 count);
syscall control(did32 descrp, int64 func, int64 arg1, int64 arg2);
syscall getc(did32 descrp);
syscall putc(did32 descrp, char ch);
syscall seek(did32 descrp, uint32 pos);
syscall write(did32 descrp, char *buffer, uint32 count);
syscall init(did32 descrp);
syscall open(did32 descrp, char *name, char *mode);
syscall close(did32 descrp);
devcall ionull();
devcall ioerr();

#endif
