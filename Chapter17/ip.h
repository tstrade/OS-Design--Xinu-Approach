#ifndef IP_H
#define IP_H

#include "../xinu_types.h"

#define IP_BCAST         0xFFFFFFFF   /* IP local broadcast address  */
#define IP_THIS          0xFFFFFFFF   /* "this host" src IP address  */
#define IP_ALLZEROS      0x00000000   /* The all-zeros IP address    */

#define IP_ICMP          1            /* ICMP protocol type for IP   */
#define IP_UDP           17           /* UDP protocol type for IP    */

#define IP_ASIZE         4            /* Bytes in an IP address      */
#define IP_HDR_LEN       20           /* Bytes in an IP header       */
#define IP_VH            0x45         /* IP version and hdr length   */

#define IP_OQSIZ         8            /* Size of IP output queue     */

struct iqentry {
  int32 iqhead;
  int32 iqtail;
  sid32 iqsem;
  struct netpacket *iqbuf[IP_OQSIZ];
};

extern struct iqentry ipoqueue;

void ip_in(struct netpacket *pktptr);
status ip_send(struct netpacket *pktptr);
void ip_local(struct netpacket *pktptr);
status ip_out(struct netpacket *pktptr);
uint16 ipcksum(struct netpacket *pktptr);
void ip_ntoh(struct netpacket *pktptr);
void ip_hton(struct netpacket *pktptr);
process ipout();
status ip_enqueue(struct netpacket *pktptr);


#endif
