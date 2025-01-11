#include "../inclusions.h"

devcall ttyread(struct dentry *devptr, char *buffer, uint32 count)
{
  struct ttycblk *typtr;
  uint32 avail;
  uint32 nread;
  uint32 firstch;
  char ch;

  if (count < 0)
    return SYSERR;

  typtr = &ttytab[devptr->dvminor];

  if (typtr->tyimode != TY_IMCOOKED) {
    /* For count of zero, return all available characters */
    if (count == 0) {
      avail = semcount(typtr->tyisem);
      if (avail == 0)
        return 0;
      else
        count = avail;
    }

    for (nread = 0; nread < count; nread++)
      *buffer++ = (char)ttygetc(devptr);

    return nread;
  }

  /* Block until input arrives */
  firstch = ttygetc(devptr);

  if (firstch == (uint32)EOF)
    return EOF;

  ch = (char)firstch;
  *buffer++ = ch;
  nread = 1;

  while ((nread < count) && (ch != TY_NEWLINE) && (ch != TY_RETURN)) {
    ch = ttygetc(devptr);
    *buffer++ = ch;
    nread++;
  }

  return nread;
}
