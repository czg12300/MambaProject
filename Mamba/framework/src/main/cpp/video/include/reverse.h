//
// Created by walljiang on 2017/04/05.
//

#ifndef GITXIUGE_REVERSE_H
#define GITXIUGE_REVERSE_H

#endif //GITXIUGE_REVERSE_H
//#include "FFmpegBase.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

int reverse(const char *srcFile, const char *outFile);
//int mp4_reverse_yuv(const char *src_filename, const char *yuv_dst_filename, const char *aac_dst_filename, int *frame_count, int *per_count, int *width, int *height);
//int yuv_to_mp4(const char *vedio_src_filename, const char *audio_src_filename, const char *dst_filename, const int *frame_count, const int *per_count, const int *width, const int *height);
//int muxing_mp4(const char *mp4_src_filename, const char *aac_src_filename, const char *dst_filename);
