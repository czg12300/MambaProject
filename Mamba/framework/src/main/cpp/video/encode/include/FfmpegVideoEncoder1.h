//
// Created by jakechen on 2017/6/17.
//

#ifndef NEWSHOWGUYS_ANDROID_FFMPEGVIDEOENCODER1_H
#define NEWSHOWGUYS_ANDROID_FFMPEGVIDEOENCODER1_H

#include <queue>
#include "FfmpegMuxer.h"
#include "FFmpegBase.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
};
namespace video {
     int startEncode(const char *file, int codecType, int keyIFrameInterval, int width, int height,
                     int frameRate,
                     int bitRate);

     void stopEncode();

     void encode(uint8_t *data, int width, int height);

}

#endif //NEWSHOWGUYS_ANDROID_FFMPEGVIDEOENCODER1_H
