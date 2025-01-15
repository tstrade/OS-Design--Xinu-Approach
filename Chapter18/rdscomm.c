#include "../inclusions.h"

status rdscomm(struct rd_msg_hdr *msg, int32 mlen,
               struct rd_msg_hdr *reply, int32 rlen, 
               struct rdscblk *rdptr)
{
    int32 i;
    int32 retval;
    int32 seq;
    uint32 localip;
    int16 rtype;
    bool8 xmit;
    int32 slot;

    /* For the first time after reboot, register the server port */
    if (!rdptr->rd_registered) {
        slot = udp_register(0, rdptr->rd_ser_port, rdptr->rd_loc_port);
        if (slot == SYSERR)
            return SYSERR;
        rdptr->rd_udpslot = slot;
        rdptr->rd_registered = TRUE;
    }

    if (NetData.ipvalid == FALSE) {
        localip = getlocalip();
        if ((int32)localip == SYSERR)
            return SYSERR;
    }
    /* Retrieve the saved UDP slot number */
    slot = rdptr->rd_udpslot;
    /* Assign message next sequence number */
    seq = rdptr->rd_seq++;
    msg->rd_seq = htonl(seq);

    /* Repeat RD_RETRIES times: send message and receive reply */
    xmit = TRUE;
    for (i = 0; i < RD_RETRIES; i++) {
        if (xmit) {
            retval = udp_sendto(slot, rdptr->rd_ser_ip, rdptr->rd_ser_port,
                                (char *)msg, mlen);
            if (retval == SYSERR) {
                kprintf("Cannot send to remote disk server\n\r");
                return SYSERR;
            }
        } else {
            xmit = TRUE;
        }

        /* Receive a reply */
        retval = udp_recv(slot, (char *)reply, rlen, RD_TIMEOUT);
        if (retval == TIMEOUT) {
            continue;
        } else if (retval == SYSERR) {
            kprintf("Error reading remote disk reply\n\r");
            return SYSERR;
        }

        /* Verify that sequence in reply matches request */
        if (ntohl(reply->rd_seq) < seq) {
            xmit = FALSE;
        } else if (ntohl(reply->rd_seq) != seq) {
            continue;
        }

        /* Verify the type in the reply matches the request */
        rtype = ntohs(reply->rd_type);
        if (rtype != (ntohs(msg->rd_type) | RD_MSG_RESPONSE)) {
            continue;
        }

        if (ntohs(reply->rd_status) != 0) {
            return SYSERR;
        }

        return OK;
    }

    /* Retries exhausted without sucess */
    kprintf("Timeout on exchange with remote disk server\n\r");
    return TIMEOUT;
}