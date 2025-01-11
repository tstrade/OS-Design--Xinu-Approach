#ifndef ETHER_H
#define ETHER_H

#include "../xinu_types.h"
#include "quark_eth.h"
#include "../Chapter14/device.h"

/* Ethernet packet format:

   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                     |                    |          |                               |
   |    Dest. MAC (6)    |    Src. MAC (6)    | Type (2) |       Data (46-1500) ...      |
   |                     |                    |          |                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

#define ETH_ADDR_LEN  6                    /* Len. of Ethernet (MAC) addr. */
typedef unsigned char Eaddr[ETH_ADDR_LEN]; /* Physical Ethernet address    */

struct etherPkt {
  byte   dst[ETH_ADDR_LEN];   /* Destination MAC address */
  byte   src[ETH_ADDR_LEN];   /* Source MAC address      */
  uint16 type;                /* Ether type field        */
  byte   data[1];             /* Packet payload          */
};

#define ETH_HDR_LEN  14        /* Length of Ethernet packet header */

/* Ethernet DMA buffer sizes */

#define ETH_MTU      1500      /* Maximum transmission unit        */
#define ETH_VLAN_LEN 4         /* Length of Ethernet vlan tag      */
#define ETH_CRC_LEN  4         /* Length of CRC on Ethernet frame  */

#define ETH_MAX_PKT_LEN ( ETH_HDR_LEN + ETH_VLAN_LEN + ETH_MTU )

#define ETH_BUF_SIZE 2048      /* Multiple of 16 greater than the
                                * max packet size (for cache align)
                                */

/* State of Ethernet interface */

#define ETH_STATE_FREE 0       /* Control block is unused          */
#define ETH_STATE_DOWN 1       /* Interface is inactive            */
#define ETH_STATE_UP   2       /* Interface is currently active    */

/* Ethernet device control functions */

#define ETH_CTRL_GET_MAC      1   /* Get the MAC for this device   */
#define ETH_CTRL_ADD_MCAST    2   /* Add a multicast address       */
#define ETH_CTRL_REMOVE_MCAST 3   /* Remove a multicast address    */

/* Ethernet multicast */

#define ETH_NUM_MCAST         32  /* Max multicast addresses       */

/* Ethernet NIC type */

#define ETH_TYPE_3C905C       1   /* 3COM 905C                     */
#define ETH_TYPE_E1000E       2   /* Intel E1000E                  */
#define ETH_TYPE_QUARK_ETH    3   /* Ethernet on Quark board       */

/* Control block for Ethernet device */
struct ethcblk {
  byte   state;          /* ETH_STATE_... as define above  */
  struct dentry *phy;    /* Physical eth device for Tx DMA */
  byte   type;           /* NIC type_... as defined above  */

  struct dentry *dev;    /* Address in device switch table */
  void   *csr;           /* Control and status reg. addr.  */
  uint32 pcidev;         /* PCI device number              */
  uint32 iobase;         /* I/O base from config           */
  uint32 flashbase;      /* Flash base from config         */
  uint32 membase;        /* Mem. base for device config    */

  void   *rxRing;        /* Ptr to array of recv ring desc */
  void   *rxBufs;        /* Ptr to Rx packet buffs in mem. */
  uint32 rxHead;         /* Index of curr. head of Rx ring */
  uint32 rxTail;         /* Index of curr. tail of Rx ring */
  uint32 rxRingSize;     /* Size of Rx ring desc. array    */
  uint32 rxIrq;          /* Count of Rx interrupt requests */

  void   *txRing;        /* Ptr to array of xmit ring desc */
  void   *txBufs;        /* Ptr to Tx packet buffs in mem. */
  uint32 txHead;         /* Index of curr. head of Tx ring */
  uint32 txTail;         /* Index of curr. tail of Tx ring */
  uint32 txRingSize;     /* Size of Tx ring desc. array    */
  uint32 txIrq;          /* Count of Tx interrupt requests */

  uint16 devAddress[ETH_ADDR_LEN]; /* MAC address          */
  uint16 addrLen;        /* Hardware address length        */
  uint16 mtu;            /* Max transmission unit (payload)*/

  uint32 errors;         /* Number of Ethernet errors      */
  sid32  isem;           /* Semaphore for Ethernet input   */
  sid32  osem;           /* Semaphore for Ethernet output  */
  uint16 istart;         /* Index of next packet in ring   */

  int16  inPool;         /* Buffer pool ID for input buffs */
  int16  outPool;        /* Buffer pool ID for output buffs*/

  int16  proms;          /* Nonzero => promiscuous mode    */

  int16  ed_mcset;       /* Nonzero => multicast recep set */
  int16  ed_mcc;         /* Count of multicast addresses   */
  Eaddr  ed_mca[ETH_NUM_MCAST]; /* Array of multicast addr */
};

extern struct ethcblk ethertab[]; /* Array of control blks */

uint16 eth_phy_read(volatile struct eth_q_csreg *csptr, uint32 regnum);
void eth_phy_write(volatile struct eth_q_csreg *csrptr, uint32 regnum, uint16 value);
int32 eth_phy_reset(volatile struct eth_q_csreg *csrptr);

int32 ethinit(struct dentry *devptr);
devcall ethread(struct dentry *dvptr, char *buf, int32 len);
devcall ethwrite(struct dentry *devptr, char *buf, int32 len);
interrupt ethhandler();
devcall ethcontrol(struct dentry *devptr, int64 func, int64 arg1, int64 arg2);

void pci_write_config_word(uint32 pcidevptr, uint16, uint16);
void get_quark_pdat_entry_data_by_id(uint32 macid, char *devaddr, uint32 addrlen);

int32 ethmcast_add(struct ethcblk *ethptr, byte *arg1);
int32 ethmcast_remove(struct ethcblk *ethptr, byte *arg1);

#endif
