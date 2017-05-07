//
// Created by jakechen on 2017/3/25.
//
#include "FFmpegBase.h"
extern "C"
{
#include "libavformat/avformat.h"
};
/**
 * 旋转视频的角度
 * @param srcFile
 * @param outFile
 * @param rotate 视频的角度
 * @return
 */
int rotate(const char *srcFile, const char *outFile, const char *rotate);