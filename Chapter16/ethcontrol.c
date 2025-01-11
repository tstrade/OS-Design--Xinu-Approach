#include "../inclusions.h"

devcall ethcontrol(struct dentry *devptr, int64 func, int64 arg1, int64 arg2)
{
  struct ethcblk *ethptr;
  int32 retval = OK;
  /* Stupid unused arg error */
  if (arg2)
    retval = OK;

  ethptr = &ethertab[devptr->dvminor];

  switch (func) {
  /* Get MAC address */
  case ETH_CTRL_GET_MAC:
    memcpy((byte *)arg1, ethptr->devAddress, ETH_ADDR_LEN);
    break;

  /* Add a multicast address */
  case ETH_CTRL_ADD_MCAST:
    retval = ethmcast_add(ethptr, (byte *)arg1);
    break;

    /* Remove a multicast address */
  case ETH_CTRL_REMOVE_MCAST:
    retval = ethmcast_remove(ethptr, (byte *)arg1);
    break;

  default:
    return SYSERR;
  }
  return retval;
}
