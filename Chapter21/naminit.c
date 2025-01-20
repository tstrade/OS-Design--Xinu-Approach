#include "../inclusions.h"

#ifndef RFILESYS
#define RFILESYS  SYSERR
#endif 

#ifndef FILESYS   
#define FILESYS   SYSERR
#endif

#ifndef LFILESYS
#define LFILESYS  SYSERR
#endif

struct nmentry nametab[NNAMES];
int32 nnames;

status naminit()
{
    did32 i;
    struct dentry *devptr;
    char tmpstr[NM_MAXLEN];
    status retval;
    char *tptr;
    char *nptr;
    char devprefix[] = "/dev/";
    int32 len;
    char ch;

    /* Set prefix table to empty */
    nnames = 0;
    for (i = 0; i < NDEVS; i++) {
        tptr = tmpstr;
        nptr = devprefix;

        /* Copy prefix into tmpstr */
        len = 0;
        while ((*tptr++ = *nptr++) != NULLCH)
            len++;
        
        /* Move pointer to position before NULLCH */
        tptr--;
        devptr = &devtab[i];
        nptr = devptr->dvname;

        /* Map device name to lower case and append */
        while (++len < NM_MAXLEN) {
            ch = *nptr++;
            if ((ch >= 'A') && (ch <= 'Z'))
                ch += 'a' - 'A';
            if ((*tptr++ = ch) == NULLCH)
                break;
        }

        if (len > NM_MAXLEN) {
            kprintf("namespace: device name %s too long\r\n", devptr->dvname);
            continue;
        }

        retval = mount(tmpstr, NULLSTR, devptr->dvnum);
        if (retval == SYSERR) {
            kprintf("namespace: cannot mount device %d\r\n", devptr->dvnum);
            continue;
        }
    }
    /* Add other prefixes (longest prefix first) */
    mount("/dev/null",  "",         NULLDEV);
    mount("/remote/",   "remote:",  RFILESYS);
    mount("/local/",    NULLSTR,    LFILESYS);
    mount("/dev/",      NULLSTR,    SYSERR);
    mount("~/",         NULLSTR,    LFILESYS);
    mount("/",          "root:",    RFILESYS);
    mount("",           "",         LFILESYS);

    return OK;
}