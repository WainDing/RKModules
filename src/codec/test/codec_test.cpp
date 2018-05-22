#include "codec.h"
#include "log.h"
#include <stdlib.h>

#define TAG "codec_test"
#define CHK_RET(ret,fun)\
    if (ret){\
        log(TAG, "fun %s failed", fun);\
        return -1;\
    } else {\
        log(TAG, "fun %s success", fun);\
    }

int main(int argc, char **argv) { 
    int ret = 0;
    unsigned char *sps;
    int spsSize = 0;
    RKMediaCodec codec;

    log(TAG, "codec test");    

    FILE *fin = fopen("test.yuv", "r");
    if (!fin) {
        log(TAG, "failed to open test.yuv");
        return -1;
    }

    FILE *fout = fopen("test.h264", "w+");
    if (!fout) {
        log(TAG, "failed to open test.h264");
        return -1;
    }

    ret = codec.Init(MPP_FMT_YUV422_YUYV, 1280, 720);
    CHK_RET(ret, "codec.Init");

    ret = codec.GetSpsPps(&sps, (unsigned int *)&spsSize);
    CHK_RET(ret, "codec.GetSpsPps");

    log(TAG, "sps:%p size:%d", sps, spsSize);

    int frmSize = 1280 * 720 * 2;
    unsigned char *data = (unsigned char *)malloc(frmSize);
    if (!data) {
        log(TAG, "failed to malloc data");
        return -1;
    }

    int eos = 0;
    MppPacket pkt = NULL;
    for (;;) {
        ret = fread(data, 1, frmSize, fin);
        if (ret != frmSize) {
           log(TAG, "read last frame");
           eos = 1;
        }

        ret = codec.Encode(data, frmSize, &pkt);
        CHK_RET(ret, "codec.Encode");

        if (pkt) {
            void *ptr = mpp_packet_get_pos(pkt);
            size_t len = mpp_packet_get_length(pkt);
            fwrite(ptr, 1, len, fout);
	    mpp_packet_deinit(&pkt);
        }
    }

    ret = codec.Deinit();
    CHK_RET(ret, "codec.Deinit");

    free(sps);
    free(data);
    fclose(fin);
    fclose(fout);

    return 0;
}
