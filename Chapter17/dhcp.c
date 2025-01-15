#include "../inclusions.h"

/* ------------------------------------------------------------------------------------------
 *    dhcp_get_opt_val() - Retrieve a pointer to the value for specified DHCP options key
 * ------------------------------------------------------------------------------------------
 */
char *dhcp_get_opt_val(const struct dhcpmsg *dmsg, uint32 dmsg_size, uint8 option_key)
{
    unsigned char *opt_tmp;
    unsigned char *eom;

    eom = (unsigned char *)dmsg + dmsg_size - 1;
    opt_tmp = (unsigned char *)dmsg->dc_opt;

    while(opt_tmp < eom) {
        /* If the option value matches, return the value */
        if ((*opt_tmp) == option_key) {
            /* Offset past the option value and the size */
            return (char *)(opt_tmp + 2);
        }
        /* Move to length octet */
        opt_tmp++;
        opt_tmp += *(uint8 *)opt_tmp + 1;
    }
    /* Option value not found */
    return NULL;
}

/* ------------------------------------------------------------------------------ 
 *    dhcp_bld_bootp_msg() - Set the common fields for all DHCP messages 
 * ------------------------------------------------------------------------------
 */
void dhcp_bld_bootp_msg(struct dhcpmsg *dmsg)
{
    uint32 xid;

    memcpy(&xid, NetData.ethucast, 4);
    memset(dmsg, 0x00, sizeof(struct dhcpmsg));

    dmsg->dc_bop = 0x01;               /* Outgoing request              */
    dmsg->dc_htype = 0x01;             /* Hardware type is Ethernet     */
    dmsg->dc_hlen = 0x06;              /* Hardware address length       */
    dmsg->dc_hops = 0x00;              /* Hop count                     */
    dmsg->dc_xid = htonl(xid);         /* Xid (Unique ID)               */
    dmsg->dc_secs = 0x0000;            /* Seconds                       */
    dmsg->dc_flags = 0x0000;           /* Flags                         */
    dmsg->dc_cip = 0x00000000;         /* Client IP address             */
    dmsg->dc_yip = 0x00000000;         /* Your IP address               */
    dmsg->dc_sip = 0x00000000;         /* Server IP address             */
    dmsg->dc_gip = 0x00000000;         /* Gateway IP address            */
    memset(dmsg->dc_chaddr, '\0', 16); /* Client hardware address       */
    memcpy(&dmsg->dc_chaddr, NetData.ethucast, ETH_ADDR_LEN);
    memset(dmsg->dc_bootp, '\0', 192); /* Zero the bootp area           */
    dmsg->dc_cookie = htonl(0x63825363); /* Magic cookie for DHCP       */
}

/* --------------------------------------------------------------------------------
 *    dhcp_bld_disc() - Handcraft a DHCP Discover message in dmsg 
 * --------------------------------------------------------------------------------
 */
int32 dhcp_bld_disc(struct dhcpmsg *dmsg)
{
    uint32 j = 0;
    dhcp_bld_bootp_msg(dmsg);

    dmsg->dc_opt[j++] = 0xFF & 53;  /* DHCP message type option      */
    dmsg->dc_opt[j++] = 0xFF &  1;  /* Option length                 */
    dmsg->dc_opt[j++] = 0xFF &  1;  /* DHCP Discover message         */
    dmsg->dc_opt[j++] = 0xFF &  0;  /* Options padding               */

    dmsg->dc_opt[j++] = 0xFF & 55;  /* DHCP parameter request list   */
    dmsg->dc_opt[j++] = 0xFF &  2;  /* Options length                */
    dmsg->dc_opt[j++] = 0xFF &  1;  /* Request subnet mask           */
    dmsg->dc_opt[j++] = 0xFF &  3;  /* Request default router addr-> */

    return (uint32)((char *)&dmsg->dc_opt[j] - (char *)dmsg + 1);
}

/* ------------------------------------------------------------------------------
 *    dhcp_bld_req() - Handcraft a DHCP request message in dmsg 
 * ------------------------------------------------------------------------------
 */
int32 dhcp_bld_req(struct dhcpmsg *dmsg, const struct dhcpmsg *dmsg_offer, uint32 dmsg_offer_size)
{
    uint32 j = 0;
    uint32 *server_ip;

    dhcp_bld_bootp_msg(dmsg);
    dmsg->dc_sip = dmsg_offer->dc_sip;

    dmsg->dc_opt[j++] = 0xFF & 53;  /* DHCP message type option      */
    dmsg->dc_opt[j++] = 0xFF &  1;  /* Option length                 */
    dmsg->dc_opt[j++] = 0xFF &  3;  /* DHCP Request message          */
    dmsg->dc_opt[j++] = 0xFF &  0;  /* Options padding               */

    dmsg->dc_opt[j++] = 0xFF & 50;  /* Requested IP                  */
    dmsg->dc_opt[j++] = 0xFF &  4;  /* Option length                 */
    *((uint32 *)&dmsg->dc_opt[j]) = dmsg_offer->dc_yip;
    j += 4;

    /* Retrieve the DHCP server IP from the DHCP options */
    server_ip = (uint32 *)dhcp_get_opt_val(dmsg_offer, dmsg_offer_size, DHCP_SERVER_ID);

    if (server_ip == 0) {
        kprintf("Unable to get server ID addr. from DHCP Offer\n");
        return SYSERR;
    }

    dmsg->dc_opt[j++] = 0xFF & 54;  /* Server IP                     */
    dmsg->dc_opt[j++] = 0xFF &  4;  /* Option length                 */
    *((uint32 *)&dmsg->dc_opt[j]) = *server_ip;
    j += 4;

    return (uint32)((char *)&dmsg->dc_opt[j] - (char *)dmsg + 1);
}

/* ------------------------------------------------------------------------------
 *    getlocalip() - Use DHCP to obtain an IP address 
 * ------------------------------------------------------------------------------
 */
uint32 getlocalip()
{
    int32 slot;
    struct dhcpmsg dmsg_snd;
    struct dhcpmsg dmsg_rcv;

    int32 i, j;
    int32 len;
    int32 inlen;
    char *optptr;
    char *eop;
    int32 msgtype;
    uint32 addrmask;
    uint32 routeraddr;
    uint32 tmp;
    uint32 *tmp_server_ip;

    slot = udp_register(0, UDP_DHCP_SPORT, UDP_DHCP_CPORT);
    if (slot == SYSERR) {
        kprintf("getlocalip: cannot register with UDP\n");
        return SYSERR;
    }

    len = dhcp_bld_disc(&dmsg_snd);
    if (len == SYSERR) {
        kprintf("getlocalip: cannot build DHCP Discover\n");
        return SYSERR;
    }

    for (i = 0; i < DHCP_RETRY; i++) {
        udp_sendto(slot, IP_BCAST, UDP_DHCP_SPORT, (char *)&dmsg_snd, len);

        /* Read 3 incoming DHCP messages and check for an offer 
         *    or wait for three timeout periods if no message 
         *    arrives.
         */
        for (j = 0; j < 3; j++) {
            inlen = udp_recv(slot, (char *)&dmsg_rcv, sizeof(struct dhcpmsg), 2000);
            if (inlen == TIMEOUT)
                continue;
            else if (inlen == SYSERR)
                return SYSERR;

            /* Check that incoming message is valid response 
             *    (ID matches our request)
             */
            if (dmsg_rcv.dc_xid != dmsg_snd.dc_xid)
                continue;

            eop = (char *)&dmsg_rcv + inlen - 1;
            optptr = (char *)&dmsg_rcv.dc_opt;
            msgtype = addrmask = routeraddr = 0;

            while (optptr < eop) {
                switch (*optptr) {
                case 53:   /* Message type */
                    msgtype = 0xFF & *(optptr + 2);
                break;
                
                case 1:    /* Subnet mask */
                    memcpy((void *)&tmp, optptr + 2, 4);
                    addrmask = ntohl(tmp);
                break;

                case 3:    /* Router address */
                    memcpy((void *)&tmp, optptr + 2, 4);
                    routeraddr = ntohl(tmp);
                    break;
                }
                /* Move to length octet */
                optptr++;
                optptr += (0xFF & *optptr) + 1;
            }
            /* Offer - send request */
            if (msgtype == 0x02) {
                len = dhcp_bld_req(&dmsg_snd, &dmsg_rcv, inlen);
                if (len == SYSERR) {
                    kprintf("getlocalip: cannot build DHCP Request\n");
                    return SYSERR;
                }
                udp_sendto(slot, IP_BCAST, UDP_DHCP_SPORT, (char *)&dmsg_snd, len);
                continue;
            } else if (dmsg_rcv.dc_opt[2] != 0x05) {
                /* If not an ack skip it */
                continue;
            }

            if (addrmask != 0)
                NetData.ipmask = addrmask;

            if (routeraddr != 0)
                NetData.iprouter = routeraddr;

            NetData.ipucast = ntohl(dmsg_rcv.dc_yip);
            NetData.ipprefix = NetData.ipucast & NetData.ipmask;
            NetData.ipbcast = NetData.ipprefix | ~NetData.ipmask;
            NetData.ipvalid = TRUE;
            udp_release(slot);

            /* Retrieve the boot server IP */
            if (dot2ip((char *)dmsg_rcv.sname, &NetData.bootserver) != OK) {
                /* Coult not retrieve the boot server from the BOOTP fields,
                 *   so use the DHCP server address
                 */
                tmp_server_ip = (uint32)dhcp_get_opt_val(&dmsg_rcv, inlen, DHCP_SERVER_ID);
                if (tmp_server_ip == 0) {
                    kprintf("Cannot get boot server addr\n");
                    return (uint32)SYSERR;
                }
                NetData.bootserver = ntohl(*tmp_server_ip);
            }
            memcpy(NetData.bootfile, dmsg_rcv.bootfile, sizeof(dmsg_rcv.bootfile));
            return NetData.ipucast;
        }
    }
    kprintf("DHCP failed to get response\n");
    udp_release(slot);
    return (uint32)SYSERR;
}