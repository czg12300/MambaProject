//
// Created by walljiang on 2017/04/12.
//

#ifndef GITXIUGE_AUDIODECODE_H_H
#define GITXIUGE_AUDIODECODE_H_H
extern "C"{
#include <libavformat/avformat.h>
};
#include "Log.h"
#include <stdio.h>
#endif //GITXIUGE_AUDIODECODE_H_H
int audioDecode(const char *srcFile1,const char *outFile,int decodeType);