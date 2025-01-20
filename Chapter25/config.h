#ifndef CONFIG_H
#define CONFIG_H

#include "../xinu_types.h"

#define CSR     0
#define IRQ     1
#define INTR    2
#define READ    3
#define WRITE   4
#define GETC    5
#define PUTC    6
#define OPEN    7  
#define CLOSE   8
#define INIT    9
#define SEEK    10
#define CONTROL 11

/* Prototypes */
int yylex(void) { /* temp */ return 0; };
void yyerror(const char *s);
int lookup(const char *str);
void newattr(int tok, int val);
int cktname(int symid);
void mktype(int deviceid);
void initattr(struct dev_ent *fstr, int tnum, int deviceid);
void mkdev(int nameid, int typid, int deviceid);
int ckdname(int devid);

#endif