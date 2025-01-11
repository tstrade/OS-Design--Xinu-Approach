#include "../inclusions.h"

devcall ethwrite(struct dentry *devptr, char *buf, int32 len)
{
  struct ethcblk *ethptr;
  struct eth_q_csreg *csrptr;
  volatile struct eth_q_tx_desc *descptr;
  int32 i;

  ethptr = &ethertab[devptr->dvminor];
  csrptr = (struct eth_q_csreg *)ethptr->csr;

  /* Wait for an empty slot in the transmit descriptor ring */
  wait (ethptr->osem);

  /* Point to the tail of the descriptor ring */
  descptr = (struct eth_q_tx_desc *)ethptr->txRing + ethptr->txTail;

  /* Increment that tail index and wrap, if needed */
  ethptr->txTail += 1;
  if (ethptr->txTail >= ethptr->txRingSize)
    ethptr->txTail = 0;

  /* Add packet length to the descriptor */
  descptr->buf1size = len;

  /* Copy packet into the buffer associated with the descriptor */
  for (i = 0; i < len; i++)
    *((char *)descptr->buffer1 + i) = *((char *)buf + i);

  /* Mark the descriptor if we are at the end of the ring */
  if (ethptr->txTail == 0)
    descptr->ctrlstat = ETH_QUARK_TDCS_TER;
  else
    descptr->ctrlstat = 0;

  /* Initialize the descriptor */
  descptr->ctrlstat |=
    (ETH_QUARK_TDCS_OWN | /* The desc is owned by DMA     */
     ETH_QUARK_TDCS_IC  | /* Interrupt after transfer     */
     ETH_QUARK_TDCS_LS  | /* Last segment of packet       */
     ETH_QUARK_TDCS_FS);  /* First segment of packet      */

  /* Un-suspend DMA on the device */
  csrptr->tpdr = 1;

  return OK;
}
