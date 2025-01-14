#include "../inclusions.h"

struct arpentry arpcache[ARP_SIZ];

/* -----------------------------------------------------------------
 *   arp_init() - Initialize ARP cache for an Ethernet interface
 * -----------------------------------------------------------------
 */
void arp_init()
{
  int32 i;
  for (i = 1; i < ARP_SIZ; i++)
    arpcache[i].arstate = AR_FREE;
}

/* -----------------------------------------------------------------
 *   arp_resolve() - Use ARP to resolve IP addr. to Ethernet addr.
 * -----------------------------------------------------------------
 */
status arp_resove(uint32 nxthop, byte mac[ETH_ADDR_LEN])
{
  intmask mask;
  struct arppacket apkt;
  int32 i;
  int32 slot;
  struct arpentry *arptr;
  int32 msg;

  /* Use MAC broadcast address for IP limited broadcast */
  if (nxthop == IP_BCAST) {
    memcpy(mac, NetData.ethbcast, ETH_ADDR_LEN);
    return OK;
  }

  /* Use MAC broadcast address for IP network broadcast */
  if (nxthop == NetData.ipbcast) {
    memcpy(mac, NetData.ethbcast, ETH_ADDR_LEN);
    return OK;
  }

  /* Ensure only one process uses ARP at a time */
  mask = disable();

  /* See if next hop address is already present in ARP cache */
  for (i = 0; i < ARP_SIZ; i++) {
    arptr = &arpcache[i];
    if (arptr->arstate == AR_FREE)
      continue;

    if (arptr->arpaddr == nxthop)
      break;
  }

  if (i < ARP_SIZ) {
    /* If entry is resolved - handle and return */
    if (arptr->arstate == AR_RESOLVED) {
      memcpy(mac, arptr->arhaddr, ARP_HALEN);
      restore(mask);
      return OK;
    }

    /* Entry is already pending - return error because
     *   only one process can be waiting at a time
     */
    if (arptr->arstate == AR_PENDING) {
      restore(mask);
      return SYSERR;
    }
  }

  /* IP address not in cache - allocate a new cache entry and
   *   send an ARP request to obtain the answer
   */
  slot = arp_alloc();
  if (slot == SYSERR) {
    restore(mask);
    return SYSERR;
  }

  arptr = &arpcache[slot];
  arptr->arstate = AR_PENDING;
  arptr->arpaddr = nxthop;
  arptr->arpid = currpid;

  /* Hand-craft an ARP request packet */
  memcpy(apkt.arp_ethdst, NetData.ethbcast, ETH_ADDR_LEN);
  memcpy(apkt.arp_ethsrc, NetData.ethucast, ETH_ADDR_LEN);

  /* Packet type is ARP */
  apkt.arp_ethtype = ETH_ARP;

  /* Hardware type is Ethernet */
  apkt.arp_htype = ARP_HTYPE;

  /* Protocol type is IP */
  apkt.arp_ptype = ARP_PTYPE;

  /* Ethernet MAC size in bytes */
  apkt.arp_hlen = 0xFF & ARP_HALEN;

  /* IP address size in bytes */
  apkt.arp_plen = 0xFF & ARP_PALEN;

  /* ARP type is Request */
  apkt.arp_op = 0xFFFF & ARP_OP_REQ;
  memcpy(apkt.arp_sndha, NetData.ethucast, ARP_HALEN);

  /* IP address of interface */
  apkt.arp_sndpa = NetData.ipucast;

  /* Target HA is unknown */
  memset(apkt.arp_tarha, '\0', ARP_HALEN);

  /* Target protocol address */
  apkt.arp_tarpa = nxthop;

  /* Convert ARP packet from host to net byte order */
  arp_hton(&apkt);

  /* Convert Ethernet header from host to net byte order */
  eth_hton((struct netpacket *)&apkt);

  /* Send the packet ARP_RETRY times and await response */
  msg = recvclr();
  for (i = 0; i < ARP_RETRY; i++) {
    write(ETHER0, (char *)&apkt, sizeof(struct arppacket));
    msg = recvtime(ARP_TIMEOUT);
    if (msg == TIMEOUT) {
      continue;
    } else if (msg == SYSERR) {
      restore(mask);
      return SYSERR;
    } else {
      break;
    }
  }

  /* If no response, return TIMEOUT */
  if (msg == TIMEOUT) {
    arptr->arstate = AR_FREE;
    restore(mask);
    return TIMEOUT;
  }

  /* Return hardware address */
  memcpy(mac, arptr->arhaddr, ARP_HALEN);
  restore(mask);
  return OK;
}

/* ------------------------------------------------------------------------
 *    arp_in() - Handle an incoming ARP packet
 * ------------------------------------------------------------------------
 */
void arp_in(struct arppacket *pktptr)
{
  intmask mask;
  struct arppacket apkt;
  int32 slot;
  struct arpentry *arptr;
  bool8 found;

  /* Convert packet from network order to host order */
  arp_ntoh(pktptr);

  /* Verify ARP is for IPv4 and Ethernet */
  if ((pktptr->arp_htype != ARP_HTYPE) || (pktptr->arp_ptype != ARP_PTYPE)) {
    freebuf((char *)pktptr);
    return;
  }

  /* Ensure only one process uses ARP at a time */
  mask = disable();

  /* Search cache for sender's IP address */
  found = FALSE;

  for (slot = 0; slot < ARP_SIZ; slot++) {
    arptr = &arpcache[slot];
    /* Skip unused entries */
    if (arptr->arstate == AR_FREE)
      continue;

    /* If sender's address matches, we've found it */
    if (arptr->arpaddr == pktptr->arp_sndpa) {
      found = TRUE;
      break;
    }
  }

  if (found) {
    /* Update sender's hardware address */
    memcpy(arptr->arhaddr, pktptr->arp_sndha, ARP_HALEN);

    /* If a process was waiting, inform the process */
    if (arptr->arstate == AR_PENDING) {
      /* Mark resolved and notify waiting process */
      arptr->arstate = AR_RESOLVED;
      send(arptr->arpid, OK);
    }
  }

  /* For an ARP reply, processing is complete */
  if (pktptr->arp_op == ARP_OP_RPLY) {
    freebuf((char *)pktptr);
    restore(mask);
    return;
  }

  /* The following is for an ARP request packet: if the local
   *   machine is not the target or the local IP address is
   *  not yet known, ignore the request (i.e., processing is complete)
   */
  if ((!NetData.ipvalid) || (pktptr->arp_tarpa != NetData.ipucast)) {
    freebuf((char *)pktptr);
    restore(mask);
    return;
  }

  /* Request has been sent to the local machine's address. So,
   *   add sender's info to cache, if not already present
   */
  if (!found) {
    slot = arp_alloc();
    if (slot == SYSERR) {
      kprintf("ARP cache overflow on interface\n");
      freebuf((char *)pktptr);
      restore(mask);
      return;
    }
    arptr = &arpcache[slot];
    arptr->arpaddr = pktptr->arp_sndpa;
    memcpy(arptr->arhaddr, pktptr->arp_sndha, ARP_HALEN);
    arptr->arstate = AR_RESOLVED;
  }

  /* Hand-craft an ARP reply packet and send back to requester */
  memcpy(apkt.arp_ethdst, pktptr->arp_sndha, ARP_HALEN);
  memcpy(apkt.arp_ethsrc, NetData.ethucast, ARP_HALEN);

  /* Frame carries ARP */
  apkt.arp_ethtype = ETH_ARP;

  /* Hardware is Ethernet */
  apkt.arp_htype = ARP_HTYPE;

  /* Protocol is IP */
  apkt.arp_ptype = ARP_PTYPE;

  /* Ethernet address size */
  apkt.arp_hlen = ARP_HALEN;

  /* IP address size */
  apkt.arp_plen = ARP_PALEN;

  /* Type is Reply */
  apkt.arp_op = ARP_OP_RPLY;

  /* Insert local Ethernet and IP address in sender fields */
  memcpy(apkt.arp_sndha, NetData.ethucast, ARP_HALEN);
  apkt.arp_sndpa = NetData.ipucast;

  /* Convert ARP packet from host to network byte order */
  arp_hton(&apkt);

  /* Convert the Ethernet header to network byte order */
  eth_hton((struct netpacket *)&apkt);

  /* Send the reply */
  write(ETHER0, (char *)&apkt, sizeof(struct arppacket));
  freebuf((char *)pktptr);
  restore(mask);
  return;
}

/* ------------------------------------------------------------------------------
 *    arp_alloc() - Find a free slot or kick out an entry to create one
 * ------------------------------------------------------------------------------
 */
int32 arp_alloc()
{
  int32 slot;

  /* Search for a free slot */
  for (slot = 0; slot < ARP_SIZ; slot++) {
    if (arpcache[slot].arstate == AR_FREE) {
      memset((char *)&arpcache[slot], NULLCH, sizeof(struct arpentry));
      return slot;
    }
  }

  /* Search for a resolved entry */
  for (slot = 0; slot < ARP_SIZ; slot++) {
    if (arpcache[slot].arstate == AR_RESOLVED) {
      memset((char *)&arpcache[slot], NULLCH, sizeof(struct arpentry));
      return slot;
    }
  }

  /* At this point, all slots are pending (should not happen) */
  kprintf("ARP cache size exceeded\n");
  return SYSERR;
}

/* ------------------------------------------------------------------------------
 *    arp_ntoh() - Convert ARP packet fields from net to host byte order
 * ------------------------------------------------------------------------------
 */
void arp_ntoh(struct arppacket *pktptr)
{
  pktptr->arp_htype = ntohs(pktptr->arp_htype);
  pktptr->arp_ptype = ntohs(pktptr->arp_ptype);
  pktptr->arp_op    = ntohs(pktptr->arp_op);
  pktptr->arp_sndpa = ntohl(pktptr->arp_sndpa);
  pktptr->arp_tarpa = ntohl(pktptr->arp_tarpa);
}

/* ------------------------------------------------------------------------------
 *    arp_hton() = Convert ARP packet fields from net to host byte order
 * ------------------------------------------------------------------------------
 */
void arp_hton(struct arppacket *pktptr)
{
  pktptr->arp_htype = htons(pktptr->arp_htype);
  pktptr->arp_ptype = htons(pktptr->arp_ptype);
  pktptr->arp_op    = htons(pktptr->arp_op);
  pktptr->arp_sndpa = htonl(pktptr->arp_sndpa);
  pktptr->arp_tarpa = htonl(pktptr->arp_tarpa);
}
