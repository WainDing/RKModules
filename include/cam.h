#ifndef __CAM_H__
#define __CAM_H__

#include <linux/videodev2.h>

typedef struct {
    void *data;
    int dataLen;
    
    struct v4l2_buffer vbuf;
    int bufIndex;
}CameraBuf;

class RKMediaCam {
public:
    RKMediaCam();
    ~RKMediaCam();
    int Init(const char *dev, unsigned int v4l2Fmt, int w, int h);
    int Deinit();
    int GetFrame(CameraBuf *buf);
    int FreeFrame(CameraBuf *buf);
private:
    int _w;
    int _h;
    int _fd;
    CameraBuf *_bufList;
};

#endif // __CAM_H__