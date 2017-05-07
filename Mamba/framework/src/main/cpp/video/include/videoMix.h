//
// Created by walljiang on 2017/04/21.
//

#ifndef GITXIUGE_VIDEOMIX_H
#define GITXIUGE_VIDEOMIX_H

#include "Log.h"
#include "vector"
extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
}

int videoMix(const char* srcFile1,const char* srcFile2,const char* outFile,int mix1,int mix2);

#endif //GITXIUGE_VIDEOMIX_H
