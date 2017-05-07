//
// Created by jakechen on 2017/3/10.
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
#ifndef XIUGE_WATERMARK_H
#define XIUGE_WATERMARK_H

class WatermarkVideoOptHandler : public VideoOptHandler {
    string watermarkCommand;
    AVFrame *filt_frame;
public:
    WatermarkVideoOptHandler(string watermarkCommand);

    int init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx, int video_index,int frameCount);

    AVFrame *optAvFrame(AVFrame *frame, int frameIndex);

    int changeFrameRate(int frameRate);

    AllowEncodeAndWriteFrameState allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt,int frameIndex);

    void releaseInEnd();

    void changeTimebase(AVStream *inStream, AVStream *outStream, AVPacket pkt);
};

#endif

/**
 * 给视频添加水印
 * @param watermarkCommand  水印生成命令"movie="+watermarkPath+"[wm];[in][wm]overlay=5:5[out]";
 * @param srcFile 输入视频
 * @param outFile 输出视频
 * @return @return >=0 ,表示成功 ，否则表示失败
 */
int addWatermark(const char *watermarkCommand, const char *srcFile, const char *outFile);
