//
// Created by walljiang on 2017/03/06.
//

#ifndef GITXIUGE_VIDEOMERGE_H
#define GITXIUGE_VIDEOMERGE_H

#endif //GITXIUGE_VIDEOMERGE_H
#include "FFmpegBase.h"

#include <jni.h>
extern "C" {
#include <libavformat/avformat.h>
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libswresample/swresample.h"
#include "libavutil/fifo.h"
#include "libavutil/audio_fifo.h"
}

int videoMerge(const char *srcFile1,const char *srcFile2,const char *outFile);