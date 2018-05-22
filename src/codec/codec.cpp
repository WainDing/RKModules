#include "codec.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

#define TAG "codec"
#define CODEC_ALIGN(x, a)         (((x)+(a)-1)&~((a)-1))

RKMediaCodec::RKMediaCodec() {
    
}

RKMediaCodec::~RKMediaCodec() {

}

MPP_RET RKMediaCodec::Init(MppFrameFormat fmt, int w, int h) {
    MPP_RET ret = MPP_OK;
    MppEncCodecCfg *codec_cfg;
    MppEncPrepCfg *prep_cfg;
    MppEncRcCfg *rc_cfg;

    _w = w;
    _h = h;
    _hor_stride = CODEC_ALIGN(_w, 16);
    _ver_stride = CODEC_ALIGN(_h, 16);
    _fmt = fmt;

    ret = mpp_create(&_ctx, &_mpi);
    if (ret) {
        log(MODULE_TAG, "failed to create codec context");
        return MPP_NOK;
    }

    ret = mpp_init(_ctx, MPP_CTX_ENC, MPP_VIDEO_CodingAVC);
    if (ret) {
        log(MODULE_TAG, "failed to init codec context");
        return MPP_NOK;
    }

    if (_fmt <= MPP_FMT_YUV420SP_VU) {
        _frm_size = _hor_stride * _ver_stride * 3 / 2;
    } else if (_fmt <= MPP_FMT_YUV422_UYVY) {
        _hor_stride *= 2;
        _frm_size = _hor_stride * _ver_stride;
    } else {
        _frm_size = _hor_stride * _ver_stride * 4;
    }

    codec_cfg = &_codec_cfg;
    prep_cfg = &_prep_cfg;
    rc_cfg = &_rc_cfg;

    int fps = 30;
    int gop = 60;
    int bps = _w * _h * 8 / fps;
    prep_cfg->change        = MPP_ENC_PREP_CFG_CHANGE_INPUT |
                              MPP_ENC_PREP_CFG_CHANGE_ROTATION |
                              MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    prep_cfg->width         = _w;
    prep_cfg->height        = _h;
    prep_cfg->hor_stride    = _hor_stride;
    prep_cfg->ver_stride    = _ver_stride;
    prep_cfg->format        = _fmt;
    prep_cfg->rotation      = MPP_ENC_ROT_0;
    ret = _mpi->control(_ctx, MPP_ENC_SET_PREP_CFG, prep_cfg);
    if (ret) {
        log(MODULE_TAG, "mpi control enc set prep cfg failed ret %d\n", ret);
        return MPP_NOK;
    }

    rc_cfg->change  = MPP_ENC_RC_CFG_CHANGE_ALL;
    rc_cfg->rc_mode = MPP_ENC_RC_MODE_CBR;
    rc_cfg->quality = MPP_ENC_RC_QUALITY_MEDIUM;

    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR) {
        /* constant bitrate has very small bps range of 1/16 bps */
        rc_cfg->bps_target   = bps;
        rc_cfg->bps_max      = bps * 17 / 16;
        rc_cfg->bps_min      = bps * 15 / 16;
    } else if (rc_cfg->rc_mode ==  MPP_ENC_RC_MODE_VBR) {
        if (rc_cfg->quality == MPP_ENC_RC_QUALITY_CQP) {
            /* constant QP does not have bps */
            rc_cfg->bps_target   = -1;
            rc_cfg->bps_max      = -1;
            rc_cfg->bps_min      = -1;
        } else {
            /* variable bitrate has large bps range */
            rc_cfg->bps_target   = bps;
            rc_cfg->bps_max      = bps * 17 / 16;
            rc_cfg->bps_min      = bps * 1 / 16;
        }
    }

    /* fix input / output frame rate */
    rc_cfg->fps_in_flex      = 0;
    rc_cfg->fps_in_num       = fps;
    rc_cfg->fps_in_denorm    = 1;
    rc_cfg->fps_out_flex     = 0;
    rc_cfg->fps_out_num      = fps;
    rc_cfg->fps_out_denorm   = 1;

    rc_cfg->gop              = gop;
    rc_cfg->skip_cnt         = 0;

    log(MODULE_TAG, "mpi_enc_test bps %d fps %d gop %d\n",
            rc_cfg->bps_target, rc_cfg->fps_out_num, rc_cfg->gop);
    ret = _mpi->control(_ctx, MPP_ENC_SET_RC_CFG, rc_cfg);
    if (ret) {
        log(MODULE_TAG, "mpi control enc set rc cfg failed ret %d\n", ret);
        return MPP_NOK;
    }

    codec_cfg->coding = MPP_VIDEO_CodingAVC;
    codec_cfg->h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                                 MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                                 MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
    codec_cfg->h264.profile  = 100;
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */
    codec_cfg->h264.level    = 40;
    codec_cfg->h264.entropy_coding_mode  = 1;
    codec_cfg->h264.cabac_init_idc  = 0;
    codec_cfg->h264.transform8x8_mode = 1;
    ret = _mpi->control(_ctx, MPP_ENC_SET_CODEC_CFG, codec_cfg);
    if (ret) {
        log(MODULE_TAG, "mpi control enc set codec cfg failed ret %d\n", ret);
        return MPP_NOK;
    }

    MppEncSeiMode sei_mode;
    sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = _mpi->control(_ctx, MPP_ENC_SET_SEI_CFG, &sei_mode);
    if (ret) {
        log(MODULE_TAG, "mpi control enc set sei cfg failed ret %d\n", ret);
        return MPP_NOK;
    }

    ret = mpp_buffer_get(NULL, &_frm_buf, _frm_size);
    if (ret) {
        log(MODULE_TAG, "failed to get buffer from codec context");
        return MPP_NOK;
    }

    ret = mpp_frame_init(&_frm);
    mpp_frame_set_width(_frm, _w);
    mpp_frame_set_height(_frm, _h);
    mpp_frame_set_hor_stride(_frm, _hor_stride);
    mpp_frame_set_ver_stride(_frm, _ver_stride);
    mpp_frame_set_fmt(_frm, _fmt);
    mpp_frame_set_eos(_frm, 0);

    return MPP_OK;
}

MPP_RET RKMediaCodec::Deinit() {
    MPP_RET ret = MPP_OK;

    ret = _mpi->reset(_ctx);
    if (ret) {
        log(MODULE_TAG, "failed to reset codec context");
        return MPP_NOK;
    }

    if (_ctx) {
        mpp_destroy(_ctx);
        _ctx = NULL;
    }

    return MPP_OK;
}

MPP_RET RKMediaCodec::GetSpsPps(RK_U8 **data, RK_U32 *dataSize) {
    MppPacket pkt;
    MPP_RET ret = MPP_OK;

    ret = _mpi->control(_ctx, MPP_ENC_GET_EXTRA_INFO, &pkt);
    if (ret) {
        log(MODULE_TAG, "failed to get extra info from codec");
        return MPP_NOK;
    }

    if (pkt) {
        void *ptr = mpp_packet_get_pos(pkt);
        size_t len = mpp_packet_get_length(pkt);

        *data = (RK_U8 *)malloc(len);
        memcpy(*data, ptr, len);
        *dataSize = len;
    }

    return MPP_OK;
}

void RKMediaCodec::read_yuv_image(RK_U8 *src, RK_U8 *dst){
    RK_U32 row = 0;
    RK_U32 index = 0;

    RK_U8 *y = dst;
    RK_U8 *u = y + _hor_stride * _ver_stride;
    RK_U8 *v = u + _hor_stride * _ver_stride / 4;

    switch (_fmt) {
    case MPP_FMT_YUV420SP : {
        for (row = 0; row < _h; row++) {
            memcpy(y + row * _hor_stride, src + index, _w);
            index += _w;
        }

        for (row = 0; row < _h / 2; row++) {
            memcpy(u + row * _hor_stride, src + index, _w);
        }
    } break;
    case MPP_FMT_YUV420P : {
        for (row = 0; row < _h; row++) {
            memcpy(y + row * _hor_stride, src + index, _w);
            index += _w;
        }

        for (row = 0; row < _h / 2; row++) {
            memcpy(u + row * _hor_stride / 2, src + index, _w / 2);
            index += _w / 2;
        }

        for (row = 0; row < _h / 2; row++) {
            memcpy(v + row * _hor_stride, src + index, _w / 2);
            index += _w / 2;
        }
    } break;
    case MPP_FMT_YUV422_YUYV :
    case MPP_FMT_YUV422_UYVY : {
        for (row = 0; row < _h; row++) {
            memcpy(y + row * _hor_stride, src + index, _w * 2);
            index += _w * 2;
        }
    } break; 
    default : {
        log(MODULE_TAG, "unsupport read yuv fmt:%d", _fmt);
    } break;
    }
}

MPP_RET RKMediaCodec::Encode(RK_U8 *data, RK_U32 dataSize, MppPacket *pkt) {
    MPP_RET ret = MPP_OK;
    MppPacket packet;

    void *ptr = mpp_buffer_get_ptr(_frm_buf);
    read_yuv_image(data, (RK_U8 *)ptr);

    ret = _mpi->encode_put_frame(_ctx, _frm);
    if (ret) {
        log(MODULE_TAG, "failed to put frame to codec context");
        return MPP_NOK;
    }

    ret = _mpi->encode_get_packet(_ctx, &packet);
    if (ret) {
        log(MODULE_TAG, "failed to get packet from codec context");
        return MPP_NOK;
    }

    if (packet) {
        *pkt = packet;
    }

    return MPP_OK;
}
