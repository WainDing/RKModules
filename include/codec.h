#ifndef __CODEC_H__
#define __CODEC_H__

#include "rk_mpi.h"

class RKMediaCodec {
public:
    RKMediaCodec();
    virtual ~RKMediaCodec();
    MPP_RET Init(MppFrameFormat fmt, int w, int h);
    MPP_RET Deinit();
    MPP_RET GetSpsPps(RK_U8 **data, RK_U32 *dataSize);
    MPP_RET Encode(RK_U8 *data, RK_U32 dataSize, MppPacket *pkt);
private:
    void read_yuv_image(RK_U8 *src, RK_U8 *dst);
    
    MppApi *_mpi;
    MppCtx _ctx;
    MppEncPrepCfg _prep_cfg;
    MppEncRcCfg _rc_cfg;
    MppEncCodecCfg _codec_cfg;
    MppFrame _frm;
    MppPacket _pkt;
    RK_U32 _w;
    RK_U32 _h;
    RK_U32 _hor_stride;
    RK_U32 _ver_stride;
    RK_U32 _frm_size;
    MppFrameFormat _fmt;
    MppBuffer _frm_buf;
};

#endif // __CODEC_H__