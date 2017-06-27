//
// Created by jakechen on 2017/5/10.
//

#include "FfmpegVideoUtilsBase.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}
namespace video {

    /**
 * 分离音视频，输出文件为：out.h264 out.aac
 * @param filePath 源文件
 * @param path 输入文件夹地址
 * @return >=0 ,表示成功 ，否则表示失败
 */
    int demuxing(const char *filePath, const char *h264, const char *aac);

    int demuxingVideo(const char *filePath, const char *h264);

    int demuxingAudio(const char *filePath, const char *aac);
}
