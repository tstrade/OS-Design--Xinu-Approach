#ifndef XINU_TYPES_H
#define XINU_TYPES_H

/* kernel.h */

/* GCC-specific varargs */

typedef __builtin_va_list va_list;

/* General type declarations used throughout the kernel */

/* Base type definitions */
typedef unsigned char uchar;    /* unsigned char type                 */
typedef unsigned short ushort;  /* unsigned short type                */
typedef unsigned int uint;      /* unsigned int type                  */
typedef unsigned long ulong;    /* unsigned long type                 */
typedef char bool;              /* boolean type                       */
typedef unsigned long size_t;   /* size type                          */

typedef	unsigned char	byte;
typedef long            int64;
typedef	int		        int32;
typedef	short		    int16;
typedef unsigned long   uint64;
typedef	unsigned int	uint32;
typedef	unsigned short	uint16;
typedef unsigned char   uint8;

/* Xinu-specific types */

typedef	int32	sid32;		/* semaphore ID				*/
typedef	int16	qid16;		/* queue ID				*/
typedef	int32	pid32;		/* process ID				*/
typedef	int32	did32;		/* device ID				*/
typedef	int16	pri16;		/* process priority			*/
typedef	uint32	umsg32;		/* message passed among processes	*/
typedef	int32	bpid32;		/* buffer pool ID			*/
typedef	byte	bool8;		/* Boolean type				*/
typedef	uint32	intmask;	/* saved interrupt mask			*/
typedef	int32	ibid32;		/* index block ID (used in file system)	*/
typedef	int32	dbid32;		/* data block ID (used in file system)	*/

/* Function declaration return types */

typedef int32	syscall;	/* system call declaration		*/
typedef int32	devcall;	/* device call declaration		*/
typedef int32	shellcmd;	/* shell command declaration		*/
typedef int32	process;	/* top-level function of a process	*/
typedef int32   device;
typedef	void	interrupt;	/* interrupt procedure			*/
typedef	int32	status;		/* returned status value (OK/SYSERR)	*/
typedef int32   thread;         /* thread declaration                   */
typedef void    exchandler;     /* exception procedure                  */
typedef int32   message;        /* message passing content              */

typedef int tid_typ;            /* thread ID type                       */

#define local	static		/* Local procedure or variable declar.	*/

/* Null pointer, character, and string definintions */
#define NULL	0		/* null pointer for linked lists	*/
#define NULLCH	'\0'		/* null character			*/
#define	NULLSTR	""		/* null string				*/

extern	qid16	readylist;	/* global ID for list of ready processes*/

#define	MINSTK	400		/* minimum stack size in bytes		*/

#define	CONTEXT	64		/* bytes in a function call context on	*/
				/* the run-time stack			*/
#define	QUANTUM	2		/* time slice in milliseconds		*/

/* Size of the stack for the null process */

#define	NULLSTK		8192	/* stack size for null process		*/

#define ARM_MODE_USR 0x10    /* Normal User Mode                                       */
#define ARM_MODE_FIQ 0x11    /* FIQ Processing Fast Interrupts Mode                    */
#define ARM_MODE_IRQ 0x12    /* IRQ Processing Standard Interrupts Mode                */
#define ARM_MODE_SVC 0x13    /* Supervisor Processing Software Interrupts Mode         */
#define ARM_MODE_ABT 0x17    /* Abort Processing memory Faults Mode                    */
#define ARM_MODE_UND 0x1B    /* Undefined Processing Undefined Instructions Mode       */
#define ARM_MODE_SYS 0x1F    /* System Running Priviledged Operating System Tasks Mode */
#define ARM_I_BIT 0x80    /* IRQs disabled when set to 1. */
#define ARM_F_BIT 0x40    /* FIQs disabled when set to 1. */

void nulluser(void);
syscall kprintf(const char *fmt, ...);
syscall kputc(char, device *);
syscall kgetc(device *);
syscall kvprintf(const char *fmt, va_list va);

#define min(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); (_a < _b) ? _a : _b; })
#define max(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); (_a > _b) ? _a : _b; })
#define DIV_ROUND_UP(num, denom) (((num) + (denom) - 1) / (denom))
#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))

/* macro to get offset to struct members */
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

/* Boolean type and constants */
#define FALSE        0          /* boolean false                      */
#define TRUE         1          /* boolean true                       */

/* Universal return constants */
#define OK        1             /* system call ok                     */
#define NULL      0             /* null pointer for linked lists      */
#define SYSERR   (-1)           /* system call failed                 */
#define EOF      (-2)           /* End-of-file (usually from read)    */
#define TIMEOUT  (-3)           /* system call timed out              */
#define NOMSG    (-4)           /* no message to receive              */


#endif
