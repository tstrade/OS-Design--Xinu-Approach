#ifndef NET_H
#define NET_H

#include "../xinu_types.h"
#include "ether.h"

/* Constants used in the networking code */

#define	ETH_ARP	    0x0806		/* Ethernet type for ARP	*/
#define	ETH_IP	    0x0800		/* Ethernet type for IP		*/

#define	IP_BCAST    0xffffffff		/* IP local broadcast address	*/
#define	IP_THIS	    0xffffffff		/* "this host" src IP address	*/
#define	IP_ALLZEROS 0x00000000		/* The all-zeros IP address	*/

#define	IP_ICMP	    1			/* ICMP protocol type for IP	*/
#define	IP_UDP	    17			/* UDP protocol type for IP	*/

#define	IP_ASIZE    4			/* bytes in an IP address	*/
#define	IP_HDR_LEN  20			/* bytes in an IP header	*/

/* Format of an Ethernet packet carrying IPv4 and UDP */

struct	netpacket	{
	byte	net_ethdst[ETH_ADDR_LEN];/* Ethernet dest. MAC address	*/
	byte	net_ethsrc[ETH_ADDR_LEN];/* Ethernet source MAC address	*/
	uint16	net_ethtype;		/* Ethernet type field		*/
	byte	net_ipvh;		/* IP version and hdr length	*/
	byte	net_iptos;		/* IP type of service		*/
	uint16	net_iplen;		/* IP total packet length	*/
	uint16	net_ipid;		/* IP datagram ID		*/
	uint16	net_ipfrag;		/* IP flags & fragment offset	*/
	byte	net_ipttl;		/* IP time-to-live		*/
	byte	net_ipproto;		/* IP protocol (actually type)	*/
	uint16	net_ipcksum;		/* IP checksum			*/
	uint32	net_ipsrc;		/* IP source address		*/
	uint32	net_ipdst;		/* IP destination address	*/
	union {
	 struct {
	  uint16	net_udpsport;	/* UDP source protocol port	*/
	  uint16	net_udpdport;	/* UDP destination protocol port*/
	  uint16	net_udplen;	/* UDP total length		*/
	  uint16	net_udpcksum;	/* UDP checksum			*/
	  byte	net_udpdata[1500-28];/* UDP payload (1500-above)*/
	 };
	 struct  {
	  byte		net_ictype;	/* ICMP message type		*/
	  byte		net_iccode;	/* ICMP code field (0 for ping)	*/
	  uint16	net_iccksum;	/* ICMP message checksum	*/
	  uint16	net_icident;	/* ICMP identifier		*/
	  uint16	net_icseq;	/* ICMP sequence number		*/
	  byte		net_icdata[1500-28];/* ICMP payload (1500-above)*/
	 };
	};
};

extern	struct	netpacket *currpkt;	/* ptr to current input packet	*/
extern	bpid32	netbufpool;		/* ID of net packet buffer pool	*/

struct	network {
  uint32 ipaddr;	        /* IP address			*/
  uint32 addrmask;		/* Subnet mask			*/
  uint32 routeraddr;		/* Address of default router	*/
  bool8	 ipvalid;		/* Is IP address valid yet?	*/
  byte   ethaddr[ETH_ADDR_LEN];	/* Ethernet address		*/
  void   *ethbcast;
  void   *ethucast;
  uint32 ipbcast;
  uint32 ipucast;
};

extern	struct	network NetData;	/* Local network interface	*/

void eth_hton(struct netpacket *netptr);

#endif
