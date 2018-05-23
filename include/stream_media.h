#ifndef __STREAM_MEDIA_H__
#define __STREAM_MEDIA_H__

#include <stdint.h>

struct crtp_t;
class RKMediaStreamContext {
public:
    RKMediaStreamContext();
    virtual ~RKMediaStreamContext();
    int InitRTProtocol(const char *addr, int port);
    int DeinitRTProtocol();
    int SendRtpPacket(const uint8_t *nalu, uint32_t nalu_size);
private:
    struct crtp_t *_crtp;   
};

#endif // __STREAM_MEDIA_H__
