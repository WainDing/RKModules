#include "cam.h"
#include "log.h"

#define TAG "cam_test"

#define CHK_RET(ret,fun)\
    if (ret) {\
        log(TAG, "error in %s", fun);\
        return -1;\
    } else {\
        log(TAG, "success run %s", fun);\
    }

int main(int argc, char **argv) {
    log(TAG, "cam_test run");
    FILE *fp = fopen("test.yuv", "w+");
    if (!fp) {
        log(TAG, "failed to open test.yuv");
        return -1;
    }

    int ret = 0;
    RKMediaCam cam;
    ret = cam.Init("/dev/video0", V4L2_PIX_FMT_YUYV, 1280, 720);
    CHK_RET(ret, "cam.Init");

    int cnt = 0;
    do {
        CameraBuf buf;
        ret = cam.GetFrame(&buf);
        CHK_RET(ret, "cam.GetFrame");

        fwrite(buf.data, 1, buf.dataLen, fp);

        ret = cam.FreeFrame(&buf);
        CHK_RET(ret, "cam.FreeFrame");
    } while (cnt ++ <= 10);

    ret = cam.Deinit();
    CHK_RET(ret, "cam.Deinit");
    fclose(fp);

    log(TAG, "cam_test success test");

    return 0;
}
