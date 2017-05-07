//
// Created by jakechen on 2017/3/13.
//
#include "FFmpegBase.h"

extern "C"
{
#include "libavformat/avformat.h"
};

/**
 * 格式转换器
 * @param srcFile  输入文件
 * @param outFile  输出文件
 * @return 大于等于0，表示成功，否则失败
 */
int remuxer(const char *srcFile, const char *outFile);