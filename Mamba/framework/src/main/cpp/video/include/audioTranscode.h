//
// Created by walljiang on 2017/04/13.
//

#ifndef GITXIUGE_AUDIOTRANSCODE_H
#define GITXIUGE_AUDIOTRANSCODE_H

#endif //GITXIUGE_AUDIOTRANSCODE_H
extern "C"{
    #include <libswresample/swresample.h>
    #include <libavformat/avformat.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/opt.h>
};

#include "Log.h"
#define BUF_SIZE_20K 2048000
#define BUF_SIZE_1K 1024000
#define SWR_CH_MAX 32   ///< Maximum number of channels

void initSwr(int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate);
void deInitSwr();
void setup_array(uint8_t* out[SWR_CH_MAX], AVFrame* in_frame, int format, int samples);
int TransSample(AVFrame *in_frame, AVFrame *out_frame,enum AVSampleFormat sample_fmt);