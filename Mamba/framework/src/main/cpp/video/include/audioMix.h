//
// Created by walljiang on 2017/04/09.
//

#ifndef GITXIUGE_AUDIOMIX_H
#define GITXIUGE_AUDIOMIX_H

#endif //GITXIUGE_AUDIOMIX_H
#include <pthread.h>
#include <unistd.h>
extern "C"{
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavutil/opt.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
};

#include "Log.h"
int audio_mix_new(const char *input1_filename, const char *input2_filename, const char *output_filename,double vol1 ,double vol2);
int audioMix(const char *srcFile1,const char *srcFile2,const char *outFile, float vol1,float vol2);