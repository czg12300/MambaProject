//
//格式封装器
// Created by jakechen on 2017/5/10.
//
#include "FfmpegVideoUtilsBase.h"
extern "C"
{
#include "libavformat/avformat.h"
};
namespace video {

    int muxing(const char *in_filename_v, const char *in_filename_a, const char *out_filename);

    int muxing(const char *in_filename_v, const char *in_filename_a, const char *out_filename,
               const char *rotate);

    int h264ToFormat(const char *in_filename_v, const char *out_filename);

    int h264ToFormat(const char *in_filename_v, const char *out_filename, const char *rotate);

    int remuxing(const char *srcFile, const char *outFile);

    int remuxing(const char *srcFile, const char *outFile, const char *rotate);
}


