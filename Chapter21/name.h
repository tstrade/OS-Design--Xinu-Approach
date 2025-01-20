#ifndef NAME_H
#define NAME_H

/* Constants that define namespace mapping table sizes */
#define NM_PRELEN       64          /* Max size of a prefix string  */
#define NM_REPLLEN      96          /* Max size of replacement      */
#define NM_MAXLEN       256         /* Max size of file name        */
#define NNAMES          40          /* Number of prefixes           */

struct nmentry {
    char  nprefix[NM_PRELEN];       /* Null-terminated prefixed     */
    char  nreplace[NM_REPLLEN];    /* Null-terminated replacement  */
    did32 ndevice;                  /* Device descriptor for prefix */
};

extern struct nmentry nametab[];
extern int32 nnames;                /* Number of entries allocated  */

/* Prototypes */
syscall mount(char *prefix, char *replace, did32 device);
int32 namlen(char *name, int32 maxlen);
devcall nammap(char *name, char newname[NM_MAXLEN], did32 namdev);
did32 namrepl(char *name, char newname[NM_MAXLEN]);
status namcpy(char *newname, char *oldname, int32 buflen);
devcall namopen(struct dentry *devptr, char *name, char *mode);
status naminit();

#endif