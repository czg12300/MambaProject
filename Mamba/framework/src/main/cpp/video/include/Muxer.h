//
// Created by jakechen on 2017/1/16.
//
#include "FFmpegBase.h"

extern "C"
{
#include "libavformat/avformat.h"
};
#ifndef _InputStreamContext
#define _InputStreamContext
/**
 * 输入文件的数据结构
 */
struct InputStreamContext {
    AVFormatContext *fmt_ctx = NULL;
    AVStream *out_stream = NULL;
    AVStream *in_stream = NULL;
    int in_stream_index;
    int out_stream_index;
};
#endif

/**
 * 将音频文件和视频裸流文件合并成MP4格式的文件
 * @param in_filename_v 视频裸流文件
 * @param in_filename_a 音频文件
 * @param out_filename 输入视频文件
 * @return >=0 ,表示成功 ，否则表示失败
 */
int muxing(const char *in_filename_v, const char *in_filename_a, const char *out_filename);

int muxingVideoFile(const char *in_filename_v, const char *in_filename_a, const char *out_filename);


int h264ToFormat(const char *in_filename_v, const char *out_filename);

int h264ToFormat(const char *in_filename_v, const char *out_filename, const char *rotate);