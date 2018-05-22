#include "mp4.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>

#define MODULE_TAG "mp4"
RKMediaMp4Context::RKMediaMp4Context() {

}

RKMediaMp4Context::~RKMediaMp4Context() {

}

int RKMediaMp4Context::CreateMp4File(const char *file,int w,int h,int timeScale,int frmRate) {
    _w = w;
    _h = h;
    _timeScale = timeScale;
    _frmRate = frmRate;

    _handle = MP4Create(file);
    if (_handle == MP4_INVALID_FILE_HANDLE) {
        log(MODULE_TAG, "failed to create mp4 file");
        return -1;
    }

    MP4SetTimeScale(_handle, _timeScale);
    
    return 0;
}

void RKMediaMp4Context::CloseMp4File() {
    if (_handle) {
        MP4Close(_handle);
        _handle = NULL;
    }
}

int RKMediaMp4Context::ReadOneNaluFromBuf(const unsigned char *data, unsigned int dataSize, unsigned int offset, MP4ENC_NaluUnit &nalu) {
    int i = offset;

    while(i < dataSize)
    {
        if(data[i++] == 0x00 &&    
            data[i++] == 0x00 &&    
            data[i++] == 0x00 &&    
            data[i++] == 0x01    
            )
        { 
            int pos = i;    
            while (pos<dataSize)    
            {
                if(data[pos++] == 0x00 &&
                    data[pos++] == 0x00 &&
                    data[pos++] == 0x00 &&
                    data[pos++] == 0x01
                    )
                {
                    break;
                }
            }
            if(pos == dataSize)
            {
                nalu.size = pos-i;
            }
            else
            {
                nalu.size = (pos-4)-i;
            }

            nalu.type = data[i]&0x1f;
            nalu.data =(unsigned char*)&data[i];
            return (nalu.size+i-offset);
        }
    }
    return 0;
}

int RKMediaMp4Context::WriteSpsAndPps(unsigned char *spsPps,int dataSize) {
    int pos = 0;
    int len = 0;

    MP4ENC_NaluUnit nalu;
    while (len = ReadOneNaluFromBuf(spsPps, dataSize, pos, nalu)) {
        if (nalu.type == 0x07)  { //sps
            log(MODULE_TAG, "write sps to mp4 file");
            _videoTrack = MP4AddH264VideoTrack(_handle, _timeScale, _timeScale / _frmRate, _w, _h, nalu.data[1], nalu.data[2], nalu.data[3], 3);
            if (_videoTrack == MP4_INVALID_TRACK_ID) {
                log(MODULE_TAG, "failed to add video track");
                return -1;
            }

            MP4SetVideoProfileLevel(_handle, 3);
            MP4AddH264SequenceParameterSet(_handle, _videoTrack, nalu.data, nalu.size);
        } else if (nalu.type == 0x08) { //pps
            log(MODULE_TAG, "write pps to mp4 file");
            MP4AddH264PictureParameterSet(_handle, _videoTrack, nalu.data, nalu.size);
        } else if (nalu.type == 0x06) { //sei
            log(MODULE_TAG, "write sei to mp4 file");
            pos += 3;
            continue;
        } else { //other
            log(MODULE_TAG, "meet unknow nalu type:%d in sps and pps", nalu.type);
            pos += 3;
            continue;
        }

        pos += len;
    }

    return 0;
}

int RKMediaMp4Context::WriteNalu(unsigned char *naluData,int naluSize) {
    int dataSize = naluSize + 4;

    unsigned char *data = (unsigned char *)malloc(dataSize);
    if (!data) {
        log(MODULE_TAG, "failed to malloc cache data");
        return -1;
    }

    data[0] = naluSize >> 24;
    data[1] = naluSize >> 16;
    data[2] = naluSize >> 8;
    data[3] = naluSize & 0xff;
    memcpy(data + 4, naluData, naluSize);
    if (!MP4WriteSample(_handle, _videoTrack, data, dataSize, MP4_INVALID_DURATION, 0, 1)) {
        log(MODULE_TAG, "failed to write nalu smaple to mp4 file");
        free(data);
        return -1;
    }

    free(data);
    return 0;
}