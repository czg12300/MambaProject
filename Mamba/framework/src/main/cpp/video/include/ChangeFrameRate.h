//
// Created by jakechen on 2017/2/5.
//

#include "FFmpegBase.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
}
int changeFrameRate(const char* file, const char* outFileDir, double frameRate);