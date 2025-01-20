#include "../inclusions.h"

void panic(char *msg)
{
    disable();
    kprintf("\n\n\rpanic: %s\n\n", msg);
    while (TRUE);
}