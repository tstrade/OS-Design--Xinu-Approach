#include "../inclusions.h"

int32 rfscomm(struct rf_msg_hdr *msg, int32 mlen, struct rf_msg_hdr *reply, int32 rlen)
{
    int32 i;
    int32 retval;
    int32 seq;
    int16 rtype;
    int32 slot;

    /* For the frist time after reboot, register server port */
    if (!Rf_data.rf_registered) {
        if ((retval = udp_register(Rf_data.rf_ser_ip, 
                                   Rf_data.rf_ser_port, 
                                   Rf_data.rf_loc_port)) == SYSERR)
            return SYSERR;
        Rf_data.rf_udp_slot = retval;
        Rf_data.rf_registered = TRUE;
    }

    /* Assign message next sequence number */
    seq = Rf_data.rf_seq++;
    msg->rf_seq = htonl(seq);

    /* Repeat RF_RETRIES times: send message and receive reply */
    for (i = 0; i < RF_RETRIES; i++) {
        retval = udp_send(Rf_data.rf_udp_slot, (char *)msg, mlen);
        if (retval == SYSERR)
        {
            kprintf("Cannot send to remote file server\n");
            return SYSERR;
        }

        retval = udp_recv(Rf_data.rf_udp_slot, (char *)reply, rlen, RF_TIMEOUT);
        if (retval == TIMEOUT) {
            continue;
        } else if (retval == SYSERR) {
            kprintf("Error reading remote file reply\n");
            return SYSERR;
        }

        /* Verify that sequence in reply matches request */
        if (ntohl(reply->rf_seq) != seq)
            continue;

        /* Verify the type in the reply matches the request */
        rtype = ntohs(reply->rf_type);
        if (rtype != (ntohs(msg->rf_type) | RF_MSG_RESPONSE))
            continue;

        return retval;
    }

    /* Reties exhausted without success */
    kprintf("Timeout on exchange with remote file server\n");
    return TIMEOUT;
}