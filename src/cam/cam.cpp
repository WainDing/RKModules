#include "cam.h"
#include "log.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

RKMediaCam::RKMediaCam() {

}

RKMediaCam::~RKMediaCam() {

}

int RKMediaCam::Init(const char *dev, unsigned int v4l2Fmt, int w, int h) {
    int ret =0;
    int i = 0;
    struct v4l2_format fmt;
    struct v4l2_capability cap;
    struct v4l2_requestbuffers req_bufs;
    enum v4l2_buf_type type;

    _w = w;
    _h = h;

    _fd = open(dev, O_RDWR, 0);
    if (_fd == -1) {
        log("cam", "failed to open dev %s because %d:%s", dev, errno, strerror(errno));
        return -1;
    }

    ret = ioctl(_fd, VIDIOC_QUERYCAP, &cap);
    if (ret == -1) {
        log("cam", "dev %s isn't v4l2 device", dev);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        log("cam", "dev %s don't supprt video capture.\n", dev);
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        log("cam", "dev %s don't support video stream.\n", dev);
        return -1;
    }

    fmt.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(_fd, VIDIOC_G_FMT, &fmt);
    if (ret == -1) {
        log("cam", "dev %s failed to get video fmt", dev);
        return -1;
    }

    fmt.fmt.pix.width = w;
    fmt.fmt.pix.height = h;
    fmt.fmt.pix.pixelformat = v4l2Fmt; 

    ret = ioctl(_fd, VIDIOC_S_FMT, &fmt);
    if (ret == -1) {
        log("cam", "dev %s failed to set video fmt", dev);
        return -1;
    }

    fmt.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(_fd, VIDIOC_G_FMT, &fmt);
    if (ret == -1) {
        log("cam", "dev %s failed to get video fmt", dev);
        return -1;
    }

    if (fmt.fmt.pix.pixelformat != v4l2Fmt ||
        fmt.fmt.pix.width != _w ||
        fmt.fmt.pix.height != _h) {
        log("cam", "dev %s doesn't support fmt:%c%c%c%c w:%d h:%d",dev ,
         fmt.fmt.pix.pixelformat & 0xff, (fmt.fmt.pix.pixelformat >> 8) & 0xff,
         (fmt.fmt.pix.pixelformat >> 16) & 0xff, (fmt.fmt.pix.pixelformat >> 24) & 0xff, _w, _h);

         return -1;
    }

    log("cam", "dev %s support fmt:%c%c%c%c w:%d h:%d", dev ,
         fmt.fmt.pix.pixelformat & 0xff, (fmt.fmt.pix.pixelformat >> 8) & 0xff,
         (fmt.fmt.pix.pixelformat >> 16) & 0xff, (fmt.fmt.pix.pixelformat >> 24) & 0xff, _w, _h);
    
    memset(&req_bufs, 0, sizeof(req_bufs));
    req_bufs.count = 10;
    req_bufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_bufs.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(_fd, VIDIOC_REQBUFS, &req_bufs);
    if (ret == -1) {
        log("cam", "failed to req bufs because %d:%s", errno, strerror(errno));
        return -1;
    }

    _bufList = (CameraBuf *)malloc(10 * sizeof(CameraBuf));
    if (!_bufList) {
        log("cam", "failed to malloc _bufList");
        return -1;
    }

    for (i = 0; i < 10; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        ret = ioctl(_fd, VIDIOC_QUERYBUF, &buf);
        if (ret == -1) {
            log("cam", "failed to query buf because %d:%s", errno, strerror(errno));
            return -1;
        }

        _bufList[i].dataLen = buf.length;
        _bufList[i].data = mmap(NULL, buf.length,
                                   PROT_READ|PROT_WRITE,
                                   MAP_SHARED, _fd,
                                   buf.m.offset);
        if (_bufList[i].data == MAP_FAILED) {
            log("cam", "failed to mmap v4l2 buf to use space");
            return -1;
        }
    }

    for (i = 0; i < 10; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        ret = ioctl(_fd, VIDIOC_QBUF, &buf);
        if (ret == -1) {
            log("cam", "failed to qbuf because %d:%s", errno, strerror(errno));
            return -1;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(_fd, VIDIOC_STREAMON, &type);
    if (ret == -1) {
        log("cam", "failed to open cam stream on");
        return -1;
    }

    return 0;
}

int RKMediaCam::Deinit() {
    int ret = 0;
    int i = 0;
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(_fd, VIDIOC_STREAMOFF, &type);
    if (ret == -1) {
        log("cam", "failed to close cam stream off");
        return -1;
    }

    for (i = 0; i < 10; i++) {
        if (munmap(_bufList[i].data, _bufList[i].dataLen) == -1) {
            log("cam", "failed to unmap v4l2buf because %d %s.\n", errno,
                    strerror(errno));
            return -1;
        }
    }

    free(_bufList);
    close(_fd);

    return 0;
}

int RKMediaCam::GetFrame(CameraBuf *buf) {
    int ret = 0;
    struct v4l2_buffer vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vbuf.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(_fd, VIDIOC_DQBUF, &vbuf);
    if (ret == -1) {
        log("cam", "failed to dequeue buf from cam");
        return -1;
    }

    buf->data = _bufList[vbuf.index].data;
    buf->dataLen = _bufList[vbuf.index].dataLen;
    buf->vbuf = vbuf;
    buf->bufIndex = vbuf.index;
    return 0;
}

int RKMediaCam::FreeFrame(CameraBuf *buf) {
    int ret = 0;

    ret = ioctl(_fd, VIDIOC_QBUF, &buf->vbuf);
    if (ret == -1) {
        log("cam", "failed to queue buf from cam");
        return -1;
    }

    return 0;
}
