#include "../inclusions.h"

status rfsndmsg(uint16 type, char *name)
{
    struct rf_msg_hdr req;
    struct rf_msg_hdr resp;
    int32 retval;
    char *to;

    /* Form request */
    req.rf_type = htons(type);
    req.rf_status = htons(0);
    req.rf_seq = 0;
    to = req.rf_name;
    while ((*to++ = *name++));

    /* Send message and receive response */
    retval = rfscomm(&req, sizeof(struct rf_msg_hdr), &resp, sizeof(struct rf_msg_hdr));

    /* Check response */
    if (retval == SYSERR) {
        return SYSERR;
    } else if (retval == TIMEOUT) {
        kprintf("Timeout during remote file server access\n");
        return SYSERR;
    } else if (ntohl(resp.rf_status) != 0) {
        return SYSERR;
    }

    return OK;
}