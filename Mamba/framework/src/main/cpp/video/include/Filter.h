//
// Created by jakechen on 2017/2/23.
//


#include "FFmpegBase.h"
#include "VideoRecodeFactory.h"

extern "C" {

#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}


/**
 * 给视频添加水印
 * @param watermarkCommand  水印生成命令"movie="+watermarkPath+"[wm];[in][wm]overlay=5:5[out]";
 * @param srcFile 输入视频
 * @param outFile 输出视频
 * @return @return >=0 ,表示成功 ，否则表示失败
 */
int addWatermark1(const char *watermarkCommand, const char *srcFile, const char *outFile);