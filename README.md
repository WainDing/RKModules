# RK多媒体库
媒体库内支持组件：
1. USB 摄像头组件
2. RK 编码器组件
3. MP4 封装组件
4. STREAM_MEDIA 流媒体组件

## 编译
build目录下寻找对应的CMake脚本
1. 编译arm linux使用build/arm-linux/build_arm_linux.sh
   cd build/arm-linux
   ./build_arm_linux.sh
2. 编译64位aarch linux使用build/aarch-linux/build_aarch_linux.sh
   cd build/aarch-linux
   ./build_aarch_linux.sh

生成的库在src目录下 librkmedia.so
头文件目录在顶层目录下的include

## demo
每个组件的demo 存在于组件目录下的test目录
1. 摄像头组件测试demo ：cam/test/cam_yuv.cpp 从摄像头获取yuv保存到文件中
2. 编码器组件测试demo ：codec/test/codec_test.cpp 读取文件并编码
3. mp4封装组件测试demo ：mp4/test/mp4_test.cpp 从摄像头获取yuv后编码 并保存为mp4文件
4. 流媒体组件测试demo ：stream_media/test/stream_media_test.cpp 从摄像头获取yuv 编码后推RTP流

## 接口
暂时只支持C++接口 后续会封装C接口

1. USB 摄像头组件
   数据结构 ：camera_buf : 摄像头采集buffer
   typedef struct {
    void *data;
    int dataLen;

    struct v4l2_buffer vbuf;
    int bufIndex;
    }CameraBuf;

   1. 初始化 int Init(const char *dev, unsigned int fmt, int w, int h); 传入usb摄像头的设备路径 如/dev/video0 fmt是v4l2采集格式 如V4L2_PIX_FMT_YUYV V4L2_PIX_FMT_NV12等 以及传入宽高
      如果摄像头不支持所设定的格式和宽高 会报错退出 需要根据所用的摄像头自行调整

   2. int Deinit(); 退出摄像头组件
   3. int GetFrame(CameraBuf *buf); 从摄像头获取一帧 数据保存在camera_buf的data指针中 
   4. int FreeFrame(CameraBuf *buf); 释放获取的摄像头帧

2. RK 编码器组件
   1. 初始化 MPP_RET Init(int w, int h, MppFrameFormat fmt); 传入宽高和输入格式 格式要和选用的摄像头格式对应
   2. MPP_RET Deinit(); 退出编码器组件
   3. MPP_RET GetSpsPps(unsigned char **data, int *data_size); 获取编码器生成的的sps和pps
   4. MPP_RET MPP_RET Encode(RK_U8 *data, RK_U32 dataSize, MppPacket *pkt); 编码一帧

3. MP4 封装组件
   1. int CreateMP4File(const char *file, int w, int h, int time_scale, int frm_rate); 创建mp4文件 需要指定文件名
   2. void CloseMP4File(); 关闭mp4文件
   3. int WriteSPSandPPS(unsigned char *sps_pps, int data_size); 写入sps和pps
   4. int WriteNalu(unsigned char *nalu_data, int nalu_size); 写入一帧h264 nalu

4. STREAM_MEDIA 流媒体组件
   1. int InitRTProtocol(const char *ip, int port); 初始化 需要传入rtp地址和端口
   2. int DeinitRTProtocol 退出流媒体组件
   3. int SendRtpPacket(const uint8_t *nalu, uint32_t nalu_size); 发送一帧h264
