#ifndef __CAMERA_MODULE_H__
#define __CAMERA_MODULE_H__

#include <stdio.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include "list.h"

#define CAMERA_NAME_MAX_LEN     (256)

typedef struct {
    void *start;
    int len;
} v4l2_buf_t;

typedef struct {
    struct list_head list;
    struct v4l2_buffer buf;
    int v4l2_idx;
    int played;
    int encoded;
} buf_list;

typedef struct {
    char dev[CAMERA_NAME_MAX_LEN];
    int handle;
    int width;
    int height;

    v4l2_buf_t *buf;
    buf_list buf_list;
    int buf_nb;

} camera_t;

int camera_init(const char *dev, camera_t **ctx);
int camera_get_frm(camera_t *ctx);
int camera_release_frm(camera_t *ctx);
int camera_deinit(camera_t *ctx);

#endif // __CAMERA_MODULE_H__
