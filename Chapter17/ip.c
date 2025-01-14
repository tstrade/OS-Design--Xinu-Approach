#include "../inclusions.h"

struct iqentry ipoqueue;

/* ------------------------------------------------------------------
 *   ip_in() - Handle an IP packet that has arrive over a network
 * ------------------------------------------------------------------
 */
void ip_in(struct netpacket *pktptr)
{
  int32 icmplen;

  /* Verify checksum (unique value that verifies data integrity) */
  if (ipcksum(pktptr) != 0) {
    kprintf("IP header checksum failed\n\r");
    freebuf((char *)pktptr);
    return;
  }

  ip_ntoh(pktptr);

  /* Ensure version and length are valid */
  if (pktptr->net_ipvh != 0x45) {
    kprintf("IP version failed\n\r");
    freebuf((char *)pktptr);
    return;
  }

  /* Verify encapsulated protocol checksums and then convert
   *    the encapsulated headers to host byte order
   */
  switch (pktptr->net_ipproto) {
  case IP_UDP:
    /* Skipping UDP checksum for now */
    udp_ntoh(pktptr);
    break;

  case IP_ICMP:
    icmplen = pktptr->net_iplen - IP_HDR_LEN;
    if (icmp_cksum((char *)&pktptr->net_ictype, icmplen) != 0) {
      freebuf((char *)pktptr);
      return;
    }
    icmp_ntoh(pktptr);
    break;

  default:
    break;
  }

  /* Deliver 255.255.255.255 to local stack */
  if (pktptr->net_ipdst == IP_BCAST) {
    ip_local(pktptr);
    return;
  }

  /* If we do not yet have a valid address, accept UDP packets
   *    (to get DHCP replies) and drop others
   */
  if (!NetData.ipvalid) {
    if (pktptr->net_ipproto == IP_UDP) {
      ip_local(pktptr);
      return;
    } else {
      freebuf((char *)pktptr);
      return;
    }
  }

  /* If packet is destined for us, accept it: otherwise, drop it */
  if ((pktptr->net_ipdst == NetData.ipucast) ||
      (pktptr->net_ipdst == NetData.ipbcast) ||
      (pktptr->net_ipdst == IP_BCAST)) {
    ip_local(pktptr);
    return;
  } else {
    freebuf((char *)pktptr);
    return;
  }
}

/* ------------------------------------------------------------------
 *   ip_send() - Send an outgoing IP datagram from the local stack
 * ------------------------------------------------------------------
 */
status ip_send(struct netpacket *pktptr)
{
  intmask mask;
  uint32 dest;
  int32 retval;
  uint32 nxthop;

  mask = disable();

  dest = pktptr->net_ipdst;

  /* Loop back to local stack if destination 127.0.0.0/8 */
  if ((dest & 0xFF000000) == 0x7F000000) {
    ip_local(pktptr);
    restore(mask);
    return OK;
  }

  /* Loop back if destination matches our IP unicast address */
  if (dest == NetData.ipucast) {
    ip_local(pktptr);
    restore(mask);
    return OK;
  }

  /* Broadcast if destination is 255.255.255.255 */
  if ((dest == IP_BCAST) || (dest == NetData.ipbcast)) {
    memcpy(pktptr->net_ethdst, NetData.ethbcast, ETH_ADDR_LEN);
    retval = ip_out(pktptr);
    restore(mask);
    return retval;
  }

  /* If destination is on the local network, next hop is the
   *    destination; otherwise, next hop is default router
   */
  if ((dest & NetData.ipmask) == NetData.ipprefix) {
    /* Next hop is the destination itself */
    nxthop = dest;
  } else {
    nxthop = NetData.iprouter;
  }

  /* Dest. invalid or no default route */
  if (nxthop == 0) {
    freebuf((char *)pktptr);
    return SYSERR;
  }

  /* Resolve the next-hop address to get a MAC address */
  retval = arp_resolve(nxthop, pktptr->net_ethdst);
  if (retval != OK) {
    freebuf((char *)pktptr);
    return SYSERR;
  }

  /* Send the packet */
  retval = ip_out(pktptr);
  restore(mask);
  return retval;
}

/* ------------------------------------------------------------------
 *   ip_local() - Deliver an IP datagram to the local stack
 * ------------------------------------------------------------------
 */
void ip_local(struct netpacket *pktptr)
{
  /* Use datagram contents to determine how to process */
  switch (pktptr->net_ipproto) {
  case IP_UDP:
    udp_in(pktptr);
    return;

  case IP_ICMP:
    icmp_in(pktptr);
    return;

  default:
    freebuf((char *)pktptr);
    return;
  }
}

/* ------------------------------------------------------------------
 *   ip_out() - Transmit an outgoing IP datagram
 * ------------------------------------------------------------------
 */
status ip_out(struct netpacket *pktptr)
{
  uint16 cksum;
  int32 len;
  int32 pktlen;
  int32 retval;

  /* Compute total packet length */
  pktlen = pktptr->net_iplen + ETH_HDR_LEN;

  /* Convert encapsulated protocol to network byte order */
  switch (pktptr->net_ipproto) {
  case IP_UDP:
    pktptr->net_udpcksum = 0;
    udp_hton(pktptr);
    /* ...skipping UDP checkum computation */
    break;

  case IP_ICMP:
    icmp_hton(pktptr);
    pktptr->net_iccksum = 0;
    len = pktptr->net_iplen - IP_HDR_LEN;
    cksum = icmp_cksum((char *)&pktptr->net_ictype, len);
    pktptr->net_iccksum = 0xFFFF & htons(cksum);
    break;

  default:
    break;
  }

  ip_hton(pktptr);

  pktptr->net_ipcksum = 0;
  cksum = ipcksum(pktptr);
  pktptr->net_ipcksum = 0xFFFF & htons(cksum);

  eth_hton(pktptr);

  /* Send packet over the Ethernet */
  retval = write(ETHER0, (char *)pktptr, pktlen);
  freebuf((char *)pktptr);

  if (retval == SYSERR)
    return SYSERR;
  else
    return OK;
}

/* ------------------------------------------------------------------
 *   ipcksum() - Compute the IP header checksum for a datagram
 * ------------------------------------------------------------------
 */
uint16 ipcksum(struct netpacket *pktptr)
{
  uint16 *hptr;
  int32 i;
  uint16 word;
  uint32 cksum;

  hptr = (uint16 *)&pktptr->net_ipvh;

  /* Sum 16-bit words in the packet */
  cksum = 0;
  for (i = 0; i < 10; i++) {
    word = *hptr++;
    cksum += (uint32)htons(word);
  }

  /* Add in carry, and take the ones-complement */
  cksum += (cksum >> 16);
  cksum = 0xFFFF & ~cksum;

  /* Use all-1s for zero */
  if (cksum == 0xFFFF)
    cksum = 0;

  return (uint16)(0xFFFF & cksum);
}

/* ------------------------------------------------------------------
 *   ip_ntoh() - Convert an IP header fields to host byte order
 * ------------------------------------------------------------------
 */
void ip_ntoh(struct netpacket *pktptr)
{
  pktptr->net_iplen = ntohs(pktptr->net_iplen);
  pktptr->net_ipid = ntohs(pktptr->net_ipid);
  pktptr->net_ipfrag = ntohs(pktptr->net_ipfrag);
  pktptr->net_ipsrc = ntohl(pktptr->net_ipsrc);
  pktptr->net_ipdst = ntohl(pktptr->net_ipdst);
}

/* ------------------------------------------------------------------
 *   ip_hton() - Convert an IP header fields to network byte order
 * ------------------------------------------------------------------
 */
void ip_hton(struct netpacket *pktptr)
{
  pktptr->net_iplen = htons(pktptr->net_iplen);
  pktptr->net_ipid = htons(pktptr->net_ipid);
  pktptr->net_ipfrag = htons(pktptr->net_ipfrag);
  pktptr->net_ipsrc = htonl(pktptr->net_ipsrc);
  pktptr->net_ipdst = htonl(pktptr->net_ipdst);
}

/* -------------------------------------------------------------------------
 *   ipout() - Process that transmits IP packets from the IP output queue
 * -------------------------------------------------------------------------
 */
process ipout()
{
  struct netpacket *pktptr;
  struct iqentry *ipqptr;
  uint32 destip;
  uint32 nxthop;
  int32 retval;

  ipqptr = &ipoqueue;

  while (1) {
    /* Obtain next packet from the IP output queue */
    wait(ipqptr->iqsem);
    pktptr = ipqptr->iqbuf[ipqptr->iqhead++];
    if (ipqptr->iqhead >= IP_OQSIZ)
      ipqptr->iqhead = 0;

    /* Fill in the MAC source address */
    memcpy(pktptr->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);

    /* Extract destination address from packet */
    destip = pktptr->net_ipdst;

    /* Sanity check: packets sent to ipout should *not* 
     *    contain a broadcast address.
     */
    if ((destip == IP_BCAST) || (destip == NetData.ipbcast)) {
      kprintf("ipout: broadcast address in packet\n\r");
      freebuf((char *)pktptr);
      continue;
    }

    /* Check whether destination is the local computer */
    if (destip == NetData.ipucast) {
      ip_local(pktptr);
      continue;
    }

    /* Check whether destination is on the local net */
    if ((destip & NetData.ipmask) == NetData.ipprefix) {
      nxthop = destip;
    } else {
      nxthop = NetData.iprouter;
    }

    /* Dest. invalid or no default route */
    if (nxthop == 0) {
      freebuf((char *)pktptr);
      continue;
    }

    /* Use ARP to resolve next-hop address */
    retval = arp_resolve(nxthop, pktptr->net_ethdst);
    if (retval != OK) {
      freebuf((char *)pktptr);
      continue;
    }

    /* Use ipout to convert byte order and send */
    ip_out(pktptr);
  }
}

/* --------------------------------------------------------------------------
 *   ip_enqueue() - Deposit an outgoing IP datagram on the IP output queue
 * --------------------------------------------------------------------------
 */
status ip_enqueue(struct netpacket *pktptr)
{
  intmask mask;
  struct iqentry *iptr;
  
  mask = disable();
  iptr = &ipoqueue;

  /* Enqueue packet on network output queue */
  iptr = &ipoqueue;
  if (semcount(iptr->iqsem) >= IP_OQSIZ) {
    kprintf("ip_enqueue: output queue overflow\n");
    freebuf((char *)pktptr);
    restore(mask);
    return SYSERR;
  }
  iptr->iqbuf[iptr->iqtail++] = pktptr;
  if (iptr->iqtail >= IP_OQSIZ)
    iptr->iqtail = 0;

  /* Signal that a packet is available */
  signal(iptr->iqsem);
  restore(mask);
  return OK;
}