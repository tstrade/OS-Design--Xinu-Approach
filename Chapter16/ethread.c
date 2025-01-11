#include "../inclusions.h"

devcall ethread(struct dentry *devptr, char *buf, int32 len)
{
  struct ethcblk *ethptr;
  struct eth_q_rx_desc *rdescptr;
  struct netpacket *pktptr;
  int32 framelen = 0;
  bool8 valid_addr;
  int32 i;

  ethptr = &ethertab[devptr->dvminor];

  while (1) {
    /* Wait until there is a packet in the receive queue */
    wait(ethptr->isem);

    /* Point to the head of the descriptor list */
    rdescptr = (struct eth_q_rx_desc *)ethptr->rxRing + ethptr->rxHead;
    pktptr = (struct netpacket *)rdescptr->buffer1;

    /* See if destination address is our unicast address */
    if (!memcmp(pktptr->net_ethdst, ethptr->devAddress, 6))
      valid_addr = TRUE;

    /* See if destination address is the broadcast address */
    else if (!memcmp(pktptr->net_ethdst, NetData.ethbcast, 6))
      valid_addr = TRUE;

    /* For multicast addresses, see if we should accept */
    else {
      valid_addr = FALSE;
      for (i = 0; i < (ethptr->ed_mcc); i++) {
        if (memcmp(pktptr->net_ethdst, ethptr->ed_mca[i], 6) == 0) {
          valid_addr = TRUE;
          break;
        }
      }
    }

    /* Accept this packet */
    if (valid_addr == TRUE) {
      framelen = (rdescptr->status >> 16) & 0x00003FFF;

      /* Only return len characters to caller */
      if (framelen > len)
        framelen  = len;

      /* Copy the packet into the caller's buffer */
      memcpy(buf, (void *)rdescptr->buffer1, framelen);
    }

    /* Increment the head of the descriptor list */
    ethptr->rxHead += 1;
    if (ethptr->rxHead >= ETH_QUARK_RX_RING_SIZE)
      ethptr->rxHead = 0;

    /* Reset the descrtor to max possible frame len */
    rdescptr->buf1size = sizeof(struct netpacket);

    /* If we reach the end of the ring, mark the descriptor */
    if (ethptr->rxHead == 0)
      rdescptr->rdctl1 |= (ETH_QUARK_RDCTL1_RER);

    /* Indicate that the descriptor is ready for DMA input */
    rdescptr->status = ETH_QUARK_RDST_OWN;
    if (valid_addr == TRUE)
      break;
  }

  /* Return number of bytes returned from the packet */
  return framelen;
}
