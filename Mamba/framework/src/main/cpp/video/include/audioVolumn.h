//
// Created by walljiang on 2017/04/07.
//


#ifndef GITXIUGE_AUDIOMERGE_H
#define GITXIUGE_AUDIOMERGE_H

#endif //GITXIUGE_AUDIOMERGE_H
extern "C"{
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavcodec/avcodec.h>
};

#include "Log.h"
int audio_vol_change_Init(AVCodecContext *encodec_ctx, double proportion);
int audio_vol_change_DeInit();
int audio_vol_change2(AVFrame *in_frame, AVFrame *out_frame);
int audio_vol_change(const char *src_filename, const char *dst_filename, double proportion);
int audioVolumn(const char *srcFile1, const char *outFile,float vol1);