#include "camera.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define cmalloc(type,cnt)   malloc(sizeof(type) * cnt)

#define cam_log(fmt,...) \
            do { \
                printf(fmt,##__VA_ARGS__); \
            } while(0)\

#define CAM_DEBUG    1
#ifdef CAM_DEBUG
FILE *fp = NULL;
#endif

static int cioctl(int fd, int cmd, void *arg)
{
    int ret = 0;
    do {
        ret = ioctl(fd, cmd, arg);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

static int camera_mmap(camera_t *ctx)
{
    struct v4l2_requestbuffers req_bufs;
    camera_t *p_ctx = ctx;
    int i = 0;

    memset(&req_bufs, 0, sizeof(req_bufs));
    req_bufs.count = 25;
    req_bufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_bufs.memory = V4L2_MEMORY_MMAP;

    if (cioctl(p_ctx->handle, VIDIOC_REQBUFS, &req_bufs) == -1) {
        cam_log("failed to req bufs for %d %s.\n", errno,
                strerror(errno));
        return -1;
    }

    p_ctx->buf_nb = req_bufs.count;
    p_ctx->buf = cmalloc(v4l2_buf_t, p_ctx->buf_nb);
    if (!p_ctx->buf) {
        cam_log("failed to malloc v4l2_buf_t.\n");
        return -2;
    }

    for (i = 0; i < p_ctx->buf_nb; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (cioctl(p_ctx->handle, VIDIOC_QUERYBUF, &buf) == -1) {
            cam_log("failed to query buf for %d %s.\n", errno,
                    strerror(errno));
            return -3;
        }

        p_ctx->buf[i].len = buf.length;
        p_ctx->buf[i].start = mmap(NULL, buf.length,
                                   PROT_READ|PROT_WRITE,
                                   MAP_SHARED, p_ctx->handle,
                                   buf.m.offset);
        if (p_ctx->buf[i].start == MAP_FAILED) {
            cam_log("failed to mmap v4l2buf to usr memory.\n");
            return -4;
        }
    }

    return 0;
}

static int camera_start_cap(camera_t *ctx)
{
    camera_t *p_ctx = ctx;
    enum v4l2_buf_type type;
    int i = 0;

    for (i = 0; i < p_ctx->buf_nb; i++) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (cioctl(p_ctx->handle, VIDIOC_QBUF, &buf) == -1) {
            cam_log("failed to qbuf for %d %s.\n", errno,
                    strerror(errno));
            return -1;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (cioctl(p_ctx->handle, VIDIOC_STREAMON, &type) == -1) {
        cam_log("failed to make stream on for %d %s.\n", errno,
                strerror(errno));
        return -2;
    }

    return 0;
}

int camera_init(const char *dev, camera_t **ctx)
{
    int ret = 0;
    camera_t *p_cam = NULL;
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    //struct v4l2_streamparm param;

    ret = access(dev, 0);
    if (ret < 0) {
        cam_log("dev %s doesn't exit.\n", dev);
        return -1;
    }

    p_cam = (camera_t *)cmalloc(camera_t, 1);
    if (!p_cam) {
        cam_log("failed to malloc memory for camera_t.\n");
        return -2;
    }

    INIT_LIST_HEAD(&(p_cam->buf_list.list));
    strcpy(p_cam->dev, dev);

    p_cam->handle = -1;
    p_cam->handle = open(dev, O_RDWR, 0);
    if (p_cam->handle == -1) {
        cam_log("failed to open dev %s for %d %s.\n", p_cam->dev,
                errno, strerror(errno));
        return -3;
    }

    if (cioctl(p_cam->handle, VIDIOC_QUERYCAP, &cap) == -1) {
        if (errno == EINVAL) {
            cam_log("dev %s isn't V4L2 dev.\n", p_cam->dev);
        } else {
            cam_log("failed to get dev cap for %d %s.\n", errno,
                    strerror(errno));
        }
        return -4;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        cam_log("dev %s don't supprt video capture.\n", p_cam->dev);
        return -5;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        cam_log("dev %s don't support video stream.\n", p_cam->dev);
        return -6;
    }

    fmt.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (cioctl(p_cam->handle, VIDIOC_G_FMT, &fmt) == -1) {
        cam_log("failed to get dev %s fmt for %d %s.\n", p_cam->dev,
                errno, strerror(errno));
        return -7;
    }

    p_cam->width = fmt.fmt.pix.width;
    p_cam->height = fmt.fmt.pix.height;

#ifdef cam_debug
    cam_log("dev msg:\n width : %d\n height : %d\n", p_cam->width,
            p_cam->height);
#endif

    if (cioctl(p_cam->handle, VIDIOC_S_FMT, &fmt) == -1) {
        cam_log("failed to set dev %s fmt for %d %s.\n", p_cam->dev,
                errno, strerror(errno));
        return -8;
    }

    ret = camera_mmap(p_cam);
    if (ret < 0) {
        cam_log("failed to do camera mmap ret %d.\n", ret);
        return -9;
    }

    ret = camera_start_cap(p_cam);
    if (ret < 0) {
        cam_log("failed to strart camera capture ret %d.\n", ret);
        return -10;
    }

    *ctx =p_cam;

    return 0;
}

int camera_get_frm(camera_t *ctx)
{
    camera_t *p_ctx = ctx;
    struct v4l2_buffer buf;
    buf_list *node, *pos, *n;
    int list_len = 0;

    node = cmalloc(buf_list, 1);
    if (!node) {
        cam_log("failed to malloc list node.\n");
        return -1;
    }

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    list_for_each_entry_safe(pos, n, &(ctx->buf_list.list),
                             buf_list, list) {
        list_len++;
    }

    if (list_len >= 25)
        return -2;

    if (cioctl(p_ctx->handle, VIDIOC_DQBUF, &buf) == -1) {
        cam_log("failed to dqbuf for %d %s.\n", errno,
                strerror(errno));
        return -2;
    }

    node->buf = buf;
    node->v4l2_idx = buf.index;
    node->played = 0;
    node->encoded = 0;

    assert(buf.index < (unsigned int)p_ctx->buf_nb);

    list_add_tail(&node->list, &(p_ctx->buf_list.list));
    /*
     * p_ctx->buf[buf.index].start : pointer to yuv data
     * p_ctx->buf[buf.index].len : length of yuv data
     */
#ifdef CAM_DEBUG
    {
        if (!fp) {
            fp = fopen("test.yuv", "w+");
            if (!fp) {
                cam_log("failed to open test.yuv.\n");
            }
        }

        fwrite(p_ctx->buf[buf.index].start, p_ctx->buf[buf.index].len,
                1, fp);
    }
#endif

    return 0;
}

int camera_release_frm(camera_t *ctx)
{
    int ret = 0;
    int got_pic = 0;
    buf_list *pos, *n;
    camera_t *p_ctx = ctx;

    if (list_empty(&(p_ctx->buf_list.list)))
        return -1;

    list_for_each_entry_safe (pos, n, &(p_ctx->buf_list.list),
                              buf_list, list) {
        if (pos->played && pos->encoded) {
            ret = cioctl(p_ctx->handle, VIDIOC_QBUF, &pos->buf);
            if (ret == -1) {
                cam_log("failed to qbuf for %d %s.\n", errno,
                        strerror(errno));
                return -2;
            }

            pos->encoded = 0;
            pos->played = 0;
            got_pic = 1;

            /*
             * free resouce
             */
            list_del_init(&pos->list);
            free(pos);
        }
    }

    if (!got_pic) {
        cam_log("failed to find a node which is used.\n");
    }

    return 0;
}

int camera_deinit(camera_t *ctx)
{
    camera_t *p_ctx = ctx;
    enum v4l2_buf_type type;
    int i = 0;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(cioctl(p_ctx->handle,VIDIOC_STREAMOFF,&type) == -1) {
        cam_log("failed to make stream off for %d %s.\n", errno,
                strerror(errno));
        return -1;
    }

    for (i = 0; i < p_ctx->buf_nb; i++) {
        if (munmap(p_ctx->buf[i].start, p_ctx->buf[i].len) == -1) {
            cam_log("failed to unmap v4l2buf for %d %s.\n", errno,
                    strerror(errno));
            return -2;
        }
    }

    free(p_ctx->buf);
    free(p_ctx);
    close(p_ctx->handle);

#ifdef CAM_DEBUG
    fclose(fp);
#endif

    return 0;
}




