#include "../inclusions.h"

/* -----------------------------------------------------------------------------------------
 *    mount() - Add a prefix mapping to the name space
 * -----------------------------------------------------------------------------------------
 */
syscall mount(char *prefix, char *replace, did32 device)
{
    intmask mask;
    struct nmentry *namptr;
    int32 psiz, rsiz;
    int32 i;

    mask = disable();

    psiz = namlen(prefix, NM_PRELEN);
    rsiz = namlen(replace, NM_REPLLEN);
    if((psiz == SYSERR) || (rsiz == SYSERR) || (isbaddev(device)) || (nnames >= NNAMES)) {
        restore(mask);
        return SYSERR;
    }

    namptr = &nametab[nnames];
    for (i = 0; i < psiz; i++)
        namptr->nprefix[i] = *prefix++;

    for (i = 0; i < rsiz; i++)
        namptr->nreplace[i] = *replace++;

    namptr->ndevice = device;
    nnames++;

    restore(mask);
    return OK;
}

/* --------------------------------------------------------------------
 *    namlen() - Compute the length of a string stopping at maxlen 
 * --------------------------------------------------------------------
 */
int32 namlen(char *name, int32 maxlen)
{
    int32 i;
    for (i = 0; i < maxlen; i++)
        if (*name++ == NULLCH)
            return i + 1;       /* Include NULLCH in length */

    return SYSERR;
}