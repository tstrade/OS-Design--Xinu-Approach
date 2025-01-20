#include "../inclusions.h"

/* --------------------------------------------------------------------------
 *    nammap() - Using namespace, map name to new name and new device 
 * --------------------------------------------------------------------------
 */
devcall nammap(char *name, char newname[NM_MAXLEN], did32 namdev)
{
    did32 newdev;
    char tmpname[NM_MAXLEN];
    int32 iter;

    /* Place original name in temporary buffer and null terminate */
    if (namcpy(tmpname, name, NM_MAXLEN) == SYSERR)
        return SYSERR;

    /* Repeatedly substitute the name prefix until a non-namespace */
    /*    device is reached or an iteration limit is exceeded      */
    for (iter = 0; iter < nnames; iter++) {
        newdev = namrepl(tmpname, newname);
        if (newdev != namdev) {
            namcpy(tmpname, newname, NM_MAXLEN);
            return newdev;
        }
    }
    return SYSERR;
}

/* ------------------------------------------------------------------------------
 *    namrepl() - Use the name table to perform prefix substitution 
 * ------------------------------------------------------------------------------
 */
did32 namrepl(char *name, char newname[NM_MAXLEN])
{
    int32 i;
    char *pptr;
    char *rptr;
    char *optr;
    char *nptr;
    char olen;
    int32 plen;
    int32 rlen;
    int32 remain;
    struct nmentry *namptr;

    /* Search name table for first prefix that matches */
    for (i = 0; i < nnames; i++) {
        namptr = &nametab[i];
        optr = name;
        pptr = namptr->nprefix;

        /* Compare prefix to string and count prefix size */
        for (plen = 0; *pptr != NULLCH; plen++) {
            if (*pptr != *optr)
                break;
            pptr++;
            optr++;
        }

        if (*pptr != NULLCH)
            continue;

        /* Found a match - check that replacement string plus   */
        /* bytes remaining at the end of the original name will */
        /* fit into new name buffer. Ignore null on replacement */
        /* string, but keep null on remainder of name.          */
        olen = namlen(name, NM_MAXLEN);
        rlen = namlen(namptr->nreplace, NM_MAXLEN) - 1;
        remain = olen - plen;
        if ((rlen + remain) > NM_MAXLEN)
            return (did32)SYSERR;

        /* Place replacement string followed by remainder of    */
        /*    original name (and null) into new name buffer     */
        nptr = newname;
        rptr = namptr->nreplace;
        for ( ; rlen > 0; rlen--)
            *nptr++ = *rptr++;
        for ( ; remain > 0; remain--)
            *nptr++ = *optr++;
        return namptr->ndevice;
    }
    return (did32)SYSERR;
}

/* -------------------------------------------------------------------------
 *    namcpy() - Copy a name from one buffer to another, checking length  
 * -------------------------------------------------------------------------
 */
status namcpy( char *newname, char *oldname, int32 buflen)
{
    char *nptr;
    char *optr;
    int32 cnt;

    nptr = newname;
    optr = oldname;

    for (cnt = 0; cnt < buflen; cnt++)
        if ((*nptr++ = *optr++) == NULLCH)
            return OK;

    /* Buffer filled before copy completed */
    return SYSERR;
}
