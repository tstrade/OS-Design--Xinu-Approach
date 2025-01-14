#include "../inclusions.h"

struct udpentry udptab[UDP_SLOTS];

/* ------------------------------------------------------------------
 *   udp_init() - Initialize all entries in the UDP endpoint table
 * ------------------------------------------------------------------
 */
void udp_init()
{
    int32 i;

    for (i = 0; i < UDP_SLOTS; i++)
        udptab[i].udstate = UDP_FREE;

    return;
}

/* ------------------------------------------------------------------
 *   udp_in() - Handle an incoming UDP packet
 * ------------------------------------------------------------------
 */
void udp_in(struct netpacket *pktptr)
{
    intmask mask;
    int32 i;
    struct udpentry *udptr;

    mask = disable();

    for (i = 0; i < UDP_SLOTS; i++) {
        udptr = &udptab[i];
        if (udptr->udstate == UDP_FREE)
            continue;

        if ((pktptr->net_udpdport == udptr->udlocport) &&
            ((udptr->udremport == 0) || 
                (pktptr->net_udpsport == udptr->udremport)) &&
            (((udptr->udremip == 0) ||
                (pktptr->net_ipsrc == udptr->udremip)))) {

            if (udptr->udcount >= UDP_QSIZ) {
                udptr->udcount++;
                udptr->udqueue[udptr->udtail++] = pktptr;
                if (udptr->udtail >- UDP_QSIZ)
                    udptr->udtail = 0;

                if (udptr->udstate == UDP_RECV) {
                    udptr->udstate = UDP_USED;
                    send(udptr->udpid, OK);
                }
                restore(mask);
                return;
            }
        }
    }

    /* No match - simply discard packet */
    freebuf((char *)pktptr);
    restore(mask);
    return;
}

/* ------------------------------------------------------------------
 *   udp_register() - Register a remote IP, remote port & local port
 *                     to receive incoming UDP messages from the
 *                     specified remote site sent to the specified
 *                     local port.
 * ------------------------------------------------------------------
 */
uid32 udp_register(uint32 remip, uint16 remport, uint16 locport)
{
    intmask mask;
    int32 slot;
    struct udpentry *udptr;

    mask = disable();

    for (slot = 0; slot < UDP_SLOTS; slot++) {
        udptr = &udptab[slot];
        if (udptr->udstate == UDP_FREE)
            continue;

        /* Look at this entry in table */
        if ((remport == udptr->udremport) &&
            (locport == udptr->udlocport) &&
            (remip == udptr->udremip)) {
            /* Request is already in the table */
            restore(mask);
            return SYSERR;
        }
    }

    /* Find a free slot and allocate it */
    for (slot = 0; slot < UDP_SLOTS; slot++) {
        udptr = &udptab[slot];
        if (udptr->udstate != UDP_FREE)
            continue;

        udptr->udstate = UDP_USED;
        udptr->udremip = remip;
        udptr->udremport = remport;
        udptr->udlocport = locport;
        udptr->udhead = 0;
        udptr->udtail = 0;
        udptr->udcount = 0;
        udptr->udpid = currpid;
        restore(mask);
        return slot;
    }

    restore(mask);
    return SYSERR;
}

/* ------------------------------------------------------------------
 *   udp_recv() - Receive a UDP packet
 * ------------------------------------------------------------------
 */
int32 udp_recv(uid32 slot, char *buff, int32 len, uint32 timeout)
{
    intmask mask;
    struct udpentry *udptr;
    umsg32 msg;
    struct netpacket *pktptr;
    int32 i;
    int32 msglen;
    char *udataptr;

    mask = disable();

    /* Verify that the slot is valid */
    if ((slot < 0) || (slot >= UDP_SLOTS)) {
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

    if (udptr->udstate != UDP_USED) {
        restore(mask);
        return SYSERR;
    }

    /* Wait for a packet to arrive */
    if (udptr->udcount == 0) {
        udptr->udstate = UDP_RECV;
        udptr->udpid = currpid;
        msg = recvclr();
        msg = recvtime(timeout);
        if (msg == TIMEOUT) {
            restore(mask);
            return TIMEOUT;
        } else if (msg != OK) {
            restore(mask);
            return SYSERR;
        }
    }

    /* Packet has arrive -- dequeue it */
    pktptr = udptr->udqueue[udptr->udhead++];
    if (udptr->udhead >= UDP_QSIZ)
        udptr->udhead = 0;

    udptr->udcount--;

    /* Copy UDP data from packet into caller's buffer */
    msglen = pktptr->net_udplen - UDP_HDR_LEN;
    udataptr = (char *)&pktptr->net_udpdata;
    if (len < msglen) 
        msglen = len;

    for (i = 0; i < msglen; i++)
        *buff++ = *udataptr++;

    freebuf((char *)pktptr);
    restore(mask);
    return msglen;
}

/* --------------------------------------------------------------------------
 *   udp_recvaddr() - Receive a UDP packet and record the sender's address
 * --------------------------------------------------------------------------
 */
int32 udp_recvaddr(uid32 slot, uint32 *remip, uint16 *remport, char *buff, int32 len, uint32 timeout)
{
    intmask mask;
    struct udpentry *udptr;
    umsg32 msg;
    struct netpacket *pktptr;
    int32 msglen;
    int32 i;
    char *udataptr;

    mask = disable();

    /* Verify that the slot is valid */
    if ((slot < 0) || (slot >= UDP_SLOTS)) {
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

    if (udptr->udstate != UDP_USED) {
        restore(mask);
        return SYSERR;
    }

    /* Wait for a packet to arrive */
    if (udptr->udcount == 0) {
        udptr->udstate = UDP_RECV;
        udptr->udpid = currpid;
        msg = recvclr();
        msg = recvtime(timeout);
        udptr->udstate = UDP_USED;
        if (msg == TIMEOUT) {
            restore(mask);
            return TIMEOUT;
        } else if (msg != OK) {
            restore(mask);
            return SYSERR;
        }
    }

    /* Packet has arrived -- dequeue it */
    pktptr = udptr->udqueue[udptr->udhead++];
    if (udptr->udhead >= UDP_QSIZ)
        udptr->udhead = 0;

    /* Record sender's IP address and UDP port number */
    *remip = pktptr->net_ipsrc;
    *remport = pktptr->net_udpsport;

    udptr->udcount--;

    /* Copy UDP data from packet into caller's buffer */
    msglen = pktptr->net_udplen - UDP_HDR_LEN;
    udataptr = (char *)&pktptr->net_udpdata;
    if (len < msglen)
        msglen = len;

    for (i = 0; i < msglen; i++)
        *buff++ = *udataptr++;

    freebuf((char *)pktptr);
    restore(mask);
    return msglen;
}

/* ------------------------------------------------------------------
 *   udp_send() - Send a UDP packet using info in a UDP table entry
 * ------------------------------------------------------------------
 */
status udp_send(uid32 slot, char *buff, int32 len)
{
    intmask mask;
    struct netpacket *pktptr;
    int32 pktlen;
    static uint16 ident = 1;
    char *udataptr;
    uint32 remip;
    uint16 remport;
    uint16 locport;
    uint16 locip;

    struct udpentry *udptr;

    mask = disable();

    /* Verify that the slot is valid */
    if ((slot < 0) || (slot >= UDP_SLOTS)) {
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

    /* Verify that the slot has been registers and is valid */
    if (udptr->udstate == UDP_FREE) {
        restore(mask);
        return SYSERR;
    }

    /* Verify that the slot has a specified remote address */
    remip = udptr->udremip;
    if (remip == 0) {
        restore(mask);
        return SYSERR;
    }

    locip = NetData.ipucast;
    remport = udptr->udremport;
    locport = udptr->udlocport;

    /* Allocate a network buffer to hold the packet */
    pktptr = (struct netpacket *)getbuf(netbufpool);

    if ((int32)pktptr == SYSERR) {
        restore(mask);
        return SYSERR;
    }

    /* Compute packet length as UDP data size + fixed header size */
    pktlen = ((char *)&pktptr->net_udpdata - (char *)pktptr) + len;

    /* Create a UDP packet in pktptr */
    memcpy((char *)pktptr->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);
    pktptr->net_ethtype = 0x0800;
    pktptr->net_ipvh = 0x45;
    pktptr->net_iptos = 0;
    pktptr->net_iplen = pktlen - ETH_HDR_LEN;
    pktptr->net_ipid = ident++;
    pktptr->net_ipfrag = 0x0000;
    pktptr->net_ipttl = 0xFF;
    pktptr->net_ipproto = IP_UDP;
    pktptr->net_ipcksum = 0x0000;
    pktptr->net_ipsrc = locip;
    pktptr->net_ipdst = remip;

    pktptr->net_udpsport = locport;
    pktptr->net_udpdport = remport;
    pktptr->net_udplen = (uint16)(UDP_HDR_LEN + len);
    pktptr->net_udpcksum = 0x0000;
    udataptr = (char *)pktptr->net_udpdata;
    for ( ; len > 0; len--)
        *udataptr++ = *buff++;

    /* Call ipsend to send the datagtram */
    ip_send(pktptr);
    restore(mask);
    return OK;
}

/* ------------------------------------------------------------------
 *   udp_sento() - Send a UDP packet to a specified destination
 * ------------------------------------------------------------------
 */
status udp_sento(uid32 slot, uint32 remip, uint16 remport, char *buff, int32 len)
{
    intmask mask;
    struct netpacket *pktptr;
    int32 pktlen;
    static uint16 ident = 1;
    struct udpentry *udptr;
    char *udataptr;

    mask = disable();

    /* Verify that the slot is valid */
    if ((slot < 0) || (slot >= UDP_SLOTS)) {
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

    /* Verify that the slot has been registered and is valid */
    if (udptr->udstate == UDP_FREE) {
        restore(mask);
        return SYSERR;
    }

    /* Allocate a network buffer to hold the packet */
    pktptr = (struct netpacket *)getbuf(netbufpool);

    if ((int32)pktptr == SYSERR) {
        restore(mask);
        return SYSERR;
    }

    /* Compute packet length as UDP data size + fixed header size */
    pktlen = ((char *)&pktptr->net_udpdata - (char *)pktptr) + len;

    /* Create a UDP packet in pktptr */
    memcpy((char *)pktptr->net_ethsrc, NetData.ethucast, ETH_ADDR_LEN);
    pktptr->net_ethtype = 0x0800;
    pktptr->net_ipvh = 0x45;
    pktptr->net_iptos = 0x00;
    pktptr->net_iplen = pktlen - ETH_HDR_LEN;
    pktptr->net_ipid = ident++;
    pktptr->net_ipfrag = 0x0000;
    pktptr->net_ipttl = 0xFF;
    pktptr->net_ipproto = IP_UDP;
    pktptr->net_ipcksum = 0x0000;
    pktptr->net_ipsrc = NetData.ipucast;
    pktptr->net_ipdst = remip;

    pktptr->net_udpsport = udptr->udlocport;
    pktptr->net_udpdport = remport;
    pktptr->net_udplen = (uint16)(UDP_HDR_LEN + len);
    pktptr->net_udpcksum = 0x0000;
    udataptr = (char *)pktptr->net_udpdata;
    for ( ; len > 0; len--)
        *udataptr++ = *buff++;

    /* Call ipsend to send the datagram */
    ip_send(pktptr);
    restore(mask);
    return OK;
}

/* ------------------------------------------------------------------
 *   udp_release() - Release a previously registered UDP slot
 * ------------------------------------------------------------------
 */
status udp_release(uid32 slot)
{
    intmask mask;
    struct udpentry *udptr;
    struct netpacket *pktptr;

    mask = disable();

    /* Verify that the slot is valid */
    if ((slot < 0) || (slot >= UDP_SLOTS)) {
        restore(mask);
        return SYSERR;
    }

    udptr = &udptab[slot];

    if (udptr->udstate == UDP_FREE) {
        restore(mask);
        return SYSERR;
    }

    /* Defer rescheduling to preven freebuf from switching context */
    resched_cntl(DEFER_START);
    while (udptr->udcount > 0) {
        pktptr = udptr->udqueue[udptr->udhead++];
        if (udptr->udhead >= UDP_QSIZ)
            udptr->udhead = 0;

        freebuf((char *)pktptr);
        udptr->udcount--;
    }
    udptr->udstate = UDP_FREE;
    resched_cntl(DEFER_STOP);
    restore(mask);
    return OK;
}

/* ------------------------------------------------------------------------
 *   udp_ntoh() - Convert UDP header fields from net to host byte order
 * ------------------------------------------------------------------------
 */
void udp_ntoh(struct netpacket *pktptr)
{
    pktptr->net_udpsport = ntohs(pktptr->net_udpsport);
    pktptr->net_udpdport = ntohs(pktptr->net_udpdport);
    pktptr->net_udplen = ntohs(pktptr->net_udplen);
    return;
}

/* ------------------------------------------------------------------------
 *   udp_hton() - Convert UDP header fields from host to net byte order
 * ------------------------------------------------------------------------
 */
void udp_hton(struct netpacket *pktptr)
{
    pktptr->net_udpsport = htons(pktptr->net_udpsport);
    pktptr->net_udpdport = htons(pktptr->net_udpdport);
    pktptr->net_udplen = htons(pktptr->net_udplen);
    return;
}
