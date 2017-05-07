//
// Created by walljiang on 2017/04/09.
//
#include "audioMix.h"

int audioMix(const char *srcFile1,const char *srcFile2,const char *outFile, float vol1, float vol2){
    int ret = 0;
    ret = audio_mix_new(srcFile1, srcFile2, outFile,vol1,vol2);
    return ret;
}