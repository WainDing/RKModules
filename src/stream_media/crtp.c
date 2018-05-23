#include "crtp.h"
#include "log.h"

#define MODULE_TAG "crtp"
#define cmalloc(type,nb)    malloc(nb*sizeof(type))

static int socket_init(char *ip, int port, int *fd) {
    int ret = 0;
    const optval = 1;
    struct sockaddr_in sock;

    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    sock.sin_addr.s_addr = inet_addr(ip);

    ret = socket(AF_INET, SOCK_DGRAM, 0);
    if (ret < 0) {
        log(MODULE_TAG, "failed to exec socket %d.\n", ret);
        return -1;
    }
    *fd = ret;

    ret = setsockopt(*fd, SOL_SOCKET, SO_BROADCAST,
                     &optval, sizeof(optval));
    if (ret < 0) {
        log(MODULE_TAG, "failed to exec setsocketopt %d.\n", ret);
        return -2;
    }

    ret = connect(*fd, (struct sockaddr *)&sock,
                  sizeof(struct sockaddr));
    if (ret < 0) {
        log(MODULE_TAG, "failed to exec connect %d.\n", ret);
        return -3;
    }

    return 0;
}

int crtp_init(struct crtp_t **ctx, const char *dstip, int dstport) {
    int ret = 0;
    struct crtp_t *p_ctx = NULL;

    p_ctx = cmalloc(struct crtp_t, 1);
    if (!p_ctx) {
        log(MODULE_TAG, "failed to malloc crtp_t.\n");
        return -1;
    }

    strcpy(p_ctx->ip, dstip);
    p_ctx->nb_frms = 0;
    p_ctx->port = dstport;
    p_ctx->rtp_hdr.timestamp = 0;
    p_ctx->rtp_hdr.ssrc = RTP_DEFAULT_SSRC;
    p_ctx->rtp_hdr.seq_no = 0;
    p_ctx->buf = cmalloc(char, MAX_MTU_SIZE);
    if (!p_ctx->buf) {
        log(MODULE_TAG, "failed to malloc p_ctx->buf.\n");
        return -2;
    }

    ret = socket_init(p_ctx->ip, p_ctx->port, &p_ctx->fd);
    if (ret < 0) {
        log(MODULE_TAG, "failed to exec socket_init %d.\n", ret);
        return -1;
    }

    *ctx = p_ctx;
    return 0;
}

int crtp_deinit(struct crtp_t **ctx) {
    if ((*ctx)->buf) {
        free((*ctx)->buf);
        (*ctx)->buf = NULL;
    }

    if (*ctx) {
        free(*ctx);
        *ctx = NULL;
    }

    return 0;
}

int crtp_send_pkt(struct crtp_t *ctx, const uint8_t *nalu,
                  uint32_t size) {
    int len = 0;
    int ret = 0;

    if (!ctx) {
        log(MODULE_TAG, "Null pointer,init crtp_t ctx first.\n");
        return -1;
    }

    if (ctx->nb_frms == 0) {
        gettimeofday(&ctx->t_curr, NULL);
    }

    if (size <= MAX_RTP_PKG_LEN) {
        len = 0;
        memset(ctx->buf, 0, MAX_MTU_SIZE);
        ctx->nb_frms++;
        gettimeofday(&ctx->t_run, NULL);
        ctx->rtp_hdr.timestamp = (((ctx->t_run.tv_sec - ctx->t_curr.tv_sec) * 1000 +
                                  (ctx->t_run.tv_usec - ctx->t_curr.tv_usec) / 1000) * 90000 / 1000) * 9 / 10;

        struct rtp_hdr_t *rtp_hdr = (struct rtp_hdr_t *)&ctx->buf[0];
        rtp_hdr->csrc_len = 0;
        rtp_hdr->extension = 0;
        rtp_hdr->padding = 0;
        rtp_hdr->version = 2;
        rtp_hdr->payload_type = RTP_PAYLOAD_TYPE_H264;
        rtp_hdr->seq_no = htons(++(ctx->rtp_hdr.seq_no) % UINT16_MAX);
        rtp_hdr->timestamp = htonl(ctx->rtp_hdr.timestamp);
        rtp_hdr->ssrc = htonl(ctx->rtp_hdr.ssrc);
        len += 12;

        struct nalu_hdr_t *nalu_hdr = (struct nalu_hdr_t *)&ctx->buf[12];
        nalu_hdr->f = (nalu[0] & 0x80) >> 7;
        nalu_hdr->nri = (nalu[0] & 0x60) >> 5;
        nalu_hdr->type = (nalu[0] & 0x1f);
        len += 1;

        memcpy(&ctx->buf[13], nalu + 1, size - 1);
        len += size - 1;

        ret = send(ctx->fd, (void *)ctx->buf, len, 0);
        log(MODULE_TAG, "send rtp pkt %d.\n", len);
    } else {
        int i = 0;
        uint32_t send_pkt = size / MAX_RTP_PKG_LEN;
        uint32_t pkt_nb = size % MAX_RTP_PKG_LEN ?
                          (send_pkt + 1) : send_pkt;
        uint32_t last_pkt = (send_pkt == pkt_nb) ?
                            MAX_RTP_PKG_LEN : size % MAX_RTP_PKG_LEN;

        gettimeofday(&ctx->t_run, NULL);
        ctx->rtp_hdr.timestamp = (((ctx->t_run.tv_sec - ctx->t_curr.tv_sec) * 1000 +
                                  (ctx->t_run.tv_usec - ctx->t_curr.tv_usec) / 1000) * 90000 / 1000) * 9 / 10;

        for (i = 0; i < pkt_nb; i++) {
            len = 0;
            memset(ctx->buf, 0, MAX_MTU_SIZE);

            ctx->nb_frms++;

            struct rtp_hdr_t *rtp_hdr = (struct rtp_hdr_t *)&ctx->buf[0];
            rtp_hdr->csrc_len = 0;
            rtp_hdr->extension = 0;
            rtp_hdr->padding = 0;
            rtp_hdr->version = 2;
            rtp_hdr->marker = 0;
            rtp_hdr->payload_type = RTP_PAYLOAD_TYPE_H264;
            rtp_hdr->seq_no = htons(++(ctx->rtp_hdr.seq_no) % UINT16_MAX);
            rtp_hdr->timestamp = htonl(ctx->rtp_hdr.timestamp);
            rtp_hdr->ssrc = htonl(ctx->rtp_hdr.ssrc);
            len += 12;

            struct nalu_hdr_t *nalu_hdr = (struct nalu_hdr_t *)&ctx->buf[12];
            nalu_hdr->f = (nalu[0] & 0x80) >> 7;
            nalu_hdr->nri = (nalu[0] & 0x60) >> 5;
            nalu_hdr->type = 28;
            len += 1;

            struct fu_hdr_t *fu_hdr = (struct fu_hdr_t *)&ctx->buf[13];
            fu_hdr->r = 0;
            fu_hdr->type = nalu[0] & 0x1f;
            len += 1;

            if (i == 0) {
                rtp_hdr->marker = 0;
                fu_hdr->s = 1;
                fu_hdr->e = 0;

                memcpy(&ctx->buf[14], nalu + 1, MAX_RTP_PKG_LEN - 1);
                len += MAX_RTP_PKG_LEN - 1;
            } else if (i < pkt_nb - 1) {
                rtp_hdr->marker = 0;
                fu_hdr->s = 0;
                fu_hdr->e = 0;

                memcpy(&ctx->buf[14], nalu + i * MAX_RTP_PKG_LEN,
                       MAX_RTP_PKG_LEN);
                len += MAX_RTP_PKG_LEN;
            } else {
                rtp_hdr->marker = 1;
                fu_hdr->s = 0;
                fu_hdr->e = 1;

                memcpy(&ctx->buf[14], nalu + i * MAX_RTP_PKG_LEN,
                       last_pkt);
                len += last_pkt;
            }

            ret = send(ctx->fd, (void *)ctx->buf, len, 0);
            log(MODULE_TAG, "send rtp pkt %d.\n", len);
        }
    }

    return ret;
}
