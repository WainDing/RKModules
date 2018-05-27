#include <stdio.h>
#include "cam.h"
#include "codec.h"
#include "stream_media.h"
#include "log.h"

#define TAG "video_encode_test"
#define RET_VAL(ret,fun)\
    if (ret) {\
        log(TAG, "%s failed", fun);\
        return -1;\
    } else {\
        log(TAG, "%s success", fun);\
    }

#define W 1280
#define H 720

int main(int argc, char **argv) {
    int ret = 0;
    RKMediaCam cam;
    RKMediaCodec codec;
    RKMediaStreamContext media;

    log(TAG, "video_encode2push_rtp");

    ret = cam.Init("/dev/video0", V4L2_PIX_FMT_YUYV, W, H);
    RET_VAL(ret, "cam.Init");

    ret = cam.Deinit();
    RET_VAL(ret, "cam.Deinit");
    return 0;
}
