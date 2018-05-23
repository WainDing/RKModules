#ifndef __CRTP_H__
#define __CRTP_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_MTU_SIZE       (1500)
#define MAX_RTP_PKG_LEN    (1400)
#define RTP_DEFAULT_FPS    (25)
#define RTP_DEFAULT_SSRC   (10)
#define RTP_PAYLOAD_TYPE_H264    (96)

struct rtp_hdr_t{
    /* byte 0 */
    uint8_t csrc_len:   4; // identify csrc num
    uint8_t extension:  1; // identify extern header
    uint8_t padding:    1; // identify padding bit
    uint8_t version:    2; // identify rtp version

    /* byte 1 */
    uint8_t payload_type:   7; // identify payload type
    uint8_t marker:         1; // identify payload status

    /* byte 2-3 */
    uint16_t seq_no:        16; // identify sequence number

    /* byte 4-7 */
    uint32_t timestamp:     32; // identify time stamp

    /* byte 8-11 */
    uint32_t ssrc:  32; // identify ssrc
} __attribute__((packed));

struct nalu_hdr_t {
    uint8_t type:   5; // identify nalu type
    uint8_t nri:    2; // identify reference bit
    uint8_t f:      1; // identify forbidden bit
} __attribute__((packed));

struct fu_hdr_t {
    uint8_t type:   5;
    uint8_t r:      1;
    uint8_t e:      1;
    uint8_t s:      1;
} __attribute__((packed));

struct crtp_t {
    int fd;
    uint8_t *buf;
    char ip[256];
    int  port;
    int  nb_frms;

    struct timeval t_run;
    struct timeval t_curr;
    struct rtp_hdr_t rtp_hdr;
};

int crtp_init(struct crtp_t **ctx, const char *dstip, int dstport);
int crtp_deinit(struct crtp_t **ctx);
int crtp_send_pkt(struct crtp_t *ctx, const uint8_t *nalu,
                  uint32_t size);

#endif // __CRTP_H__
