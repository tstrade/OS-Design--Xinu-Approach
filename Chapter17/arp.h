#ifndef ARP_H
#define ARP_H

#include "../xinu_types.h"
#include "../Chapter16/ether.h"

/* Items related to ARP - definition of cache and the packet format */

#define ARP_HALEN    6          /* Size of Ethernet MAC address */
#define ARP_PALEN    4          /* Size of IP address           */

#define ARP_HTYPE    1          /* Ethernet hardware type       */
#define ARP_PTYPE    0x0800     /* IP protocol type             */

#define ARP_OP_REQ   1          /* Request op code              */
#define ARP_OP_RPLY  2          /* Reply op code                */

#define ARP_SIZ      16         /* Number of entries in a cache */

#define ARP_RETRY    3          /* Num. retries for ARP request */

#define ARP_TIMEOUT  300        /* Retry timer in milliseconds  */

/* State of an ARP cache entry */

#define AR_FREE      0
#define AR_PENDING   1
#define AR_RESOLVED  2

#pragma pack(2)
struct arppacket {
  byte   arp_ethdst[ETH_ADDR_LEN];
  byte   arp_ethsrc[ETH_ADDR_LEN];
  uint16 arp_ethtype;
  uint16 arp_htype;
  uint16 arp_ptype;
  byte   arp_hlen;
  byte   arp_plen;
  uint16 arp_op;
  byte   arp_sndha[ARP_HALEN];
  uint32 arp_sndpa;
  byte   arp_tarha[ARP_HALEN];
  uint32 arp_tarpa;
};

#pragma pack()

struct arpentry {
  int32  arstate;
  uint32 arpaddr;
  pid32  arpid;
  byte   arhaddr[ARP_HALEN];
};

extern struct arpentry arpcache[];

void arp_init();
status arp_resolve(uint32 nxthop, byte mac[ETH_ADDR_LEN]);
void arp_in(struct arppacket *pktptr);
int32 arp_alloc();

void arp_ntoh(struct arppacket *pktptr);
void arp_hton(struct arppacket *pktptr);

/* <arpa/inet.h> -  Converts the unsigned integer from network byte order to host byte order. */
uint16 ntohs(uint16 hostshort);
uint32 ntohl(uint32 hostlong);
/* <arpa/inet.h> -  Converts the unsigned integer from host byte order to network byte order. */
uint16 htons(uint16 hostshort);
uint32 htonl(uint32 hostlong);

#endif
