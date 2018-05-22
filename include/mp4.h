#ifndef __MP4_H__
#define __MP4_H__

#include "mp4v2/mp4v2.h"

typedef struct _MP4ENC_NaluUnit    
{    
    int type;    
    int size;    
    unsigned char *data;    
}MP4ENC_NaluUnit;

class RKMediaMp4Context {
public:
    RKMediaMp4Context();
    virtual ~RKMediaMp4Context();
    int CreateMp4File(const char *file,int w,int h,int timeScale,int frmRate);
    void CloseMp4File();
    int WriteSpsAndPps(unsigned char *spsPps,int dataSize);
    int WriteNalu(unsigned char *naluData,int naluSize);
private:
    int ReadOneNaluFromBuf(const unsigned char *data, unsigned int dataSize, unsigned int offset, MP4ENC_NaluUnit &nalu);

    int _w;
    int _h;
    int _frmRate;
    int _timeScale;

    MP4FileHandle _handle;
    MP4TrackId _videoTrack;   
};

#endif // __MP4_H__
