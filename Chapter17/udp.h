#ifndef UDP_H
#define UDP_H

#include "../xinu_types.h"

typedef uint32 uid32;

#define UDP_SLOTS      6      /* Number of open UDP endpoints  */
#define UDP_QSIZ       8      /* Packets enqueued per endpoint */

#define UDP_DHCP_CPORT 68     /* DHCP client port number       */
#define UDP_DHCP_SPORT 67     /* DHCP server port number       */

/* Constants for state of an entry */
#define UDP_FREE       0      /* Entry is unused               */
#define UDP_USED       1      /* Entry is in use               */
#define UDP_RECV       2      /* Entry has a process waiting   */

#define UDP_ANYIF      -2     /* Register an endpoint for any  */
                              /*    interface on the machine   */

#define UDP_HDR_LEN    8      /* Length of a UDP header        */
                              
struct udpentry {
    int32  udstate;
    uint32 udremip;
    uint16 udremport;
    uint16 udlocport;
    int32  udhead;
    int32  udtail;
    int32  udcount;
    pid32  udpid;
    struct netpacket *udqueue[UDP_QSIZ];
};

extern struct udpentry udptab[];

void udp_init();
void udp_in(struct netpacket *pktptr);
uid32 udp_register(uint32 remip, uint16 remport, uint16 locport);
int32 udp_recv(uid32 slot, char *buff, int32 len, uint32 timeout);
int32 udp_recvaddr(uid32 slot, uint32 *remip, uint16 *remport, char *buff, int32 len, uint32 timeout);
status udp_send(uid32 slot, char *buff, int32 len);
status udp_sento(uid32 slot, uint32 remip, uint16 remport, char *buff, int32 len);
status udp_release(uid32 slot);
void udp_ntoh(struct netpacket *pktptr);
void udp_hton(struct netpacket *pktptr);

#endif