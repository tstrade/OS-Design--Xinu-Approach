#include "../inclusions.h"

interrupt ethhandler()
{
  struct ethcblk *ethptr;
  struct eth_q_csreg *csrptr;
  struct eth_q_tx_desc *tdescptr;
  struct eth_q_rx_desc *rdescptr;
  volatile uint32 sr;
  uint32 count;

  ethptr = &ethertab[devtab[ETHER0].dvminor];
  csrptr = (struct eth_q_csreg *)ethptr->csr;

  /* Copy the status register into a local variable */
  sr = csrptr->sr;

  /* If there is no interrupt pending, return */
  if ((csrptr->sr & ETH_QUARK_SR_NIS) == 0)
    return;

  /* Acknowledge the interrupt */
  csrptr->sr = sr;

  /* Check status register to figure out the source of interrupt */
  /* Transmit interrupt */
  if (sr & ETH_QUARK_SR_TI) {
    /* Pointer to the head of transmit desc ring */
    tdescptr = (struct eth_q_tx_desc *)ethptr->txRing + ethptr->txHead;

    /* Start packet count at zero */
    count = 0;

    /* Repeat until we process all the descriptor slots */
    while (ethptr->txHead != ethptr->txTail) {
      /* If the descriptor is owned by DMA, stop here */
      if (tdescptr->ctrlstat & ETH_QUARK_TDCS_OWN)
        break;

      /* Descriptor was processed; increment count */
      count++;

      /* Go to the next descriptor */
      tdescptr += 1;

      /* Increment the head of the transmit desc ring */
      ethptr->txHead += 1;
      if (ethptr->txHead >= ethptr->txRingSize) {
        ethptr->txHead = 0;
        tdescptr = (struct eth_q_tx_desc *)ethptr->txRing;
      }
    }
    /* 'count' packets were processed by DMA, and slots are
     * now free; signal the semaphore accordingly
     */
    signaln(ethptr->osem, count);
  }

  /* Receive interrupt */
  if (sr & ETH_QUARK_SR_RI) {
    /* Get pointer to the tail of the receive desc list */
    rdescptr = (struct eth_q_rx_desc *)ethptr->rxRing + ethptr->rxTail;
    count = 0;

    /* Repeat until we have receive maximum no. packets
     * that can fit in queue
     */
    while (count <= ethptr->rxRingSize) {
      /* If the descriptor is owned by the DMA, stop */
      if (rdescptr->status & ETH_QUARK_RDST_OWN)
        break;

      /* Descriptor was processed; increment count */
      count++;

      /* Go to the next descriptor */
      rdescptr += 1;

      /* Increment the tail index of the rx desc ring */
      ethptr->rxTail += 1;
      if (ethptr->rxTail >= ethptr->rxRingSize) {
        ethptr->rxTail = 0;
        rdescptr = (struct eth_q_rx_desc *)ethptr->rxRing;
      }
    }
    /* 'count' packets were receive and are available,
     * so signal the semaphore accordingly
     */
    signaln(ethptr->isem, count);
  }
  return;
}
