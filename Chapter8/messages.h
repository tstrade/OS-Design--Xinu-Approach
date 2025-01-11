#ifndef MESSAGES_H
#define MESSAGES_H

#include "../xinu_types.h"

syscall send(pid32 pid, umsg32 msg);
umsg32 receive();
umsg32 recvclr();

#endif
