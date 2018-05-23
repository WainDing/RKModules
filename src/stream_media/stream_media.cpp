#include "stream_media.h"
#include "log.h"

extern "C" {
#include "crtp.h"
}

#define MODULE_TAG "stream_media"

RKMediaStreamContext::RKMediaStreamContext() {

}

RKMediaStreamContext::~RKMediaStreamContext() {

}

int RKMediaStreamContext::InitRTProtocol(const char *addr, int port) {
    int ret = 0;
    _crtp = NULL;
    
    ret = crtp_init(&_crtp, addr, port);
    if (ret) {
        log(MODULE_TAG, "failed to init crtp context");
        return -1;
    }

    return 0;
} 

int RKMediaStreamContext::DeinitRTProtocol() {
    int ret = 0;

    ret = crtp_deinit(&_crtp);
    if (ret) {
        log(MODULE_TAG, "failed to deinit crtp context");
        return -1;
    }

    return 0;
}

int RKMediaStreamContext::SendRtpPacket(const uint8_t *nalu, uint32_t nalu_size) {
    int ret = 0;

    ret = crtp_send_pkt(_crtp, nalu, nalu_size);
    if (ret) {
        log(MODULE_TAG, "failed to send crtp packet");
        return -1;
    }

    return 0;
}
