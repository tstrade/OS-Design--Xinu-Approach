#include "../inclusions.h"

struct ethcblk ethertab[1];

/* ----------------------------------------------------------
 *    eth_phy_read() - Ready a PHY register
 * ----------------------------------------------------------
 */
uint16 eth_phy_read(volatile struct eth_q_csreg *csrptr, uint32 regnum)
{
  uint32 retries;

  while (csrptr->gmiiar & ETH_QUARK_GMIIAR_GB);

  /* Prepare the GMII address register for read transaction */
  csrptr->gmiiar =
    (1 << 11)             | /* Physical Layer Address = 1   */
    (regnum << 6)         | /* PHY Register Number          */
    (ETH_QUARK_GMIIAR_CR) | /* GMII Clock Range 100-150 MHz */
    (ETH_QUARK_GMIIAR_GB);  /* Start the transaction        */

  /* Wait for transaction to complete */
  retries = 0;
  while(csrptr->gmiiar & ETH_QUARK_GMIIAR_GB) {
    DELAY(ETH_QUARK_INIT_DELAY);
    if ((++retries) > ETH_QUARK_MAX_RETRIES)
      return 0;
  }

  /* Transaction is complete, read the PHY register value from
   * the GMII data register
   */
  return (uint16)csrptr->gmiidr;
}

/* ---------------------------------------------------------------
 *    eth_phy_write() - Write a PHY register
 * ---------------------------------------------------------------
 */
void eth_phy_write(volatile struct eth_q_csreg *csrptr, uint32 regnum, uint16 value)
{
  uint32 retries;

  /* Wait for the MII to be ready */
  while (csrptr->gmiiar & ETH_QUARK_GMIIAR_GB);

  /* Write the value to GMII data register */
  csrptr->gmiidr = (uint32)value;

  /* Prepare the GMII address register for write transaction */
  csrptr->gmiiar =
    (1 << 11)             | /* Physical Layer Address = 1   */
    (regnum << 6)         | /* PHY Register Number          */
    (ETH_QUARK_GMIIAR_CR) | /* GMII Clock Range 100-150 Mhz */
    (ETH_QUARK_GMIIAR_GW) | /* Write transaction            */
    (ETH_QUARK_GMIIAR_GB);  /* Start the transaction        */

  /* Wait til the transaction is complete */
  retries = 0;
  while (csrptr->gmiiar & ETH_QUARK_GMIIAR_GB) {
    DELAY(ETH_QUARK_INIT_DELAY);
    if ((++retries) > ETH_QUARK_MAX_RETRIES)
      return;
  }
}

/* ----------------------------------------------------------------
 *    eth_phy_reset() - Reset an Ethernet PHY
 * ----------------------------------------------------------------
 */
int32 eth_phy_reset(volatile struct eth_q_csreg *csrptr)
{
  uint16 value;
  uint32 retries;

  /* Read the PHY control register (register 0) */
  value = eth_phy_read(csrptr, 0);

  /* Set bit 15 in control register to reset the PHY */
  eth_phy_write(csrptr, 0, (value | 0x8000));

  /* Wait for PHY reset process to complete */
  retries = 0;
  while (eth_phy_read(csrptr, 0) & 0x8000) {
    DELAY(ETH_QUARK_INIT_DELAY);
    if ((++retries) > ETH_QUARK_MAX_RETRIES)
      return SYSERR;
  }

  /* See if the PHY has auto-negotiation capability */
  value = eth_phy_read(csrptr, 1); /* PHY Status register */

  /* Auto-negotiation capable */
  if (value & 0x0008) {
    /* Wait for the auto-negotiation process to complete */
    retries = 0;
    while ((eth_phy_read(csrptr, 1) & 0x0020) == 0) {
      DELAY(ETH_QUARK_INIT_DELAY);
      if ((++retries) > ETH_QUARK_INIT_DELAY)
        return SYSERR;
    }
  }

  /* Wait for the Link to be Up */
  retries = 0;
  while ((eth_phy_read(csrptr, 1) & 0x0004) == 0) {
    DELAY(ETH_QUARK_INIT_DELAY);
    if ((++retries) > ETH_QUARK_MAX_RETRIES)
      return SYSERR;
  }

  DELAY(ETH_QUARK_INIT_DELAY);

  kprintf("Ethernet Link is Up\n");

  return OK;
}

/* -----------------------------------------------------------------
 *    ethinit() - Initialize the Intel Quark Ethernet Device
 * -----------------------------------------------------------------
 */
int32 ethinit(struct dentry *devptr)
{
  struct ethcblk *ethptr;
  volatile struct eth_q_csreg *csrptr;
  struct eth_q_tx_desc *tx_descs;
  struct eth_q_rx_desc *rx_descs;
  struct netpacket *pktptr;
  void *temptr;
  uint32 retries;
  int32 retval;
  uint32 i;

  ethptr = &ethertab[devptr->dvminor];

  ethptr->csr = (struct eth_q_csreg *)devptr->dvcsr;
  csrptr = (struct eth_q_csreg *)ethptr->csr;

  /* Enable CSR Memory Space, Enable Bus Master */
  pci_write_config_word(ethptr->pcidev, 0x4, 0x0006);


  /* Reset the PHY */
  retval = eth_phy_reset(csrptr);
  if (retval == SYSERR)
    return SYSERR;

  /* Reset the Ethernet MAC */
  csrptr->bmr |= ETH_QUARK_BMR_SWR;

  /* Wait for the MAC Reset process to complete */
  retries = 0;
  while (csrptr->bmr & ETH_QUARK_BMR_SWR) {
    DELAY(ETH_QUARK_INIT_DELAY);
    if ((++retries) > ETH_QUARK_MAX_RETRIES)
      return SYSERR;
  }

  /* Transmit Store and Forward */
  csrptr->omr |= ETH_QUARK_OMR_TSF;

  /* Set the interrupt handler */
  set_evec(devptr->dvirq, (uint64)devptr->dvintr);

  /* Set the MAC Speed = 100Mbps, Full Duplex mode */
  csrptr->maccr |= (ETH_QUARK_MACCR_RMIISPD100 | ETH_QUARK_MACCR_DM);

  /* Reset the MAC Counters */
  csrptr->mmccr |= ETH_QUARK_MMC_CNTFREEZ | ETH_QUARK_MMC_CNTRST;

  /* Retrieve the MAC address from SPI flash */
  get_quark_pdat_entry_data_by_id(QUARK_MAC1_ID, (char *)(ethptr->devAddress), ETH_ADDR_LEN);

  kprintf("MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n",
          0xFF & ethptr->devAddress[0],
          0xFF & ethptr->devAddress[1],
          0xFF & ethptr->devAddress[2],
          0xFF & ethptr->devAddress[3],
          0xFF & ethptr->devAddress[4],
          0xFF & ethptr->devAddress[5]);

  /* Add the MAC address read from SPI flash into the macaddr regs. for addr. filtering */
  csrptr->macaddr0l = (uint32)(*((uint32 *)ethptr->devAddress));
  csrptr->macaddr0h = ((uint32)(*((uint16 *)(ethptr->devAddress + 4))) | 0x80000000);

  ethptr->txRingSize = ETH_QUARK_TX_RING_SIZE;

  /* Allocate memory for the transmit ring */
  temptr = (void *)getmem(sizeof(struct eth_q_tx_desc) * (ethptr->txRingSize + 1));
  if ((int64)temptr == SYSERR)
    return SYSERR;

  memset(temptr, 0, sizeof(struct eth_q_tx_desc) * (ethptr->txRingSize + 1));

  /* The transmit descriptors need to be 4-byte aligned */
  ethptr->txRing = (void *)(((uint64)temptr + 3) & (~3));

  /* Allocate memory for transmit buffers */
  ethptr->txBufs = (void *)getmem(sizeof(struct netpacket) * (ethptr->txRingSize + 1));
  if ((int64)ethptr->txBufs == SYSERR)
    return SYSERR;

  ethptr->txBufs = (void *)(((uint64)ethptr->txBufs + 3) & (~3));

  /* Pointers to initialize transmit descriptors */
  tx_descs = (struct eth_q_tx_desc *)ethptr->txRing;
  pktptr = (struct netpacket *)ethptr->txBufs;

  /* Initialize the transmit descriptors */
  for (i = 0; i < ethptr->txRingSize; i++)
    tx_descs[i].buffer1 = (uint64)(pktptr + i);

  /* Create the output synchronization semaphore */
  ethptr->osem = semcreate(ethptr->txRingSize);
  if ((int64)ethptr->osem == SYSERR)
    return SYSERR;

  ethptr->rxRingSize = ETH_QUARK_RX_RING_SIZE;

  /* Allocate memory for the receive descriptors */
  temptr = (void *)getmem(sizeof(struct eth_q_rx_desc) * (ethptr->rxRingSize + 1));
  if ((int64)temptr == SYSERR)
    return SYSERR;

  memset(temptr, 0, sizeof(struct eth_q_rx_desc) * (ethptr->rxRingSize + 1));

  /* Receive descriptors must be 4-byte aligned */
  ethptr->rxRing = (struct eth_q_rx_desc *)(((uint64)temptr + 3) & (~3));

  /* Allocate memory for the receive buffers */
  ethptr->rxBufs = (void *)getmem(sizeof(struct netpacket) * (ethptr->rxRingSize + 1));
  if ((int64)ethptr->rxBufs == SYSERR)
    return SYSERR;

  /* Receive buffers must be 4-byte aligned */
  ethptr->rxBufs = (void *)(((uint64)ethptr->rxBufs + 3) & (~3));

  /* Pointer to initialize receive descriptors */
  rx_descs = (struct eth_q_rx_desc *)ethptr->rxRing;

  /* Pointer to data buffers */
  pktptr = (struct netpacket *)ethptr->rxBufs;

  /* Initialize the receive descriptors */
  for (i = 0; i < ethptr->rxRingSize; i++) {
    rx_descs[i].status = ETH_QUARK_RDST_OWN;
    rx_descs[i].buf1size = (uint32)sizeof(struct netpacket);
    rx_descs[i].buffer1 = (uint64)(pktptr + i);
  }

  /* Indicate end of ring on last descriptor */
  rx_descs[ethptr->rxRingSize - 1].buf1size |= (ETH_QUARK_RDCTL1_RER);

  /* Create the input synchronization semaphore */
  ethptr->isem = semcreate(0);
  if ((int64)ethptr->isem == SYSERR)
    return SYSERR;

  /* Enable the Transmit and Receive Interrupts */
  csrptr->ier = (ETH_QUARK_IER_NIE |
                 ETH_QUARK_IER_TIE |
                 ETH_QUARK_IER_RIE);

  /* Initialize the transmit descriptor base address */
  csrptr->tdla = (uint64)ethptr->txRing;

  /* Initialize the receive descriptor base address */
  csrptr->rdla = (uint64)ethptr->rxRing;

  /* Enable the MAC Receiver and Transmitter */
  csrptr->maccr |= (ETH_QUARK_MACCR_TE | ETH_QUARK_MACCR_RE);

  /* Start the Transmit and Receive Processes in the DMA */
  csrptr->omr |= (ETH_QUARK_OMR_ST | ETH_QUARK_OMR_SR);

  return OK;
}
