//
// Created by jakechen on 2017/4/21.
//
#include "FFmpegBase.h"

extern "C" {
#include <libavutil/frame.h>
}
#ifndef XIUGE_BEAUTYAVFRAME_H
#define XIUGE_BEAUTYAVFRAME_H


void beautifyAlgorithm(AVFrame *rgb_frame);

void freeM();

#endif //XIUGE_BEAUTYAVFRAME_H
