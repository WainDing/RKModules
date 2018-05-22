#include "log.h"
#include "cam.h"
#include "codec.h"
#include "mp4.h"
#include <stdlib.h>

#define TAG "video_encode2save_mp4"
#define RET_VAL(ret,fun)\
    if(ret){\
        log(TAG,"%s failed",fun);\
        return -1;\
    } else {\
        log(TAG, "%s success");\
    }

#define W 1280
#define H 720

int main(int argc, char **argv) {
    int ret = 0;
    RKMediaCam cam;
    RKMediaCodec codec;
    RKMediaMp4Context mp4;

    log(TAG, "video_encode2save_mp4");
    ret = cam.Init("/dev/video0", MPP_FMT_YUV422_YUYV, W, H);
    RET_VAL(ret, "cam.Init");
    ret = codec.Init(MPP_FMT_YUV420SP, W, H);
    RET_VAL(ret, "codec.Init");
    ret = mp4.CreateMp4File("test.mp4", W, H, 90000, 30);
    RET_VAL(ret, "mp4.CreateMp4File");

    // get sps and pps
    unsigned char *sps = NULL;
    unsigned int spsLen = 0;
    ret = codec.GetSpsPps(&sps, &spsLen);
    RET_VAL(ret, "codec.GetSpsPps");

    ret = mp4.WriteSpsAndPps(sps, spsLen);
    RET_VAL(ret, "mp4.WriteSpsAndPps");

    int cnt = 0;
    do {
        CameraBuf buf;
        MppPacket pkt = NULL;

        ret = cam.GetFrame(&buf);
        RET_VAL(ret, "cam.GetFrame");

        ret = codec.Encode((unsigned char *)buf.data, buf.dataLen, &pkt);
        RET_VAL(ret, "codec.Encode");

        if (pkt) {
            void *ptr = mpp_packet_get_pos(pkt);
            size_t len = mpp_packet_get_length(pkt);

            ret = mp4.WriteNalu((unsigned char *)ptr, len);
            RET_VAL(ret, "mp4.WriteNalu");

            mpp_packet_deinit(&pkt);
        }

        cam.FreeFrame(&buf);
    } while (cnt++ <= 100);

    free(sps);
    ret = cam.Deinit();
    RET_VAL(ret, "cam.Deinit");
    ret = codec.Deinit();
    RET_VAL(ret, "codec.Deinit");
    mp4.CloseMp4File();

    return 0;
}