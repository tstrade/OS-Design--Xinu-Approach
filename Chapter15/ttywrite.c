#include "../inclusions.h"

devcall ttywrite(struct dentry *devptr, char *buffer, uint32 count)
{
  if (count < 0)
    return SYSERR;
  else if (count == 0)
    return OK;

  /* Write count characters one at a time */
  for ( ; count > 0; count--)
    ttyputc(devptr, *buffer++);

  return OK;
}
