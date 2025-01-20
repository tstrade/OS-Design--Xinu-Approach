#include "../inclusions.h"

int32 *marks[MAXMARK];
int32 nmarks;
sid32 mkmutex;

/* --------------------------------------------------
 *    markinit() - Called once at system startup 
 * --------------------------------------------------
 */
void markinit()
{
    nmarks = 0;
    mkmutex = semcreate(1);
}

/* -----------------------------------------------------
 *    mark() - Mark a specified memory location 
 * -----------------------------------------------------
 */
status mark(int32 *loc)
{
    if ((*loc >= 0) && (*loc < nmarks) && (marks[*loc] == loc))
        return OK;

    if (nmarks >= MAXMARK)
        return SYSERR;

    wait(mkmutex);
    marks[(*loc) = nmarks++] = loc;
    signal(mkmutex);
    return OK;
}