#ifndef LFILESYS_H
#define LFILESYS_H

#include "../xinu_types.h"

/********************************************************************************************/
/*                                                                                          */
/*                            Local File System Data Structures                             */
/*                                                                                          */
/*    A local file system uses a random-access disk composed of 512-byte sectores numbered  */
/* 0 through N-1.  We assume disk hardware can read or write any sector at random, but must */
/* transfer an entire sector.  Thus, to write a few bytes, the file system must read the    */
/* sector, replace the bytes, and then write the sector back to disk.  Xinu's local file    */
/* system divides the disk as follows: sector 0 is a directory, the next K sectors          */
/* constitute an index area, and the remaining sectors comprise a data area.  The data area */
/* is easiest to understand: the sector holds one data block (d-block) that stores contents */
/* from one of the files (or is on a free list of unused data blocks).  We think of the     */
/* index area as holding an array of index blocks (i-blocks) numbered 0 through I-1.  A     */
/* given sector in the index area holds 7 index blocks, which are each 72 bytes long. Given */
/* an i-block number, the file system must calculate the disk sector in which the i-block   */
/* is located and the byte offset within the sector at which the i-block resides.           */
/* Internally, a file is known by the i-block index of the first i-block for the file. The  */
/* directory contains a list of file names and the i-block number of the first i-block for  */
/* the file.  The directory also holds the i-block number for a list of free i-blocks and a */
/* data block number of the firsts data block on a list of free data blocks.                */
/*                                                                                          */
/********************************************************************************************/

#ifndef Nlfl
#define Nffl 1
#endif

/* Use the remote disk device if no disk is defined (file system *assumes* the underlying */
/* disk has a block size of 512 bytes)                                                    */

#ifndef LF_DISK_DEV
#define LF_DISK_DEV       SYSERR
#endif

/* Temporary until defined elsewhere */
#define F_MODE_R 0
#define F_MODE_W 1
#define F_MODE_RW 2
#define F_MODE_O 3
#define F_MODE_N 4
/*************************************/

#define LF_MODE_R         F_MODE_R         /* Mode bit for "read"           */
#define LF_MODE_W         F_MODE_W         /* Mode bit for "write"          */
#define LF_MODE_RW        F_MODE_RW        /* Mode bit for "read/write"     */
#define LF_MODE_O         F_MODE_O         /* Mode bit for "old"            */
#define LF_MODE_N         F_MODE_N         /* Mode bit for "new"            */

#define LF_BLKSIZ         512              /* Block size for local file sys  */
#define LF_NAME_LEN       16               /* Length of file name plus nil   */
#define LF_NUM_DIR_ENT    20               /* Number of entries in directory */

#define LF_FREE           0                /* Slave device is available      */
#define LF_USED           1                /* Slave device is in use         */

#define LF_INULL          (ibid32) -1      /* Index block null pointer       */
#define LF_DNULL          (dbid32) -1      /* Data block null pointer        */
#define LF_IBLEN          16               /* Data blocks ptrs per i-block   */
#define LF_IDATA          8192             /* Bytes of data indexed by i-b   */

#define LF_IMASK          0x00001FFF       /* Mask for index block number    */
#define LF_DMASK          0x00001FFF       /* Mask for data block number     */

#define LF_AREA_IB       1                 /* First sector of i-blocks       */
#define LF_AREA_DIR      0                 /* First sector of directory      */

#define DFILL            0                 /* Fill value for data blocks     */

struct lfiblk {
    ibid32 ib_next;
    uint32 ib_offset;
    dbid32 ib_dba[LF_IBLEN];
};

/* Conversion functions below assume 7 index blocks per disk block */

/* Conversion between index block number and disk sector number */
#define ib2sect(ib)      (((ib)/7) + LF_AREA_IB)

/* Conversion between index block number and relative offset within disk sector */
#define ib2disp(ib)      (((ib)%7) * sizeof(struct lfiblk))

struct ldentry {
    uint32 ld_size;
    ibid32 ld_ilist;
    char ld_name[LF_NAME_LEN];
};

struct lfdbfree {
    dbid32 lf_nextdb;
    char lf_unused[LF_BLKSIZ - sizeof(dbid32)];
};

#pragma pack(2)
struct lfdir {
    dbid32 lfd_dfree;
    ibid32 lfd_ifree;
    int32  lfd_nfiles;
    struct ldentry lfd_files[LF_NUM_DIR_ENT];
    char   padding[20];
};
#pragma pack()

struct lfdata {
    did32  lf_dskdev;
    sid32  lf_mutex;
    struct lfdir lf_dir;
    bool8  lf_dirpresent;
    bool8  lf_dirdirty;
};

struct lflcblk {
    byte    lfstate;
    did32   lfdev;
    sid32   lfmutex;
    struct  ldentry *lfdirptr;
    int32   lfmode;
    uint32  lfpos;
    char    lfname[LF_NAME_LEN];
    ibid32  lfinum;
    struct  lfiblk lfiblock;
    dbid32  lfdnum;
    char    lfdblock[LF_BLKSIZ];
    char    *lfbyte;
    bool8   lfibdirty;
    bool8   lfdbdirty;
};

extern struct lfdata Lf_data;
extern struct lflcblk lfltab[];

/* Control functions */

/* Temporary until defined elsewhere */
#define F_CTL_DEL   1
#define F_CTL_TRUNC 2
#define F_CTL_SIZE  3
/*************************************/

#define LF_CTL_DEL   F_CTL_DEL
#define LF_CTL_TRUNC F_CTL_TRUNC
#define LF_CTL_SIZE  F_CTL_SIZE

/* Prototypes */
void lfibclear(struct lfiblk *ibptr, int32 offset);
void lfibget(did32 diskdev, ibid32 inum, struct lfiblk *ibuff);
status lfibput(did32 diskdev, ibid32 inum, struct lfiblk *ibuff);
ibid32 lfiballoc();
dbid32 lfdballoc(struct lfdbfree *dbuff);
status lfdbfree(did32 diskdev, dbid32 dnum);
devcall lfsopen(struct dentry *devptr, char *name, char *mode);
int32 lfgetmode(char *mode);
devcall lflclose(struct dentry *devptr);
void lfflush(struct lflcblk *lfptr);

#endif