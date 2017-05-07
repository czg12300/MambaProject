//
// Created by jakechen on 2017/1/12.
//

#include "FFmpegBase.h"
#include "FFmpegUtil.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
}
/**
 * 提取视频的关键帧
 * @param filePath 源视频文件
 * @param picPath 关键帧图片位置
 * @return >=0 ,表示成功 ，否则表示失败
 */
int demuxing_key_frame(const char* filePath, const char* picPath);