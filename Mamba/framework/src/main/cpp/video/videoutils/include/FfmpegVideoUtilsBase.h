//
// Created by jakechen on 2017/5/12.
//
#include "FFmpegBase.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}
namespace video {
#ifndef VIDEO_COMMON
#define VIDEO_COMMON
    struct OutputStreamContext {
        AVFormatContext *fmt_ctx = NULL;
        AVStream *stream = NULL;
        int streamIndex = 0;
    };
#endif

    OutputStreamContext *
    applyOutputStreamContext(AVFormatContext *fmt_ctx, const char *file, AVMediaType type);

}